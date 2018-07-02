/******************************************************************************
 *  Copyright (C) 2018 Broadcom.
 *  The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *
 *  This program is the proprietary software of Broadcom and/or its licensors,
 *  and may only be used, duplicated, modified or distributed pursuant to
 *  the terms and conditions of a separate, written license agreement executed
 *  between you and Broadcom (an "Authorized License").  Except as set forth in
 *  an Authorized License, Broadcom grants no license (express or implied),
 *  right to use, or waiver of any kind with respect to the Software, and
 *  Broadcom expressly reserves all rights in and to the Software and all
 *  intellectual property rights therein. IF YOU HAVE NO AUTHORIZED LICENSE,
 *  THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY, AND SHOULD
 *  IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE SOFTWARE.
 *
 *  Except as expressly set forth in the Authorized License,
 *
 *  1.     This program, including its structure, sequence and organization,
 *  constitutes the valuable trade secrets of Broadcom, and you shall use all
 *  reasonable efforts to protect the confidentiality thereof, and to use this
 *  information only in connection with your use of Broadcom integrated circuit
 *  products.
 *
 *  2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 *  "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 *  OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *  RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *  IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 *  A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 *  ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME
 *  THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 *  3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM
 *  OR ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 *  INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 *  RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 *  HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 *  EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 *  WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 *  FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
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
#define VERBOSE_LOGGING 0

#if VERBOSE_LOGGING
#define DBGLOG(...)   ((void)ALOG(LOG_VERBOSE, LOG_TAG, __VA_ARGS__))
#else
#define DBGLOG(...)   ((void)0)
#endif

static bool s_expose_fences = false;

static bool AndroidToBeglFormat(BEGL_BufferFormat *result, unsigned plane, int androidFormat, unsigned sandBits)
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
      case 8:
         switch (plane)
         {
         case 0: *result = BEGL_BufferFormat_eY8;     break;
         case 1: *result = BEGL_BufferFormat_eCb8Cr8; break;
         /* TODO: for Pierre: determine Cr-Cb ordering, add BEGL_BufferFormat_eCr8Cb8 */
         default: ok = false; break;
         }
         break;
      case 10:
         switch (plane)
         {
         case 0: *result = BEGL_BufferFormat_eY10;     break;
         case 1: *result = BEGL_BufferFormat_eCb10Cr10; break;
         /* TODO: for Pierre: determine Cr-Cb ordering, add BEGL_BufferFormat_eCr10Cb10 */
         default: ok = false; break;
         }
         break;
      default: ok = false; break;
      }
      break;
   }

   return ok;
}

static bool BeglToAndroidFormat(int *androidFormat, BEGL_BufferFormat format)
{
   bool  ok = true;

   if (BeglFormatIsSand(format))
   {
      *androidFormat = HAL_PIXEL_FORMAT_YV12;
      return true;
   }

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

   if (!isAndroidNativeWindow(anw))
   {
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

static BEGL_Colorimetry NexusToBEGLColorPrimaries(NEXUS_MatrixCoefficients ncp)
{
   switch (ncp)
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

static BEGL_Error GetInfo(ANativeWindowBuffer_t *buffer, uint32_t plane,
      BEGL_SurfaceInfo *info)
{
   bool                    ok = true;
   private_handle_t const *hnd;
   PSHARED_DATA            pSharedData;
   NEXUS_MemoryBlockHandle sharedBlockHandle = NULL;
   void                   *pMemory;

   hnd = (private_handle_t const*)buffer->handle;

   GetBlockHandle(hnd, &sharedBlockHandle);
   if (sharedBlockHandle == NULL)
      return BEGL_Fail;
   NEXUS_MemoryBlock_Lock(sharedBlockHandle, &pMemory);
   if (pMemory == NULL)
      return BEGL_Fail;
   pSharedData = (PSHARED_DATA)pMemory;

   AndroidToBeglFormat(&info->format, plane, buffer->format, pSharedData->container.vDepth);

   info->width     = buffer->width;
   info->height    = buffer->height;
   info->miplevels = 1;

   if (BeglFormatIsSand(info->format))
      // sand format gets information from decoder through gralloc buffer.
      info->colorimetry = NexusToBEGLColorPrimaries(pSharedData->container.vColorSpace);
   else if (info->format == BEGL_BufferFormat_eYV12 ||
            info->format == BEGL_BufferFormat_eYUV422)
      // native format is likely software decoder, assume BT.601 by default for now.
      info->colorimetry = BEGL_Colorimetry_XvYCC_601;
   else
      info->colorimetry = BEGL_Colorimetry_RGB;

   if (BeglFormatIsSand(info->format))
   {
      uint32_t byteWidth = pSharedData->container.vsWidth *
         (pSharedData->container.vImageWidth + pSharedData->container.vsWidth - 1) / pSharedData->container.vsWidth;

      assert(info->width  == pSharedData->container.vImageWidth);
      assert(info->height == pSharedData->container.vImageHeight);
      assert(plane < 2);

      info->contiguous     = true;
      info->pitchBytes     = 0;
      info->cachedAddr     = NULL;   // Sand video can't be mapped
      info->secure         = false;  // TODO : if Android supports secure video
      info->stripeWidth    = pSharedData->container.vsWidth;
      switch (plane)
      {
      case 0:
         info->stripedHeight  = pSharedData->container.vsLumaHeight;
         info->physicalOffset = pSharedData->container.vLumaAddr + pSharedData->container.vLumaOffset;
         info->byteSize       = byteWidth * pSharedData->container.vsLumaHeight;
         break;
      case 1:
         info->stripedHeight  = pSharedData->container.vsChromaHeight;
         info->physicalOffset = pSharedData->container.vChromaAddr + pSharedData->container.vChromaOffset;
         info->byteSize       = byteWidth * pSharedData->container.vsChromaHeight;
         break;
      default:
         ok = false;
         break;
      }

      if (ok)
      {
         DBGLOG("[sand2tex][NB]:gr:%p::%ux%u::l:%" PRIx64 "::lo:%x:%p::c:%" PRIx64 ":co:%x:%p::%d,%d,%d::%d-bit",
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
            pSharedData->container.vDepth);

         pSharedData->container.vHwTex = 1;
      }
   }
   else
   {
      assert(plane == 0);

      info->contiguous     = true;
      info->physicalOffset = hnd->nxSurfacePhysicalAddress;
      info->cachedAddr     = (void*)(intptr_t)hnd->nxSurfaceAddress;
      info->byteSize       = hnd->oglSize;
      info->pitchBytes     = hnd->oglStride;
   }

   NEXUS_MemoryBlock_Unlock(sharedBlockHandle);

   return ok ? BEGL_Success : BEGL_Fail;
}

static BEGL_NativeBuffer DispSurfaceAcquire(void *context, uint32_t target,
      void *eglObject, uint32_t plane, BEGL_SurfaceInfo *info)
{
   switch (target)
   {
   case EGL_NATIVE_BUFFER_ANDROID:
   case BEGL_SWAPCHAIN_BUFFER:
   {
      ANativeWindowBuffer_t *buffer = (ANativeWindowBuffer_t*)eglObject;
      if (!isAndroidNativeBuffer(buffer))
      {
         ALOGE("%s buf=%p, not native buffer", __FUNCTION__, buffer);
         return NULL;
      }

      if (GetInfo(buffer, plane, info) == BEGL_Fail)
      {
         ALOGE("%s buf=%p, can't get buffer info", __FUNCTION__, buffer);
         return NULL;
      }

      buffer->common.incRef(&buffer->common);
      return (BEGL_NativeBuffer)buffer;
   }
   case BEGL_PIXMAP_BUFFER:
   default:
      ALOGE("%s invalid target", __FUNCTION__);
      return NULL;
   }
}

static BEGL_Error DispSurfaceRelease(void *context, uint32_t target,
      uint32_t plane, BEGL_NativeBuffer nativeBuffer)
{
   switch (target)
   {
   case EGL_NATIVE_BUFFER_ANDROID:
   case BEGL_SWAPCHAIN_BUFFER:
   {
      ANativeWindowBuffer_t *buffer = (ANativeWindowBuffer_t*)nativeBuffer;
      if (!isAndroidNativeBuffer(buffer))
      {
         ALOGE("%s buf=%p, not native buffer", __FUNCTION__, buffer);
         return BEGL_Fail;
      }

      buffer->common.decRef(&buffer->common);
      return BEGL_Success;
   }
   case BEGL_PIXMAP_BUFFER:
   default:
      ALOGE("%s invalid target", __FUNCTION__);
      return BEGL_Fail;
   }
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

static BEGL_SwapchainBuffer DispGetNextSurface(
   void *context,
   void *nativeWindow,
   BEGL_BufferFormat format,
   bool secure,
   int *age,
   int *fence
   )
{
   ANativeWindow  *anw = (ANativeWindow*) nativeWindow;
   ANativeWindowBuffer_t  *buffer;
   int res;

   if (anw == NULL)
   {
      ALOGE("%s anw=%p", __FUNCTION__, anw);
      return NULL;
   }

   if (!isAndroidNativeWindow(anw))
   {
      ALOGE("%s anw=%p, not native window", __FUNCTION__, anw);
      return NULL;
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
      return NULL;

   MaybeRemoveFence(fence);

   res = anw->query(anw, NATIVE_WINDOW_BUFFER_AGE, age);
   if (res == -ENODEV)
      assert(0);

   return (BEGL_SwapchainBuffer)buffer;
}

static BEGL_Error DispDisplaySurface(void *context, void *nativeWindow,
      BEGL_SwapchainBuffer nativeBackBuffer, int fence, int interval)
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
   {
      ALOGE("%s anw=%p buffer=%p, not native window", __FUNCTION__, anw, buffer);
      return err;
   }

   MaybeRemoveFence(&fence);

   anw->queueBuffer(anw, buffer, fence);

   return  BEGL_Success;
}

static BEGL_Error DispCancelSurface(void *context, void *nativeWindow,
      BEGL_SwapchainBuffer nativeBackBuffer, int fence)
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
   {
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

   if (!isAndroidNativeWindow(anw))
   {
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

   if (!isAndroidNativeWindow(anw))
   {
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
   disp->GetPixmapFormat            = NULL; /* Android doesn't have pixmaps */
   disp->SurfaceAcquire             = DispSurfaceAcquire;
   disp->SurfaceRelease             = DispSurfaceRelease;
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
