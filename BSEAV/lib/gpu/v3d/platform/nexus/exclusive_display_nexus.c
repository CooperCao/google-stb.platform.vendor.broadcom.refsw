/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "default_nexus.h"
#include "display_nexus.h"

#include "nexus_platform.h"

#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d.h"
#endif

#include <EGL/egl.h>
#include <EGL/eglext_brcm.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <semaphore.h>
#include <errno.h>

#define NX_MMA                   "V3D_USE_MMA"

#define MAX_SWAP_BUFFERS 3
#define MAX_QUEUE_SIZE       16  /* needs to be big enough to accomodate MAX_SWAP_BUFFERS + the additional fences */

enum
{
   NATIVE_WINDOW_INFO_MAGIC = 0xABBA601D,
   PIXMAP_INFO_MAGIC = 0x15EEB1A5
};

typedef void (*BufferGetRequirementsFunc)(BEGL_PixmapInfoEXT *bufferRequirements, BEGL_BufferSettings *bufferConstrainedRequirements);

static uint32_t   windowCount       = 0;

typedef struct {
   int      queue[MAX_QUEUE_SIZE + 1];
   int      head;
   int      tail;
   int      count;
   sem_t    lock;
} NXPL_FenceQueue;

typedef struct
{
   BufferGetRequirementsFunc  bufferGetRequirementsFunc;
   NEXUS_DISPLAYHANDLE        display;
   NEXUS_HEAPHANDLE           heap;
   uint32_t                   winWidth;
   uint32_t                   winHeight;
   uint32_t                   winX;
   uint32_t                   winY;
   bool                       stretch;
   NXPL_DisplayType           displayType;
   BEGL_MemoryInterface       *memInterface;
   BEGL_HWInterface           *hwInterface;
   BKNI_EventHandle           vsyncEvent;
   bool                       useMMA;
} NXPL_DisplayData;

/* There will be one WINPL_WindowState for each WINPL_SurfCompNativeWindow */
typedef struct
{
   uint32_t               actualSwapChainCount;
   sem_t                  throttleSemaphore;

#if NEXUS_HAS_GRAPHICS2D
   /* If the GFD0 is not in our heap, then bridge with M2MC (better than black screen) */
   NEXUS_SURFACEHANDLE    gfdBacking[2];
   uint32_t               gfdCurrentBuffer;
   BKNI_EventHandle       checkpointEvent;
   BKNI_EventHandle       packetSpaceAvailableEvent;
   NEXUS_Graphics2DHandle gfx;
#endif

   NXPL_FenceQueue            *waitQueue;
   NXPL_FenceQueue            *signalQueue;

} NXPL_WindowState;

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
static void vsync_callback(void *context, int param __attribute__ ((unused)))
{
   /* Simply trigger the event */
   BKNI_SetEvent((BKNI_EventHandle)context);
}

static BEGL_Error DispBufferDisplay(void *context, BEGL_BufferDisplayState *state)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_BufferData *buffer = (NXPL_BufferData*)state->buffer;
   NXPL_NativeWindowInfoEXT *nw = (NXPL_NativeWindowInfoEXT*)state->windowState.window;
   NXPL_WindowState *windowState = (NXPL_WindowState *)state->windowState.platformState;
   NEXUS_GraphicsFramebuffer3D fb3d;
   NEXUS_SURFACEHANDLE surface;
   int fence;

   BKNI_ResetEvent(data->vsyncEvent);

   if (nw->width != data->winWidth || nw->height != data->winHeight ||
       nw->x != data->winX || nw->y != data->winY ||
       nw->stretch != data->stretch)
   {
      /* Window settings have changed */
      NEXUS_GraphicsSettings graphicsSettings;
      NEXUS_Error err;

      /* setup the display & callback */
      NEXUS_Display_GetGraphicsSettings(data->display, &graphicsSettings);
      graphicsSettings.enabled = true;
      graphicsSettings.position.x = nw->x;
      graphicsSettings.position.y = nw->y;

      if (!nw->stretch)
      {
         graphicsSettings.position.width = nw->width;
         graphicsSettings.position.height = nw->height;
      }
      else
      {
         float scaledPosX = nw->x * ((float)graphicsSettings.position.width / nw->width);
         float scaledPosY = nw->y * ((float)graphicsSettings.position.height / nw->height);

         graphicsSettings.position.x = (int16_t)scaledPosX;
         graphicsSettings.position.y = (int16_t)scaledPosY;

         graphicsSettings.position.width -= (2 * scaledPosX);
         graphicsSettings.position.height -= (2 * scaledPosY);
      }
      graphicsSettings.clip.width = nw->width;
      graphicsSettings.clip.height = nw->height;
      graphicsSettings.frameBufferCallback.param = 0;
      graphicsSettings.frameBufferCallback.context = data->vsyncEvent;
      graphicsSettings.frameBufferCallback.callback = vsync_callback;

      err = NEXUS_Display_SetGraphicsSettings(data->display, &graphicsSettings);
      if (err)
         printf("NEXUS_Display_SetGraphicsSettings() failed\n");

      /* update width and height, etc. */
      data->winWidth = nw->width;
      data->winHeight = nw->height;
      data->winX = nw->x;
      data->winY = nw->y;
      data->stretch = nw->stretch;
   }

#if NEXUS_HAS_GRAPHICS2D
   if (windowState->gfdBacking[0])
   {
      NEXUS_Error rc;
      NEXUS_Graphics2DBlitSettings blitSettings;
      NEXUS_Graphics2D_GetDefaultBlitSettings(&blitSettings);

      /* blit from buffer->surface to GFD accessible version */
      surface = windowState->gfdBacking[windowState->gfdCurrentBuffer];

      blitSettings.source.surface = buffer->surface;
      blitSettings.output.surface = surface;
      blitSettings.colorOp = NEXUS_BlitColorOp_eCopySource;
      blitSettings.alphaOp = NEXUS_BlitAlphaOp_eCopySource;

      while (1)
      {
         rc = NEXUS_Graphics2D_Blit(windowState->gfx, &blitSettings);
         if (rc == NEXUS_GRAPHICS2D_QUEUE_FULL)
            BKNI_WaitForEvent(windowState->packetSpaceAvailableEvent, BKNI_INFINITE);
         else
            break;
      }

      rc = NEXUS_Graphics2D_Checkpoint(windowState->gfx, NULL); /* require to execute queue */
      if (rc == NEXUS_GRAPHICS2D_QUEUED)
         BKNI_WaitForEvent(windowState->checkpointEvent, BKNI_INFINITE);

      windowState->gfdCurrentBuffer = (windowState->gfdCurrentBuffer + 1) & 0x1;
   }
   else
#endif
      surface = buffer->surface;

   NEXUS_Graphics_GetDefaultFramebuffer3D(&fb3d);

   fb3d.main = surface;

   if (data->displayType == NXPL_2D)
      fb3d.orientation = NEXUS_VideoOrientation_e2D;
   else if (data->displayType == NXPL_3D_LEFT_RIGHT)
      fb3d.orientation == NEXUS_VideoOrientation_e3D_LeftRight;
   else if (data->displayType == NXPL_3D_OVER_UNDER)
      fb3d.orientation == NEXUS_VideoOrientation_e3D_OverUnder;

   NEXUS_Display_SetGraphicsFramebuffer3D(data->display, &fb3d);

   /* Do we need to wait for the vsync? */
   if (state->waitVSync)
      BKNI_WaitForEvent(data->vsyncEvent, BKNI_INFINITE);

   fence = PopQueue(windowState->signalQueue);
   if (fence != -1)
      data->hwInterface->FenceSignal(fence);

   sem_post(&windowState->throttleSemaphore);

   return BEGL_Success;
}

/* Flush pending displays until they are all done, then removes all buffers from display. Will block until complete. */
static BEGL_Error DispWindowUndisplay(void *context, BEGL_WindowState *windowState)
{
   NXPL_DisplayData         *data = (NXPL_DisplayData*)context;
   NXPL_NativeWindowInfoEXT *nw = (NXPL_NativeWindowInfoEXT*)windowState->window;
   NEXUS_GraphicsSettings   graphicsSettings;
   NEXUS_Error              err;

   NEXUS_Display_GetGraphicsSettings(data->display, &graphicsSettings);

   graphicsSettings.enabled = false;
   graphicsSettings.frameBufferCallback.callback = NULL;

   err = NEXUS_Display_SetGraphicsSettings(data->display, &graphicsSettings);
   if (err)
      printf("NEXUS_Display_SetGraphicsSettings() failed during platform shutdown\n");

   /* Reset last seen data, so that the next display call will set them correctly */
   data->winWidth = 0;
   data->winHeight = 0;
   data->winX = 0;
   data->winY = 0;
   data->stretch = 0;

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
   case BEGL_BufferFormat_eA8R8G8B8_Texture :
      bpp = 32;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eA8_R8_G8_B8;
      break;
   case BEGL_BufferFormat_eR8G8B8A8_Texture :
      bpp = 32;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eR8_G8_B8_A8;
      break;
   case BEGL_BufferFormat_eB8G8R8A8_Texture :
      bpp = 32;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eB8_G8_R8_A8;
      break;
   }

   surfSettings.width = settings->width;
   surfSettings.height = settings->height;
   surfSettings.alignment = settings->alignment;
   surfSettings.pitch = settings->pitchBytes;
   surfSettings.heap = data->heap;

   secure = !!(settings->secure);

   if (secure && !data->useMMA)
   {
      printf("secure mode only operates under MMA - export V3D_USE_MMA=1 and retry\n");
      exit(0);
   }

   surfaceBacking = data->memInterface->Alloc(data->memInterface->context, settings->totalByteSize, 4096, secure);
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
      settings->physOffset = data->memInterface->ConvertCachedToPhysical(data->memInterface->context, surfSettings.pMemory);
      settings->cachedAddr = surfSettings.pMemory;
   }

   buffer->settings = *settings;

/*
   printf("DispBufferCreate %x\n", buffer->surface);
*/

   return (BEGL_BufferHandle)buffer;
}

/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context, BEGL_BufferDisplayState *bufferState)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_BufferData *buffer = (NXPL_BufferData*)bufferState->buffer;

/*
   printf("DispBufferDestroy %x\n", buffer->surface);
*/

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
   NXPL_NativeWindowInfoEXT *nw = (NXPL_NativeWindowInfoEXT*)window;

   if (nw != NULL)
   {
      char *val;

      if (flags & BEGL_WindowInfoWidth)
         info->width = nw->width;
      if (flags & BEGL_WindowInfoHeight)
         info->height = nw->height;

      if (flags & BEGL_WindowInfoSwapChainCount)
      {
         val = getenv("V3D_DOUBLE_BUFFER");
         if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
            info->swapchain_count = 2;
         else
            info->swapchain_count = MAX_SWAP_BUFFERS;
      }

      if (flags & BEGL_WindowInfoRefreshRate)
      {
         NXPL_DisplayData    *data = (NXPL_DisplayData*)context;
         NEXUS_DisplayStatus status;

         NEXUS_Display_GetStatus(data->display, &status);
         info->refreshRateMilliHertz = status.refreshRate;
      }

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_BufferHandle DispBufferGet(void *context, BEGL_BufferSettings *settings)
{
   NXPL_DisplayData     *data = (NXPL_DisplayData*)context;
   NXPL_BufferData      *buffer = NULL;
   NXPL_WindowState     *windowState = (NXPL_WindowState *)settings->windowState.platformState;

   buffer = (NXPL_BufferData*)DispBufferCreate(context, settings);

   if (buffer)
   {
      if (settings->usage - BEGL_BufferUsage_eSwapChain0 >= windowState->actualSwapChainCount)
         windowState->actualSwapChainCount = settings->usage - BEGL_BufferUsage_eSwapChain0 + 1;
   }

   return (BEGL_BufferHandle)buffer;
}

static BEGL_Error DispBufferAccess(void *context, BEGL_BufferAccessState *bufferAccess)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;

   if (data)
   {
      int newFence;
      NXPL_WindowState *windowState = (NXPL_WindowState *)bufferAccess->windowState.platformState;
      NXPL_BufferData  *buffer = bufferAccess->buffer;

      sem_wait_infinite(&windowState->throttleSemaphore);

      data->hwInterface->FenceOpen(data->hwInterface->context, &newFence);

      PushQueue(windowState->waitQueue, newFence);
      PushQueue(windowState->signalQueue, newFence);

      /* not the same as the value just pushed above.  Adds latency */
      bufferAccess->fence = PopQueue(windowState->waitQueue);

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window)
{
   NXPL_WindowState *windowState = (NXPL_WindowState *)malloc(sizeof(NXPL_WindowState));
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   uint32_t          i;
#if NEXUS_HAS_GRAPHICS2D
   NEXUS_HEAPHANDLE     gfd0Heap, v3dHeap;
   NEXUS_MemoryStatus   gfd0Status, v3dStatus;
#endif

   memset(windowState, 0, sizeof(NXPL_WindowState));

#if NEXUS_HAS_GRAPHICS2D
   /* get GFD0 heap and NEXUS_OFFSCREEN_SURFACE */
   gfd0Heap = NEXUS_Platform_GetFramebufferHeap(0);
   v3dHeap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);

   NEXUS_Heap_GetStatus(gfd0Heap, &gfd0Status);
   NEXUS_Heap_GetStatus(v3dHeap, &v3dStatus);

   if (gfd0Status.memcIndex != v3dStatus.memcIndex)
   {
      NEXUS_Graphics2DSettings gfxSettings;
      NEXUS_SurfaceCreateSettings surfSettings;
      NXPL_NativeWindowInfoEXT *nw = (NXPL_NativeWindowInfoEXT*)window;
      printf("NXPL : DispWindowStateCreate() INFO: Create a GFD compatible backing\n");

      NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);
      surfSettings.width = nw->width;
      surfSettings.height = nw->height;
      surfSettings.pixelFormat = NEXUS_PixelFormat_eA8_B8_G8_R8;
      surfSettings.heap = NEXUS_Platform_GetFramebufferHeap(0);

      windowState->gfdBacking[0] = NEXUS_Surface_Create(&surfSettings);
      if (!windowState->gfdBacking[0]) {
         printf("\n\nNXPL : DispWindowStateCreate() ERROR.\nUnable to allocate GFD gfdBacking[0]\n");
         goto error0;
      }
      windowState->gfdBacking[1] = NEXUS_Surface_Create(&surfSettings);
      if (!windowState->gfdBacking[1]) {
         printf("\n\nNXPL : DispWindowStateCreate() ERROR.\nUnable to allocate GFD gfdBacking[1]\n");
         goto error1;
      }

      BKNI_CreateEvent(&windowState->checkpointEvent);
      BKNI_CreateEvent(&windowState->packetSpaceAvailableEvent);

      windowState->gfx = NEXUS_Graphics2D_Open(NEXUS_ANY_ID, NULL);
      NEXUS_Graphics2D_GetSettings(windowState->gfx, &gfxSettings);
      gfxSettings.checkpointCallback.callback = vsync_callback;
      gfxSettings.checkpointCallback.context = windowState->checkpointEvent;
      gfxSettings.packetSpaceAvailable.callback = vsync_callback;
      gfxSettings.packetSpaceAvailable.context = windowState->packetSpaceAvailableEvent;
      NEXUS_Graphics2D_SetSettings(windowState->gfx, &gfxSettings);
   }
#endif

   InitQueue(&windowState->waitQueue);
   InitQueue(&windowState->signalQueue);

   /* add additional semaphore to run further ahead without blocking */
   sem_init(&windowState->throttleSemaphore, 0, MAX_SWAP_BUFFERS);
   for (i = 0; i < MAX_SWAP_BUFFERS; i++)
      PushQueue(windowState->waitQueue, -1);
   PushQueue(windowState->signalQueue, -1);

   return (void*)windowState;

error1:
   NEXUS_Surface_Destroy(windowState->gfdBacking[0]);

error0:
   free(windowState);
   return NULL;
}

static BEGL_Error DispWindowStateDestroy(void *context, void *swapChainCtx)
{
   NXPL_DisplayData *data = (NXPL_DisplayData*)context;
   NXPL_WindowState *windowState = (NXPL_WindowState *)swapChainCtx;
   uint32_t          i;

   if (windowState)
   {
#if NEXUS_HAS_GRAPHICS2D
      NEXUS_HEAPHANDLE     gfd0Heap, v3dHeap;
      NEXUS_MemoryStatus   gfd0Status, v3dStatus;

      /* get GFD0 heap and NEXUS_OFFSCREEN_SURFACE */
      gfd0Heap = NEXUS_Platform_GetFramebufferHeap(0);
      v3dHeap = NEXUS_Platform_GetFramebufferHeap(NEXUS_OFFSCREEN_SURFACE);

      NEXUS_Heap_GetStatus(gfd0Heap, &gfd0Status);
      NEXUS_Heap_GetStatus(v3dHeap, &v3dStatus);

      if (gfd0Status.memcIndex != v3dStatus.memcIndex)
      {
         if (windowState->gfdBacking[0])
            NEXUS_Surface_Destroy(windowState->gfdBacking[0]);
         if (windowState->gfdBacking[1])
            NEXUS_Surface_Destroy(windowState->gfdBacking[1]);

         if (windowState->checkpointEvent)
            BKNI_DestroyEvent(windowState->checkpointEvent);
         if (windowState->packetSpaceAvailableEvent)
            BKNI_DestroyEvent(windowState->packetSpaceAvailableEvent);

         if (windowState->gfx)
            NEXUS_Graphics2D_Close(windowState->gfx);
      }
#endif
      sem_destroy(&windowState->throttleSemaphore);

      /* waitqueue should be empty, signalQueue could have stuff in it */
      while (!QueueEmpty(windowState->signalQueue))
      {
         int fence = PopQueue(windowState->signalQueue);
         data->hwInterface->FenceSignal(fence);
      }

      TermQueue(windowState->waitQueue);
      TermQueue(windowState->signalQueue);

      memset(windowState, 0, sizeof(NXPL_WindowState));
      free(windowState);
      return BEGL_Success;
   }

   return BEGL_Fail;
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
                                                   NEXUS_DISPLAYHANDLE display, BEGL_DisplayCallbacks *displayCallbacks)
{
   NXPL_DisplayData        *data;
   BEGL_DisplayInterface   *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));

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

         disp->context = (void*)data;
         disp->BufferDisplay = DispBufferDisplay;
         disp->BufferCreate = DispBufferCreate;
         disp->BufferGet = DispBufferGet;
         disp->BufferDestroy = DispBufferDestroy;
         disp->BufferGetCreateSettings = DispBufferGetCreateSettings;
         disp->BufferAccess = DispBufferAccess;
         disp->WindowUndisplay = DispWindowUndisplay;
         disp->WindowGetInfo = DispWindowGetInfo;
         disp->WindowPlatformStateCreate = DispWindowStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
         disp->SurfaceVerifyImageTarget = DispSurfaceVerifyImageTarget;
         disp->DecodeNativeFormat = DispDecodeNativeFormat;

         data->memInterface = memIface;
         data->hwInterface = hwIface;
         data->display = display;
         data->bufferGetRequirementsFunc = displayCallbacks->BufferGetRequirements;

         BKNI_CreateEvent(&data->vsyncEvent);

         /* Store the heap settings */
         data->heap = NXPL_MemHeap(memIface);

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

void NXPL_SetDisplayType(NXPL_PlatformHandle handle, NXPL_DisplayType type)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL && data->displayInterface != NULL && data->displayInterface->context != NULL)
   {
      NXPL_DisplayData *dd = (NXPL_DisplayData*)data->displayInterface->context;
      dd->displayType = type;
   }
}

void NXPL_DestroyDisplayInterface(BEGL_DisplayInterface *disp)
{
   if (disp != NULL)
   {
      NXPL_DisplayData *data = (NXPL_DisplayData *)disp->context;
      if (data != NULL)
      {
         if (data->vsyncEvent)
            BKNI_DestroyEvent(data->vsyncEvent);

         free(data);
      }

      memset(disp, 0, sizeof(BEGL_DisplayInterface));
      free(disp);
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

bool NXPL_CreateCompatiblePixmapEXT(NXPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SURFACEHANDLE *surface,
   BEGL_PixmapInfoEXT *info)
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

bool NXPL_CreateCompatiblePixmap(NXPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SURFACEHANDLE *surface,
   BEGL_PixmapInfo *info)
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

   if (data != NULL &&
       data->displayInterface != NULL &&
       data->displayInterface->BufferDestroy != NULL)
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
      info->magic      = NATIVE_WINDOW_INFO_MAGIC;
   }
}

/* Exclusive display mode only allows ONE window */
void *NXPL_CreateNativeWindowEXT(const NXPL_NativeWindowInfoEXT *info)
{
   NXPL_NativeWindowInfoEXT *nw;

   if (info == NULL)
      return NULL;

   assert(info->magic == NATIVE_WINDOW_INFO_MAGIC);

   if (__sync_fetch_and_add(&windowCount, 1) != 0)
   {
      __sync_fetch_and_sub(&windowCount, 1);
      return NULL;
   }

   nw = (NXPL_NativeWindowInfoEXT*)malloc(sizeof(NXPL_NativeWindowInfoEXT));
   memset(nw, 0, sizeof(NXPL_NativeWindowInfoEXT));
   if (nw != NULL)
      *nw = *info;

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

void NXPL_UpdateNativeWindowEXT(void *native, const NXPL_NativeWindowInfoEXT *info)
{
   if (info != NULL && native != NULL)
      *((NXPL_NativeWindowInfoEXT*)native) = *info;
}

void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info)
{
   /* deprecated API */
   NXPL_NativeWindowInfoEXT *nw = (NXPL_NativeWindowInfoEXT*)native;

   if (info != NULL && nw != NULL)
   {
      nw->width = info->width;
      nw->height = info->height;
      nw->x = info->x;
      nw->y = info->y;
      nw->stretch = info->stretch;
      nw->clientID = info->clientID;
      nw->zOrder = info->zOrder;
   }
}

void NXPL_DestroyNativeWindow(void *nativeWin)
{
   if (nativeWin != NULL)
   {
      NXPL_NativeWindowInfoEXT *nw = (NXPL_NativeWindowInfoEXT*)nativeWin;

      memset(nw, 0, sizeof(NXPL_NativeWindowInfoEXT));
      free(nw);

      __sync_fetch_and_sub(&windowCount, 1);
   }
}
