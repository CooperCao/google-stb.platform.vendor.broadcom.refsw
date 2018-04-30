/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <memory>
#include <cassert>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/begl_platform.h>

#include "default_directfb.h"

#include "nexus_surface.h"
#include "nexus_memory.h"
#include "nexus_platform.h"

#include "windowstate.h"
#include "bitmap.h"
#include "display_helpers.h"
#include "../helpers/semaphore.h"

#include "nexus_base_mmap.h"

enum
{
   PIXMAP_INFO_MAGIC = 0x15EEB1A5
};

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
   { \
      int err = static_cast<int>(x);                              \
      if (err != DFB_OK) {                                        \
         fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ );   \
         DirectFBErrorFatal( #x, static_cast<DFBResult>(err) );   \
      }                                                           \
   }

static BEGL_SwapchainBuffer DispBufferDequeue(void *context __attribute__((unused)), void *platformState,
                                              BEGL_BufferFormat format __attribute__((unused)), int *fd)
{
   auto windowState = static_cast<WindowState *>(platformState);

   // not needed on directfb
   *fd = -1;

   auto bitmap = windowState->GetBitmap();

   return static_cast<BEGL_SwapchainBuffer>(bitmap);
}

static BEGL_Error DispBufferQueue(void *context, void *platformState, BEGL_SwapchainBuffer buffer, int swap_interval, int fd)
{
   if (buffer != NULL)
   {
      auto data = static_cast<DBPL_Display *>(context);
      auto windowState = static_cast<WindowState *>(platformState);
      std::unique_ptr<helper::Semaphore> fence;
      if (fd != -1)
         fence = std::move(std::unique_ptr<helper::Semaphore>(static_cast<helper::Semaphore*>(data->hwInterface->FenceGet(data->hwInterface->context, fd))));
      else
         fence = std::move(std::unique_ptr<helper::Semaphore>(new helper::Semaphore(1)));

      if (fence)
         fence->wait();

      auto bitmap = static_cast<WrappedBitmap*>(buffer);

      auto surface = bitmap->GetSurface();
      DFBCHECK( surface->Flip( surface, NULL, swap_interval > 0 ? DSFLIP_WAITFORSYNC : DSFLIP_NONE ));
   }

   return BEGL_Success;
}

static BEGL_Error DispBufferCancel(void *context __attribute__((unused)), void *platformState __attribute__((unused)),
                                   BEGL_SwapchainBuffer buffer __attribute__((unused)), int fd __attribute__((unused)))
{
   return BEGL_Success;
}

static void *DispWindowStateCreate(void *context __attribute__((unused)), BEGL_WindowHandle window, bool secure __attribute__((unused)))
{
   std::unique_ptr<WindowState> windowState(new WindowState(window));

   return static_cast<void*>(windowState.release());
}

static BEGL_Error DispWindowStateDestroy(void *context __attribute__((unused)), void *swapChainCtx)
{
   auto windowState = std::unique_ptr<WindowState>(static_cast<WindowState *>(swapChainCtx));
   return BEGL_Success;
}

static void GetCreateSettings(IDirectFBSurface *surface, BEGL_BufferSettings *settings)
{
   void *addr;
   int pitch;

   DFBCHECK( surface->Lock( surface, DSLF_WRITE, &addr, &pitch));
   settings->cachedAddr = addr;
   settings->pitchBytes = pitch;
   settings->physOffset = NEXUS_AddrToOffset(settings->cachedAddr);
   assert(sizeof(int) == sizeof(settings->width));
   assert(sizeof(int) == sizeof(settings->height));
   DFBCHECK( surface->GetSize ( surface, reinterpret_cast<int *>(&settings->width), reinterpret_cast<int *>(&settings->height)));
   DFBSurfacePixelFormat dfb_format;
   DFBCHECK( surface->GetPixelFormat ( surface, &dfb_format));
   DfbToBeglFormat(dfb_format, &settings->format);
   DFBCHECK( surface->Unlock( surface ));
}

static BEGL_NativeBuffer DispAcquireNativeBuffer(void *context __attribute__((unused)),
   uint32_t target,
   void *eglObject,
   BEGL_BufferSettings *settings)
{
   switch (target)
   {
   case BEGL_SWAPCHAIN_BUFFER:
      {
         auto surface = static_cast<WrappedBitmap*>(eglObject)->GetSurface();
         GetCreateSettings(surface, settings);
         return surface;
      }
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_PIXMAP_BUFFER:
      {
         auto surface = static_cast<IDirectFBSurface *>(eglObject);
         GetCreateSettings(surface, settings);
         return surface;
      }
   default:
      return NULL;
   }
}

static BEGL_Error DispReleaseNativeBuffer(void *context __attribute__((unused)),
   uint32_t target, BEGL_NativeBuffer)
{
   switch (target)
   {
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_SWAPCHAIN_BUFFER:
   case BEGL_PIXMAP_BUFFER:
   {
      // DFB has no meaningful reference counting
      return BEGL_Success;
   }
   default:
      return BEGL_Fail;
   }
}

typedef struct DBPL_DisplayInterface : BEGL_DisplayInterface
{
   DBPL_Display data;
} DBPL_DisplayInterface;

extern "C" BEGL_DisplayInterface *DBPL_CreateDisplayInterface(IDirectFB *dfb,
                                                              BEGL_HWInterface     *hwIface,
                                                              BEGL_MemoryInterface *memIface)
{
   std::unique_ptr<DBPL_DisplayInterface> disp(new DBPL_DisplayInterface());
   if (!disp)
      return NULL;

   disp->context = &disp->data;
   disp->BufferDequeue = DispBufferDequeue;
   disp->BufferQueue = DispBufferQueue;
   disp->BufferCancel = DispBufferCancel;
   disp->WindowPlatformStateCreate = DispWindowStateCreate;
   disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
   disp->AcquireNativeBuffer = DispAcquireNativeBuffer;
   disp->ReleaseNativeBuffer = DispReleaseNativeBuffer;

   disp->data.dfb = dfb;
   disp->data.memInterface = memIface;
   disp->data.hwInterface = hwIface;

   return disp.release();
}

extern "C" void DBPL_DestroyDisplayInterface(BEGL_DisplayInterface *p)
{
   auto disp = std::unique_ptr<DBPL_DisplayInterface>(static_cast<DBPL_DisplayInterface *>(p));
}

extern "C" void DBPL_GetDefaultPixmapInfoEXT(BEGL_PixmapInfoEXT *info)
{
   if (info != NULL)
   {
      memset(info, 0, sizeof(BEGL_PixmapInfoEXT));

      info->format = BEGL_BufferFormat_INVALID;
      info->magic = PIXMAP_INFO_MAGIC;
   }
}

extern "C" bool DBPL_CreateCompatiblePixmapEXT(DBPL_PlatformHandle handle, void **pixmapHandle, IDirectFBSurface **surface,
   BEGL_PixmapInfoEXT *info)
{
   auto data = static_cast<BEGL_DriverInterfaces*>(handle);
   auto disp = static_cast<DBPL_DisplayInterface *>(data->displayInterface);

   assert(info->magic == PIXMAP_INFO_MAGIC);

   if (surface == NULL)
      return false;

   DFBSurfaceDescription desc = {};
   desc.flags = static_cast<DFBSurfaceDescriptionFlags>(DSDESC_CAPS |
                                                        DSDESC_WIDTH |
                                                        DSDESC_HEIGHT |
                                                        DSDESC_PIXELFORMAT);
   desc.caps = DSCAPS_GL;
   desc.width = info->width;
   desc.height = info->height;
   BeglToDfbFormat(info->format, &desc.pixelformat);

   IDirectFBSurface *dfbSurface;
   DFBCHECK( disp->data.dfb->CreateSurface( disp->data.dfb, &desc, &dfbSurface ));

   *pixmapHandle = static_cast<void*>(dfbSurface);
   *surface = dfbSurface;
   return true;
}

extern "C" void DBPL_DestroyCompatiblePixmap(DBPL_PlatformHandle handle __attribute__((unused)), void *pixmapHandle)
{
   auto dfbSurface = static_cast<IDirectFBSurface *>(pixmapHandle);
   if (dfbSurface)
      DFBCHECK( dfbSurface->Release( dfbSurface ));
}
