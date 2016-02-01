/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
This is a default implementation of a Nexus platform layer used by V3D.
This illustrates one way in which the abstract memory interface might be
implemented. You can replace this with your own custom version if preferred.
=============================================================================*/

#include "nexus_memory.h"
#include "nexus_platform.h"
#include "nexus_graphicsv3d.h"
#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d.h"
#endif
#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif
#include "memory_nexus_priv.h"
#include "packet_rgba.h"
#include "packet_yv12.h"

/* Turn on to log memory access pattern to text file from 3D driver */
/*#define LOG_MEMORY_PATTERN*/

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>

#ifdef ANDROID
#include <cutils/properties.h>
#endif

/* default block size */
#define BLOCK_SIZE (16*1024*1024)

#ifdef ANDROID
#define NX_MMA                   "ro.nx.mma"
#define NX_HEAP_GROW             "ro.nx.heap.grow"
#else
#define NX_MMA                   "V3D_USE_MMA"
#define NX_HEAP_GROW             "V3D_GROW_SIZE"
#endif

#ifdef LOG_MEMORY_PATTERN
static FILE *sLogFile = NULL;
#endif

#ifdef SINGLE_PROCESS
#define NEXUS_HEAPCONFIG NEXUS_PlatformConfiguration
#else
#define NEXUS_HEAPCONFIG NEXUS_ClientConfiguration
#endif

/*****************************************************************************
 * Memory interface
 *****************************************************************************/
/* Allocate aligned device memory, and return the cached address, or NULL on failure.*/
static void *MemAlloc(void *context, size_t numBytes, uint32_t alignment)
{
   NXPL_MemoryData               *data = (NXPL_MemoryData*)context;

   if (data->useMMA)
   {
      NEXUS_MemoryBlockHandle       allocedBlock;
      allocedBlock = NEXUS_MemoryBlock_Allocate(data->heapMap.heap, numBytes, 4096/*alignment*/, NULL);
      if (!allocedBlock)
      {
         int growRC = NEXUS_SUCCESS;
         while (allocedBlock == NULL && growRC == NEXUS_SUCCESS)
         {
            uint32_t numGrowBlocks = 1;

            if (numBytes > data->heapGrow)
               numGrowBlocks = numBytes / data->heapGrow + 1;

#ifdef NXCLIENT_SUPPORT
            growRC = NxClient_GrowHeap(NXCLIENT_DYNAMIC_HEAP);
#else
            growRC = NEXUS_Platform_GrowHeap(data->heapMap.heap, numGrowBlocks * data->heapGrow);
#endif
            if (growRC == NEXUS_SUCCESS)
               allocedBlock = NEXUS_MemoryBlock_Allocate(data->heapMap.heap, numBytes, 4096/*alignment*/, NULL);
            else
            {
               printf("Unable to GROW heap\n");
               break;
            }
         }
      }

#ifdef LOG_MEMORY_PATTERN
      if (sLogFile)
         fprintf(sLogFile, "A %u %u = %u\n", numBytes, alignment, (uint32_t)allocedBlock);
#endif
      return (void *)allocedBlock;
   }
   else
   {
      NEXUS_MemoryAllocationSettings settings;
      void *allocedOffset = NULL;

      NEXUS_Memory_GetDefaultAllocationSettings(&settings);

      settings.alignment = alignment;
      settings.heap = data->heapMap.heap;  /* Use the primary heap */

      NEXUS_Memory_Allocate(numBytes, &settings, &allocedOffset);

#ifdef LOG_MEMORY_PATTERN
      if (sLogFile)
         fprintf(sLogFile, "A %u %u = %u\n", numBytes, alignment, (uint32_t)alloced);
#endif

      if (allocedOffset == NULL)
      {
#ifndef NDEBUG
         if (data && data->heapMap.heap)
         {
            NEXUS_MemoryStatus  status;
            NEXUS_Heap_GetStatus(data->heapMap.heap, &status);

            printf("Platform layer MemAlloc failed : \n");
            printf("total heap size = %d\n", status.size);
            printf("num allocs = %d\n", status.numAllocs);
            printf("free = %d\n", status.free);
            printf("largest free block = %d\n", status.largestFreeBlock);
         }
#endif
      }
      return allocedOffset;
   }
}

/* Free a previously allocated block of device memory. Pass a cached address.*/
static void MemFree(void *context, void *pCached)
{
   NXPL_MemoryData *data = (NXPL_MemoryData*)context;

   if (data->useMMA)
   {
      NEXUS_MemoryBlockHandle alloced = (NEXUS_MemoryBlockHandle)pCached;

#ifdef LOG_MEMORY_PATTERN
      if (sLogFile)
         fprintf(sLogFile, "F %u\n", (uint32_t)alloced);
#endif

      /* Under Nexus, this also handles both the unlocks for us. */

      /*printf("MemFreeBlock %p\n", handle);*/
      NEXUS_MemoryBlock_Free(alloced);
   }
   else
   {
#ifdef LOG_MEMORY_PATTERN
      if (sLogFile)
         fprintf(sLogFile, "F %u\n", (uint32_t)pCached);
#endif

#ifndef NDEBUG
      if (pCached == 0)
         printf("NXPL : Trying to free NULL pointer\n");
#endif

      NEXUS_Memory_Free(pCached);
   }
}

static bool IsValidHeapCachedPtr(void *context, void *pCached)
{
   bool res = false;
   NXPL_MemoryData *data = (NXPL_MemoryData*)context;

   if ((pCached >= data->heapMap.heapStartCached) &&
      (pCached < (data->heapMap.heapStartCached + data->heapMap.heapSize)))
      res = true;

   return res;
}

/* Return a physical device memory offset given a cached pointer. Returns 0 on failure.*/
/* This will work for the primary heap and any extra heaps that may have been registered. */
static uint32_t MemConvertCachedToPhysical(void *context, void *pCached)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;

#ifndef NDEBUG
   if (pCached == 0)
      printf("NXPL : Trying to convert NULL pointer\n");
#endif

   return data->heapMap.heapStartPhys + ((uintptr_t)pCached - (uintptr_t)data->heapMap.heapStartCached);
}

/* Return a cached memory pointer given a physical device memory offset. Returns 0 on failure.*/
static void *MemConvertPhysicalToCached(void *context, uint32_t offset)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;

   return (void*)((uintptr_t)data->heapMap.heapStartCached + (offset - data->heapMap.heapStartPhys));
}

/* Flush the cache for the given address range.*/
static void MemFlushCache(void *context, void *pCached, size_t numBytes)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;

   if (data->useMMA)
   {
      /* dump everything for now */
      NEXUS_FlushCache(0, ~0);
   }
   else
   {
      bool              validPtr = IsValidHeapCachedPtr(context, pCached);

      if (!validPtr || numBytes > data->heapMap.heapSize)
         NEXUS_Memory_FlushCache(data->heapMap.heapStartCached, data->heapMap.heapSize);
      else
         NEXUS_Memory_FlushCache(pCached, numBytes);
   }
}

/* Retrieve some information about the memory system */
static uint32_t MemGetInfo(void *context, BEGL_MemInfoType type)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;

   switch (type)
   {
   case BEGL_MemHeapStartPhys :
      return data->heapMap.heapStartPhys;
      break;
   case BEGL_MemHeapEndPhys   :
      return data->heapMap.heapStartPhys + data->heapMap.heapSize - 1;
      break;
   case BEGL_MemCacheLineSize :
      return data->l2CacheSize;
      break;

   case BEGL_MemLargestBlock  :
      {
#ifdef LOG_MEMORY_PATTERN
         if (sLogFile)
            fprintf(sLogFile, "B\n");
#endif
         struct NEXUS_MemoryStatus  status;
         NEXUS_Heap_GetStatus(data->heapMap.heap, &status);
         return status.largestFreeBlock;
         break;
      }

   case BEGL_MemFree          :
      {
#ifdef LOG_MEMORY_PATTERN
         if (sLogFile)
            fprintf(sLogFile, "M\n");
#endif
         struct NEXUS_MemoryStatus  status;
         NEXUS_Heap_GetStatus(data->heapMap.heap, &status);
         return status.free;
         break;
      }

   }

   return 0;
}

#if NEXUS_HAS_GRAPHICS2D
static void completeCallback(void *pParam, int iParam)
{
   BSTD_UNUSED(iParam);
   BKNI_SetEvent(pParam);
}

static void MemCopy2d(void *context, BEGL_MemCopy2d *params)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;

   if (data && params)
   {
      /* as this is a straight 32bit memcpy it actually works for either of the buffer
         formats which dont require format conversion */
      if (params->format == BEGL_BufferFormat_eA8B8G8R8)
         memCopy2d_rgba(data, params);
      else if (params->format == BEGL_BufferFormat_eYV12_Texture)
         memCopy2d_yv12(data, params);
   }
}
#endif

static void DebugHeap(NEXUS_HeapHandle heap)
{
   NEXUS_MemoryStatus   status;
   NEXUS_Error          rc;

   rc = NEXUS_Heap_GetStatus(heap, &status);
   BDBG_ASSERT(!rc);

   printf("MEMC%d, physical addr 0x%08x, size %9d (0x%08x), alignment %2d, base ptr %p, high water %9d, guardbanding? %c\n",
         status.memcIndex, status.offset, status.size, status.size, status.alignment, status.addr,
         status.highWatermark,
         status.guardBanding?'y':'n');
}

/* dummy - feed it through for non MMA */
static void *MemLock(void *context, void *handle, uintptr_t *offset)
{
   void *res = NULL;
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;

   if (data->useMMA)
   {
      NEXUS_Addr  devPtr;
      NEXUS_Error err;

      err = NEXUS_MemoryBlock_LockOffset((NEXUS_MemoryBlockHandle)handle, &devPtr);
      if (err != NEXUS_SUCCESS)
         return 0;
      *offset = (uintptr_t)devPtr;

      err = NEXUS_MemoryBlock_Lock((NEXUS_MemoryBlockHandle)handle, &res);
      if (err != NEXUS_SUCCESS)
      {
         NEXUS_MemoryBlock_UnlockOffset((NEXUS_MemoryBlockHandle)handle);
         return NULL;
      }
   }
   else
   {
      res = handle;
      *offset = (uintptr_t)handle;
   }
   return res;
}

static void MemUnlock(void *context, void *handle)
{
   NXPL_MemoryData   *data = (NXPL_MemoryData*)context;

   if (data->useMMA)
   {
      NEXUS_MemoryBlock_UnlockOffset((NEXUS_MemoryBlockHandle)handle);
      NEXUS_MemoryBlock_Unlock((NEXUS_MemoryBlockHandle)handle);
   }
   return;
}

/* Not a complete implementation, just enough to match what nx_ashmem is doing */
static unsigned long long memparse(const char *ptr, char **retptr)
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

   return ret;
}

__attribute__((visibility("default")))
BEGL_MemoryInterface *NXPL_CreateMemInterface(BEGL_HWInterface *hwIface)
{
   NXPL_MemoryData *data;
   BEGL_MemoryInterface *mem = (BEGL_MemoryInterface*)malloc(sizeof(BEGL_MemoryInterface));

#ifdef LOG_MEMORY_PATTERN
   sLogFile = fopen("MemoryLog.txt", "w");
#endif

   if (mem != NULL)
   {
      data = (NXPL_MemoryData*)malloc(sizeof(NXPL_MemoryData));
      memset(mem, 0, sizeof(BEGL_MemoryInterface));

      if (data != NULL)
      {
#ifdef ANDROID
         char val[PROPERTY_VALUE_MAX];
#else
         char *val;
#endif
#if NEXUS_HAS_GRAPHICS2D
         NEXUS_Graphics2DSettings gfxSettings;
#endif
#ifndef SINGLE_PROCESS
         NEXUS_ClientConfiguration clientConfig;
#endif
         NEXUS_MemoryStatus memStatus;

         memset(data, 0, sizeof(NXPL_MemoryData));

#ifdef ANDROID
         /* default off */
         property_get(NX_MMA, val, "0");
#else
         val = getenv(NX_MMA);
#endif
         data->useMMA = false;
         if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
            data->useMMA = true;

#ifdef ANDROID
         property_get(NX_HEAP_GROW, val, "0");
#else
         val = getenv(NX_HEAP_GROW);
#endif
         data->heapGrow = BLOCK_SIZE;
         if (val)
         {
            data->heapGrow = memparse(val, NULL);
            /* memparse could return zero */
            if (data->heapGrow == 0)
               data->heapGrow = BLOCK_SIZE;
         }

         mem->context = (void*)data;
         mem->Alloc = MemAlloc;
         mem->Free = MemFree;
         mem->FlushCache = MemFlushCache;
         if (!data->useMMA)
         {
            /* this is only valid when NEXUS maps entire heap */
            mem->ConvertCachedToPhysical = MemConvertCachedToPhysical;
            mem->ConvertPhysicalToCached = MemConvertPhysicalToCached;
         }
         mem->GetInfo = MemGetInfo;
         mem->MemLock = MemLock;
         mem->MemUnlock = MemUnlock;
#if NEXUS_HAS_GRAPHICS2D
         mem->MemCopy2d = MemCopy2d;
#endif

#ifndef SINGLE_PROCESS
         NEXUS_Platform_GetClientConfiguration(&clientConfig);

         if (data->useMMA)
            data->heapMap.heap = clientConfig.heap[4];
         else if (clientConfig.mode == NEXUS_ClientMode_eUntrusted)
            data->heapMap.heap = clientConfig.heap[0];
         else
            data->heapMap.heap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);
#else
         /* If you change this, then the heap must also change in nexus_platform.c
            With refsw NEXUS_OFFSCREEN_SURFACE is the only heap guarenteed to be valid for v3d to use */
         data->heapMap.heap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);
#endif

         NEXUS_Heap_GetStatus(data->heapMap.heap, &memStatus);
         data->heapMap.heapStartCached = memStatus.addr;
         data->heapMap.heapStartPhys = memStatus.offset;
         data->heapMap.heapSize = memStatus.size;
         data->l2CacheSize = memStatus.alignment;

#ifndef NDEBUG
         printf("NXPL : NXPL_CreateMemInterface() INFO.\nVirtual (cached) %p, Physical %p, Size %p, Alignment %d\n",
            data->heapMap.heapStartCached,
            data->heapMap.heapStartPhys,
            data->heapMap.heapSize,
            data->l2CacheSize);
#endif

#if NEXUS_HAS_GRAPHICS2D
         BKNI_CreateEvent(&data->checkpointEvent);
         BKNI_CreateEvent(&data->packetSpaceAvailableEvent);

#ifndef NDEBUG
         printf("NXPL : Open DMA %d\n", NEXUS_NUM_2D_ENGINES - 1);
#endif
         data->gfx = NEXUS_Graphics2D_Open(NEXUS_NUM_2D_ENGINES - 1, NULL);
         NEXUS_Graphics2D_GetSettings(data->gfx, &gfxSettings);
         gfxSettings.checkpointCallback.callback = completeCallback;
         gfxSettings.checkpointCallback.context = data->checkpointEvent;
         gfxSettings.packetSpaceAvailable.callback = completeCallback;
         gfxSettings.packetSpaceAvailable.context = data->packetSpaceAvailableEvent;
         NEXUS_Graphics2D_SetSettings(data->gfx, &gfxSettings);

         /* created as a 2k x strip height with 16bpp as a temp scratch space */
         data->yuvScratch = NEXUS_MemoryBlock_Allocate(data->heapMap.heap,
            2048 * STRIP_HEIGHT * 2, 4096/*alignment*/, NULL);
         NEXUS_MemoryBlock_LockOffset(data->yuvScratch, &data->yuvScratchPhys);
#endif
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

__attribute__((visibility("default")))
void NXPL_DestroyMemInterface(BEGL_MemoryInterface *mem)
{
   if (mem != NULL)
   {
      if (mem->context != NULL)
      {
         NXPL_MemoryData *data = (NXPL_MemoryData*)mem->context;

#if NEXUS_HAS_GRAPHICS2D
         if (data->checkpointEvent)
            BKNI_DestroyEvent(data->checkpointEvent);
         if (data->packetSpaceAvailableEvent)
            BKNI_DestroyEvent(data->packetSpaceAvailableEvent);

         if (data->gfx)
            NEXUS_Graphics2D_Close(data->gfx);

         if (data->yuvScratchPhys)
            NEXUS_MemoryBlock_UnlockOffset(data->yuvScratch);
         if (data->yuvScratch)
            NEXUS_MemoryBlock_Free(data->yuvScratch);
#endif

         free(mem->context);
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

/* Return the primary memory heap */
__attribute__((visibility("default")))
NEXUS_HeapHandle NXPL_MemHeap(BEGL_MemoryInterface *mem)
{
   if (mem != NULL)
   {
      NXPL_MemoryData *data = (NXPL_MemoryData*)mem->context;
      return data->heapMap.heap;
   }
   return NULL;
}
