/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <semaphore.h>
#include <time.h>
#include <pthread.h>
#include <cutils/properties.h>

#include <sync/sync.h>

#include "default_android.h"

#include "nexus_surface.h"
#include "nexus_memory.h"
#include "sched_nexus.h"
#include "nexus_platform.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gralloc_priv.h>

#define MAX_DEQUEUE_BUFFERS   2

/* Set this to get a lot of debug log info */
#define VERBOSE_LOGGING 0

#if VERBOSE_LOGGING
#define DBGLOG(...)   ((void)ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#else
#define DBGLOG(...)   ((void)0)
#endif

static bool s_expose_fences = false;

static bool AndroidToBeglFormat(BEGL_BufferFormat *result, int androidFormat)
{
   bool  ok = true;

   switch (androidFormat)
   {
   case HAL_PIXEL_FORMAT_RGBA_8888:   *result = BEGL_BufferFormat_eA8B8G8R8;           break;
   case HAL_PIXEL_FORMAT_RGBX_8888:   *result = BEGL_BufferFormat_eX8B8G8R8;           break;
   case HAL_PIXEL_FORMAT_RGB_888:     *result = BEGL_BufferFormat_eX8B8G8R8;           break;
   case HAL_PIXEL_FORMAT_RGB_565:     *result = BEGL_BufferFormat_eR5G6B5;             break;
   case HAL_PIXEL_FORMAT_YV12:        *result = BEGL_BufferFormat_eYV12;               break;
   default:                           *result = BEGL_BufferFormat_INVALID; ok = false; break;
   }

   return ok;
}

static bool BeglToAndroidFormat(int *androidFormat, BEGL_BufferFormat format)
{
   bool  ok = true;

   switch (format)
   {
   case BEGL_BufferFormat_eA8B8G8R8 : *androidFormat = HAL_PIXEL_FORMAT_RGBA_8888;    break;
   case BEGL_BufferFormat_eX8B8G8R8 : *androidFormat = HAL_PIXEL_FORMAT_RGBX_8888;    break;
   case BEGL_BufferFormat_eR5G6B5   : *androidFormat = HAL_PIXEL_FORMAT_RGB_565;      break;
   case BEGL_BufferFormat_eYV12     : *androidFormat = HAL_PIXEL_FORMAT_YV12;         break;
   default:                            ok = false;                                    break;
   }

   return ok;
}

static int isAndroidNativeWindow(ANativeWindow * win)
{
   int res = (win && (win->common.magic == ANDROID_NATIVE_WINDOW_MAGIC)) ? 1 : 0;
   if (!res)
   {
      ALOGE("==========================================================================");
      ALOGE("%s INVALID WINDOW =%p ", __FUNCTION__, win);
      ALOGE("==========================================================================");
   }
   return res;
}

static int isAndroidNativeBuffer(ANativeWindowBuffer_t *buf)
{
   int res = (buf && (buf->common.magic == ANDROID_NATIVE_BUFFER_MAGIC)) ? 1 : 0;
   if (!res)
   {
      ALOGE("==========================================================================");
      ALOGE("%s INVALID BUFFER buffer=%p", __FUNCTION__, buf);
      ALOGE("==========================================================================");
   }
   return res;
}

static BEGL_Error DispGetNativeFormat(void *context, BEGL_BufferFormat format, uint32_t *nativeFormat)
{
   *nativeFormat = 0;
   return BeglToAndroidFormat((int *)nativeFormat, format) ? BEGL_Success : BEGL_Fail;
}


/* Called to determine current size of the window referenced by the opaque window handle.
 * This is needed by EGL in order to know the size of a native 'window'. */
static BEGL_Error DispWindowGetInfo(void *context,
                                    void *nativeWindow,
                                    BEGL_WindowInfoFlags flags,
                                    BEGL_WindowInfo *info)
{
   ANativeWindow *anw = (ANativeWindow*)nativeWindow;

   if (anw == NULL || info == NULL)
      return BEGL_Fail;

   if (!isAndroidNativeWindow(anw))
      return BEGL_Fail;

   if (flags & BEGL_WindowInfoWidth)
   {
      int res = anw->query(anw, NATIVE_WINDOW_WIDTH, (int *)&info->width);
      if (res == -ENODEV)
         assert(0);
   }
   if (flags & BEGL_WindowInfoHeight)
   {
      int res = anw->query(anw, NATIVE_WINDOW_HEIGHT, (int *)&info->height);
      if (res == -ENODEV)
         assert(0);
   }

   if (flags & BEGL_WindowInfoSwapChainCount)
   {
      info->swapchain_count = MAX_DEQUEUE_BUFFERS;
   }

   return BEGL_Success;
}

static BEGL_Error DispGetNativeSurface(void *context,
      uint32_t eglTarget, void *eglSurface, void **nativeSurface)
{
   BSTD_UNUSED(context);

   if (eglTarget != EGL_NATIVE_BUFFER_ANDROID)
      return BEGL_Fail;
   *nativeSurface = eglSurface;
   return BEGL_Success;
}

static BEGL_Error DispSurfaceGetInfo(void *context, void *nativeSurface, BEGL_SurfaceInfo *info)
{
   ANativeWindowBuffer_t *buffer = (ANativeWindowBuffer_t*)nativeSurface;
   bool                  ok       = false;
   private_handle_t const* hnd;

   if (!isAndroidNativeBuffer(buffer) || info == NULL)
      return BEGL_Fail;

   hnd = (private_handle_t const*)buffer->handle;

   info->physicalOffset = hnd->nxSurfacePhysicalAddress;
   info->cachedAddr     = (void*)(intptr_t)hnd->nxSurfaceAddress;
   info->byteSize       = hnd->oglSize;
   info->pitchBytes     = hnd->oglStride;
   info->width          = buffer->width;
   info->height         = buffer->height;
   AndroidToBeglFormat(&info->format, buffer->format);

   return BEGL_Success;
}

static BEGL_Error DispSurfaceChangeRefCount(void *context, void *nativeBackBuffer, BEGL_RefCountMode incOrDec)
{
   ANativeWindowBuffer_t *buffer = (ANativeWindowBuffer_t*)nativeBackBuffer;

   if (!isAndroidNativeBuffer(buffer))
      return BEGL_Fail;

   switch (incOrDec)
   {
   case BEGL_Increment : buffer->common.incRef(&buffer->common); break;
   case BEGL_Decrement : buffer->common.decRef(&buffer->common); break;
   default             : assert(0); return BEGL_Fail;
   }
   return BEGL_Success;
}

static void MaybeRemoveFence(int *fence)
{
   if (!s_expose_fences && *fence != -1)
   {
      sync_wait(*fence, -1);
      close(*fence);
      *fence = -1;
   }
}

static BEGL_Error DispGetNextSurface(
   void *context,
   void *nativeWindow,
   BEGL_BufferFormat format,
   BEGL_BufferFormat *actualFormat,
   void **nativeBackBuffer,
   bool secure,
   int *fence)
{
   ANativeWindow  *anw = (ANativeWindow*) nativeWindow;
   ANativeWindowBuffer_t  *buffer;
   BEGL_Error err = BEGL_Fail;
   int res;
   *nativeBackBuffer = NULL;

   if (anw == NULL)
   {
      ALOGE("%s anw=%p", __FUNCTION__, anw);
      return err;
   }

   if (!isAndroidNativeWindow(anw))
      return err;

   do
   {
      res = anw->dequeueBuffer(anw, &buffer, fence);
      if (res != 0)
      {
         /* android can legitimately get a EBUSY, retry again */
         ALOGE("%s anw=%p dequeueBuffer error=%d", __FUNCTION__, anw, res);
         usleep(1);
      }
   }
   while (res == -EBUSY);

   if (res != 0)
      return err;

   MaybeRemoveFence(fence);

   *nativeBackBuffer = buffer;
   return  BEGL_Success;
}

static BEGL_Error DispDisplaySurface(void *context, void *nativeWindow,
      void *nativeBackBuffer, int fence, int interval)
{
   BSTD_UNUSED(interval);
   ANativeWindow  *anw = (ANativeWindow*) nativeWindow;
   ANativeWindowBuffer_t  *buffer = (ANativeWindowBuffer_t*) nativeBackBuffer;
   BEGL_Error err = BEGL_Fail;

   if (anw == NULL || buffer == NULL)
   {
      ALOGE("%s anw=%p buffer=%p", __FUNCTION__, anw, buffer);
      return err;
   }

   if (!isAndroidNativeWindow(anw))
      return err;

   MaybeRemoveFence(&fence);

   anw->queueBuffer(anw, buffer, fence);

   return  BEGL_Success;
}


static BEGL_Error DispCancelSurface(void *context, void *nativeWindow, void *nativeBackBuffer,
      int fence)
{
   ANativeWindow  *anw = (ANativeWindow*) nativeWindow;
   ANativeWindowBuffer_t  *buffer = (ANativeWindowBuffer_t*) nativeBackBuffer;
   BEGL_Error err = BEGL_Fail;

   if (anw == NULL || buffer == NULL)
   {
      ALOGE("%s anw=%p buffer=%p", __FUNCTION__, anw, buffer);
      return err;
   }

   if (!isAndroidNativeWindow(anw))
      return err;

   MaybeRemoveFence(&fence);

   anw->cancelBuffer(anw, buffer, fence);

   return BEGL_Success;
}

static bool  DisplayPlatformSupported(void *context, uint32_t platform)
{
   BSTD_UNUSED(context);
   return platform == EGL_PLATFORM_ANDROID_KHR;
}

bool DispSetDefaultDisplay(void *context, void *display)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(display);
   return true;
}

void *GetDefaultDisplay(void *context)
{
   return (void *)1;
}

static void *DispWindowStateCreate(void *context, void *nativeWindow)
{
   ANativeWindow *anw = (ANativeWindow*) nativeWindow;
   unsigned int   mub = 1;

   if (!anw)
      NULL;

   if (!isAndroidNativeWindow(anw))
      return NULL;

   if (anw->query(anw, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, (int *)&mub) != 0)
   {
      ALOGE("Unable to query NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS\n");
   }
   else
   {
      DBGLOG("==========================================================================");
      ALOGE("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS = %d", mub);
      DBGLOG("==========================================================================");
   }

   if (native_window_set_buffer_count(anw, MAX_DEQUEUE_BUFFERS + mub) != 0)
   {
      ALOGE("DispBufferGet(): Unable to set the number of buffers to %d", (MAX_DEQUEUE_BUFFERS + mub));
   }

   /* surface texture - force to 1 */
   anw->setSwapInterval(anw, 1);

   anw->common.incRef(&anw->common);
   return anw;
}


static BEGL_Error DispWindowStateDestroy(void *context, void *nativeWindow)
{
   ANativeWindow *anw = (ANativeWindow*) nativeWindow;

   if (!anw)
      return BEGL_Success;

   if (!isAndroidNativeWindow(anw))
      return BEGL_Fail;

   anw->common.decRef(&anw->common);
   return BEGL_Success;
}

static const char *GetClientExtensions(void *context)
{
   BSTD_UNUSED(context);
   return "EGL_KHR_platform_android";
}

#define EXTS_WITHOUT_FENCE_SYNC \
   "EGL_ANDROID_framebuffer_target " \
   "EGL_ANDROID_recordable"

#define EXTS_ALL \
   "EGL_ANDROID_native_fence_sync " \
   EXTS_WITHOUT_FENCE_SYNC

static const char *GetDisplayExtensions(void *context)
{
   BSTD_UNUSED(context);
   return s_expose_fences ? EXTS_ALL : EXTS_WITHOUT_FENCE_SYNC;
}

BEGL_DisplayInterface *CreateAndroidDisplayInterface(BEGL_SchedInterface *schedIface)
{
   BEGL_DisplayInterface *disp;

   disp = calloc(1, sizeof(*disp));

   if (disp == NULL)
      return NULL;

   s_expose_fences = property_get_bool("ro.v3d.fence.expose", false);
   ALOGE("%s v3d expose fences to other modules %s", __FUNCTION__,
         s_expose_fences ? "on": "off");

   disp->context = NULL;
   disp->WindowGetInfo              = DispWindowGetInfo;
   disp->GetNativeSurface           = DispGetNativeSurface;
   disp->SurfaceGetInfo             = DispSurfaceGetInfo;
   disp->SurfaceChangeRefCount      = DispSurfaceChangeRefCount;
   disp->GetNextSurface             = DispGetNextSurface;
   disp->DisplaySurface             = DispDisplaySurface;
   disp->CancelSurface              = DispCancelSurface;
   disp->PlatformSupported          = DisplayPlatformSupported;
   disp->SetDefaultDisplay          = DispSetDefaultDisplay;
   disp->WindowPlatformStateCreate  = DispWindowStateCreate;
   disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
   disp->GetNativeFormat            = DispGetNativeFormat;
   disp->GetClientExtensions        = GetClientExtensions;
   disp->GetDisplayExtensions       = GetDisplayExtensions;

   return disp;
}

void DestroyAndroidDisplayInterface(BEGL_DisplayInterface *disp)
{
   free(disp);
}
