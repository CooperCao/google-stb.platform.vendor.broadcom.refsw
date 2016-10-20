/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "EGL/egl.h"
#include "nexus_memory.h"
#include "nexus_platform.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include <stdio.h>
#include <inttypes.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include <drm/brcmv3d_drm.h>

#define MMU_PAGE_SIZE (64 * 1024)
#define MMU_FIRST_VALID_PAGE MMU_PAGE_SIZE

#define UNUSED(A) (void)A

/* Turn on to log memory access pattern to text file from 3D driver */
/*#define LOG_MEMORY_PATTERN*/

#ifdef LOG_MEMORY_PATTERN
static const bool use_memory_log = true;
#else
static const bool use_memory_log = false;
#endif

/* Turn on to log to stderr instead of a file */
#define LOG_TO_STDERR

#ifdef LOG_TO_STDERR
static const bool log_to_stderr = true;
#else
static const bool log_to_stderr = false;
#endif

static FILE *sLogFile = NULL;

/* Determine if we pass allocation debug descriptions to the kernel driver */
#ifndef NDEBUG
static const bool use_alloc_desc_str = true;
#else
static const bool use_alloc_desc_str = false;
#endif

typedef struct
{
   int       fd;
   int       l2_cache_size;
   bool      force_writecombine;
   bool      clear_on_alloc;
   uint64_t  pagetable_physaddr;
   uint32_t  mmu_va_size;

   /*
    * For secure memory allocations, which cannot come from DRM/CMA memory
    */
   NEXUS_HeapHandle secure_heap;
   uint32_t processId;
   /*
    * A "hack" to map the Nexus heap from which bin memory is allocated by
    * the Magnum driver, which must be at the same virtual address for each
    * client until we can work out how to tie bin allocations to a specific
    * DRM file handles, both in user and kernel modes. We need mappings
    * for both the secure and unsecure heaps.
    */
   struct drm_v3d_gem_create_ext offscreen_heap_mapping;
   int64_t offscreen_heap_translation;
   struct drm_v3d_gem_create_ext secure_offscreen_heap_mapping;
   int64_t secure_offscreen_heap_translation;
} DRM_MemoryContext;

typedef struct
{
   uint32_t handle;
   uint32_t hw_addr;
   uint32_t flags;
   uint32_t real_size;
   bool     secure;
   union
   {
      uint64_t mmap_offset;
      NEXUS_MemoryBlockHandle nexus_handle; /* For wrapped Secure allocations */
   };
} DRM_MemoryBlock;


static inline bool opt_bool(const char *option)
{
   char *val = getenv(option);
   if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
      return true;

   return false;
}

/*****************************************************************************
 * DRM IOCTL helpers.
 *****************************************************************************/
#define DRM_INFO_MAX_SIZE 62

static void PrintDRMDriverInfo(int fd)
{
   char name[DRM_INFO_MAX_SIZE+1];
   char date[DRM_INFO_MAX_SIZE+1];
   char desc[DRM_INFO_MAX_SIZE+1];
   struct drm_version v;

   v.name_len = v.date_len = v.desc_len = DRM_INFO_MAX_SIZE;
   v.name = name;
   v.date = date;
   v.desc = desc;

   if (ioctl(fd, DRM_IOCTL_VERSION, &v) < 0)
   {
      perror("Unable to get DRM driver version information");
      return;
   }

   v.name[v.name_len] = '\0';
   v.date[v.date_len] = '\0';
   v.desc[v.desc_len] = '\0';

   fprintf(stderr, "DRM: %s Version %d.%d.%d %s, %s\n",
      v.name, v.version_major, v.version_minor, v.version_patchlevel,
      v.date, v.desc);
}

static int GetMmuPagetable(DRM_MemoryContext *ctx)
{
   struct drm_v3d_mmu_pt s;

   if (ioctl(ctx->fd, DRM_IOCTL_V3D_GET_MMU_PAGETABLE, &s) < 0)
   {
      perror("Failed to get MMU pagetable address from DRM device");
      return 0;
   }

   ctx->pagetable_physaddr = s.pt_phys;
   ctx->mmu_va_size = s.va_size;

   return 1;
}

static inline uint64_t GetFileToken(int fd)
{
   struct drm_v3d_file_private_token s = {};

   if (ioctl(fd, DRM_IOCTL_V3D_GET_FILE_TOKEN, &s) < 0)
      return 0ULL;

   return s.token;
}

static inline uint32_t CreateGemObject(int fd, struct drm_v3d_gem_create *s)
{
   if (ioctl(fd, DRM_IOCTL_V3D_GEM_CREATE, s) < 0)
      return 0;

   return s->handle;
}

static inline uint32_t CreateGemExtObject(int fd, struct drm_v3d_gem_create_ext *s)
{
   if (ioctl(fd, DRM_IOCTL_V3D_GEM_CREATE_EXT, s) < 0)
      return 0;

   return s->handle;
}

static inline uint64_t GemMapOffset(int fd, uint32_t handle)
{
   struct drm_v3d_gem_mmap_offset s = { .handle = handle };

   if (ioctl(fd, DRM_IOCTL_V3D_GEM_MMAP_OFFSET, &s) < 0)
      return 0ULL;

   return s.offset;
}

static inline void CloseGemHandle(int fd, uint32_t handle)
{
   struct drm_gem_close s = { .handle = handle };

   if (ioctl(fd, DRM_IOCTL_GEM_CLOSE, &s) < 0)
      perror("Failed to close DRM GEM handle");
}

/*
 * Helper for MemAllocBlock, used for clearing non-secure allocations only
 */
static void ClearMemoryBlock(DRM_MemoryContext *ctx, DRM_MemoryBlock *block)
{
   void *ptr;

   BDBG_ASSERT(!block->secure);

   block->mmap_offset = GemMapOffset(ctx->fd, block->handle);

   BDBG_ASSERT(block->mmap_offset != 0);

   ptr = mmap(NULL, block->real_size, PROT_READ | PROT_WRITE, MAP_SHARED, ctx->fd, block->mmap_offset);

   BDBG_ASSERT(ptr != 0);

   memset(ptr, 0, block->real_size);

   munmap(ptr, block->real_size);
}

/* Forward decl to wrap secure allocations */
static BEGL_MemHandle MemWrapExternal(void *context, uint64_t physaddr, size_t length, const char *desc);

static DRM_MemoryBlock *AllocateSecureBlock(DRM_MemoryContext *ctx, size_t numBytes, size_t align, const char *desc)
{
   NEXUS_Addr addr;
   NEXUS_MemoryBlockHandle handle;
   DRM_MemoryBlock *block;

   handle = NEXUS_MemoryBlock_Allocate_tagged(ctx->secure_heap, numBytes, align, NULL, desc, ctx->processId);

   if (!handle)
      return NULL;

   if (NEXUS_MemoryBlock_LockOffset(handle, &addr) != NEXUS_SUCCESS)
      goto error;

   block = MemWrapExternal(ctx, addr, numBytes, desc);
   if (!block)
      goto error;

   block->secure = true;
   block->nexus_handle = handle;
   return block;

error:
   NEXUS_MemoryBlock_Free(handle);
   return NULL;
}

/*****************************************************************************
 * Memory interface
 *****************************************************************************/
static BEGL_MemHandle MemAllocBlock(void *context, size_t numBytes, size_t alignment, uint32_t flags, const char *desc)
{
   struct drm_v3d_gem_create cs = {};
   DRM_MemoryContext *ctx = (DRM_MemoryContext*)context;
   DRM_MemoryBlock *block;

   if (flags & BEGL_USAGE_SECURE)
      return AllocateSecureBlock(ctx, numBytes, alignment, desc);

   block = (DRM_MemoryBlock*)malloc(sizeof(DRM_MemoryBlock));
   if (!block)
      goto error;

   if (alignment > MMU_PAGE_SIZE)
   {
      if (use_memory_log)
         fprintf(sLogFile, "A %zu %zu = UNSUPPORTED ALIGNMENT\n", numBytes, alignment);

      goto error;
   }

   cs.size = numBytes;
   cs.desc = use_alloc_desc_str ? desc : NULL;

   if ((flags & BEGL_USAGE_COHERENT) ||
       !(flags & BEGL_USAGE_CPU_READ) ||
       ctx->force_writecombine)
   {
      cs.flags |= V3D_CREATE_CPU_WRITECOMBINE;
   }

   if (!(flags & BEGL_USAGE_V3D_WRITE))
      cs.flags |= V3D_CREATE_HW_READONLY;

   if (!CreateGemObject(ctx->fd,&cs))
   {
      if (use_memory_log)
         fprintf(sLogFile, "A %zu %zu = FAILED\n", numBytes, alignment);

      goto error;
   }

   block->handle = cs.handle;
   block->flags = cs.flags;
   block->hw_addr = cs.hw_addr;
   block->real_size = cs.size;
   block->secure = false;
   block->mmap_offset = 0;

   if (ctx->clear_on_alloc)
      ClearMemoryBlock(ctx, block);

   if (use_memory_log)
      fprintf(sLogFile, "A %zu %zu = %p (%u @ %#x, %u bytes)\n", numBytes, alignment, block, block->handle, block->hw_addr, block->real_size);

   return block;

error:
   free(block);
   return NULL;
}

static void MemFreeBlock(void *context, BEGL_MemHandle h)
{
   DRM_MemoryContext *ctx = (DRM_MemoryContext*)context;
   DRM_MemoryBlock *block = (DRM_MemoryBlock *)h;

   if (use_memory_log)
      fprintf(sLogFile, "F %p (%u)\n", block, block->handle);

   CloseGemHandle(ctx->fd, block->handle);
   if (block->secure)
      NEXUS_MemoryBlock_Free(block->nexus_handle);

   free(block);
}

static BEGL_MemHandle MemWrapExternal(void *context, uint64_t physaddr, size_t length, const char *desc)
{
   struct drm_v3d_gem_create_ext cs = {};
   DRM_MemoryContext *ctx = (DRM_MemoryContext*)context;
   DRM_MemoryBlock *block = (DRM_MemoryBlock*)malloc(sizeof(DRM_MemoryBlock));

   if (!block)
      goto error;

   cs.phys = physaddr;
   cs.size = length;
   cs.desc = use_alloc_desc_str ? desc : NULL;

   if (!CreateGemExtObject(ctx->fd, &cs))
   {
      if (use_memory_log)
         fprintf(sLogFile, "W %#llx %zu = FAILED\n", (unsigned long long)physaddr, length);

      goto error;
   }

   block->handle = cs.handle;
   block->hw_addr = cs.hw_addr;
   block->real_size = cs.size;
   block->secure = false;
   block->flags = 0;
   block->mmap_offset = 0;

   if (use_memory_log)
      fprintf(sLogFile, "W %#llx %zu = %p (%u, %#x)\n", (unsigned long long)physaddr, length, block, block->handle, block->hw_addr);

   return block;
error:
   free(block);
   return NULL;
}

static void *MemMapBlock(void *context, BEGL_MemHandle h, size_t offset, size_t length, uint32_t usage_flags)
{
   DRM_MemoryContext *ctx = (DRM_MemoryContext*)context;
   DRM_MemoryBlock *block = (DRM_MemoryBlock *)h;
   int map_prot = PROT_NONE;
   void *ptr;

   if (block->secure)
   {
      if (use_memory_log)
         fprintf(sLogFile, "M %p (%u) %zu %zu = FAILED SECURE BLOCK\n", block, block->handle, offset, length);

      return NULL;
   }

   /*
    * This maps the entire object, which is actually what the abstract
    * layer calling this actually wants to happen as it caches the mapping,
    * hence offset must be zero.
    *
    * This avoids any possible issues with the offset and length not being
    * page aligned/sized.
    */
   BDBG_ASSERT(offset == 0);

   if (offset != 0)
   {
      if (use_memory_log)
         fprintf(sLogFile, "M %p (%u) %zu %zu = FAILED UNSUPPORTED OFFSET\n", block, block->handle, offset, length);

      return NULL;
   }

   if (block->real_size < (offset + length))
   {
      if (use_memory_log)
         fprintf(sLogFile, "M %p (%u) %zu %zu = FAILED LENGTH TOO BIG\n", block, block->handle, offset, length);

      return NULL;
   }

   if (!block->mmap_offset)
      block->mmap_offset = GemMapOffset(ctx->fd, block->handle);

   if (!block->mmap_offset)
   {
      if (use_memory_log)
         fprintf(sLogFile, "M %p (%u) %zu %zu = FAILED TO GET DRM OFFSET\n", block, block->handle, offset, length);

      return NULL;
   }

   if (usage_flags & BEGL_USAGE_CPU_READ)
      map_prot |= PROT_READ;

   if (usage_flags & BEGL_USAGE_CPU_WRITE)
      map_prot |= PROT_WRITE;

   ptr = mmap(NULL, block->real_size, map_prot, MAP_SHARED, ctx->fd, block->mmap_offset);

   if (use_memory_log)
      fprintf(sLogFile, "M %p (%u) %zu %zu = %p\n", block, block->handle, offset, length, ptr);

   return ptr;
}

static void MemUnmapBlock(void *context, BEGL_MemHandle h, void *cpu_ptr, size_t length)
{
   DRM_MemoryBlock *block = (DRM_MemoryBlock *)h;
   UNUSED(context);

   if (use_memory_log)
      fprintf(sLogFile, "m %p (%u) %p %zu\n", block, block->handle, cpu_ptr, length);

   munmap(cpu_ptr, block->real_size);
}

static uint32_t MemLockBlock(void *context, BEGL_MemHandle h)
{
   DRM_MemoryBlock *block = (DRM_MemoryBlock *)h;

   if (use_memory_log)
      fprintf(sLogFile, "L %p (%u) = %#x\n", block, block->handle, block->hw_addr);

   return block->hw_addr;
}

static void MemUnlockBlock(void *context, BEGL_MemHandle h)
{
   DRM_MemoryBlock *block = (DRM_MemoryBlock *)h;
   UNUSED(context);

   if (use_memory_log)
      fprintf(sLogFile, "l %p (%u)\n", block, block->handle);
}

/* Flush the cache for the given address range.*/
static void MemFlushCache(void *context, BEGL_MemHandle h, void *pCached, size_t numBytes)
{
   DRM_MemoryBlock *block = (DRM_MemoryBlock *)h;
   char *p = pCached;
   const size_t sz = 2 * 1024 * 1024; // Avoid the broken bcmdriver cacheflush for >3MB

   UNUSED(context);
   BDBG_ASSERT(pCached != NULL);

   /*
    * Optimize the write combined cases, we don't want to take the penalty of
    * of flushing things that cannot be in the cache, but we do need an
    * ARM bufferable memory barrier to be technically correct.
    *
    * Note that we must flush virtual ranges in externally wrapped objects
    * because we do not know where they came from and what mapping they have.
    */
   if (block && (block->flags & V3D_CREATE_CPU_WRITECOMBINE))
   {
      if (use_memory_log)
         fprintf(sLogFile, "B %p (%u) %p %zu\n", block, block ? block->handle : 0, pCached, numBytes);

      /*
       * Helpfully this does not appear to be a priviledged instruction and
       * it is the same for v7 and v8 instruction sets (at least it is in the
       * Linux barrier.h implementations for each of the ARM architectures).
       */
      __asm__ __volatile__ ("dmb oshst" : : : "memory");
      return;
   }

   if (use_memory_log)
      fprintf(sLogFile, "C %p (%u) %p %zu\n",
                        block, block ? block->handle : 0, pCached, numBytes);

   while (numBytes > sz)
   {
      NEXUS_FlushCache(p, sz);
      p += sz;
      numBytes -= sz;
   }

   if (numBytes > 0)
      NEXUS_FlushCache(p, numBytes);
}

/* Retrieve some information about the memory system */
static uint64_t MemGetInfo(void *context, BEGL_MemInfoType type)
{
   DRM_MemoryContext *ctx = (DRM_MemoryContext*)context;

   switch (type)
   {
   case BEGL_MemCacheLineSize:
      return (uint64_t)ctx->l2_cache_size;

   case BEGL_MemPagetablePhysAddr:
      return ctx->pagetable_physaddr;

   case BEGL_MemMmuMaxVirtAddr:
      return ctx->mmu_va_size-1;

   case BEGL_MemMmuUnsecureBinTranslation:
      return (uint64_t)ctx->offscreen_heap_translation;

   case BEGL_MemMmuSecureBinTranslation:
      return (uint64_t)ctx->secure_offscreen_heap_translation;

   case BEGL_MemPlatformToken:
      return GetFileToken(ctx->fd);

   case BEGL_MemLargestBlock:
      if (use_memory_log)
        fprintf(sLogFile, "B\n");

      /* Smallest largest block allocation required by OpenCL */
      return (uint64_t)128 * 1024 * 1024;

   case BEGL_MemFree:
      if (use_memory_log)
        fprintf(sLogFile, "G\n");

      /* What CAN do we do here if anything? nothing seems to use it */
      break;

   case BEGL_MemPrintHeapData:
      PrintDRMDriverInfo(ctx->fd);
      break;

   default:
      break;
   }

   return 0;
}

static bool ConfigureNexusHeapMappings(DRM_MemoryContext *ctx)
{
   NEXUS_HeapHandle heap;
   NEXUS_MemoryStatus memStatus;
#ifndef SINGLE_PROCESS
   NEXUS_ClientConfiguration clientConfig;
   NEXUS_Platform_GetClientConfiguration(&clientConfig);
#endif

   /*
    * The unsecure heap used for bin memory in the magnum module will always
    * be this one.
    */
   heap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);

   if (heap == NULL)
   {
      fprintf(stderr, "DRM: Cannot get Nexus offscreen heap to map into MMU\n");
      return false;
   }

   if (NEXUS_Heap_GetStatus(heap, &memStatus) != NEXUS_SUCCESS)
      goto error;

   ctx->l2_cache_size = memStatus.alignment;

   /*
    * Map the offscreen heap into this instance's MMU pagetable
    */
   ctx->offscreen_heap_mapping.phys = memStatus.offset;
   ctx->offscreen_heap_mapping.size = memStatus.size;
   ctx->offscreen_heap_mapping.desc = "NEXUS Offscreen Heap";
   if (!CreateGemExtObject(ctx->fd, &ctx->offscreen_heap_mapping))
   {
      perror("Unable to map Nexus offscreen heap into a GEM object");
      goto error;
   }

   ctx->offscreen_heap_translation =
      memStatus.offset - ctx->offscreen_heap_mapping.hw_addr;

   /* Now for the secure heap */
#ifndef SINGLE_PROCESS
   heap = clientConfig.heap[NXCLIENT_SECURE_GRAPHICS_HEAP];
#else
   heap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE);
#endif

   if (heap != NULL)
   {
      if (NEXUS_Heap_GetStatus(heap, &memStatus) != NEXUS_SUCCESS)
         goto error;

      ctx->secure_offscreen_heap_mapping.phys = memStatus.offset;
      ctx->secure_offscreen_heap_mapping.size = memStatus.size;
      ctx->secure_offscreen_heap_mapping.desc = "NEXUS Secure Offscreen Heap";
      if (!CreateGemExtObject(ctx->fd, &ctx->secure_offscreen_heap_mapping))
      {
         perror("Unable to map Nexus secure offscreen heap into a GEM object");
         goto error;
      }

      ctx->secure_heap = heap;
      ctx->secure_offscreen_heap_translation =
         memStatus.offset - ctx->secure_offscreen_heap_mapping.hw_addr;

      ctx->processId = (uint32_t)getpid();
   }
   else
   {
      fprintf(stderr,"DRM: No Nexus secure graphics heap available\n");
   }

   return true;

error:
   return false;
}

BEGL_MemoryInterface *CreateDRMMemoryInterface(void)
{
   char devicepath[128];
   DRM_MemoryContext *ctx;
   BEGL_MemoryInterface *mem;
   char *opt;
   int devicenum = 0;

   if (opt_bool("V3D_DRM_DISABLE"))
      return NULL;

   ctx = (DRM_MemoryContext*)calloc(1, sizeof(DRM_MemoryContext));
   mem = (BEGL_MemoryInterface*)calloc(1, sizeof(BEGL_MemoryInterface));

   if (!mem || !ctx)
      goto error;

   if (use_memory_log)
   {
      sLogFile = log_to_stderr ? stderr : fopen("MemoryLog.txt", "w");
      if (!sLogFile)
      {
         perror("Unable to open log file");
         goto error;
      }
   }

   opt = getenv("V3D_DRM_DEVICE_NUM");
   if (opt)
      devicenum = atoi(opt);

   sprintf(devicepath,"/dev/dri/card%d", devicenum);
   fprintf(stderr,"DRM: Trying to open: %s\n", devicepath);
   ctx->fd = open(devicepath, O_RDWR);
   if (ctx->fd < 0)
   {
      perror("Unable to open device");
      goto error;
   }

   PrintDRMDriverInfo(ctx->fd);

#ifdef SINGLE_PROCESS
   /*
    * Normally the driver will not clear pagetable entries on free'd GEM
    * objects during a close of the filehandle when an application gets
    * killed.
    *
    * This is to prevent job stalls and pagetable exceptions for hardware
    * jobs still using the memory until the magnum driver cleans everying
    * up when the client gets de-registered.
    *
    * But, in single process userspace mode if the application gets killed,
    * everything gets killed and the driver has no choice but to release
    * memory straight away. In that case we would prefer the pagetable
    * entries be cleared to minimize any window of opportunity for the hardware
    * to write to memory that has been released back to CMA.
    */
   ioctl(ctx->fd, DRM_IOCTL_V3D_CLEAR_PT_ON_CLOSE);
#endif

   if (!GetMmuPagetable(ctx))
      goto error;

   ctx->force_writecombine = opt_bool("V3D_DRM_FORCE_WRITECOMBINE");
   if (ctx->force_writecombine)
      fprintf(stderr, "DRM: Forcing all allocations to be writecombined\n");

   ctx->clear_on_alloc = opt_bool("V3D_DRM_CLEAR_ON_ALLOC");
   if (ctx->clear_on_alloc)
      fprintf(stderr, "DRM: Clearing memory on alloc\n");

   if (!ConfigureNexusHeapMappings(ctx))
      goto error;

   mem->context      = (void*)ctx;
   mem->FlushCache   = MemFlushCache;
   mem->GetInfo      = MemGetInfo;
   mem->Alloc        = MemAllocBlock;
   mem->Free         = MemFreeBlock;
   mem->Map          = MemMapBlock;
   mem->Unmap        = MemUnmapBlock;
   mem->Lock         = MemLockBlock;
   mem->Unlock       = MemUnlockBlock;
   mem->WrapExternal = MemWrapExternal;

   return mem;

error:
   free(ctx);
   free(mem);
   return NULL;
}

void DestroyDRMMemoryInterface(BEGL_MemoryInterface *mem)
{
   if (!log_to_stderr && sLogFile)
   {
      fclose(sLogFile);
      sLogFile = NULL;
   }

   if (mem)
   {
      DRM_MemoryContext *ctx = (DRM_MemoryContext*)mem->context;

      close(ctx->fd);
      free(ctx);
      free(mem);
   }
}
