/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "default_nexus.h"
#include "display_nexus.h"

#include "nexus_platform.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include <EGL/egl.h>
#include <EGL/eglext_brcm.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>

#define NX_MMA                   "V3D_USE_MMA"

#define NUM_SWAP_BUFFERS 3
#define MAX_QUEUE_SIZE       16  /* needs to be big enough to accomodate MAX_SWAP_BUFFERS + the additional fences */

enum
{
   NATIVE_WINDOW_INFO_MAGIC = 0xABBA601D,
   PIXMAP_INFO_MAGIC = 0x15EEB1A5
};

typedef struct {
   int      queue[MAX_QUEUE_SIZE + 1];
   int      head;
   int      tail;
   int      count;
   sem_t    lock;
} NXPL_FenceQueue;

typedef struct
{
   NXPL_NativeWindowInfoEXT      windowAttr;
   NXPL_NativeWindowInfoEXT      installedWindowAttr;
   uint32_t                      clientID;
   NEXUS_SurfaceClientHandle     surfaceClient;
#ifdef NXCLIENT_SUPPORT
   NxClient_AllocResults         allocResults;
#endif
} NXPL_SurfCompNativeWindow;

/* There will be one NXPL_WindowState for each NXPL_SurfCompNativeWindow */
typedef struct
{
   sem_t                      numBuffers;
   bool                       hasBeenDisplayed;
   NXPL_FenceQueue            *waitQueue;
   NXPL_FenceQueue            *signalQueue;
} NXPL_WindowState;

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

/* this version is not interruptible via signal */
static int sem_wait_infinite(sem_t *sem)
{
   int res;
   while ((res = sem_wait(sem)) == -1 && errno == EINTR)
      continue;
   return res;
}

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
      sem_wait_infinite(&q->lock);

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

   sem_wait_infinite(&q->lock);

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

   sem_wait_infinite(&q->lock);
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
   BEGL_BufferDisplayState *state = (BEGL_BufferDisplayState *)param;
   NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)state->windowState.window;
   NXPL_WindowState *windowState = (NXPL_WindowState *)state->windowState.platformState;
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
         int fence = PopQueue(windowState->signalQueue);
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
   NXPL_WindowState *windowState = (NXPL_WindowState *)state->windowState.platformState;
   NEXUS_Error err;

   /*printf("DispBufferDisplay\n");*/

   if (!windowState->hasBeenDisplayed ||
       data->displayType != data->lastDisplayType)
   {
      /* Window settings have changed */
      NEXUS_SurfaceClientSettings clientSettings;

      windowState->hasBeenDisplayed = true;

      /* setup the display & callback */
      NEXUS_SurfaceClient_GetSettings(nw->surfaceClient, &clientSettings);
      /* client does not control position */
      clientSettings.recycled.context = context;
      clientSettings.recycled.param = (int)state;
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

   sem_post(&windowState->numBuffers);

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
   bool                           secure;

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

   secure = !!(settings->secure);

   if (secure && !data->useMMA)
   {
      printf("secure mode only operates under MMA - export V3D_USE_MMA=1 and retry\n");
      exit(0);
   }

   surfaceBacking = data->memInterface->Alloc(data->memInterface->context, settings->totalByteSize, 4096, settings->secure);
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

         if (!secure)
         {
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
            settings->cachedAddr = NULL;
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
   comp.colorBlend = nw->windowAttr.colorBlend;
   comp.alphaBlend = nw->windowAttr.alphaBlend;

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
static BEGL_Error DispWindowGetInfo(void *context __attribute__ ((unused)),
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
      NXPL_WindowState                 *windowState = (NXPL_WindowState *)bufferAccess->windowState.platformState;
      NEXUS_SurfaceCreateSettings      createSettings;
      NEXUS_SurfaceClientStatus        clientStatus;
      unsigned int                     res;
      BEGL_BufferDisplayState          bufferState;
      BEGL_BufferSettings              bufferSettings;
      BEGL_PixmapInfoEXT               info = { 0 };
      NEXUS_MemoryAllocationSettings   memSettings;
      int                              newFence;
      void                             *surfaceBacking = NULL;

      NXPL_GetDefaultPixmapInfoEXT(&info);

      if (windowState == NULL || nw == NULL)
         return BEGL_Fail;

      sem_wait_infinite(&windowState->numBuffers);

      data->hwInterface->FenceOpen(data->hwInterface->context, &newFence);

      PushQueue(windowState->waitQueue, newFence);
      PushQueue(windowState->signalQueue, newFence);

      /* not the same as the value just pushed above.  Adds latency */
      bufferAccess->fence = PopQueue(windowState->waitQueue);

      /* The buffer is now locked for our use - check that surface compositor hasn't resized its window.
         If it has, we need to resize the underlying surface. */
      if (nw->surfaceClient != NULL)
      {
         bufferSettings = buffer->settings;

         if (nw->windowAttr.width != bufferSettings.width || nw->windowAttr.height != bufferSettings.height)
         {
            bool secure;
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
            info.secure = bufferSettings.secure;
            info.openvg = bufferSettings.openvg;
            info.colorFormat = bufferSettings.colorFormat;
            data->displayCallbacks.BufferGetRequirements(&info, &bufferSettings);

            /* Make a new surface */
            createSettings.width = bufferSettings.width;
            createSettings.height = bufferSettings.height;
            createSettings.pitch = bufferSettings.pitchBytes;
            createSettings.alignment = bufferSettings.alignment;
            createSettings.heap = data->heap;

            secure = !!(bufferSettings.secure);

            surfaceBacking = data->memInterface->Alloc(data->memInterface->context, bufferSettings.totalByteSize, 4096, bufferSettings.secure);
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

                  if (!secure)
                  {
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
                     bufferSettings.cachedAddr = NULL;
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

         /* if any of the presenation settings have changed, copy the windowInfo into the surface */
         int windowTest = memcmp(&nw->windowAttr, &nw->installedWindowAttr, sizeof(NXPL_NativeWindowInfoEXT));
         if (windowTest)
            setNxClientRect(nw);
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

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window __attribute__ ((unused)))
{
   NXPL_WindowState *windowState = (NXPL_WindowState *)malloc(sizeof(NXPL_WindowState));
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;

   if (windowState)
   {
      int i;
      memset(windowState, 0, sizeof(NXPL_WindowState));

      InitQueue(&windowState->waitQueue);
      InitQueue(&windowState->signalQueue);

      /* add additional semaphore to run further ahead without blocking */
      sem_init(&windowState->numBuffers, 0, NUM_SWAP_BUFFERS);

      for (i = 0; i < NUM_SWAP_BUFFERS; i++)
         PushQueue(windowState->waitQueue, -1);
      PushQueue(windowState->signalQueue, -1);
   }

   return (void*)windowState;
}

static BEGL_Error DispWindowStateDestroy(void *context, void *swapChainCtx)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_WindowState *windowState = (NXPL_WindowState *)swapChainCtx;
   uint32_t          i;

   if (windowState)
   {
      /* waitqueue should be empty, signalQueue could have stuff in it */
      while (!QueueEmpty(windowState->signalQueue))
      {
         int fence = PopQueue(windowState->signalQueue);
         data->hwInterface->FenceSignal(fence);
      }

      sem_destroy(&windowState->numBuffers);

      TermQueue(windowState->waitQueue);
      TermQueue(windowState->signalQueue);

      memset(windowState, 0, sizeof(NXPL_WindowState));
      free(windowState);
   }
}

static BEGL_Error DispSurfaceVerifyImageTarget(void *context __attribute__ ((unused)),
   void *buffer __attribute__ ((unused)), uint32_t eglTarget)
{
   return eglTarget == EGL_IMAGE_WRAP_BRCM_BCG ? BEGL_Success : BEGL_Fail;
}

static BEGL_Error DispDecodeNativeFormat(void *context __attribute__ ((unused)),
   void *buffer,
   BEGL_BufferSettings *settings)
{
   if (settings && buffer)
   {
      memset(settings, 0, sizeof(BEGL_BufferSettings));

      EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *native_buffer = (EGL_IMAGE_WRAP_BRCM_BCG_IMAGE_T *)buffer;

      settings->format = native_buffer->format;
      settings->width = native_buffer->width;
      settings->height = native_buffer->height;
      settings->pitchBytes = native_buffer->stride;

      settings->physOffset = native_buffer->offset;
      settings->cachedAddr = native_buffer->storage;

      return BEGL_Success;
   }
   else
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
         disp->WindowPlatformStateCreate = DispWindowStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
         disp->SurfaceVerifyImageTarget = DispSurfaceVerifyImageTarget;
         disp->DecodeNativeFormat = DispDecodeNativeFormat;

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
                                BEGL_PixmapInfoEXT *bufferRequirements,
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

void NXPL_GetDefaultPixmapInfoEXT(BEGL_PixmapInfoEXT *info)
{
   if (info != NULL)
   {
      memset(info, 0, sizeof(BEGL_PixmapInfoEXT));

      info->format = BEGL_BufferFormat_INVALID;
      info->magic = PIXMAP_INFO_MAGIC;
   }
}

bool NXPL_CreateCompatiblePixmapEXT(NXPL_PlatformHandle handle, void **pixmapHandle,
   NEXUS_SURFACEHANDLE *surface, BEGL_PixmapInfoEXT *info)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   assert(info->magic == PIXMAP_INFO_MAGIC);

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

bool NXPL_CreateCompatiblePixmap(NXPL_PlatformHandle handle, void **pixmapHandle,
   NEXUS_SURFACEHANDLE *surface, BEGL_PixmapInfo *info)
{
   BEGL_PixmapInfoEXT   infoEXT;

   NXPL_GetDefaultPixmapInfoEXT(&infoEXT);

   infoEXT.width = info->width;
   infoEXT.height = info->height;
   infoEXT.format = info->format;

   return NXPL_CreateCompatiblePixmapEXT(handle, pixmapHandle, surface, &infoEXT);
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

void NXPL_GetDefaultNativeWindowInfoEXT(NXPL_NativeWindowInfoEXT *info)
{
   if (info != NULL)
   {
      memset(info, 0, sizeof(NXPL_NativeWindowInfoEXT));
      static NEXUS_BLENDEQUATION colorBlend = { NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eSourceAlpha, false, NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eInverseSourceAlpha, false, NEXUS_BlendFactor_eZero };
      static NEXUS_BLENDEQUATION alphaBlend = { NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eDestinationAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero };
      info->colorBlend = colorBlend;
      info->alphaBlend = alphaBlend;
      info->magic = NATIVE_WINDOW_INFO_MAGIC;
   }
}

void *NXPL_CreateNativeWindowEXT(const NXPL_NativeWindowInfoEXT *info)
{
   NXPL_SurfCompNativeWindow *nw;

   if (info == NULL)
      return NULL;

   assert(info->magic == NATIVE_WINDOW_INFO_MAGIC);

   nw = (NXPL_SurfCompNativeWindow*)malloc(sizeof(NXPL_SurfCompNativeWindow));

   if (nw != NULL)
   {
#ifdef NXCLIENT_SUPPORT
      int rc;
      NxClient_AllocSettings allocSettings;
#endif
      memset(nw, 0, sizeof(NXPL_SurfCompNativeWindow));

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
          NxClient_GetSurfaceClientComposition(nw->clientID, &comp);
          comp.colorBlend = nw->windowAttr.colorBlend;
          comp.alphaBlend = nw->windowAttr.alphaBlend;
          NxClient_SetSurfaceClientComposition(nw->clientID, &comp);
      }
#endif
   }

   return nw;
}

void *NXPL_CreateNativeWindow(const NXPL_NativeWindowInfo *info)
{
   /* deprecated API */
   NXPL_NativeWindowInfoEXT infoEXT;
   NXPL_GetDefaultNativeWindowInfoEXT(&infoEXT);

   infoEXT.width = info->width;
   infoEXT.height = info->height;
   infoEXT.x = info->x;
   infoEXT.y = info->y;
   infoEXT.stretch = info->stretch;
   infoEXT.clientID = info->clientID;
   infoEXT.zOrder = info->zOrder;

   return NXPL_CreateNativeWindowEXT(&infoEXT);
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

      memset(nw, 0, sizeof(NXPL_SurfCompNativeWindow));
      free(nw);
   }
}

void NXPL_UpdateNativeWindowEXT(void *native, const NXPL_NativeWindowInfoEXT *info)
{
   NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)native;

   /*printf("NXPL_UpdateNativeWindow %dx%d\n", info->width, info->height);*/
   if (info != NULL && nw != NULL)
      /* structure copy */
      nw->windowAttr = *info;
}

void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info)
{
   /* deprecated API */
   NXPL_SurfCompNativeWindow *nw = (NXPL_SurfCompNativeWindow*)native;

   if (info != NULL && nw != NULL)
   {
      nw->windowAttr.width = info->width;
      nw->windowAttr.height = info->height;
      nw->windowAttr.x = info->x;
      nw->windowAttr.y = info->y;
      nw->windowAttr.stretch = info->stretch;
      nw->windowAttr.clientID = info->clientID;
      nw->windowAttr.zOrder = info->zOrder;
   }
}
