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

#include "display_helpers.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <gralloc_priv.h>

#define MAX_DEQUEUE_BUFFERS   2

/* Set this to get a lot of debug log info */
#define VERBOSE_LOGGING 1

#if VERBOSE_LOGGING
#define DBGLOG(...)   ((void)ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#else
#define DBGLOG(...)   ((void)0)
#endif

static bool s_expose_fences = false;

static bool AndroidToBeglFormat(BEGL_BufferFormat *result, int androidFormat, unsigned sandBits)
{
   bool  ok = true;

   switch (androidFormat)
   {
   default:                           *result = BEGL_BufferFormat_INVALID; ok = false; break;
   case HAL_PIXEL_FORMAT_RGBA_8888:   *result = BEGL_BufferFormat_eA8B8G8R8;           break;
   case HAL_PIXEL_FORMAT_RGBX_8888:   *result = BEGL_BufferFormat_eX8B8G8R8;           break;
   case HAL_PIXEL_FORMAT_RGB_888:     *result = BEGL_BufferFormat_eX8B8G8R8;           break;
   case HAL_PIXEL_FORMAT_RGB_565:     *result = BEGL_BufferFormat_eR5G6B5;             break;
   case HAL_PIXEL_FORMAT_YV12:
      switch (sandBits)
      {
      case 0:  *result = BEGL_BufferFormat_eYV12;   break;
      case 8:  *result = BEGL_BufferFormat_eSAND8;  break;
      case 10: *result = BEGL_BufferFormat_eSAND10; break;
      default: ok = false; break;
      }
      break;
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
   case BEGL_BufferFormat_eSAND8    : *androidFormat = HAL_PIXEL_FORMAT_YV12;         break;
   case BEGL_BufferFormat_eSAND10   : *androidFormat = HAL_PIXEL_FORMAT_YV12;         break;
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
      ALOGE("%s INVALID WINDOW win=%p, magic=%d, version=%d", __FUNCTION__,
         win, win?win->common.magic:-1, win?win->common.version:-1);
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
      ALOGE("%s INVALID BUFFER buffer=%p, magic=%d, version=%d", __FUNCTION__,
         buf, buf?buf->common.magic:-1, buf?buf->common.version:-1);
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

   if (!isAndroidNativeWindow(anw)) {
      ALOGE("%s anw=%p, not native window", __FUNCTION__, anw);
      return BEGL_Fail;
   }

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

   if (flags & BEGL_WindowInfoBackBufferAge)
   {
      int res = anw->query(anw, NATIVE_WINDOW_BUFFER_AGE, (int *)&info->backBufferAge);
      if (res == -ENODEV)
         assert(0);
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

static void GetBlockHandle(private_handle_t const *hnd, NEXUS_MemoryBlockHandle *block)
{
   int rc;
   struct nx_ashmem_getmem getmem;

   if (block != NULL)
      *block = 0;

   if (hnd == NULL || block == NULL)
      return;

   if (hnd->sdata >= 0)
   {
      rc = ioctl(hnd->sdata, NX_ASHMEM_GET_BLK, &getmem);
      if (rc >= 0)
         *block = (NEXUS_MemoryBlockHandle)getmem.hdl;
      else
      {
         int err = errno;
         ALOGE("GetBlockHandle(fd:%d)::fail:%d::errno=%d", hnd->sdata, rc, err);
         return;
      }
   }
}

static BEGL_Colorimetry NexusToBEGLColorPrimaries(NEXUS_MatrixCoefficients nmc)
{
   switch (nmc)
   {
   case NEXUS_MatrixCoefficients_eHdmi_RGB              : return BEGL_Colorimetry_RGB;
   case NEXUS_MatrixCoefficients_eItu_R_BT_709          : return BEGL_Colorimetry_BT_709;
   case NEXUS_MatrixCoefficients_eUnknown               : return BEGL_Colorimetry_Unknown;
   case NEXUS_MatrixCoefficients_eDvi_Full_Range_RGB    : return BEGL_Colorimetry_Dvi_Full_Range_RGB;
   case NEXUS_MatrixCoefficients_eFCC                   : return BEGL_Colorimetry_FCC;
   case NEXUS_MatrixCoefficients_eItu_R_BT_470_2_BG     : return BEGL_Colorimetry_BT_470_2_BG;
   case NEXUS_MatrixCoefficients_eSmpte_170M            : return BEGL_Colorimetry_Smpte_170M;
   case NEXUS_MatrixCoefficients_eSmpte_240M            : return BEGL_Colorimetry_Smpte_240M;
   case NEXUS_MatrixCoefficients_eXvYCC_709             : return BEGL_Colorimetry_XvYCC_709;
   case NEXUS_MatrixCoefficients_eXvYCC_601             : return BEGL_Colorimetry_XvYCC_601;
   case NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL     : return BEGL_Colorimetry_BT_2020_NCL;
   case NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL      : return BEGL_Colorimetry_BT_2020_CL;
   case NEXUS_MatrixCoefficients_eHdmi_Full_Range_YCbCr : return BEGL_Colorimetry_Hdmi_Full_Range_YCbCr;
   default                                              : return BEGL_Colorimetry_XvYCC_601;
   }
}

static BEGL_Error DispSurfaceGetInfo(void *context, void *nativeSurface, BEGL_SurfaceInfo *info)
{
   ANativeWindowBuffer_t  *buffer = (ANativeWindowBuffer_t*)nativeSurface;
   bool                    ok = false;
   private_handle_t const *hnd;
   PSHARED_DATA            pSharedData;
   NEXUS_MemoryBlockHandle sharedBlockHandle = NULL;
   void                   *pMemory;

   if (!isAndroidNativeBuffer(buffer) || info == NULL) {
      ALOGE("%s buf=%p hnd=%p info=%p, not native buffer",
         __FUNCTION__, buffer, buffer?buffer->handle:NULL, info);
      return BEGL_Fail;
   }

   hnd = (private_handle_t const*)buffer->handle;

   GetBlockHandle(hnd, &sharedBlockHandle);
   if (sharedBlockHandle == NULL)
      return BEGL_Fail;
   NEXUS_MemoryBlock_Lock(sharedBlockHandle, &pMemory);
   if (pMemory == NULL)
      return BEGL_Fail;
   pSharedData = (PSHARED_DATA)pMemory;

   AndroidToBeglFormat(&info->format, buffer->format, pSharedData->container.vDepth);

   info->width     = buffer->width;
   info->height    = buffer->height;
   info->miplevels = 1;
   info->colorimetry = BEGL_Colorimetry_RGB;

   if (info->format == BEGL_BufferFormat_eSAND8 || info->format == BEGL_BufferFormat_eSAND10)
   {
      uint32_t byteWidth = pSharedData->container.vsWidth *
         (pSharedData->container.vImageWidth + pSharedData->container.vsWidth - 1) / pSharedData->container.vsWidth;

      assert(info->width  == pSharedData->container.vImageWidth);
      assert(info->height == pSharedData->container.vImageHeight);

      info->contiguous     = true;
      info->pitchBytes     = 0;
      info->physicalOffset = pSharedData->container.vLumaAddr + pSharedData->container.vLumaOffset;
      info->chromaOffset   = pSharedData->container.vChromaAddr + pSharedData->container.vChromaOffset;
      info->cachedAddr     = NULL;   // Sand video can't be mapped
      info->secure         = false;  // TODO : if Android supports secure video
      info->chromaByteSize = byteWidth * pSharedData->container.vsChromaHeight;
      info->stripeWidth    = pSharedData->container.vsWidth;
      info->lumaStripedHeight   = pSharedData->container.vsLumaHeight;
      info->chromaStripedHeight = pSharedData->container.vsChromaHeight;
      info->lumaAndChromaInSameAllocation = pSharedData->container.vLumaBlock == pSharedData->container.vChromaBlock;
      info->colorimetry    = NexusToBEGLColorPrimaries((NEXUS_MatrixCoefficients)pSharedData->container.vColorSpace);
      if (info->lumaAndChromaInSameAllocation)
      {
         // byteSize represents the combined luma/chroma buffer when lumaAndChromaInSameAllocation
         info->byteSize = info->chromaOffset - info->physicalOffset + info->chromaByteSize;
      }
      else
         info->byteSize = byteWidth * pSharedData->container.vsLumaHeight;

      DBGLOG("[sand2tex][NB]:gr:%p::%ux%u::l:%" PRIx64 "::lo:%x:%p::c:%" PRIx64 ":co:%x:%p::%d,%d,%d::%d-bit::%d",
         hnd,
         pSharedData->container.vImageWidth,
         pSharedData->container.vImageHeight,
         pSharedData->container.vLumaAddr,
         pSharedData->container.vLumaOffset,
         pSharedData->container.vLumaBlock,
         pSharedData->container.vChromaAddr,
         pSharedData->container.vChromaOffset,
         pSharedData->container.vChromaBlock,
         pSharedData->container.vsWidth,
         pSharedData->container.vsLumaHeight,
         pSharedData->container.vsChromaHeight,
         pSharedData->container.vDepth,
         pSharedData->container.vColorSpace);

      ok = true;
   }
   else
   {
      info->contiguous     = true;
      info->physicalOffset = hnd->nxSurfacePhysicalAddress;
      info->cachedAddr     = (void*)(intptr_t)hnd->nxSurfaceAddress;
      info->byteSize       = hnd->oglSize;
      info->pitchBytes     = hnd->oglStride;
   }

   NEXUS_MemoryBlock_Unlock(sharedBlockHandle);

   return BEGL_Success;
}

static BEGL_Error DispSurfaceChangeRefCount(void *context, void *nativeBackBuffer, BEGL_RefCountMode incOrDec)
{
   ANativeWindowBuffer_t *buffer = (ANativeWindowBuffer_t*)nativeBackBuffer;

   if (!isAndroidNativeBuffer(buffer)) {
      ALOGE("%s buf=%p, not native buffer", __FUNCTION__, buffer);
      return BEGL_Fail;
   }

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

   if (!isAndroidNativeWindow(anw)) {
      ALOGE("%s anw=%p, not native window", __FUNCTION__, anw);
      return err;
   }

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

   if (!isAndroidNativeWindow(anw)) {
      ALOGE("%s anw=%p buffer=%p, not native window", __FUNCTION__, anw, buffer);
      return err;
   }

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

   if (!isAndroidNativeWindow(anw)) {
      ALOGE("%s anw=%p buffer=%p, not native window", __FUNCTION__, anw, buffer);
      return err;
   }

   MaybeRemoveFence(&fence);

   anw->cancelBuffer(anw, buffer, fence);

   return BEGL_Success;
}


static void *DispWindowStateCreate(void *context, void *nativeWindow)
{
   ANativeWindow *anw = (ANativeWindow*) nativeWindow;
   unsigned int   mub = 1;

   if (!anw)
      NULL;

   if (!isAndroidNativeWindow(anw)) {
      ALOGE("%s anw=%p, not native window", __FUNCTION__, anw);
      return NULL;
   }

   if (anw->query(anw, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, (int *)&mub) != 0)
   {
      ALOGE("Unable to query NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS\n");
   }
   else
   {
      DBGLOG("==========================================================================");
      DBGLOG("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS = %d", mub);
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

   if (!isAndroidNativeWindow(anw)) {
      ALOGE("%s anw=%p, not native window", __FUNCTION__, anw);
      return BEGL_Fail;
   }

   anw->common.decRef(&anw->common);
   return BEGL_Success;
}

#define EXTS_WITHOUT_FENCE_SYNC \
   "EGL_ANDROID_framebuffer_target " \
   "EGL_ANDROID_recordable " \
   "EGL_ANDROID_image_native_buffer"

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
   ALOGV("%s v3d expose fences to other modules %s", __FUNCTION__,
         s_expose_fences ? "on": "off");

   disp->context                    = schedIface;
   disp->WindowGetInfo              = DispWindowGetInfo;
   disp->GetNativeSurface           = DispGetNativeSurface;
   disp->SurfaceGetInfo             = DispSurfaceGetInfo;
   disp->SurfaceChangeRefCount      = DispSurfaceChangeRefCount;
   disp->GetNextSurface             = DispGetNextSurface;
   disp->DisplaySurface             = DispDisplaySurface;
   disp->CancelSurface              = DispCancelSurface;
   disp->WindowPlatformStateCreate  = DispWindowStateCreate;
   disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
   disp->GetNativeFormat            = DispGetNativeFormat;
   disp->GetDisplayExtensions       = GetDisplayExtensions;

   return disp;
}

void DestroyAndroidDisplayInterface(BEGL_DisplayInterface *disp)
{
   free(disp);
}

bool DisplayAcquireNexusSurfaceHandles(NEXUS_StripedSurfaceHandle *stripedSurf, NEXUS_SurfaceHandle *surf,
                                       void *nativeSurface)
{
   ANativeWindowBuffer_t     *srcBuf = (ANativeWindowBuffer_t*)nativeSurface;
   private_handle_t const    *srcHnd;
   NEXUS_MemoryBlockHandle    sharedBlockHandle = NULL;
   PSHARED_DATA               pSharedData = NULL;
   void                      *pMemory = NULL;
   bool                       result = false;

   *stripedSurf = NULL;
   *surf        = NULL; // We only need striped source support in Android right now, so this stays NULL

   if (nativeSurface == NULL || !isAndroidNativeBuffer(srcBuf)) {
      ALOGE("%s buf=%p hnd=%p, not native buffer",
         __FUNCTION__, srcBuf, srcBuf?srcBuf->handle:NULL);
      goto error;
   }

   srcHnd = (private_handle_t const*)srcBuf->handle;

   GetBlockHandle(srcHnd, &sharedBlockHandle);
   if (sharedBlockHandle == NULL)
      goto error;
   NEXUS_MemoryBlock_Lock(sharedBlockHandle, &pMemory);
   if (pMemory == NULL)
      goto error;

   pSharedData = (PSHARED_DATA)pMemory;

   BEGL_BufferFormat srcFormat;
   AndroidToBeglFormat(&srcFormat, srcBuf->format, pSharedData->container.vDepth);
   if (srcFormat != BEGL_BufferFormat_eSAND8 && srcFormat != BEGL_BufferFormat_eSAND10)
      goto error;

   // Wrap the source as a NEXUS_StripedSurface
   NEXUS_StripedSurfaceCreateSettings   sscs;
   NEXUS_StripedSurface_GetDefaultCreateSettings(&sscs);

   sscs.imageWidth          = srcBuf->width;
   sscs.imageHeight         = srcBuf->height;
   sscs.stripedWidth        = pSharedData->container.vsWidth;
   sscs.lumaPixelFormat     = srcFormat == BEGL_BufferFormat_eSAND8 ? NEXUS_PixelFormat_eY8 : NEXUS_PixelFormat_eY10;
   sscs.chromaPixelFormat   = srcFormat == BEGL_BufferFormat_eSAND8 ? NEXUS_PixelFormat_eCb8_Cr8 : NEXUS_PixelFormat_eCb10_Cr10;
   sscs.bufferType          = NEXUS_VideoBufferType_eFrame;
   sscs.lumaStripedHeight   = pSharedData->container.vsLumaHeight;
   sscs.chromaStripedHeight = pSharedData->container.vsChromaHeight;
   sscs.lumaBuffer          = pSharedData->container.vLumaBlock;
   sscs.chromaBuffer        = pSharedData->container.vChromaBlock;
   sscs.lumaBufferOffset    = pSharedData->container.vLumaOffset;
   sscs.chromaBufferOffset  = pSharedData->container.vChromaOffset;

   DBGLOG("[sand2tex][SS]:gr:%p::%ux%u::l:%" PRIx64 "::lo:%x:%p::c:%" PRIx64 ":co:%x:%p::%d,%d,%d::%d-bit::%d",
      srcHnd,
      pSharedData->container.vImageWidth,
      pSharedData->container.vImageHeight,
      pSharedData->container.vLumaAddr,
      pSharedData->container.vLumaOffset,
      pSharedData->container.vLumaBlock,
      pSharedData->container.vChromaAddr,
      pSharedData->container.vChromaOffset,
      pSharedData->container.vChromaBlock,
      pSharedData->container.vsWidth,
      pSharedData->container.vsLumaHeight,
      pSharedData->container.vsChromaHeight,
      pSharedData->container.vDepth,
      pSharedData->container.vColorSpace);

   *stripedSurf = NEXUS_StripedSurface_Create(&sscs);
   if (*stripedSurf == NULL)
      goto error;

good:
   result = true;
   goto finish;

error:
   result = false;
   if (*stripedSurf)
      NEXUS_StripedSurface_Destroy(*stripedSurf);

finish:
   if (sharedBlockHandle != NULL)
      NEXUS_MemoryBlock_Unlock(sharedBlockHandle);

   return result;
}

void DisplayReleaseNexusSurfaceHandles(NEXUS_StripedSurfaceHandle stripedSurf, NEXUS_SurfaceHandle surf)
{
   BSTD_UNUSED(surf);
   if (stripedSurf)
      NEXUS_StripedSurface_Destroy(stripedSurf);
}
