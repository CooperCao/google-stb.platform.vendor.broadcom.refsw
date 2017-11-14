/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <memory>
#include <cassert>
#include <list>
#include <atomic>
#include <cstring>
#include <cstdio>

#include "EGL/egl.h"
#include "EGL/eglext_brcm.h"

#include "bitmap.h"
#include "display_priv.h"
#include "worker.h"
#include "windowstate.h"
#include "windowinfo.h"
#include "../helpers/extent.h"

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

/* Request creation of an appropriate display buffer. Only the 3D driver knows the size and alignment constraints, so the
* buffer create request must come from the driver. settings->totalByteSize is the size of the memory that the driver needs.
* We could have just requested a block of memory using the memory interface, but by having the platform layer create a 'buffer'
* it can actually create whatever type it desires directly, and then only have to deal with that type. For example, in a Nexus
* platform layer, this function might be implemented to create a NEXUS_Surface (with the correct memory constraints of course).
* When the buffer handle is passed out during BufferDisplay, the platform layer can simply use it as a NEXUS_Surface. It
* doesn't have to wrap the memory each time, or perform any lookups. Since the buffer handle is opaque to the 3d driver, the
* platform layer has complete freedom. */
static BEGL_BufferHandle DispBufferCreate(void *context, BEGL_PixmapInfoEXT const *bufferRequirements)
{
   auto data = static_cast<NXPL_Display*>(context);
   auto bitmap = new nxpl::Bitmap(context, bufferRequirements);
   return static_cast<BEGL_BufferHandle>(bitmap);
}

/* Destroy a buffer previously created with BufferCreate */
static BEGL_Error DispBufferDestroy(void *context __attribute__((unused)), BEGL_BufferDisplayState *bufferState)
{
   auto bitmap = static_cast<nxpl::Bitmap *>(bufferState->buffer);
   delete bitmap;
   return BEGL_Success;
}

static BEGL_BufferHandle DispBufferDequeue(void *context, void *platformState, BEGL_BufferFormat format, int *fd)
{
   auto windowState = static_cast<nxpl::WindowState *>(platformState);

   auto nw = static_cast<nxpl::NativeWindowInfo*>(windowState->GetWindowHandle());
   auto windowExtent = nw->GetExtent2D();
   auto windowType = nw->GetType();

   helper::Extent2D bitmapExtent;

   auto bitmap = windowState->PopFreeQ();
   if (bitmap)
      bitmapExtent = bitmap->GetExtent2D();

   // resize or create
   if (windowExtent != bitmapExtent)
   {
      auto data = static_cast<NXPL_Display *>(context);
      assert(data != NULL);

      BEGL_PixmapInfoEXT bufferRequirements = {};
      bufferRequirements.width = windowExtent.GetWidth();
      bufferRequirements.height = windowExtent.GetHeight();
      bufferRequirements.format = format;
      bufferRequirements.secure = windowState->IsSecure();

      //printf("create surface %d %d\n", windowExtent.GetWidth(), windowExtent.GetHeight());

      std::unique_ptr<nxpl::Bitmap> tmp(static_cast<nxpl::Bitmap*>(DispBufferCreate(context, &bufferRequirements)));
      bitmap = std::move(tmp);
   }

   // not needed on win32
   *fd = -1;

   // m_info contains other presentation info other than size, so update
   bitmap->UpdateWindowInfo(*nw);

   auto buffer = bitmap.release();
   return static_cast<BEGL_BufferHandle>(buffer);
}

static BEGL_Error DispBufferQueue(void *context, void *platformState, BEGL_BufferHandle buffer, int swap_interval, int fd)
{
   if (buffer != NULL)
   {
      auto data = static_cast<NXPL_Display *>(context);
      auto windowState = static_cast<nxpl::WindowState *>(platformState);
      std::unique_ptr<nxpl::Bitmap> bitmap(static_cast<nxpl::Bitmap*>(buffer));
      static_assert(sizeof(int) == sizeof(void *), "doesn't function as a 64bit compile!");
      std::unique_ptr<helper::Semaphore> fence;
      if (fd != -1)
         fence = std::move(std::unique_ptr<helper::Semaphore>(reinterpret_cast<helper::Semaphore*>(fd)));
      else
         fence = std::move(std::unique_ptr<helper::Semaphore>(new helper::Semaphore(1)));

      std::unique_ptr<nxpl::DispItem> dispItem(new nxpl::DispItem(std::move(bitmap), std::move(fence), swap_interval));
      windowState->PushDispQ(std::move(dispItem));
   }

   return BEGL_Success;
}

static BEGL_Error DispBufferCancel(void *context, void *platformState, BEGL_BufferHandle buffer, int fd __attribute__((unused)))
{
   if (buffer != NULL)
   {
      auto data = static_cast<NXPL_Display *>(context);
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

static BEGL_Error DispSurfaceGetInfo(void *context __attribute__((unused)),
   uint32_t target __attribute__((unused)),
   void *buffer,
   BEGL_BufferSettings *settings)
{
   if (settings && buffer)
   {
      *settings = static_cast<nxpl::Bitmap*>(buffer)->GetCreateSettings();
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

typedef struct NXPL_DisplayInterface : BEGL_DisplayInterface
{
   NXPL_Display data;
} NXPL_DisplayInterface;

extern "C" BEGL_DisplayInterface *NXPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
   BEGL_HWInterface     *hwIface,
   NEXUS_DISPLAYHANDLE display,
   BEGL_DisplayCallbacks *displayCallbacks)
{
   assert(displayCallbacks->BufferGetRequirements != NULL);

   std::unique_ptr<NXPL_DisplayInterface> disp(new NXPL_DisplayInterface());
   if (!disp)
      return NULL;

   disp->context = &disp->data;
   disp->BufferCreate = DispBufferCreate;
   disp->BufferDequeue = DispBufferDequeue;
   disp->BufferQueue = DispBufferQueue;
   disp->BufferCancel = DispBufferCancel;
   disp->BufferDestroy = DispBufferDestroy;
   disp->DefaultOrientation = DispDefaultOrientation;
   disp->WindowPlatformStateCreate = DispWindowStateCreate;
   disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
   disp->SurfaceGetInfo = DispSurfaceGetInfo;

   disp->data.memInterface = memIface;
   disp->data.hwInterface = hwIface;
   disp->data.display = display;
   disp->data.bufferGetRequirementsFunc = displayCallbacks->BufferGetRequirements;

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
                "Start 0x%llX, Size %p\n", memStatus.offset, memStatus.size);
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

extern "C" bool NXPL_BufferGetRequirements(NXPL_PlatformHandle handle,
   BEGL_PixmapInfoEXT *bufferRequirements,
   BEGL_BufferSettings * bufferConstrainedRequirements)
{
   auto data = static_cast<BEGL_DriverInterfaces*>(handle);

   if (data != NULL && data->displayCallbacks.BufferGetRequirements != NULL)
   {
      data->displayCallbacks.BufferGetRequirements(bufferRequirements, bufferConstrainedRequirements);
      return true;
   }

   return false;
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

   if (!(data != NULL && data->displayCallbacks.PixmapCreateCompatiblePixmap != NULL))
      return false;

   if (surface == NULL)
      return false;

   BEGL_BufferHandle buffer = data->displayCallbacks.PixmapCreateCompatiblePixmap(info);
   if (buffer == NULL)
      return false;

   *pixmapHandle = static_cast<void*>(buffer);
   *surface = static_cast<nxpl::Bitmap*>(buffer)->GetSurface();
   return true;
}

extern "C" bool NXPL_CreateCompatiblePixmap(NXPL_PlatformHandle handle,
   void **pixmapHandle, NEXUS_SurfaceHandle *surface,
   BEGL_PixmapInfo *info)
{
   BEGL_PixmapInfoEXT   infoEXT;

   NXPL_GetDefaultPixmapInfoEXT(&infoEXT);

   infoEXT.width = info->width;
   infoEXT.height = info->height;
   infoEXT.format = info->format;

   return NXPL_CreateCompatiblePixmapEXT(handle, pixmapHandle, surface, &infoEXT);
}

extern "C" void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle handle, void *pixmapHandle)
{
   BEGL_DriverInterfaces *data = static_cast<BEGL_DriverInterfaces*>(handle);

   if (data != NULL &&
      data->displayInterface != NULL &&
      data->displayInterface->BufferDestroy != NULL)
   {
      BEGL_BufferDisplayState state = {};
      state.buffer = static_cast<BEGL_BufferHandle>(pixmapHandle);

      data->displayInterface->BufferDestroy(data->displayInterface->context, &state);
   }
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
