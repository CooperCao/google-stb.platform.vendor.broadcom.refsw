/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#include <EGL/begl_memplatform.h>
#include "memory_android.h"
#include "nexus_heap_selection.h"
#include "nexus_platform.h"
#include "nexus_base_mmap.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif
#include "fatal_error.h"

#include <EGL/egl.h>
#include <ctype.h>

#include <cutils/log.h>
#include <cutils/properties.h>
#define PRINTF ALOGD

#include "nx_ashmem.h"
#include <fcntl.h>
#include <linux/brcmstb/proc_info_proxy.h>

/* default block size */
#define BLOCK_SIZE (16*1024*1024)

#ifndef NDEBUG
#define DEBUG_PRINTF PRINTF
#else
static int dummy(void) { return 0; }
#define DEBUG_PRINTF 1 ? dummy() : PRINTF
#endif

#define UNUSED(A) (void)A

#define MAX_PROCESS_NAME 256
#define MAX_DESC_NAME    256


/* Turn on to log memory access pattern to text file from 3D driver */
/*#define LOG_MEMORY_PATTERN*/

#include <malloc.h>
#include <memory.h>
#include <stdlib.h>

#ifdef LOG_MEMORY_PATTERN
static FILE *sLogFile = NULL;
#endif

enum
{
   eNO_PROBLEMS            = 0,
   eNOT_HD_GRAPHICS_HEAP   = 1 << 0,
   eCROSSES_256MB          = 1 << 1,
   eGUARD_BANDED           = 1 << 2,
   eWRONG_TYPE             = 1 << 3,
   eCROSSES_1GB            = 1 << 4,
   eTOO_SMALL              = 1 << 5
};

typedef struct
{
   NEXUS_HeapHandle        heap;
   void                    *heapStartCached;
   uint32_t                heapStartPhys;
   uint32_t                heapSize;

} ANPL_HeapMapping;

typedef struct
{
   int fd;
   NEXUS_MemoryBlockHandle hdl;
   uint32_t size;

} ANPL_MemoryTracker;

typedef struct
{
   ANPL_HeapMapping        heaps[4];   /* 0 is NXCLIENT_DEFAULT_HEAP, 1 is NXCLIENT_DYNAMIC_HEAP */
   uint32_t                numHeaps;
   int                     l2_cache_size;
   bool                    useDynamicMMA;
   int                     heapGrow;
   uint32_t                processId;
   char                    alloc_name[PROPERTY_VALUE_MAX];
   bool                    allowsMovable;
   int                     tracker;

} ANPL_MemoryContext;

#ifdef SINGLE_PROCESS
#define NEXUS_HEAPCONFIG NEXUS_PlatformConfiguration
#else
#define NEXUS_HEAPCONFIG NEXUS_ClientConfiguration
#endif

/* Not a complete implementation, just enough to match what nx_ashmem is doing */
static unsigned long long memparse(const char *ptr, char **retptr, unsigned long long def)
{
   char *endptr;
   char mod;
   BSTD_UNUSED(retptr);

   unsigned long long ret = strtoull(ptr, &endptr, 0);
   mod = toupper(*endptr);

   if (mod == 'G')
      ret <<= 30;
   else if (mod == 'M')
      ret <<= 20;
   else if (mod == 'K')
      ret <<= 10;

   if (ret == 0)
      ret = def;

   return ret;
}

static bool UseDynamicMMAHeap(int *growSize)
{
   char val[PROPERTY_VALUE_MAX];
   bool useDynamicMMA = false;      /* Default non-dynamic */

   *growSize = BLOCK_SIZE;

   property_get("ro.nx.mma", val, "0");

   if (val[0] == 't' || val[0] == 'T' || val[0] == '1')
      useDynamicMMA = true;

   property_get("ro.nx.heap.grow", val, "0");
   if (val[0] != '0')
      *growSize = memparse(val, NULL, BLOCK_SIZE);

   return useDynamicMMA;
}

static bool UseMovableBlocks(void)
{
   char val[PROPERTY_VALUE_MAX];
   bool useMovableBlocks = false;

   property_get("ro.nx.v3d.no_map_lock", val, "0");
   if (atoi(val) != 0)
      useMovableBlocks = true;

   return useMovableBlocks;
}

/*****************************************************************************
 * Memory interface
 *****************************************************************************/
void EGL_nexus_trim_cma(void *nexus_client);
extern void *v3d_get_nexus_client_context();
static BEGL_MemHandle MemAllocBlock(void *context, size_t numBytes, size_t alignment, uint32_t flags, const char *desc)
{
   ANPL_MemoryContext                  *data = (ANPL_MemoryContext*)context;
   NEXUS_MemoryBlockHandle             block;
   int                                 memBlkFd = -1;
   ANPL_MemoryTracker                  *memTracker = (ANPL_MemoryTracker*)malloc(sizeof(ANPL_MemoryTracker));

   if (memTracker == NULL)
   {
      return NULL;
   }

   if (!data->allowsMovable)
   {
      goto alloc_default;
   }

   memBlkFd = open(data->alloc_name, O_RDWR, 0);
   if (memBlkFd >= 0)
   {
      struct nx_ashmem_alloc ashmem_alloc;
      ashmem_alloc.size = numBytes;
      ashmem_alloc.align = alignment;
      ashmem_alloc.movable = data->allowsMovable;
      int ret = ioctl(memBlkFd, NX_ASHMEM_SET_SIZE, &ashmem_alloc);
      if (ret < 0)
      {
         close(memBlkFd);
         memBlkFd = -1;
      }
      else
      {
         struct nx_ashmem_getmem ashmem_getmem;
         ret = ioctl(memBlkFd, NX_ASHMEM_GETMEM, &ashmem_getmem);
         if (ret < 0)
         {
            void *nexus_client = v3d_get_nexus_client_context();
            EGL_nexus_trim_cma(nexus_client);
            ret = ioctl(memBlkFd, NX_ASHMEM_GETMEM, &ashmem_getmem);
            if (ret < 0)
            {
               close(memBlkFd);
               memBlkFd = -1;
            }
         }

         if (!ret && memBlkFd >= 0)
         {
            memTracker->fd = memBlkFd;
            memTracker->hdl = (NEXUS_MemoryBlockHandle)(intptr_t)ashmem_getmem.hdl;
            /* block has been allocated, early exit. */
            return (BEGL_MemHandle) memTracker;
         }
      }
   }

   /* fallback to default allocation scheme. */
alloc_default:
   memTracker->fd = -1;

#if defined(NEXUS_MemoryBlock_Allocate) /* NEXUS_MemoryBlock_Allocate became a define when
                                           NEXUS_MemoryBlock_Allocate_tagged started to exist */
   block = NEXUS_MemoryBlock_Allocate_tagged(data->heaps[0].heap, numBytes, alignment, NULL, desc, data->processId);
#else
   block = NEXUS_MemoryBlock_Allocate(data->heaps[0].heap, numBytes, alignment, NULL);
#endif

#ifndef SINGLE_PROCESS
   if (!block)
   {
      int growRC = NEXUS_SUCCESS;
      int trim_cma = 0;

      while (block == NULL && growRC == NEXUS_SUCCESS)
      {
#ifndef NXCLIENT_SUPPORT
         uint32_t numGrowBlocks = 1;

         if (numBytes > data->heapGrow)
            numGrowBlocks = numBytes / data->heapGrow + 1;
#endif

         /* multiprocess we can grow the heap and try again */
#ifdef NXCLIENT_SUPPORT
         growRC = NxClient_GrowHeap(NXCLIENT_DYNAMIC_HEAP);
#else
         growRC = NEXUS_Platform_GrowHeap(data->heaps[0].heap, numGrowBlocks * data->heapGrow);
#endif
         if (growRC == NEXUS_SUCCESS)
         {
#if defined(NEXUS_MemoryBlock_Allocate) /* NEXUS_MemoryBlock_Allocate became a define when
                                           NEXUS_MemoryBlock_Allocate_tagged started to exist */
            block = NEXUS_MemoryBlock_Allocate_tagged(data->heaps[0].heap, numBytes, alignment, NULL, desc, data->processId);
#else
            block = NEXUS_MemoryBlock_Allocate(data->heaps[0].heap, numBytes, alignment, NULL);
#endif
         }
         else
         {
            if (!trim_cma) {
               /* Last chance, try to free up some memory. */
               void *nexus_client = v3d_get_nexus_client_context();
               EGL_nexus_trim_cma(nexus_client);
               growRC = NEXUS_SUCCESS;
               trim_cma = 1;
            } else {
               /* Cannot grow any more */
               break;
            }
         }
      }
   }
#endif

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "A %u %u = %u\n", numBytes, alignment, (uint32_t)block);
#endif

   if (block == NULL)
   {
      free(memTracker);
      /*PRINTF("Alloc block = %p\n", block);*/
      return NULL;
   }

   memTracker->hdl = block;
   memTracker->size = numBytes;

   if (data->tracker >= 0) {
      struct proxy_info_memtrack memtrack;
      memtrack.pid = getpid();
      memtrack.size = numBytes;
      memtrack.act = 1;
      ioctl(data->tracker, PROC_INFO_IOCTL_PROXY_SET_MEMTRACK, &memtrack);
   }

   /*PRINTF("Alloc block = %p\n", block);*/
   return (BEGL_MemHandle) memTracker;
}

static void MemFreeBlock(void *context, BEGL_MemHandle handle)
{
   ANPL_MemoryTracker *memTracker = (ANPL_MemoryTracker*)handle;
   ANPL_MemoryContext *data = (ANPL_MemoryContext*)context;

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "F %u\n", (uint32_t)memTracker->hdl);
#endif

   /*PRINTF("MemFreeBlock %p\n", handle);*/
   if (memTracker->fd != -1)
   {
      close(memTracker->fd);
   }
   else
   {
      NEXUS_MemoryBlock_Free(memTracker->hdl);
   }

   if (data->tracker >= 0) {
      struct proxy_info_memtrack memtrack;
      memtrack.pid = getpid();
      memtrack.size = memTracker->size;
      memtrack.act = -1;
      ioctl(data->tracker, PROC_INFO_IOCTL_PROXY_SET_MEMTRACK, &memtrack);
   }

   free(memTracker);
}

static void *MemMapBlock(void *context, BEGL_MemHandle h, size_t offset, size_t length, uint32_t usage_flags)
{
   void        *ret = NULL;
   NEXUS_Error err;
   ANPL_MemoryTracker *memTracker = (ANPL_MemoryTracker*)h;

   UNUSED(context);
   UNUSED(length);
   UNUSED(usage_flags);

   err = NEXUS_MemoryBlock_Lock(memTracker->hdl, &ret);
   if (err != NEXUS_SUCCESS)
      return NULL;

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "M %u %u %u %p\n", (uint32_t)h, offset, length, (uint8_t *)ret + offset);
#endif

   /*PRINTF("MemMapBlock %p = %p\n", h, ret);*/

   return (uint8_t *)ret + offset;
}

static void MemUnmapBlock(void *context, BEGL_MemHandle h, void *cpu_ptr, size_t length)
{
   ANPL_MemoryTracker *memTracker = (ANPL_MemoryTracker*)h;

   UNUSED(context);
   UNUSED(cpu_ptr);
   UNUSED(length);

   /*PRINTF("MemUnmapBlock %p\n", h);*/

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "m %u %p %u\n", (uint32_t)h, cpu_ptr, length);
#endif

   NEXUS_MemoryBlock_Unlock(memTracker->hdl);
}

static uint32_t MemLockBlock(void *context, BEGL_MemHandle h)
{
   NEXUS_Addr  devPtr;
   NEXUS_Error err;
   ANPL_MemoryTracker *memTracker = (ANPL_MemoryTracker*)h;

   UNUSED(context);

   err = NEXUS_MemoryBlock_LockOffset(memTracker->hdl, &devPtr);
   if (err != NEXUS_SUCCESS)
      return 0;

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "L %u %u\n", (uint32_t)h, (uint32_t)(devPtr & 0xFFFFFFFF));
#endif

   /*PRINTF("MemLockBlock %p = %llx\n", h, devPtr);*/

   BDBG_ASSERT(devPtr >> 32 == 0);

   return (uint32_t)(devPtr & 0xFFFFFFFF);
}

static void MemUnlockBlock(void *context, BEGL_MemHandle h)
{
   UNUSED(context);
   ANPL_MemoryTracker *memTracker = (ANPL_MemoryTracker*)h;

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "l %u\n", (uint32_t)h);
#endif

   NEXUS_MemoryBlock_UnlockOffset(memTracker->hdl);
}

/* Flush the cache for the given address range.*/
static void MemFlushCache(void *context, BEGL_MemHandle handle, void *pCached, size_t numBytes)
{
   UNUSED(context);
   (void)handle;
   BDBG_ASSERT(pCached != NULL);

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "C %u %p %u\n", (uint32_t)handle, pCached, numBytes);
#endif

   /* Avoid Nexus ARM cache flush using broken set/way approach for flushes >= 3MB */
   while (numBytes > 0)
   {
      size_t const maxBytesThisTime = 2*1024*1024;
      size_t numBytesThisTime = numBytes < maxBytesThisTime ? numBytes : maxBytesThisTime;

      NEXUS_FlushCache(pCached, numBytesThisTime);
      pCached = (uint8_t*)pCached + numBytesThisTime;
      numBytes -= numBytesThisTime;
   }
}

/* Retrieve some information about the memory system */
static uint64_t MemGetInfo(void *context, BEGL_MemInfoType type)
{
  ANPL_MemoryContext   *data = (ANPL_MemoryContext*)context;

  switch (type)
  {
  case BEGL_MemCacheLineSize :
     return (uint64_t)data->l2_cache_size;
     break;

  case BEGL_MemLargestBlock  :
     {
#ifdef LOG_MEMORY_PATTERN
        if (sLogFile)
           fprintf(sLogFile, "B\n");
#endif
        struct NEXUS_MemoryStatus  status;
        NEXUS_Heap_GetStatus(data->heaps[0].heap, &status);
        return (uint64_t)status.largestFreeBlock;
        break;
     }

  case BEGL_MemFree          :
     {
#ifdef LOG_MEMORY_PATTERN
        if (sLogFile)
           fprintf(sLogFile, "G\n");
#endif
        struct NEXUS_MemoryStatus  status;
        NEXUS_Heap_GetStatus(data->heaps[0].heap, &status);
        return (uint64_t)status.free;
        break;
     }

  case BEGL_MemPrintHeapData:
     {
        NEXUS_Heap_Dump(data->heaps[0].heap);
        return 0;
     }
  default:
     {
        break;
     }
  }

  return 0;
}

//static void DebugHeap(NEXUS_HeapHandle heap)
//{
//   NEXUS_MemoryStatus   status;
//   NEXUS_Error          rc;
//
//   rc = NEXUS_Heap_GetStatus(heap, &status);
//   BDBG_ASSERT(!rc);
//
//   PRINTF("MEMC%d, physical addr 0x%08x, size %9d (0x%08x), alignment %2d, base ptr %p, high water %9d, guardbanding? %c\n",
//         status.memcIndex, (unsigned int)status.offset, status.size, status.size, status.alignment, status.addr,
//         status.highWatermark,
//         status.guardBanding?'y':'n');
//}

BEGL_MemoryInterface *CreateAndroidMemoryInterface(void)
{
   ANPL_MemoryContext   *ctx = NULL;
   BEGL_MemoryInterface *mem = (BEGL_MemoryInterface*)malloc(sizeof(BEGL_MemoryInterface));
   char device[PROPERTY_VALUE_MAX];

#ifdef LOG_MEMORY_PATTERN
   sLogFile = fopen("MemoryLog.txt", "w");
#endif

   if (mem != NULL)
   {
      ctx = (ANPL_MemoryContext*)malloc(sizeof(ANPL_MemoryContext));
      memset(mem, 0, sizeof(BEGL_MemoryInterface));

      if (ctx != NULL)
      {
         NEXUS_MemoryStatus memStatus;

         memset(ctx, 0, sizeof(ANPL_MemoryContext));

         ctx->tracker = open("/dev/bcmpip", O_RDWR);
         if (ctx->tracker < 0)
            ALOGE("failed to allocate tracker");

         ctx->useDynamicMMA = UseDynamicMMAHeap(&ctx->heapGrow);
         ctx->allowsMovable = UseMovableBlocks();

         ctx->numHeaps = 1;
         ctx->processId = (uint32_t)getpid();

         mem->context = (void*)ctx;
         mem->FlushCache = MemFlushCache;
         mem->GetInfo    = MemGetInfo;

         mem->Alloc         = MemAllocBlock;
         mem->Free          = MemFreeBlock;
         mem->Map           = MemMapBlock;
         mem->Unmap         = MemUnmapBlock;
         mem->Lock          = MemLockBlock;
         mem->Unlock        = MemUnlockBlock;

         ctx->heaps[0].heap = ctx->useDynamicMMA ? GetDynamicHeap() : GetDefaultHeap();
         if (!ctx->heaps[0].heap)
            FATAL_ERROR("Could not get Nexus heap\n");

         property_get("ro.nexus.ashmem.devname", device, NULL);
         if (strlen(device))
         {
            strcpy(ctx->alloc_name, "/dev/");
            strcat(ctx->alloc_name, device);
         }
         NEXUS_Heap_GetStatus(ctx->heaps[0].heap, &memStatus);
         ctx->heaps[0].heapStartCached = memStatus.addr;
         ctx->heaps[0].heapStartPhys = memStatus.offset;
         ctx->heaps[0].heapSize = memStatus.size;
         ctx->l2_cache_size = memStatus.alignment;

         DEBUG_PRINTF("NXPL : NXPL_CreateMemInterface() INFO.\nVirtual (cached) %p, Physical 0x%08x, Size %d, Alignment %d\n",
            ctx->heaps[0].heapStartCached,
            ctx->heaps[0].heapStartPhys,
            ctx->heaps[0].heapSize,
            ctx->l2_cache_size);
      }
      else
      {
         goto error0;
      }
   }
   return mem;

error0:
   free(mem);
   return NULL;
}

void DestroyAndroidMemoryInterface(BEGL_MemoryInterface *mem)
{
   if (mem != NULL)
   {
      if (mem->context != NULL)
      {
         ANPL_MemoryContext *ctx = (ANPL_MemoryContext*)mem->context;

         if (ctx->tracker >= 0) {
            close(ctx->tracker);
            ctx->tracker = -1;
         }

         free(ctx);
      }

      memset(mem, 0, sizeof(BEGL_MemoryInterface));
      free(mem);

#ifdef LOG_MEMORY_PATTERN
      if (sLogFile)
      {
         fclose(sLogFile);
         sLogFile = NULL;
      }
#endif
   }
}
