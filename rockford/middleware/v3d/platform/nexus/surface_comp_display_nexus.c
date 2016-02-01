/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
DESC
=============================================================================*/

#include "default_nexus.h"
#include "display_nexus.h"

#include "nexus_platform.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define NX_MMA                   "V3D_USE_MMA"

#define NUM_SWAP_BUFFERS 3
#define MAX_QUEUE_SIZE       16  /* needs to be big enough to accomodate MAX_SWAP_BUFFERS + the additional fences */

typedef struct {
   int      queue[MAX_QUEUE_SIZE + 1];
   int      head;
   int      tail;
   int      count;
   sem_t    lock;
} NXPL_FenceQueue;

typedef struct
{
   NXPL_NativeWindowInfo      windowAttr;
   NXPL_NativeWindowInfo      installedWindowAttr;
   uint32_t                   clientID;
   NEXUS_SurfaceClientHandle  surfaceClient;
   sem_t                      numBuffers;
   bool                       hasBeenDisplayed;
#ifdef NXCLIENT_SUPPORT
   NxClient_AllocResults      allocResults;
#endif
   NXPL_FenceQueue            *waitQueue;
   NXPL_FenceQueue            *signalQueue;
} NXPL_SurfCompNativeWindow;

/* Context data for the entire display */
typedef struct
{
   BEGL_DisplayCallbacks      displayCallbacks;
   NEXUS_HEAPHANDLE           heap;
   NXPL_DisplayType           displayType;
   NXPL_DisplayType           lastDisplayType;
   BEGL_MemoryInterface       *memInterface;
   BEGL_HWInterface           *hwInterface;
   pthread_mutex_t            mutex;
   bool                       useMMA;
} NXPL_DisplayData;

/*****************************************************************************/

static void InitQueue(NXPL_FenceQueue **q)
{
   if (q != NULL)
   {
      NXPL_FenceQueue * ret = (NXPL_FenceQueue *)malloc(sizeof(NXPL_FenceQueue));

      if (ret != NULL)
      {
         ret->head = 0;
         ret->tail = MAX_QUEUE_SIZE - 1;
         ret->count = 0;
         sem_init(&ret->lock, 0, 1);
         *q = ret;
      }
   }
}

static void TermQueue(NXPL_FenceQueue *q)
{
   if (q != NULL)
   {
      sem_destroy(&q->lock);
      free(q);
   }
}

static void PushQueue(NXPL_FenceQueue *q, int x)
{
   if (q != NULL)
   {
      sem_wait(&q->lock);

      if (q->count >= MAX_QUEUE_SIZE)
         printf("NXPL : Overflow x=%d\n", x);
      else
      {
         /* cause a wrap */
         q->tail = (q->tail + 1) % MAX_QUEUE_SIZE;
         q->queue[q->tail] = x;
         q->count = q->count + 1;
      }

      sem_post(&q->lock);
   }
}

static int PopQueue(NXPL_FenceQueue *q)
{
   int x;

   sem_wait(&q->lock);

   if (q->count <= 0)
      printf("NXPL : Underflow\n");
   else
   {
      x = q->queue[q->head];
      q->head = (q->head + 1) % MAX_QUEUE_SIZE;
      q->count = q->count - 1;
   }

   sem_post(&q->lock);

   return(x);
}

static bool QueueEmpty(NXPL_FenceQueue *q)
{
   bool res;

   sem_wait(&q->lock);
   res = (q->count == 0);
   sem_post(&q->lock);

   return res;
}

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/
static void RecycleCallback(void *context, int param)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)param;
   unsigned          n = 0, j;

   /*printf("RecycleCallback\n");*/

   do
   {
      NEXUS_SurfaceHandle surface_list[NUM_SWAP_BUFFERS];
      int rc = NEXUS_SurfaceClient_RecycleSurface(nw->surfaceClient, surface_list, NUM_SWAP_BUFFERS, &n);
      if (rc)
         return;

      for (j = 0; j < n; j++)
      {
         int fence = PopQueue(nw->signalQueue);
         if (fence != -1)
            data->hwInterface->FenceSignal(fence);
      }
   }
   while (n >= NUM_SWAP_BUFFERS);   /* Loop again if we got all our buffers recycled, in case more are waiting */
}

static BEGL_Error DispBufferDisplay(void *context, BEGL_BufferDisplayState *state)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_BufferData *buffer = (NXPL_BufferData*)state->buffer;
   NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)state->windowState.window;
   NEXUS_Error err;

   /*printf("DispBufferDisplay\n");*/

   if (!nw->hasBeenDisplayed ||
       data->displayType != data->lastDisplayType)
   {
      /* Window settings have changed */
      NEXUS_SurfaceClientSettings clientSettings;

      nw->hasBeenDisplayed = true;

      /* setup the display & callback */
      NEXUS_SurfaceClient_GetSettings(nw->surfaceClient, &clientSettings);
      /* client does not control position */
      clientSettings.recycled.context = context;
      clientSettings.recycled.param = (int)nw;
      clientSettings.recycled.callback = RecycleCallback;

      switch (data->displayType)
      {
      default :
      case NXPL_2D :             clientSettings.orientation = NEXUS_VideoOrientation_e2D; break;
      case NXPL_3D_LEFT_RIGHT :  clientSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight; break;
      case NXPL_3D_OVER_UNDER :  clientSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder; break;
      }
      /*printf("Setting NEXUS_SurfaceClient_SetSettings for %p\n", windowState->surfaceClient);*/
      err = NEXUS_SurfaceClient_SetSettings(nw->surfaceClient, &clientSettings);
      if (err)
        printf("NEXUS_SurfaceClient_SetSettings() failed\n");

      data->lastDisplayType = data->displayType;
   }

   /*printf("%p NEXUS_SurfaceClient_PushSurface %p\n", (uint32_t)pthread_self(), buffer->surface);*/

   err = NEXUS_SurfaceClient_PushSurface(nw->surfaceClient, buffer->surface, NULL, false);
   if (err)
      printf("NEXUS_SurfaceClient_PushSurface() failed\n");

   sem_post(&nw->numBuffers);

   return BEGL_Success;
}

/* Flush pending displays until they are all done, then removes all buffers from display. Will block until complete. */
static BEGL_Error DispWindowUndisplay(void *context, BEGL_WindowState *windowState)
{
   NXPL_DisplayData        *data = (NXPL_DisplayData*)context;
   NXPL_SurfCompNativeWindow   *nw = (NXPL_SurfCompNativeWindow*)windowState->window;

   if (nw)
   {
      if (nw->surfaceClient)
         NEXUS_SurfaceClient_Clear(nw->surfaceClient);

      /* waitqueue should be empty, signalQueue could have stuff in it */
      while (!QueueEmpty(nw->signalQueue))
      {
         int fence = PopQueue(nw->signalQueue);
         data->hwInterface->FenceSignal(fence);
      }
   }

   return BEGL_Success;
}

/* Request creation of an appropriate display buffer. Only the 3D driver knows the size and alignment constraints, so the
 * buffer create request must come from the driver. settings->totalByteSize is the size of the memory that the driver needs.
 * We could have just requested a block of memory using the memory interface, but by having the platform layer create a 'buffer'
 * it can actually create whatever type it desires directly, and then only have to deal with that type. For example, in a Nexus
 * platform layer, this function might be implemented to create a NEXUS_Surface (with the correct memory constraints of course).
 * When the buffer handle is passed out during BufferDisplay, the platform layer can simply use it as a NEXUS_Surface. It
 * doesn't have to wrap the memory each time, or perform any lookups. Since the buffer handle is opaque to the 3d driver, the
 * platform layer has complete freedom. */
static BEGL_BufferHandle DispBufferCreate(void *context, BEGL_BufferSettings *settings)
{
   NXPL_DisplayData              *data = (NXPL_DisplayData*)context;
   NXPL_BufferData               *buffer = NULL;
   NEXUS_SurfaceCreateSettings    surfSettings;
   NEXUS_MemoryAllocationSettings memSettings;
   uint32_t                       bpp;
   void                          *surfaceBacking = NULL;

   buffer = (NXPL_BufferData*)malloc(sizeof(NXPL_BufferData));
   memset(buffer, 0, sizeof(NXPL_BufferData));

   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);

   switch (settings->format)
   {
   case BEGL_BufferFormat_eA8B8G8R8 :
      bpp = 32;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eR8G8B8A8 :
      bpp = 32;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
      break;
   case BEGL_BufferFormat_eX8B8G8R8 :
      bpp = 32;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eX8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eR8G8B8X8 :
      bpp = 32;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_X8;
      break;
   case BEGL_BufferFormat_eR5G6B5 :
   case BEGL_BufferFormat_eR5G6B5_Texture :
      bpp = 16;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eR5_G6_B5;
      break;
   case BEGL_BufferFormat_eYUV422_Texture :
      bpp = 16;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eCr8_Y18_Cb8_Y08;
      break;
   case BEGL_BufferFormat_eVUY224_Texture :
      bpp = 16;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eY08_Cb8_Y18_Cr8;
      break;
   case BEGL_BufferFormat_eA8B8G8R8_Texture :
      bpp = 32;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
      break;
   case BEGL_BufferFormat_eR8G8B8A8_Texture :
      bpp = 32;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
      break;
   }

   surfSettings.width = settings->width;
   surfSettings.height = settings->height;
   surfSettings.alignment = settings->alignment;
   surfSettings.pitch = settings->pitchBytes;
   surfSettings.heap = data->heap;

   NEXUS_Memory_GetDefaultAllocationSettings(&memSettings);
   memSettings.alignment = settings->alignment;
   memSettings.heap = data->heap;

   surfaceBacking = data->memInterface->Alloc(data->memInterface->context, settings->totalByteSize, 4096);
   if (surfaceBacking != NULL)
   {
      if (data->useMMA)
         surfSettings.pixelMemory = surfaceBacking;
      else
         surfSettings.pMemory = surfaceBacking;

      buffer->surface = NEXUS_Surface_Create(&surfSettings);
   }

   if (data->useMMA)
   {
      NEXUS_Addr  devPtr;
      NEXUS_Error err;
      /* driver expects locked and mapped surfaces */
      err = NEXUS_MemoryBlock_LockOffset((NEXUS_MemoryBlockHandle)surfaceBacking, &devPtr);
      if (err == NEXUS_SUCCESS)
      {
         void *res = NULL;
         settings->physOffset = (uint32_t)devPtr;
         err = NEXUS_MemoryBlock_Lock((NEXUS_MemoryBlockHandle)surfaceBacking, &res);
         if (err == NEXUS_SUCCESS)
            settings->cachedAddr = res;
         else
         {
            /* failed, unlock */
            NEXUS_MemoryBlock_UnlockOffset((NEXUS_MemoryBlockHandle)surfaceBacking);
            settings->cachedAddr = NULL;
            settings->physOffset = 0;
         }
      }
      else
         settings->physOffset = 0;
   }
   else
   {
      data->memInterface->ConvertCachedToPhysical(data->memInterface->context, surfSettings.pMemory);
      settings->cachedAddr = surfSettings.pMemory;
   }

   buffer->settings = *settings;

   /*printf("DispBufferCreate %x\n", buffer->surface);*/

   return (BEGL_BufferHandle)buffer;
}

/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context, BEGL_BufferDisplayState *bufferState)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_BufferData *buffer = (NXPL_BufferData*)bufferState->buffer;

   /*printf("DispBufferDestroy %x\n", buffer->surface);*/

   if (buffer != NULL)
   {
      if (buffer->surface)
      {
         NEXUS_SurfaceCreateSettings createSettings;
         void *surfaceBacking;
         NEXUS_Surface_GetCreateSettings(buffer->surface, &createSettings);
         NEXUS_Surface_Destroy(buffer->surface);

         if (data->useMMA)
            surfaceBacking = createSettings.pixelMemory;
         else
            surfaceBacking = createSettings.pMemory;

         if (surfaceBacking != NULL)
            data->memInterface->Free(data->memInterface->context, surfaceBacking);
      }

      memset(buffer, 0, sizeof(NXPL_BufferData));
      free(buffer);
   }

   return BEGL_Success;
}

static void setNxClientRect(NXPL_SurfCompNativeWindow *nw)
{
#ifdef NXCLIENT_SUPPORT
   NEXUS_SurfaceComposition   comp;

   NxClient_GetSurfaceClientComposition(nw->allocResults.surfaceClient[0].id, &comp);

   /* TODO set comp.zorder to control z-order (0 = bottom layer) */
   if (!nw->windowAttr.stretch)
   {
      comp.virtualDisplay.width  = 0;
      comp.virtualDisplay.height = 0;
      comp.position.width  = nw->windowAttr.width;
      comp.position.height = nw->windowAttr.height;
   }
   else
   {
      comp.virtualDisplay.width  = nw->windowAttr.width;
      comp.virtualDisplay.height = nw->windowAttr.height;
      comp.position.width  = nw->windowAttr.width - (2 * nw->windowAttr.x);
      comp.position.height = nw->windowAttr.height - (2 * nw->windowAttr.y);
   }
   comp.position.x = nw->windowAttr.x;
   comp.position.y = nw->windowAttr.y;
   comp.zorder = nw->windowAttr.zOrder;

   /* update the programmed position - structure copy */
   nw->installedWindowAttr = nw->windowAttr;

   NxClient_SetSurfaceClientComposition(nw->allocResults.surfaceClient[0].id, &comp);
#endif
}

/* Wraps our swap chain surfaces */
static BEGL_BufferHandle DispBufferGet(void *context, BEGL_BufferSettings *settings)
{
   NXPL_DisplayData              *data = (NXPL_DisplayData*)context;
   NXPL_BufferData               *buffer = NULL;
   NXPL_SurfCompNativeWindow     *nw = (NXPL_SurfCompNativeWindow*)settings->windowState.window;

   buffer = (NXPL_BufferData*)DispBufferCreate(context, settings);
   if (buffer)
      setNxClientRect(nw);

   return (BEGL_BufferHandle)buffer;
}

/* Get information about a created window buffer */
static BEGL_Error DispBufferGetCreateSettings(void *context, BEGL_BufferHandle bufHandle, BEGL_BufferSettings *settings)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_BufferData *buffer = (NXPL_BufferData*)bufHandle;

   if (buffer != NULL)
   {
      *settings = buffer->settings;
      return BEGL_Success;
   }

   return BEGL_Fail;
}

/* Called to determine current size of the window referenced by the opaque window handle.
 * This is needed by EGL in order to know the size of a native 'window'. */
static BEGL_Error DispWindowGetInfo(void *context,
                                    BEGL_WindowHandle window,
                                    BEGL_WindowInfoFlags flags,
                                    BEGL_WindowInfo *info)
{
   NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)window;

   if (nw != NULL)
   {
      NEXUS_SurfaceClientStatus clientStatus;

      if (flags & BEGL_WindowInfoWidth || flags & BEGL_WindowInfoHeight)
      {
#ifndef NXCLIENT_SUPPORT
         if (nw->surfaceClient != NULL)
         {
            /* THIS IS CALLED ONCE AT START OF FRAME, MAKE SURE YOU DON'T CALL
               NEXUS_SurfaceClient_GetStatus() AGAIN.  NEXUS CHANGES THIS ASYNCHRONOUSLY! */
            NEXUS_SurfaceClient_GetStatus(nw->surfaceClient, &clientStatus);
            nw->windowAttr.width = info->width = clientStatus.position.width;
            nw->windowAttr.height = info->height = clientStatus.position.height;
         }
         else
#endif
         {
            info->width = nw->windowAttr.width;
            info->height = nw->windowAttr.height;
         }
      }
      info->swapchain_count = NUM_SWAP_BUFFERS; /* We implement triple buffering for the surface compositor, in this very file  */

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_Error DispBufferAccess(void *context, BEGL_BufferAccessState *bufferAccess)
{
   NXPL_DisplayData  *data = (NXPL_DisplayData*)context;

   /*printf("DispBufferAccess\n");*/

   if (data)
   {
      /* Not interested in actual lock/unlock functionality, but we will use lock to wait for the next available buffer */
      NXPL_BufferData                  *buffer = bufferAccess->buffer;
      NXPL_SurfCompNativeWindow        *nw = (NXPL_SurfCompNativeWindow*)bufferAccess->windowState.window;
      NEXUS_SurfaceCreateSettings      createSettings;
      NEXUS_SurfaceClientStatus        clientStatus;
      unsigned int                     res;
      BEGL_BufferDisplayState          bufferState;
      BEGL_BufferSettings              bufferSettings;
      BEGL_PixmapInfo                  info;
      NEXUS_MemoryAllocationSettings   memSettings;
      int                              newFence;
      void                             *surfaceBacking = NULL;

      sem_wait(&nw->numBuffers);

      data->hwInterface->FenceOpen(data->hwInterface->context, &newFence);

      PushQueue(nw->waitQueue, newFence);
      PushQueue(nw->signalQueue, newFence);

      /* not the same as the value just pushed above.  Adds latency */
      bufferAccess->fence = PopQueue(nw->waitQueue);

      /* The buffer is now locked for our use - check that surface compositor hasn't resized its window.
         If it has, we need to resize the underlying surface. */
      if (nw != NULL && nw->surfaceClient != NULL)
      {
         bufferSettings = buffer->settings;

         if (nw->windowAttr.width != bufferSettings.width || nw->windowAttr.height != bufferSettings.height)
         {
            /*printf("DispBufferAccess resizing nw=%dx%d, bufferSettings=%dx%d\n", nw->windowAttr.width, nw->windowAttr.height, bufferSettings.width, bufferSettings.height);*/

            bufferSettings.width = nw->windowAttr.width;
            bufferSettings.height = nw->windowAttr.height;

            /* Delete the old buffer surface */
            if (buffer->surface)
            {
               NEXUS_Surface_GetCreateSettings(buffer->surface, &createSettings);
               NEXUS_Surface_Destroy(buffer->surface);
               buffer->surface = NULL;

               if (data->useMMA)
                  surfaceBacking = createSettings.pixelMemory;
               else
                  surfaceBacking = createSettings.pMemory;

               if (surfaceBacking != NULL)
                  data->memInterface->Free(data->memInterface->context, surfaceBacking);
            }

            info.format = bufferSettings.format;
            info.width = bufferSettings.width;
            info.height = bufferSettings.height;
            data->displayCallbacks.BufferGetRequirements(&info, &bufferSettings);

            /* Make a new surface */
            createSettings.width = bufferSettings.width;
            createSettings.height = bufferSettings.height;
            createSettings.pitch = bufferSettings.pitchBytes;
            createSettings.alignment = bufferSettings.alignment;
            createSettings.heap = data->heap;

            surfaceBacking = data->memInterface->Alloc(data->memInterface->context, bufferSettings.totalByteSize, 4096);
            if (surfaceBacking != NULL)
            {
               if (data->useMMA)
                  createSettings.pixelMemory = surfaceBacking;
               else
                  createSettings.pMemory = surfaceBacking;
               buffer->surface = NEXUS_Surface_Create(&createSettings);
            }

            if (data->useMMA)
            {
               NEXUS_Addr  devPtr;
               NEXUS_Error err;
               /* driver expects locked and mapped surfaces */
               err = NEXUS_MemoryBlock_LockOffset((NEXUS_MemoryBlockHandle)surfaceBacking, &devPtr);
               if (err == NEXUS_SUCCESS)
               {
                  void *res = NULL;
                  bufferSettings.physOffset = (uint32_t)devPtr;
                  err = NEXUS_MemoryBlock_Lock((NEXUS_MemoryBlockHandle)surfaceBacking, &res);
                  if (err == NEXUS_SUCCESS)
                     bufferSettings.cachedAddr = res;
                  else
                  {
                     /* failed, unlock */
                     NEXUS_MemoryBlock_UnlockOffset((NEXUS_MemoryBlockHandle)surfaceBacking);
                     bufferSettings.cachedAddr = NULL;
                     bufferSettings.physOffset = 0;
                  }
               }
               else
                  bufferSettings.physOffset = 0;
            }
            else
            {
               bufferSettings.physOffset = data->memInterface->ConvertCachedToPhysical(data->memInterface->context, createSettings.pMemory);
               bufferSettings.cachedAddr = createSettings.pMemory;
            }

            buffer->settings = bufferSettings;
         }

         if ((nw->windowAttr.width != nw->installedWindowAttr.width) ||
             (nw->windowAttr.height != nw->installedWindowAttr.height) ||
             (nw->windowAttr.x != nw->installedWindowAttr.x) ||
             (nw->windowAttr.y != nw->installedWindowAttr.y) ||
             (nw->windowAttr.stretch != nw->installedWindowAttr.stretch) ||
             (nw->windowAttr.clientID != nw->installedWindowAttr.clientID) ||
             (nw->windowAttr.zOrder != nw->installedWindowAttr.zOrder))
         {
            setNxClientRect(nw);
         }
      }

      if (nw != NULL)
      {
         buffer->settings.width = nw->windowAttr.width;
         buffer->settings.height = nw->windowAttr.height;
      }

      return BEGL_Success;
   }

   return BEGL_Fail;
}

BEGL_DisplayInterface *NXPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
                                                   BEGL_HWInterface     *hwIface,
                                                   BEGL_DisplayCallbacks *displayCallbacks)
{
   NXPL_DisplayData *data;
   BEGL_DisplayInterface *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));

   if (disp != NULL)
   {
      data = (NXPL_DisplayData*)malloc(sizeof(NXPL_DisplayData));
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      if (data != NULL)
      {
         NEXUS_MemoryStatus            memStatus;
         int topTwoBitsHeapStart;
         int topTwoBitsHeapEnd;
         char * val;

         memset(data, 0, sizeof(NXPL_DisplayData));

         val = getenv(NX_MMA);
         data->useMMA = false;
         if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
            data->useMMA = true;

         /* create the mutex to stop the buffer being locked twice */
         pthread_mutex_init(&data->mutex, NULL);

         disp->context = (void*)data;
         disp->BufferDisplay = DispBufferDisplay;
         disp->BufferCreate = DispBufferCreate;
         disp->BufferGet = DispBufferGet;
         disp->BufferDestroy = DispBufferDestroy;
         disp->BufferGetCreateSettings = DispBufferGetCreateSettings;
         disp->BufferAccess = DispBufferAccess;
         disp->WindowGetInfo = DispWindowGetInfo;
         disp->WindowUndisplay = DispWindowUndisplay;

         data->memInterface = memIface;
         data->hwInterface = hwIface;
         data->displayCallbacks = *displayCallbacks;

         /* Store the heap settings */
         data->heap = NXPL_MemHeap(memIface);

         if (!data->useMMA)
         {
            NEXUS_Heap_GetStatus(data->heap, &memStatus);
            topTwoBitsHeapStart = memStatus.offset >> 30;
            topTwoBitsHeapEnd = (memStatus.offset + memStatus.size - 1) >> 30;
            if (topTwoBitsHeapStart != topTwoBitsHeapEnd)
            {
               printf("\n\nNXPL : NXPL_CreateDisplayInterface() ERROR.\nThe Heap you have selected in your platform layer straddles a 1GB boundary\n"
                      "Start 0x%llX, Size %p\n", memStatus.offset, memStatus.size);
               goto error1;
            }
         }
      }
      else
      {
         goto error0;
      }
   }
   return disp;

error1:
   free(data);

error0:
   free(disp);
   return NULL;
}

NEXUS_SurfaceClientHandle NXPL_CreateVideoWindowClient(void *native, unsigned windowId)
{
   NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)native;
   NEXUS_SurfaceClientHandle ret = NULL;

   if (nw && nw->surfaceClient)
      ret = NEXUS_SurfaceClient_AcquireVideoWindow(nw->surfaceClient, windowId);

   return ret;
}

void NXPL_ReleaseVideoWindowClient(NEXUS_SurfaceClientHandle handle)
{
   NEXUS_SurfaceClient_ReleaseVideoWindow(handle);
}

uint32_t NXPL_GetClientID (void *native)
{
   uint32_t ret = 0;

   NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)native;

   if (nw)
      ret = nw->clientID;

   return ret;
}

void NXPL_SetDisplayType(NXPL_PlatformHandle handle, NXPL_DisplayType type)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL && data->displayInterface != NULL && data->displayInterface->context != NULL)
   {
      NXPL_DisplayData *dd = (NXPL_DisplayData*)data->displayInterface->context;
      dd->displayType = type;
   }
}

void NXPL_DestroyDisplayInterface(BEGL_DisplayInterface *mem)
{
   if (mem != NULL)
   {
      if (mem->context != NULL)
      {
         NXPL_DisplayData *data = (NXPL_DisplayData*)mem->context;
         pthread_mutex_destroy(&data->mutex);
         free(mem->context);
      }

      memset(mem, 0, sizeof(BEGL_DisplayInterface));
      free(mem);
   }
}

bool NXPL_BufferGetRequirements(NXPL_PlatformHandle handle,
                                BEGL_PixmapInfo *bufferRequirements,
                                BEGL_BufferSettings * bufferConstrainedRequirements)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL && data->displayCallbacks.BufferGetRequirements != NULL)
   {
      data->displayCallbacks.BufferGetRequirements(bufferRequirements, bufferConstrainedRequirements);
      return true;
   }

   return false;
}

bool NXPL_CreateCompatiblePixmap(NXPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SURFACEHANDLE *surface, BEGL_PixmapInfo *info)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL && data->displayCallbacks.PixmapCreateCompatiblePixmap != NULL)
   {
      BEGL_BufferHandle buffer = data->displayCallbacks.PixmapCreateCompatiblePixmap(info);
      if (buffer != NULL)
      {
         *pixmapHandle = (void*)buffer;
         *surface = ((NXPL_BufferData*)buffer)->surface;
         return true;
      }
   }

   return false;
}

void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle handle, void *pixmapHandle)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL && data->displayInterface != NULL)
   {
      BEGL_BufferDisplayState bufferState;
      memset(&bufferState, 0, sizeof(BEGL_BufferDisplayState));
      bufferState.buffer = (BEGL_BufferHandle)pixmapHandle;

      data->displayInterface->BufferDestroy(data->displayInterface->context, &bufferState);
   }
}

void *NXPL_CreateNativeWindow(const NXPL_NativeWindowInfo *info)
{
   NXPL_SurfCompNativeWindow *nw;

   if (info == NULL)
      return NULL;

   nw = (NXPL_SurfCompNativeWindow*)malloc(sizeof(NXPL_SurfCompNativeWindow));

   if (nw != NULL)
   {
#ifdef NXCLIENT_SUPPORT
      int rc;
      NxClient_AllocSettings allocSettings;
#endif
      int i;
      memset(nw, 0, sizeof(NXPL_SurfCompNativeWindow));

      InitQueue(&nw->waitQueue);
      InitQueue(&nw->signalQueue);

      /* add additional semaphore to run further ahead without blocking */
      sem_init(&nw->numBuffers, 0, NUM_SWAP_BUFFERS);
      for (i = 0; i < NUM_SWAP_BUFFERS; i++)
         PushQueue(nw->waitQueue, -1);

      /* structure copy - needs to be before assignment of nw->clientID */
      nw->windowAttr = *info;

#ifdef NXCLIENT_SUPPORT
      NxClient_GetDefaultAllocSettings(&allocSettings);
      allocSettings.surfaceClient = 1;
      rc = NxClient_Alloc(&allocSettings, &nw->allocResults);
      if (rc) {
          rc = BERR_TRACE(rc);
          return NULL;
      }

      /* Attach the surface client for this swapChain. There is one swapChain per native window, so
         we will have one client per native window. */
      nw->clientID = nw->allocResults.surfaceClient[0].id;
#else
      nw->clientID = info->clientID;
#endif

      nw->surfaceClient = NEXUS_SurfaceClient_Acquire(nw->clientID);

      if (!nw->surfaceClient)
      {
         printf("Failed to acquire compositing client surface for client id %d", nw->clientID);
         return NULL;
      }

#ifdef NXCLIENT_SUPPORT
      /* default our SurfaceClient to blend */
      {
          NEXUS_SurfaceComposition comp;
          static NEXUS_BlendEquation colorBlend = { NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eSourceAlpha, false, NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eInverseSourceAlpha, false, NEXUS_BlendFactor_eZero };
          static NEXUS_BlendEquation alphaBlend = { NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eDestinationAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero };
          NxClient_GetSurfaceClientComposition(nw->clientID, &comp);
          comp.colorBlend = colorBlend;
          comp.alphaBlend = alphaBlend;
          NxClient_SetSurfaceClientComposition(nw->clientID, &comp);
      }
#endif
   }

   return nw;
}

void NXPL_DestroyNativeWindow(void *nativeWin)
{
   if (nativeWin != NULL)
   {
      NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)nativeWin;

      if (nw->surfaceClient)
         NEXUS_SurfaceClient_Release(nw->surfaceClient);

#ifdef NXCLIENT_SUPPORT
      NxClient_Free(&nw->allocResults);
#endif

      sem_destroy(&nw->numBuffers);

      TermQueue(nw->waitQueue);
      TermQueue(nw->signalQueue);

      memset(nw, 0, sizeof(NXPL_SurfCompNativeWindow));
      free(nw);
   }
}

void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info)
{
   NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)native;

   /*printf("NXPL_UpdateNativeWindow %dx%d\n", info->width, info->height);*/
   if (info != NULL && nw != NULL)
      /* structure copy */
      nw->windowAttr = *info;
}
