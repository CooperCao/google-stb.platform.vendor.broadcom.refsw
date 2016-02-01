/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "nexus_memory.h"
#include "nexus_platform.h"
#include "nexus_base_mmap.h"
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include <EGL/egl.h>
#include <ctype.h>

#define PRINTF printf

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

} NXPL_HeapMapping;

typedef struct
{
   NXPL_HeapMapping        heaps[4];   /* 0 is NXCLIENT_DEFAULT_HEAP, 1 is NXCLIENT_DYNAMIC_HEAP */
   uint32_t                numHeaps;
   int                     l2_cache_size;
   bool                    useDynamicMMA;
   int                     heapGrow;
   uint32_t                processId;
} NXPL_MemoryContext;

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
   bool useDynamicMMA = false;      /* Default non-dynamic */

   *growSize = BLOCK_SIZE;

   char *val = getenv("V3D_USE_MMA");
   if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
      useDynamicMMA = true;

   val = getenv("V3D_GROW_SIZE");
   if (val)
      *growSize = memparse(val, NULL, BLOCK_SIZE);

   return useDynamicMMA;
}

/*****************************************************************************
 * Memory interface
 *****************************************************************************/
static BEGL_MemHandle MemAllocBlock(void *context, size_t numBytes, size_t alignment, uint32_t flags, const char *desc)
{
   NXPL_MemoryContext                  *data = (NXPL_MemoryContext*)context;
   NEXUS_MemoryBlockHandle             block;

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
            /* Cannot grow any more */
            break;
         }
      }
   }
#endif

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "A %u %u = %u\n", numBytes, alignment, (uint32_t)block);
#endif

   /*PRINTF("Alloc block = %p\n", block);*/
   return block;
}

static void MemFreeBlock(void *context, BEGL_MemHandle handle)
{
   NEXUS_MemoryBlockHandle block = (NEXUS_MemoryBlockHandle)handle;

   UNUSED(context);

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "F %u\n", (uint32_t)block);
#endif

   /*PRINTF("MemFreeBlock %p\n", handle);*/
   NEXUS_MemoryBlock_Free(block);
}

static void *MemMapBlock(void *context, BEGL_MemHandle h, size_t offset, size_t length, uint32_t usage_flags)
{
   void        *ret = NULL;
   NEXUS_Error err;

   UNUSED(context);
   UNUSED(length);
   UNUSED(usage_flags);

   err = NEXUS_MemoryBlock_Lock((NEXUS_MemoryBlockHandle)h, &ret);
   if (err != NEXUS_SUCCESS)
      return NULL;

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "M %u %u %u %p\n", (uint32_t)h, offset, length, ret + offset);
#endif

   /*PRINTF("MemMapBlock %p = %p\n", h, ret);*/

   return ret + offset;
}

static void MemUnmapBlock(void *context, BEGL_MemHandle h, void *cpu_ptr, size_t length)
{
   UNUSED(context);
   UNUSED(cpu_ptr);
   UNUSED(length);

   /*PRINTF("MemUnmapBlock %p\n", h);*/

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "m %u %p %u\n", (uint32_t)h, cpu_ptr, length);
#endif

   NEXUS_MemoryBlock_Unlock((NEXUS_MemoryBlockHandle)h);
}

static uint32_t MemLockBlock(void *context, BEGL_MemHandle h)
{
   NEXUS_Addr  devPtr;
   NEXUS_Error err;

   UNUSED(context);

   err = NEXUS_MemoryBlock_LockOffset((NEXUS_MemoryBlockHandle)h, &devPtr);
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

#ifdef LOG_MEMORY_PATTERN
   if (sLogFile)
      fprintf(sLogFile, "l %u\n", (uint32_t)h);
#endif

   NEXUS_MemoryBlock_UnlockOffset((NEXUS_MemoryBlockHandle)h);
}

/* Flush the cache for the given address range.*/
static void MemFlushCache(void *context, BEGL_MemHandle handle, void *pCached, size_t numBytes)
{
   UNUSED(context);

   if (handle == NULL || pCached == NULL)
   {
#ifdef LOG_MEMORY_PATTERN
      if (sLogFile)
         fprintf(sLogFile, "C %u %p %u\n", (uint32_t)handle, 0, ~0);
#endif

      NEXUS_FlushCache(0, ~0);
   }
   else
   {
#ifdef LOG_MEMORY_PATTERN
      if (sLogFile)
         fprintf(sLogFile, "C %u %p %u\n", (uint32_t)handle, pCached, numBytes);
#endif

      NEXUS_FlushCache(pCached, numBytes);
   }
}

/* Retrieve some information about the memory system */
static uint64_t MemGetInfo(void *context, BEGL_MemInfoType type)
{
  NXPL_MemoryContext   *data = (NXPL_MemoryContext*)context;

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

/* Return the primary memory heap */
__attribute__((visibility("default")))
NEXUS_HeapHandle NXPL_MemHeap(BEGL_MemoryInterface *mem)
{
   if (mem != NULL)
   {
      NXPL_MemoryContext *data = (NXPL_MemoryContext*)mem->context;
      return data->heaps[0].heap;
   }
   return NULL;
}

BEGL_MemoryInterface *CreateMemoryInterface(void)
{
   NXPL_MemoryContext   *ctx = NULL;
   BEGL_MemoryInterface *mem = (BEGL_MemoryInterface*)malloc(sizeof(BEGL_MemoryInterface));

#ifdef LOG_MEMORY_PATTERN
   sLogFile = fopen("MemoryLog.txt", "w");
#endif

   if (mem != NULL)
   {
      ctx = (NXPL_MemoryContext*)malloc(sizeof(NXPL_MemoryContext));
      memset(mem, 0, sizeof(BEGL_MemoryInterface));

      if (ctx != NULL)
      {
         NEXUS_MemoryStatus memStatus;

         memset(ctx, 0, sizeof(NXPL_MemoryContext));

         ctx->useDynamicMMA = UseDynamicMMAHeap(&ctx->heapGrow);

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

#ifndef SINGLE_PROCESS
         {
            NEXUS_ClientConfiguration clientConfig;
            NEXUS_Platform_GetClientConfiguration(&clientConfig);

            if (ctx->useDynamicMMA)
               ctx->heaps[0].heap = clientConfig.heap[4];
            else if (clientConfig.mode == NEXUS_ClientMode_eUntrusted)
               ctx->heaps[0].heap = clientConfig.heap[0];
            else
               ctx->heaps[0].heap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);
         }
#else
         /* If you change this, then the heap must also change in nexus_platform.c
            With refsw NEXUS_OFFSCREEN_SURFACE is the only heap guaranteed to be valid for v3d to use */
         ctx->heaps[0].heap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);
#endif

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

void DestroyMemoryInterface(BEGL_MemoryInterface *mem)
{
   if (mem != NULL)
   {
      if (mem->context != NULL)
      {
         NXPL_MemoryContext *ctx = (NXPL_MemoryContext*)mem->context;

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
