/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <memory>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>

#include "default_directfb.h"
#include "private_directfb.h"
#include "directfb_helpers.h"

#include "nexus_base_mmap.h"

#include "../common/fence_interface.h"

#include "sched_nexus.h"

#include "windowstate.h"

typedef struct
{
   IDirectFB               *dfb;
   BEGL_SchedInterface     *schedIface;
   FenceInterface          fenceIface;
} DBPL_Display;

typedef struct DBPL_DisplayInterface : BEGL_DisplayInterface
{
   DBPL_Display data;
} DBPL_DisplayInterface;

enum
{
   PIXMAP_INFO_MAGIC = 0x15EEB1A5
};

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          int err = static_cast<int>(x);                              \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, static_cast<DFBResult>(err) ); \
          }                                                           \
     }

/* Called to determine current size of the window referenced by the opaque window handle.
 * This is needed by EGL in order to know the size of a native 'window'. */
static BEGL_Error DispWindowGetInfo(void *context __attribute__((unused)),
                                    void *platformState,
                                    BEGL_WindowInfoFlags flags,
                                    BEGL_WindowInfo *info)
{
   if (!platformState)
      return BEGL_Fail;

   auto windowState = static_cast<WindowState *>(platformState);
   auto surface = windowState->GetBitmap()->GetSurface();

   if ((flags & BEGL_WindowInfoWidth) || (flags & BEGL_WindowInfoHeight))
   {
      assert(sizeof(int) == sizeof(info->width));
      assert(sizeof(int) == sizeof(info->height));
      DFBCHECK( surface->GetSize ( surface, reinterpret_cast<int *>(&info->width), reinterpret_cast<int *>(&info->height)));
   }

   if (flags & BEGL_WindowInfoSwapChainCount)
   {
      DFBSurfaceCapabilities ret_caps;
      DFBCHECK( surface->GetCapabilities( surface, &ret_caps ) );
      if (ret_caps & DSCAPS_DOUBLE)
         info->swapchain_count = 2;
      else if (ret_caps & DSCAPS_TRIPLE)
         info->swapchain_count = 3;
      else
         info->swapchain_count = 1;
   }

   return BEGL_Success;
}

static BEGL_BufferFormat DispGetPixmapFormat(void *context __attribute__((unused)), void *pixmap)
{
   auto surface = static_cast<IDirectFBSurface *>(pixmap);

   DFBSurfacePixelFormat dfb_format;
   DFBCHECK( surface->GetPixelFormat ( surface, &dfb_format ) );

   BEGL_BufferFormat res;
   DfbToBeglFormat(dfb_format, &res);

   return res;
}

static void GetCreateSettings(IDirectFBSurface *surface, BEGL_SurfaceInfo *info)
{
   void *addr;
   int pitch;

   DFBCHECK( surface->Lock( surface, DSLF_WRITE, &addr, &pitch));
   info->cachedAddr     = addr;
   info->pitchBytes     = pitch;
   info->physicalOffset = NEXUS_AddrToOffset(info->cachedAddr);
   info->secure         = false;
   info->contiguous     = true;
   info->colorimetry    = BEGL_Colorimetry_RGB;
   info->miplevels      = 1;
   info->stripeWidth    = 0;
   assert(sizeof(int) == sizeof(info->width));
   assert(sizeof(int) == sizeof(info->height));
   DFBCHECK( surface->GetSize ( surface, reinterpret_cast<int *>(&info->width), reinterpret_cast<int *>(&info->height)));
   DFBSurfacePixelFormat dfb_format;
   DFBCHECK( surface->GetPixelFormat ( surface, &dfb_format));
   DfbToBeglFormat(dfb_format, &info->format);
   DFBCHECK( surface->Unlock( surface ));
}

static BEGL_NativeBuffer DispSurfaceAcquire(void *context __attribute__((unused)), uint32_t target,
                                            void *eglObject, uint32_t plane __attribute__((unused)),
                                            BEGL_SurfaceInfo *info)
{
   switch (target)
   {
   case BEGL_SWAPCHAIN_BUFFER:
      {
         auto surface = static_cast<WrappedBitmap*>(eglObject)->GetSurface();
         GetCreateSettings(surface, info);
         return surface;
      }
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_PIXMAP_BUFFER:
      {
         auto surface = static_cast<IDirectFBSurface *>(eglObject);
         GetCreateSettings(surface, info);
         return surface;
      }
   default:
      return nullptr;
   }
}

static BEGL_Error DispSurfaceRelease(void *context __attribute__((unused)), uint32_t target,
      uint32_t plane __attribute__((unused)), BEGL_NativeBuffer buffer __attribute__((unused)))
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

static BEGL_SwapchainBuffer DispGetNextSurface(
   void *context __attribute__((unused)),
   void *platformState,
   BEGL_BufferFormat format __attribute__((unused)),
   bool secure __attribute__((unused)),
   int *age,
   int *fence
)
{
   auto windowState = static_cast<WindowState *>(platformState);

   // not needed on directfb
   *fence = -1;
   *age = 0;

   auto bitmap = windowState->GetBitmap();

   return static_cast<BEGL_SwapchainBuffer>(bitmap);
}

static BEGL_Error DispDisplaySurface(void *context, void *platformState,
      BEGL_SwapchainBuffer buffer, int fence, int interval)
{
   if (buffer)
   {
      auto disp = static_cast<DBPL_DisplayInterface *>(context);
      auto windowState = static_cast<WindowState *>(platformState);
      if (fence != -1)
      {
         FenceInterface_Wait(&disp->data.fenceIface, fence, 2000);
         FenceInterface_Destroy(&disp->data.fenceIface, &fence);
      }

      auto bitmap = static_cast<WrappedBitmap*>(buffer);

      auto surface = bitmap->GetSurface();

      DFBCHECK( surface->Flip( surface, NULL, interval > 0 ? DSFLIP_WAITFORSYNC : DSFLIP_NONE ));
   }

   return BEGL_Success;
}

static BEGL_Error DispCancelSurface(void *context __attribute__((unused)), void *nativeWindow __attribute__((unused)),
      BEGL_SwapchainBuffer nativeSurface __attribute__((unused)), int fence __attribute__((unused)))
{
   return BEGL_Success;
}

static void *DispWindowPlatformStateCreate(void *context __attribute__((unused)), void *native)
{
   std::unique_ptr<WindowState> windowState(new WindowState(native));
   return static_cast<void*>(windowState.release());
}

static BEGL_Error DispWindowPlatformStateDestroy(void *context __attribute__((unused)), void *platformState)
{
   auto windowState = std::unique_ptr<WindowState>(static_cast<WindowState *>(platformState));
   return BEGL_Success;
}

extern "C" BEGL_DisplayInterface *CreateDirectFBDisplayInterface(IDirectFB *dfb,
                                                                 BEGL_SchedInterface *schedIface)
{
   std::unique_ptr<DBPL_DisplayInterface> disp(new DBPL_DisplayInterface());
   if (!disp)
      return nullptr;

   disp->context = &disp->data;
   disp->WindowGetInfo              = DispWindowGetInfo;
   disp->GetPixmapFormat            = DispGetPixmapFormat;
   disp->SurfaceAcquire             = DispSurfaceAcquire;
   disp->SurfaceRelease             = DispSurfaceRelease;
   disp->GetNextSurface             = DispGetNextSurface;
   disp->DisplaySurface             = DispDisplaySurface;
   disp->CancelSurface              = DispCancelSurface;
   disp->WindowPlatformStateCreate  = DispWindowPlatformStateCreate;
   disp->WindowPlatformStateDestroy = DispWindowPlatformStateDestroy;

   disp->data.dfb = dfb;
   disp->data.schedIface = schedIface;

   InitFenceInterface(&disp->data.fenceIface, schedIface);

   return disp.release();
}

extern "C" void DestroyDirectFBDisplayInterface(BEGL_DisplayInterface *p)
{
   auto disp = std::unique_ptr<DBPL_DisplayInterface>(static_cast<DBPL_DisplayInterface *>(p));
}

extern "C" void DBPL_GetDefaultPixmapInfoEXT(BEGL_PixmapInfoEXT *info)
{
   if (info)
   {
      memset(info, 0, sizeof(BEGL_PixmapInfoEXT));

      info->format = BEGL_BufferFormat_INVALID;
      info->magic = PIXMAP_INFO_MAGIC;
   }
}

extern "C" bool DBPL_CreateCompatiblePixmapEXT(DBPL_PlatformHandle handle, void **pixmapHandle, IDirectFBSurface **surface,
   BEGL_PixmapInfoEXT *info)
{
   auto data = static_cast<DBPL_InternalPlatformHandle*>(handle);
   auto disp = static_cast<DBPL_DisplayInterface *>(data->displayInterface);

   assert(info->magic == PIXMAP_INFO_MAGIC);

   if (surface == nullptr)
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

   IDirectFBSurface *dfb_surface;
   DFBCHECK( disp->data.dfb->CreateSurface( disp->data.dfb, &desc, &dfb_surface ));

   *pixmapHandle = static_cast<void*>(dfb_surface);
   *surface = dfb_surface;
   return true;
}

extern "C" void DBPL_DestroyCompatiblePixmap(DBPL_PlatformHandle handle __attribute__((unused)), void *pixmapHandle)
{
   auto dfbSurface = static_cast<IDirectFBSurface *>(pixmapHandle);
   if (dfbSurface)
      DFBCHECK( dfbSurface->Release( dfbSurface ));
}
