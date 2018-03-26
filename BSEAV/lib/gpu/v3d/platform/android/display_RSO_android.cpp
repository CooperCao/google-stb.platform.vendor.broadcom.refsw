/******************************************************************************
*  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
******************************************************************************/
#include <memory>
#include <chrono>
#include <thread>

#include <assert.h>

#include "default_RSO_android.h"

#include "nexus_surface.h"
#include "nexus_memory.h"
#include "nexus_platform.h"

#include <cutils/properties.h>
#include <log/log.h>

#define NX_MMA "ro.nx.mma"

#define AN_BUFFERS (3)

typedef struct
{
   BEGL_MemoryInterface          *memInterface;
   BEGL_HWInterface              *hwInterface;
   bool                          useMMA;
} RSOAN_Display;

static bool AndroidToBeglFormat(int androidFormat, BEGL_BufferFormat *result)
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

static bool BeglToAndroidFormat(BEGL_BufferFormat format, int *androidFormat)
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

extern NEXUS_HeapHandle NXPL_MemHeap(BEGL_MemoryInterface *mem);

static int isANativeWindow(ANativeWindow *win)
{
   int res = (win && (win->common.magic == ANDROID_NATIVE_WINDOW_MAGIC)) ? 1 : 0;
   if (!res)
   {
      ALOGD("==========================================================================");
      ALOGD("INVALID WINDOW INVALID WINDOW INVALID WINDOW INVALID WINDOW INVALID WINDOW");
      ALOGD("==========================================================================");
   }
   return res;
}

static int isANativeWindowBuffer(ANativeWindowBuffer *buf)
{
   int res = (buf && (buf->common.magic == ANDROID_NATIVE_BUFFER_MAGIC)) ? 1 : 0;
   if (!res)
   {
      ALOGE("==========================================================================");
      ALOGE("INVALID BUFFER buffer=%p", buf);
      ALOGE("==========================================================================");
   }
   return res;
}

/*****************************************************************************
* Display driver interface
*****************************************************************************/

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window, bool secure [[gnu::unused]])
{
   auto awin = static_cast<ANativeWindow*>(window);

   if (!isANativeWindow(awin))
      return nullptr;

   unsigned int mub = 1;
   if (awin->query(awin, NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS, (int *)&mub) != 0)
      ALOGD("Unable to query NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS");
   else
   {
      ALOGD("==========================================================================");
      ALOGD("NATIVE_WINDOW_MIN_UNDEQUEUED_BUFFERS = %d", mub);
      ALOGD("==========================================================================");
   }

   if (native_window_set_buffer_count(awin, (AN_BUFFERS + mub)) != 0)
      ALOGD("DispBufferGet(): Unable to set the number of buffers to %d", (AN_BUFFERS + mub));

   /* surface texture - force to 1 */
   awin->setSwapInterval(awin, 1);

   awin->common.incRef(&awin->common);

   return static_cast<void*>(awin);
}

static BEGL_Error DispWindowStateDestroy(void *context, void *swapChainCtx)
{
   auto awin = static_cast<ANativeWindow*>(swapChainCtx);

   if (!awin)
      return BEGL_Success;

   if (!isANativeWindow(awin))
      return BEGL_Fail;

   awin->common.decRef(&awin->common);
   return BEGL_Success;
}

static BEGL_Error DispGetNativeFormat(void *context [[gnu::unused]],
   BEGL_BufferFormat bufferFormat,
   uint32_t * nativeFormat)
{
   BEGL_Error res = BEGL_Success;

   if (nativeFormat != nullptr)
   {
      switch (bufferFormat)
      {
      case BEGL_BufferFormat_eA8B8G8R8: *nativeFormat = HAL_PIXEL_FORMAT_RGBA_8888; break;
      case BEGL_BufferFormat_eX8B8G8R8: *nativeFormat = HAL_PIXEL_FORMAT_RGBX_8888; break;
      case BEGL_BufferFormat_eR5G6B5: *nativeFormat = HAL_PIXEL_FORMAT_RGB_565; break;
      default: res = BEGL_Fail; break;
      }
   }

   return res;
}

static BEGL_Error DispSurfaceGetInfo(void *context,
   uint32_t target [[gnu::unused]],
   void *buffer,
   BEGL_BufferSettings *settings)
{
   auto abuf = static_cast<ANativeWindowBuffer*>(buffer);

   if (!isANativeWindowBuffer(abuf) || (settings == nullptr))
      return BEGL_Fail;

   auto hnd = static_cast<private_handle_t const*>(abuf->handle);

   settings->physOffset = hnd->nxSurfacePhysicalAddress;
   settings->cachedAddr = reinterpret_cast<void *>(hnd->nxSurfaceAddress);
   settings->pitchBytes = hnd->oglStride;
   settings->width = abuf->width;
   settings->height = abuf->height;
   AndroidToBeglFormat(abuf->format, &settings->format);

   return BEGL_Success;
}

static BEGL_Error DispSurfaceChangeRefCount(void *context,
   uint32_t target [[gnu::unused]],
   void *buffer, BEGL_RefCountMode incOrDec)
{
   auto abuf = static_cast<ANativeWindowBuffer*>(buffer);

   if (!isANativeWindowBuffer(abuf))
      return BEGL_Fail;

   switch (incOrDec)
   {
   case BEGL_Increment: abuf->common.incRef(&abuf->common); break;
   case BEGL_Decrement: abuf->common.decRef(&abuf->common); break;
   default: assert(0); return BEGL_Fail;
   }
   return BEGL_Success;
}

static BEGL_BufferHandle DispBufferDequeue(void *context, void *platformState, BEGL_BufferFormat format, int *fd)
{
   auto anw = static_cast<ANativeWindow*>(platformState);
   if (anw == NULL)
   {
      ALOGE("%s anw=%p", __FUNCTION__, anw);
      return nullptr;
   }

   if (!isANativeWindow(anw))
      return nullptr;

   ANativeWindowBuffer *buffer = nullptr;
   int res;
   do
   {
      res = anw->dequeueBuffer(anw, &buffer, fd);
      if (res != 0)
      {
         /* android can legitimately get a EBUSY, retry again */
         ALOGE("%s anw=%p dequeueBuffer error=%d", __FUNCTION__, anw, res);
         std::this_thread::sleep_for(std::chrono::microseconds(1));
      }
   }
   while (res == -EBUSY);

   if (res != 0)
      return nullptr;

   return static_cast<BEGL_BufferHandle>(buffer);
}

static BEGL_Error DispBufferQueue(void *context, void *platformState, BEGL_BufferHandle buffer, int swap_interval, int fd)
{
   auto anw = static_cast<ANativeWindow*>(platformState);
   auto *abuf = static_cast<ANativeWindowBuffer*>(buffer);

   if (anw == NULL || abuf == NULL)
   {
      ALOGE("%s anw=%p buffer=%p", __FUNCTION__, anw, abuf);
      return BEGL_Fail;
   }

   if (!isANativeWindow(anw))
      return BEGL_Fail;

   anw->queueBuffer(anw, abuf, fd);

   return BEGL_Success;
}

static BEGL_Error DispBufferCancel(void *context, void *platformState, BEGL_BufferHandle buffer, int fd)
{
   auto anw = static_cast<ANativeWindow*>(platformState);
   auto *abuf = static_cast<ANativeWindowBuffer*>(buffer);

   if (anw == NULL || abuf == NULL)
   {
      ALOGE("%s anw=%p buffer=%p", __FUNCTION__, anw, abuf);
      return BEGL_Fail;
   }

   if (!isANativeWindow(anw))
      return BEGL_Fail;

   anw->cancelBuffer(anw, abuf, fd);

   return BEGL_Success;
}

__attribute__((visibility("default")))
extern "C" BEGL_DisplayInterface *RSOANPL_CreateDisplayInterface(BEGL_MemoryInterface *mem,
   BEGL_HWInterface *hw,
   BEGL_DisplayCallbacks *displayCallbacks)
{
   std::unique_ptr<BEGL_DisplayInterface> disp(new BEGL_DisplayInterface());
   if (disp == nullptr)
      return nullptr;

   std::unique_ptr<RSOAN_Display> data(new RSOAN_Display());
   if (data == nullptr)
      return nullptr;

   /* default off */
   char val[PROPERTY_VALUE_MAX];
   property_get(NX_MMA, val, "0");
   if (val[0] == 't' || val[0] == 'T' || val[0] == '1')
      data->useMMA = true;
   else
      data->useMMA = false;
   data->memInterface = mem;
   data->hwInterface = hw;

   disp->context = static_cast<void*>(data.release());
   disp->BufferDequeue = DispBufferDequeue;
   disp->BufferQueue = DispBufferQueue;
   disp->BufferCancel = DispBufferCancel;
   disp->WindowPlatformStateCreate = DispWindowStateCreate;
   disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
   disp->GetNativeFormat = DispGetNativeFormat;
   disp->SurfaceGetInfo = DispSurfaceGetInfo;
   disp->SurfaceChangeRefCount = DispSurfaceChangeRefCount;

   return disp.release();
}

__attribute__((visibility("default")))
extern "C" void RSOANPL_DestroyDisplayInterface(BEGL_DisplayInterface *disp)
{
   if (disp)
   {
      auto data = static_cast<RSOAN_Display *>(disp->context);
      delete data;
      delete disp;
   }
}

__attribute__((visibility("default")))
extern "C" bool RSOANPL_BufferGetRequirements(RSOANPL_PlatformHandle handle,
   BEGL_PixmapInfoEXT *bufferRequirements,
   BEGL_BufferSettings * bufferConstrainedRequirements)
{
   auto data = static_cast<BEGL_DriverInterfaces*>(handle);

   if (data != nullptr && data->displayCallbacks.BufferGetRequirements != nullptr)
   {
      data->displayCallbacks.BufferGetRequirements(bufferRequirements, bufferConstrainedRequirements);
      return true;
   }

   return false;
}
