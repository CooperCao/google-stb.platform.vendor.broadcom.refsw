/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  platform layer
Module   :

FILE DESCRIPTION
Android platform layer for composition
=============================================================================*/
#include <memory.h>
#include <assert.h>
#include <semaphore.h>
#include <time.h>

#include "default_RSO_android.h"

#include "nexus_surface.h"
#include "nexus_memory.h"
#include "nexus_platform.h"

#include <cutils/properties.h>

#define ANPL_TAG "RSOANPL"

#define NX_MMA "ro.nx.mma"

#define AN_BUFFERS (3)

#define ABANDONEDSCRATCH_WIDTH 64
#define ABANDONEDSCRATCH_HEIGHT 64
#define ABANDONEDSCRATCH_BPP 4

typedef struct
{
   BEGL_MemoryInterface          *memInterface;
   BEGL_HWInterface              *hwInterface;
   bool                          useMMA;

   void *                        abandonedScratch;
   uint32_t                      abandonedScratchPhysical;
   NEXUS_MemoryBlockHandle       abandonedScratchBlockHandle;
} RSOAN_Display;

typedef struct
{
   ANativeWindow                 *awin;
   BEGL_BufferSettings           settings;
   android_native_buffer_t       *surface;
   NEXUS_SurfaceHandle           internalSurface; /* only used for pixmaps, not main windows */
} RSOAN_BufferData;

/* There will be one RSOAN_WindowState for each window */
typedef struct
{
   sem_t                         sync;
   int                           bufferCount;
   int                           nextFree;
   RSOAN_BufferData              *buffers[AN_BUFFERS];
   ANativeWindow                 *awin;
} RSOAN_WindowState;

extern NEXUS_HeapHandle NXPL_MemHeap(BEGL_MemoryInterface *mem);

static int isANativeWindow(ANativeWindow * win)
{
   int res = (win && (win->common.magic == ANDROID_NATIVE_WINDOW_MAGIC)) ? 1 :0;
   if(!res)
   {
      LOGD("==========================================================================");
      LOGD("INVALID WINDOW INVALID WINDOW INVALID WINDOW INVALID WINDOW INVALID WINDOW");
      LOGD("==========================================================================");
   }
   return res;
}

static BEGL_Error grab_buffer(void *context, RSOAN_BufferData *buffer, RSOAN_WindowState *windowState)
{
   RSOAN_Display *data = (RSOAN_Display*)context;
   UNUSED(windowState);

   if (data)
   {
      /* unlock is implicit in dequeue so we don't need a case for it */
      int res;
      int fmt;
      android_native_buffer_t *surface = NULL;

      isANativeWindow(buffer->awin);

      do
      {
         res = buffer->awin->dequeueBuffer_DEPRECATED(buffer->awin, &surface);
         if (res != 0)
         {
            /* android can legitimately get a EBUSY, retry again */
#ifdef DEBUG
            LOGD("DispBufferAccess(): buffer->awin %p", buffer->awin);
            LOGD("DispBufferAccess(): dequeueBuffer error %d", res);
#endif
            usleep(1);
         }
      } while (res == -EBUSY);

      if (res != 0)
      {
         /* update BEGL_BufferSettings with new information */
         buffer->settings.physOffset = data->abandonedScratchPhysical;
         buffer->settings.pitchBytes = ABANDONEDSCRATCH_WIDTH * ABANDONEDSCRATCH_BPP;
         buffer->settings.cachedAddr = data->abandonedScratch;

         /* get the size from the android_native_buffer_t */
         buffer->settings.width = ABANDONEDSCRATCH_WIDTH;
         buffer->settings.height = ABANDONEDSCRATCH_HEIGHT;
         buffer->settings.format = BEGL_BufferFormat_eA8B8G8R8;
         buffer->surface = NULL;
      }
      else
      {
         /* LOGD("%s: %d Dequeue Done Surface: (%p) (%p) %p\n", __FUNCTION__, __LINE__, buffer->awin, buffer, surface); */
         surface->common.incRef(&surface->common);

         struct private_handle_t const* hnd =  (struct private_handle_t const*) (surface->handle);

         /* update BEGL_BufferSettings with new information */
         buffer->settings.physOffset = hnd->nxSurfacePhysicalAddress;
         buffer->settings.cachedAddr = (void *)(intptr_t)hnd->nxSurfaceAddress;
         buffer->settings.pitchBytes = hnd->oglStride;

#if 0
         if (buffer->settings.cachedAddr == 0)
         {
            LOGD("***********************************************************");
            LOGD("FAILURE, CACHED ADDRESS == 0");
            LOGD("***********************************************************");
         }
#endif
         /* get the size from the android_native_buffer_t */
         buffer->settings.width = surface->width;
         buffer->settings.height = surface->height;

         switch (surface->format)
         {
            case HAL_PIXEL_FORMAT_RGBA_8888:   buffer->settings.format = BEGL_BufferFormat_eA8B8G8R8;   break;
            case HAL_PIXEL_FORMAT_RGBX_8888:   buffer->settings.format = BEGL_BufferFormat_eX8B8G8R8;   break;
            case HAL_PIXEL_FORMAT_RGB_888:     buffer->settings.format = BEGL_BufferFormat_eX8B8G8R8;   break;
            case HAL_PIXEL_FORMAT_RGB_565:     buffer->settings.format = BEGL_BufferFormat_eR5G6B5;     break;
            default:                           buffer->settings.format = BEGL_BufferFormat_INVALID;
         }

         buffer->surface = surface;
      }

      return BEGL_Success;
   }

   return BEGL_Fail;
}

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/

static BEGL_Error DispBufferDisplay(void *context, BEGL_BufferDisplayState *state)
{
   RSOAN_Display    *data = (RSOAN_Display*)context;
   RSOAN_BufferData *buffer = (RSOAN_BufferData*)state->buffer;
   RSOAN_WindowState *windowState = (RSOAN_WindowState*)state->windowState.platformState;

   if (data)
   {
      if (buffer)
      {
         android_native_buffer_t *surface;
         /* this will assign the current surface to the local and NULL the original */
         surface = __sync_fetch_and_and(&buffer->surface, 0);

         /* queue()    -> cause a flip */
         if (buffer->awin && surface)
         {
            buffer->awin->queueBuffer_DEPRECATED(buffer->awin, surface);

            /* LOGD("%s: %d IRQ: queue surface (%p) (%p) %p\n", __FUNCTION__, __LINE__, buffer->awin, buffer, buffer->surface); */
            surface->common.decRef(&surface->common);
         }

         /* this will replace the previously posted buffer with a new one */
         grab_buffer(context, windowState->buffers[windowState->nextFree], windowState);
         windowState->nextFree++;
         if (windowState->nextFree == AN_BUFFERS)
            windowState->nextFree = 0;
      }

      /* signal anything waiting on this that it can go */
      sem_post(&windowState->sync);

      return BEGL_Success;
   }

   return BEGL_Fail;
}

/* Flush pending displays until they are all done, then removes all buffers from display. Will block until complete. */
static BEGL_Error DispWindowUndisplay(void *context, BEGL_WindowState *windowState)
{
   UNUSED(context);
   UNUSED(windowState);

   LOGD("==========================================================================");
   LOGD("DispWindowUndisplay called");
   LOGD("==========================================================================");
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
   RSOAN_Display                  *data = (RSOAN_Display*)context;
   RSOAN_BufferData               *buffer = NULL;
   NEXUS_SurfaceCreateSettings    surfSettings;
   NEXUS_MemoryAllocationSettings memSettings;
   uint32_t                       bpp;
   void                           *surfaceBacking = NULL;

   LOGD("==========================================================================");
   LOGD("DispBufferCreate called");
   LOGD("==========================================================================");

   buffer = (RSOAN_BufferData*)malloc(sizeof(RSOAN_BufferData));
   memset(buffer, 0, sizeof(RSOAN_BufferData));

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
   default :
      LOGD("==========================================================================");
      LOGD("unsupported pixel format, aborting");
      LOGD("==========================================================================");
      assert(0);
      break;
   }

   surfSettings.width = settings->width;
   surfSettings.height = settings->height;
   surfSettings.alignment = settings->alignment;
   surfSettings.pitch = settings->pitchBytes;
   surfSettings.heap = NXPL_MemHeap(data->memInterface);

   surfaceBacking = data->memInterface->Alloc(data->memInterface->context, settings->totalByteSize, 4096, false);
   if (surfaceBacking != NULL)
   {
      if (data->useMMA)
         surfSettings.pixelMemory = surfaceBacking;
      else
         surfSettings.pMemory = surfaceBacking;
      buffer->internalSurface = NEXUS_Surface_Create(&surfSettings);
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
      /* in non MMA mode just used for the Free() */
      settings->cachedAddr = surfSettings.pMemory;
      settings->physOffset = data->memInterface->ConvertCachedToPhysical(data->memInterface->context, surfSettings.pMemory);
   }

   buffer->settings = *settings;

   return (BEGL_BufferHandle)buffer;
}

/* does nothing more than create the platform abstraction */
static BEGL_BufferHandle DispBufferGet(void *context, BEGL_BufferSettings *settings)
{
   RSOAN_Display    *data = (RSOAN_Display*)context;
   RSOAN_BufferData *buffer = NULL;
   int              format;
   RSOAN_WindowState *windowState = (RSOAN_WindowState*)settings->windowState.platformState;

   buffer = (RSOAN_BufferData*)malloc(sizeof(RSOAN_BufferData));
   if (buffer)
   {
      memset(buffer, 0, sizeof(RSOAN_BufferData));

      buffer->awin = (ANativeWindow*)settings->windowState.window;

      isANativeWindow(buffer->awin);

      /* query the surface type is one of the 3d cores renderable types */
      buffer->awin->query(buffer->awin, NATIVE_WINDOW_FORMAT, &format);

      /* update the structure, as the android window may have been created differently
         to that of the egl context that was selected */
      switch (format)
      {
         case HAL_PIXEL_FORMAT_RGBA_8888:   settings->format = BEGL_BufferFormat_eA8B8G8R8;   break;
         case HAL_PIXEL_FORMAT_RGBX_8888:   settings->format = BEGL_BufferFormat_eX8B8G8R8;   break;
         case HAL_PIXEL_FORMAT_RGB_888:     settings->format = BEGL_BufferFormat_eX8B8G8R8;   break;
         case HAL_PIXEL_FORMAT_RGB_565:     settings->format = BEGL_BufferFormat_eR5G6B5;     break;
         default:                           settings->format = BEGL_BufferFormat_INVALID;     break;
      }

      /* structure copy -> grab the elements above (inputs) to the buffer structure */
      buffer->settings = *settings;

      if (windowState->bufferCount)
      {
         windowState->bufferCount--;
         /* this grabs the resource from Android */
         grab_buffer(context, buffer, windowState);
         windowState->nextFree = (settings->usage - BEGL_BufferUsage_eSwapChain0) + 1;
         if (windowState->nextFree == AN_BUFFERS)
            windowState->nextFree = 0;
      }

      windowState->buffers[settings->usage - BEGL_BufferUsage_eSwapChain0] = buffer;
   }

   return (BEGL_BufferHandle)buffer;
}

/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context, BEGL_BufferDisplayState *bufferState)
{
   RSOAN_Display    *data = (RSOAN_Display*)context;
   RSOAN_BufferData *buffer = (RSOAN_BufferData*)bufferState->buffer;
   RSOAN_WindowState *windowState = (RSOAN_WindowState*)bufferState->windowState.platformState;

   if (buffer != NULL)
   {
      assert(bufferState->windowState->window == (BEGL_WindowHandle)buffer->awin);

      if (buffer->settings.usage == BEGL_BufferUsage_ePixmap)
      {
         /* for pixmap surfaces, the surface is always created via DispBufferCreate() */
         /* in this case we stashed the nexus handle in internalSurface */
         if (buffer->internalSurface && buffer->settings.cachedAddr)
         {
            NEXUS_SurfaceCreateSettings createSettings;
            void *surfaceBacking;
            NEXUS_Surface_GetCreateSettings(buffer->internalSurface, &createSettings);
            NEXUS_Surface_Destroy(buffer->internalSurface);

            if (data->useMMA)
               surfaceBacking = createSettings.pixelMemory;
            else
               surfaceBacking = createSettings.pMemory;

            if (surfaceBacking != NULL)
               data->memInterface->Free(data->memInterface->context, surfaceBacking);
         }
         else
         {
            LOGD("DispBufferDestroy() BEGL_BufferUsage_ePixmap was invalid");
         }
      }
      else
      {
         LOGD("DispBufferDestroy : (buffer = %p)", buffer->surface);

         android_native_buffer_t *surface;

         /* this will assign the current surface to the local and NULL the original */
         surface = __sync_fetch_and_and(&buffer->surface, 0);

         /* remove any locks on a buffer which may have been claimed prior to rendering */
         if (surface)
         {
            buffer->awin->cancelBuffer_DEPRECATED(buffer->awin, surface);
            surface->common.decRef(&surface->common);
         }
      }

      memset(buffer, 0, sizeof(RSOAN_BufferData));
      free(buffer);
   }
   return BEGL_Success;
}

/* Get information about a created window buffer */
static BEGL_Error DispBufferGetCreateSettings(void *context, BEGL_BufferHandle bufHandle, BEGL_BufferSettings *settings)
{
   RSOAN_BufferData *buffer = (RSOAN_BufferData*)bufHandle;
   UNUSED(context);

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
   RSOAN_Display  *data = (RSOAN_Display*)context;
   ANativeWindow  *dst = (ANativeWindow*)window;
   int            fmt;

   /* get the size from the ANativeWindow */
   if (flags & BEGL_WindowInfoWidth)
   {
      int res = dst->query(dst, NATIVE_WINDOW_WIDTH, (int *)&info->width);
      if (res == -ENODEV)
      {
         /* surface texture has been abandoned */
         info->width = ABANDONEDSCRATCH_WIDTH;
      }
   }

   if (flags & BEGL_WindowInfoHeight)
   {
      int res = dst->query(dst, NATIVE_WINDOW_HEIGHT, (int *)&info->height);
      if (res == -ENODEV)
      {
         /* surface texture has been abandoned */
         info->height = ABANDONEDSCRATCH_HEIGHT;
      }
   }

   if (flags & BEGL_WindowInfoFormat)
   {
      int res = dst->query(dst, NATIVE_WINDOW_FORMAT, (int *)&fmt);

      if (res == -ENODEV)
      {
         info->format = BEGL_BufferFormat_eA8B8G8R8;
      }
      else
      {
         switch (fmt)
         {
            case HAL_PIXEL_FORMAT_RGBA_8888:  info->format = BEGL_BufferFormat_eA8B8G8R8;     break;
            case HAL_PIXEL_FORMAT_RGBX_8888:  info->format = BEGL_BufferFormat_eX8B8G8R8;     break;
            case HAL_PIXEL_FORMAT_RGB_888:    info->format = BEGL_BufferFormat_eX8B8G8R8;     break;
            case HAL_PIXEL_FORMAT_RGB_565:    info->format = BEGL_BufferFormat_eR5G6B5;       break;
            default:                          info->format = BEGL_BufferFormat_INVALID;       break;
         }
      }
   }

   /* make sure we initialize enough structures for the buffers we have */
   if (flags & BEGL_WindowInfoSwapChainCount)
      info->swapchain_count = AN_BUFFERS;

   return BEGL_Success;
}

extern void *client_lock_api(void);
extern void client_unlock_api(void);

static BEGL_Error DispBufferAccess(void *context, BEGL_BufferAccessState *bufferAccess)
{
   RSOAN_WindowState *windowState = (RSOAN_WindowState*)bufferAccess->windowState.platformState;
   UNUSED(context);

   client_unlock_api();

   /* just a block to wait for a semaphore from the display thread */
   sem_wait(&windowState->sync);

   client_lock_api();

   return BEGL_Success;
}

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window)
{
   RSOAN_WindowState *windowState = (RSOAN_WindowState *)malloc(sizeof(RSOAN_WindowState));
   UNUSED(context);

   if (windowState)
   {
      ANativeWindow *awin;
      int type;

      memset(windowState, 0, sizeof(RSOAN_WindowState));

      awin = (ANativeWindow*)window;

      isANativeWindow(awin);

      awin->query(awin, NATIVE_WINDOW_CONSUMER_USAGE_BITS, &type);

      if (!(type & GRALLOC_USAGE_HW_FB))
      {
         unsigned int mub = 1;
         /* regular window */
         if (awin->query(awin, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, (int *)&mub) != 0)
         {
            LOGD("Unable to query NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS\n");
         }
         else
         {
            LOGD("==========================================================================");
            LOGD("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS = %d", mub);
            LOGD("==========================================================================");
         }

         if (native_window_set_buffer_count(awin, (AN_BUFFERS + mub)) != 0)
         {
            LOGD("DispBufferGet(): Unable to set the number of buffers to %d", (AN_BUFFERS + mub));
         }

         /* surface texture - force to 1 */
         awin->setSwapInterval(awin, 1);

         sem_init(&windowState->sync, 0, (AN_BUFFERS) - 1);
         windowState->bufferCount = (AN_BUFFERS) - 1;
      }
      else
      {
         sem_init(&windowState->sync, 0, 1);
         windowState->bufferCount = 1;
      }

      /* android surface 0 */
      awin->common.incRef(&awin->common);

      /* save the ANativeWindow for DispWindowStateDestroy() */
      windowState->awin = awin;
   }

   return (void*)windowState;
}

static BEGL_Error DispWindowStateDestroy(void *context, void *swapChainCtx)
{
   RSOAN_WindowState *windowState = (RSOAN_WindowState *)swapChainCtx;
   UNUSED(context);

   if (windowState)
   {
      windowState->awin->common.decRef(&windowState->awin->common);
      sem_destroy(&windowState->sync);
      memset(windowState, 0, sizeof(RSOAN_WindowState));
      free(windowState);
      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_Error DispGetNativeFormat(void *context,
                                      BEGL_BufferFormat bufferFormat,
                                      uint32_t * nativeFormat)
{
   BEGL_Error res = BEGL_Success;
   UNUSED(context);

   if (nativeFormat != NULL)
   {
      if (bufferFormat == BEGL_BufferFormat_eA8B8G8R8)
         *nativeFormat = HAL_PIXEL_FORMAT_RGBA_8888;
      else if (bufferFormat == BEGL_BufferFormat_eX8B8G8R8)
         *nativeFormat = HAL_PIXEL_FORMAT_RGBX_8888;
      else if (bufferFormat == BEGL_BufferFormat_eR5G6B5)
         *nativeFormat = HAL_PIXEL_FORMAT_RGB_565;
      else
         res = BEGL_Fail;
   }

   return res;
}

static BEGL_Error DispDecodeNativeFormat(void *context,
                                         void *buffer,
                                         BEGL_BufferSettings *settings)
{
   if (settings && buffer)
   {
      memset(settings, 0, sizeof(BEGL_BufferSettings));

      android_native_buffer_t *androidBuffer = (android_native_buffer_t *)buffer;

      settings->format = ((struct private_handle_t *)androidBuffer->handle)->oglFormat;
      settings->width = androidBuffer->width;
      settings->height = androidBuffer->height;
      settings->pitchBytes = ((struct private_handle_t *)androidBuffer->handle)->oglStride;

      settings->physOffset = ((struct private_handle_t *)androidBuffer->handle)->nxSurfacePhysicalAddress;
      settings->cachedAddr = (void *)(intptr_t)(((struct private_handle_t *)androidBuffer->handle)->nxSurfaceAddress);

      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error DispSurfaceChangeRefCount(void *context, void *buffer, BEGL_RefCountMode incOrDec)
{
   android_native_buffer_t *androidBuffer = (android_native_buffer_t *)buffer;

   switch (incOrDec)
   {
   case BEGL_Increment: androidBuffer->common.incRef(&androidBuffer->common); break;
   case BEGL_Decrement: androidBuffer->common.decRef(&androidBuffer->common); break;
   default: assert(0); return BEGL_Fail;
   }
   return BEGL_Success;
}

static BEGL_Error DispSurfaceVerifyImageTarget(void *context,
   void *nativeSurface, uint32_t eglTarget)
{
   UNUSED(context);
   UNUSED(nativeSurface);

   return eglTarget == EGL_NATIVE_BUFFER_ANDROID ? BEGL_Success : BEGL_Fail;
}

__attribute__((visibility("default")))
BEGL_DisplayInterface *RSOANPL_CreateDisplayInterface(BEGL_MemoryInterface *mem,
                                                      BEGL_HWInterface *hw,
                                                      BEGL_DisplayCallbacks *displayCallbacks)
{
   RSOAN_Display         *data;
   BEGL_DisplayInterface *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));
   UNUSED(displayCallbacks);

   if (disp != NULL)
   {
      data = (RSOAN_Display *)malloc(sizeof(RSOAN_Display));
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      if (data != NULL)
      {
         char val[PROPERTY_VALUE_MAX];
         NEXUS_MemoryAllocationSettings memSettings;
         void                           *surfaceBacking = NULL;

         memset(data, 0, sizeof(RSOAN_Display));

         /* default off */
         property_get(NX_MMA, val, "0");
         if (val[0] == 't' || val[0] == 'T' || val[0] == '1')
            data->useMMA = true;
         else
            data->useMMA = false;

         disp->context = (void*)data;
         disp->BufferDisplay = DispBufferDisplay;
         disp->WindowUndisplay = DispWindowUndisplay;
         disp->BufferCreate = DispBufferCreate;
         disp->BufferGet = DispBufferGet;
         disp->BufferDestroy = DispBufferDestroy;
         disp->BufferGetCreateSettings = DispBufferGetCreateSettings;
         disp->BufferAccess = DispBufferAccess;
         disp->WindowGetInfo = DispWindowGetInfo;
         disp->WindowPlatformStateCreate = DispWindowStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
         disp->GetNativeFormat = DispGetNativeFormat;
         disp->DecodeNativeFormat = DispDecodeNativeFormat;
         disp->SurfaceChangeRefCount = DispSurfaceChangeRefCount;
         disp->SurfaceVerifyImageTarget = DispSurfaceVerifyImageTarget;

         /* Store the memory interface */
         data->memInterface = mem;
         /* Store the hw interface */
         data->hwInterface = hw;


         surfaceBacking = data->memInterface->Alloc(data->memInterface->context,
                                 ABANDONEDSCRATCH_WIDTH *
                                 ABANDONEDSCRATCH_HEIGHT *
                                 ABANDONEDSCRATCH_BPP, 4096, false);

         if (data->useMMA)
         {
            NEXUS_Addr  devPtr;
            NEXUS_Error err;
            void *p;

            err = NEXUS_MemoryBlock_LockOffset((NEXUS_MemoryBlockHandle)surfaceBacking, &devPtr);
            if (err != NEXUS_SUCCESS)
            {
               LOGE("OUT OF VM SPACE ---- NEXUS_MemoryBlock_LockOffset()");
               assert(0);
            }
            data->abandonedScratchPhysical = (uint32_t)devPtr;

            err = NEXUS_MemoryBlock_Lock((NEXUS_MemoryBlockHandle)surfaceBacking, &p);
            if (err != NEXUS_SUCCESS)
            {
               LOGE("OUT OF VM SPACE ---- NEXUS_MemoryBlock_Lock()");
               assert(0);
            }
            data->abandonedScratch = p;
            data->abandonedScratchBlockHandle = (NEXUS_MemoryBlockHandle)surfaceBacking;
         }
         else
         {
            /* In the non MMA case abandonedScratchBlockHandle is just the cached pointer for Free() */
            data->abandonedScratchBlockHandle =
               data->abandonedScratch = surfaceBacking;
            data->abandonedScratchPhysical = data->memInterface->ConvertCachedToPhysical(data->memInterface->context, surfaceBacking);
         }
      }
      else
      {
         goto error0;
      }
   }
   return disp;

error0:
   free(disp);
   return NULL;
}

__attribute__((visibility("default")))
void RSOANPL_DestroyDisplayInterface(BEGL_DisplayInterface * disp)
{
   if (disp)
   {
      RSOAN_Display *data = (RSOAN_Display *)disp->context;
      if (data)
      {
         data->memInterface->Free(data->memInterface->context, data->abandonedScratchBlockHandle);
         free(data);
      }

      free(disp);
   }
}

__attribute__((visibility("default")))
bool RSOANPL_BufferGetRequirements(RSOANPL_PlatformHandle handle,
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
