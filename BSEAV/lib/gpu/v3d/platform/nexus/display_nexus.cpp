/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <memory>
#include <cassert>
#include <list>
#include <atomic>
#include <cstring>
#include <cstdio>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/begl_platform.h>

#include "bitmap.h"
#include "display_priv.h"
#include "worker.h"
#include "windowstate.h"
#include "windowinfo.h"
#include "../helpers/extent.h"
#include "../common/nexus_begl_format.h"
#include "../common/nexus_surface_memory.h"

#define MAX_SWAP_BUFFERS 3

enum
{
   NATIVE_WINDOW_INFO_MAGIC = 0xABBA601D,
   PIXMAP_INFO_MAGIC = 0x15EEB1A5
};

static BEGL_Error DispDefaultOrientation(void *context __attribute__((unused)))
{
   /* return BEGL_Success to remove any orientation flags and reset to bottom up rasterization */
   return BEGL_Fail;
}

static BEGL_SwapchainBuffer DispBufferDequeue(void *context, void *platformState,
      BEGL_BufferFormat format, int *fd)
{
   auto data = static_cast<NXPL_Display *>(context);
   assert(data != NULL);

   auto windowState = static_cast<nxpl::WindowState *>(platformState);

   uint64_t before = PerfGetTimeNow();

   auto nw = static_cast<nxpl::NativeWindowInfo*>(windowState->GetWindowHandle());
   auto windowExtent = nw->GetExtent2D();

   helper::Extent2D bitmapExtent;

   auto bitmap = windowState->PopFreeQ();
   if (bitmap)
      bitmapExtent = bitmap->GetExtent2D();

   // resize or create
   if (windowExtent != bitmapExtent)
   {
      //printf("create surface %d %d\n", windowExtent.GetWidth(), windowExtent.GetHeight());

      std::unique_ptr<nxpl::Bitmap> tmp(
            new nxpl::Bitmap(context, windowExtent.GetWidth(),
                  windowExtent.GetHeight(), format, windowState->IsSecure()));
      bitmap = std::move(tmp);
   }

   // not needed on win32
   *fd = -1;

   // m_info contains other presentation info other than size, so update
   bitmap->UpdateWindowInfo(*nw);

   auto buffer = bitmap.release();

   static uint32_t eventID = 0;
   NEXUS_SurfaceHandle surface = buffer->GetSurface();

   PerfAddEventWithTime(data->eventContext, PERF_EVENT_TRACK_QUEUE, PERF_EVENT_DEQUEUE, eventID,
                        BCM_EVENT_BEGIN, before, (uintptr_t)surface, *fd);
   PerfAddEvent(data->eventContext, PERF_EVENT_TRACK_QUEUE, PERF_EVENT_DEQUEUE, eventID++,
                BCM_EVENT_END, (uintptr_t)surface, *fd);

   return static_cast<BEGL_SwapchainBuffer>(buffer);
}

static BEGL_Error DispBufferQueue(void *context, void *platformState, BEGL_SwapchainBuffer buffer, int swap_interval, int fd)
{
   if (buffer != NULL)
   {
      auto data = static_cast<NXPL_Display *>(context);
      auto windowState = static_cast<nxpl::WindowState *>(platformState);
      std::unique_ptr<nxpl::Bitmap> bitmap(static_cast<nxpl::Bitmap*>(buffer));

      static uint32_t eventID = 0;
      NEXUS_SurfaceHandle surface = bitmap->GetSurface();

      PerfAddEvent(data->eventContext, PERF_EVENT_TRACK_QUEUE, PERF_EVENT_QUEUE, eventID, BCM_EVENT_BEGIN,
                   (uintptr_t)surface, (int32_t)fd, (int32_t)swap_interval);

      std::unique_ptr<helper::Semaphore> fence;
      if (fd != -1)
         fence = std::move(std::unique_ptr<helper::Semaphore>(static_cast<helper::Semaphore*>(data->hwInterface->FenceGet(data->hwInterface->context, fd))));
      else
         fence = std::move(std::unique_ptr<helper::Semaphore>(new helper::Semaphore(1)));

      std::unique_ptr<nxpl::DispItem> dispItem(new nxpl::DispItem(std::move(bitmap), std::move(fence), swap_interval));
      windowState->PushDispQ(std::move(dispItem));

      PerfAddEvent(data->eventContext, PERF_EVENT_TRACK_QUEUE, PERF_EVENT_QUEUE, eventID++, BCM_EVENT_END,
                    (uintptr_t)surface, (int32_t)fd, (int32_t)swap_interval);
   }

   return BEGL_Success;
}

static BEGL_Error DispBufferCancel(void *context __attribute__((unused)), void *platformState, BEGL_SwapchainBuffer buffer, int fd __attribute__((unused)))
{
   if (buffer != NULL)
   {
      auto windowState = static_cast<nxpl::WindowState *>(platformState);
      std::unique_ptr<nxpl::Bitmap> bitmap(static_cast<nxpl::Bitmap*>(buffer));

      windowState->PushFreeQ(std::move(bitmap));
   }

   return BEGL_Success;
}

static void *DispWindowStateCreate(void *context, BEGL_WindowHandle window, bool secure)
{
   unsigned buffers = MAX_SWAP_BUFFERS;
   auto val = getenv("V3D_DOUBLE_BUFFER");
   if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
      buffers = 2;

   std::unique_ptr<nxpl::WindowState> windowState(new nxpl::WindowState(context, window, secure));

   if (windowState)
   {
      if (!windowState->Init(context, buffers))
      {
         printf("failed to initialize window surface\n");
         exit(0);
      }
   }

   return static_cast<void*>(windowState.release());
}

static BEGL_Error DispWindowStateDestroy(void *context __attribute__((unused)), void *swapChainCtx)
{
   auto windowState = static_cast<nxpl::WindowState *>(swapChainCtx);

   delete windowState;
   return BEGL_Success;
}

static BEGL_NativeBuffer DispAcquireNativeBuffer(void *context __attribute__((unused)),
   uint32_t target,
   void *eglObject,
   BEGL_BufferSettings *settings)
{
   switch (target)
   {
   case BEGL_SWAPCHAIN_BUFFER:
      return (BEGL_NativeBuffer)AcquireNexusSurfaceMemory(
            static_cast<nxpl::Bitmap*>(eglObject)->GetSurface(), settings);
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_PIXMAP_BUFFER:
      return (BEGL_NativeBuffer)AcquireNexusSurfaceMemory(
            static_cast<NEXUS_SurfaceHandle>(eglObject), settings);
   default:
      return NULL;
   }
}

static BEGL_Error DispReleaseNativeBuffer(void *context __attribute__((unused)),
   uint32_t target,
   BEGL_NativeBuffer buffer)
{
   switch (target)
   {
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_SWAPCHAIN_BUFFER:
   case BEGL_PIXMAP_BUFFER:
      ReleaseNexusSurfaceMemory(static_cast<NEXUS_MemoryBlockHandle>(buffer));
      return BEGL_Success;
   default:
      return BEGL_Fail;
   }
}

typedef struct NXPL_DisplayInterface : BEGL_DisplayInterface
{
   NXPL_Display data;
} NXPL_DisplayInterface;

extern "C" BEGL_DisplayInterface *NXPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
   BEGL_HWInterface     *hwIface,
   NEXUS_DISPLAYHANDLE display,
   EventContext *eventContext)
{
   std::unique_ptr<NXPL_DisplayInterface> disp(new NXPL_DisplayInterface());
   if (!disp)
      return NULL;

   disp->context = &disp->data;
   disp->BufferDequeue = DispBufferDequeue;
   disp->BufferQueue = DispBufferQueue;
   disp->BufferCancel = DispBufferCancel;
   disp->DefaultOrientation = DispDefaultOrientation;
   disp->WindowPlatformStateCreate = DispWindowStateCreate;
   disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
   disp->AcquireNativeBuffer = DispAcquireNativeBuffer;
   disp->ReleaseNativeBuffer = DispReleaseNativeBuffer;

   disp->data.memInterface = memIface;
   disp->data.hwInterface = hwIface;
   disp->data.display = display;
   disp->data.eventContext = eventContext;

   BEGL_HWInfo info;
   disp->data.hwInterface->GetInfo(disp->data.hwInterface->context, &info);
   if (info.techRev < 3)
   {
      NEXUS_MemoryStatus memStatus;
      NEXUS_Heap_GetStatus(NXPL_MemHeap(disp->data.memInterface), &memStatus);
      auto ttstart = memStatus.offset >> 30;
      auto ttend = (memStatus.offset + memStatus.size - 1) >> 30;
      if (ttstart != ttend)
      {
         printf("\n\nNXPL : NXPL_CreateDisplayInterface() ERROR.\nThe Heap you have selected in your platform layer straddles a 1GB boundary\n"
                "Start 0x%llX, Size 0x%x\n", memStatus.offset, memStatus.size);
         goto error;
      }
   }

   return disp.release();

error:
   return NULL;
}

extern "C" void NXPL_DestroyDisplayInterface(BEGL_DisplayInterface *p)
{
   auto disp = std::unique_ptr<NXPL_DisplayInterface>(static_cast<NXPL_DisplayInterface *>(p));
}

extern "C" void NXPL_GetDefaultPixmapInfoEXT(BEGL_PixmapInfoEXT *info)
{
   if (info != NULL)
   {
      memset(info, 0, sizeof(BEGL_PixmapInfoEXT));

      info->format = BEGL_BufferFormat_INVALID;
      info->magic = PIXMAP_INFO_MAGIC;
   }
}

extern "C" bool NXPL_CreateCompatiblePixmapEXT(NXPL_PlatformHandle handle,
   void **pixmapHandle, NEXUS_SurfaceHandle *surface,
   BEGL_PixmapInfoEXT *info)
{
   auto data = static_cast<BEGL_DriverInterfaces*>(handle);

   assert(info->magic == PIXMAP_INFO_MAGIC);

   if (surface == NULL)
      return false;

   NEXUS_SurfaceCreateSettings surfSettings;
   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);

   surfSettings.pixelFormat = BeglToNexusFormat(info->format);
   if (surfSettings.pixelFormat == NEXUS_PixelFormat_eUnknown)
   {
      printf("%s: incompatible format!\n", __FUNCTION__);
      return false;
   }

   surfSettings.heap = info->secure ?
         NXPL_MemHeapSecure(data->memInterface) :
         NXPL_MemHeap(data->memInterface);
   if (!surfSettings.heap)
   {
      printf("%s: no heap!\n", __FUNCTION__);
      return false;
   }

   surfSettings.width = info->width;
   surfSettings.height = info->height;
   surfSettings.compatibility.graphicsv3d = true;
   NEXUS_SurfaceHandle nexusSurface = NEXUS_Surface_Create(&surfSettings);
   if (!nexusSurface)
   {
      printf("%s: could not create compatible surface!\n", __FUNCTION__);
      return false;
   }

   *pixmapHandle = nexusSurface;
   *surface = nexusSurface;
   return true;
}

extern "C" void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle /*handle*/, void *pixmapHandle)
{
   auto nexusSurface = static_cast<NEXUS_SurfaceHandle>(pixmapHandle);
   if (nexusSurface)
      NEXUS_Surface_Destroy(nexusSurface);
}

extern "C" void NXPL_GetDefaultNativeWindowInfoEXT(NXPL_NativeWindowInfoEXT *info)
{
   if (info != NULL)
   {
      memset(info, 0, sizeof(NXPL_NativeWindowInfoEXT));
      static NEXUS_BlendEquation colorBlend = { NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eSourceAlpha, false, NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eInverseSourceAlpha, false, NEXUS_BlendFactor_eZero };
      static NEXUS_BlendEquation alphaBlend = { NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eDestinationAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero };
      info->colorBlend = colorBlend;
      info->alphaBlend = alphaBlend;
      info->type = NXPL_2D;
      info->magic = NATIVE_WINDOW_INFO_MAGIC;
   }
}

extern "C" void *NXPL_CreateNativeWindowEXT(const NXPL_NativeWindowInfoEXT *info)
{
   if (info == NULL)
      return NULL;

   assert(info->magic == NATIVE_WINDOW_INFO_MAGIC);

   std::unique_ptr<nxpl::NativeWindowInfo> nw(new nxpl::NativeWindowInfo(info));
   if (!nw)
      return NULL;

   if (!nw->Init())
      return NULL;

   return static_cast<void*>(nw.release());
}

extern "C" void *NXPL_CreateNativeWindow(const NXPL_NativeWindowInfo *info)
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

extern "C" void NXPL_UpdateNativeWindowEXT(void *native, const NXPL_NativeWindowInfoEXT *info)
{
   auto nw = static_cast<nxpl::NativeWindowInfo*>(native);

   if (info && nw)
      nw->UpdateNativeWindowInfo(info);
}

extern "C" void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info)
{
   /* deprecated API */
   auto nw = static_cast<nxpl::NativeWindowInfo*>(native);

   if (info && nw)
      nw->UpdateNativeWindowInfo(info);
}

extern "C" void NXPL_UpdateNativeWindowDisplayType(void *native, NXPL_DisplayType type)
{
   /* deprecated API */
   auto nw = static_cast<nxpl::NativeWindowInfo*>(native);

   if (nw)
      nw->UpdateNativeWindowInfo(type);
}

extern "C" void NXPL_DestroyNativeWindow(void *native)
{
   auto nw = static_cast<nxpl::NativeWindowInfo*>(native);

   if (nw)
      nw->Term();
   delete nw;
}

extern "C" NEXUS_SURFACECLIENTHANDLE NXPL_CreateVideoWindowClient(void *native __attribute__((unused)), unsigned windowId __attribute__((unused)))
{
   NEXUS_SURFACECLIENTHANDLE ret = NULL;
#ifndef SINGLE_PROCESS
   auto nw = static_cast<nxpl::NativeWindowInfo*>(native);

   if (nw && nw->GetSurfaceClient())
      ret = NEXUS_SurfaceClient_AcquireVideoWindow(nw->GetSurfaceClient(), windowId);
#endif
   return ret;
}

extern "C" void NXPL_ReleaseVideoWindowClient(NEXUS_SURFACECLIENTHANDLE handle __attribute__((unused)))
{
#ifndef SINGLE_PROCESS
   NEXUS_SurfaceClient_ReleaseVideoWindow(handle);
#endif
}

extern "C" uint32_t NXPL_GetClientID(void *native)
{
   auto nw = static_cast<nxpl::NativeWindowInfo*>(native);

   uint32_t ret = 0;
   if (nw)
      ret = nw->GetClientID();
   return ret;
}
