/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "display_wl_client.h"

#include "display_interface.h"
#include "display_interface_wl_client.h"
#include "display_framework.h"
#include "private_nexus.h"
#include "platform_common.h"
#include "fence_queue.h"
#include "surface_interface_wl_client.h"
#include "display_surface.h"
#include "wayland_nexus_client.h"
#include "wayland_egl_priv.h"
#include "sched_nexus.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-util.h>
#include <string.h>

#include "../../common/surface_interface_nexus.h"

#ifdef NXPL_PLATFORM_EXCLUSIVE
#error Wayland platform requires multi-process mode!
#endif

#define SWAPCHAIN_COUNT 3

typedef struct WaylandClientWindow
{
   WaylandClientPlatform *platform;

   struct wl_egl_window *wl_egl_window;

   DisplayInterface display_interface;

   DisplayFramework display_framework;
} WaylandClientWindow;

static bool WlcInit(void *context)
{
   WaylandClientPlatform *platform = (WaylandClientPlatform *)context;
   if (InitWaylandClient(&platform->client, platform->client.display))
   {
      InitFenceInterface(&platform->fence_interface, platform->schedInterface);
      return SurfaceInterface_InitWlClient(&platform->surface_interface,
               &platform->client);
   }
   return false;
}

static void WlcTerminate(void *context)
{
   WaylandClientPlatform *platform = (WaylandClientPlatform *)context;
   Interface_Destroy(&platform->surface_interface.base);
   Interface_Destroy(&platform->fence_interface.base);
   DestroyWaylandClient(&platform->client);
}

static BEGL_Error WlcWindowGetInfo(void *context, void *nativeWindow,
      BEGL_WindowInfoFlags flags, BEGL_WindowInfo *info)
{
   UNUSED(context);
   WaylandClientWindow *window = (WaylandClientWindow *) nativeWindow;

   if (window)
   {
      if (flags & BEGL_WindowInfoWidth)
         info->width = window->wl_egl_window->width;
      if (flags & BEGL_WindowInfoHeight)
         info->height = window->wl_egl_window->height;
      if (flags & BEGL_WindowInfoSwapChainCount)
         info->swapchain_count = SWAPCHAIN_COUNT;
      return BEGL_Success;
   }
   return BEGL_Fail;
}

static BEGL_Error WlcGetNativeSurface(void *context,
      uint32_t eglTarget, void *eglSurface, void **nativeSurface)
{
   BSTD_UNUSED(context);

   if (eglTarget != EGL_NATIVE_PIXMAP_KHR)
      return BEGL_Fail;
   *nativeSurface = eglSurface;
   return BEGL_Success;
}

static BEGL_Error WlcSurfaceGetInfo(void *context, void *surface, BEGL_SurfaceInfo *info)
{
   BSTD_UNUSED(context);

   if (surface != NULL && surface != (void*)0xFFFFFFFF && info != NULL)
   {
      WaylandClientBuffer *cb = (WaylandClientBuffer *)surface;
      *info = cb->info;
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error WlcSurfaceChangeRefCount(void *context, void *nativeSurface, BEGL_RefCountMode incOrDec)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(nativeSurface);
   BSTD_UNUSED(incOrDec);

   /* Nothing to do in Wayland client platform */
   return BEGL_Success;
}

static BEGL_Error WlcGetNextSurface(void *context, void *nativeWindow,
      BEGL_BufferFormat format, BEGL_BufferFormat *actualFormat,
      void **nativeSurface, bool secure, int *fence)
{
   UNUSED(context);
   UNUSED(secure);

   WaylandClientWindow *window = (WaylandClientWindow *) nativeWindow;

   if (window == NULL || nativeSurface == NULL || actualFormat == NULL)
      return BEGL_Fail;

   *nativeSurface = DisplayFramework_GetNextSurface(
         &window->display_framework, format, secure, fence);

   if (!*nativeSurface)
   {
      *actualFormat = BEGL_BufferFormat_INVALID;
      return BEGL_Fail;
   }
   return BEGL_Success;
}

static BEGL_Error WlcDisplaySurface(void *context, void *nativeWindow,
      void *nativeSurface, int fence, int interval)
{
   UNUSED(context);
   WaylandClientWindow *window = (WaylandClientWindow *) nativeWindow;
   if (window && nativeSurface)
   {
      DisplayFramework_DisplaySurface(&window->display_framework,
            nativeSurface, fence, interval);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error WlcCancelSurface(void *context, void *nativeWindow,
      void *nativeSurface, int fence)
{
   UNUSED(context);
   WaylandClientWindow *window = (WaylandClientWindow *) nativeWindow;
   if (window && nativeSurface)
   {
      DisplayFramework_CancelSurface(&window->display_framework, nativeSurface,
            fence);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static bool WlcPlatformSupported(void *context, uint32_t platform)
{
   UNUSED(context);
   return platform == EGL_PLATFORM_WAYLAND_KHR;
}

static bool WlcSetDefaultDisplay(void *context, void *display)
{
   WaylandClientPlatform *platform = (WaylandClientPlatform *)context;
   if (platform->client.display == EGL_NO_DISPLAY)
   {
      platform->client.display = display;
      return true;
   }
   else
      return platform->client.display == display; /* disallow change */
}

static void *WlcGetDefaultDisplay(void *context)
{
   WaylandClientPlatform *platform = (WaylandClientPlatform *)context;
   return platform->client.display;
}

static const char *WlcGetClientExtensions(void *context)
{
   BSTD_UNUSED(context);
   return "EGL_KHR_platform_wayland EGL_EXT_platform_wayland";
}

static const char *WlcGetDisplayExtensions(void *context)
{
   BSTD_UNUSED(context);
   return NULL;
}

static void WlcResizeCallback(void *context, struct wl_egl_window * wl_egl_window)
{
   WaylandClientWindow *window = (WaylandClientWindow *) context;

   window->display_framework.window_info.width = wl_egl_window->width;
   window->display_framework.window_info.height = wl_egl_window->height;
}

static void *WlcWindowPlatformStateCreate(void *context, void *native)
{
   WaylandClientPlatform *platform = (WaylandClientPlatform *)context;
   struct wl_egl_window *wl_egl_window = (struct wl_egl_window *)native;
   WaylandClientWindow *window = NULL;

   if (platform && wl_egl_window && wl_egl_window->surface)
   {
      window = calloc(1, sizeof(*window));
      if (window)
      {
         window->platform = platform;
         window->wl_egl_window = wl_egl_window;
         window->wl_egl_window->resize.context = window;
         window->wl_egl_window->resize.callback = WlcResizeCallback;

         DisplayInterface_InitWlClient(&window->display_interface,
               &platform->client, &platform->fence_interface,
               window->wl_egl_window, SWAPCHAIN_COUNT);

         if (!DisplayFramework_Start(&window->display_framework,
               &window->display_interface,
               &platform->fence_interface,
               &platform->surface_interface,
               window->wl_egl_window->width,
               window->wl_egl_window->height,
               SWAPCHAIN_COUNT))
         {
            Interface_Destroy(&window->display_interface.base);
            free(window);
            window = NULL;
         }
      }
   }
   return window;
}

static BEGL_Error WlcWindowPlatformStateDestroy(void *context, void *windowState)
{
   UNUSED(context);
   WaylandClientWindow *window = (WaylandClientWindow *) windowState;

   if (window)
   {
      DisplayFramework_Stop(&window->display_framework);

      Interface_Destroy(&window->display_interface.base);

      window->wl_egl_window->resize.callback = NULL;
      window->wl_egl_window->resize.context = NULL;
      window->wl_egl_window = NULL;

#ifndef NDEBUG
      /* catch some cases of use after free */
      memset(window, 0, sizeof(*window));
#endif
      free(window);
   }
   return BEGL_Success;
}

struct BEGL_DisplayInterface *CreateDisplayInterfaceWaylandClient(
      WaylandClientPlatform *platform)
{
   BEGL_DisplayInterface *disp = calloc(1, sizeof(*disp));
   if (disp)
   {
      disp->context = platform;
      disp->Init = WlcInit;
      disp->Terminate = WlcTerminate;
      disp->WindowGetInfo = WlcWindowGetInfo;
      disp->GetNativeSurface = WlcGetNativeSurface;
      disp->SurfaceGetInfo = WlcSurfaceGetInfo;
      disp->SurfaceChangeRefCount = WlcSurfaceChangeRefCount;
      disp->GetNextSurface = WlcGetNextSurface;
      disp->DisplaySurface = WlcDisplaySurface;
      disp->CancelSurface = WlcCancelSurface;
      disp->PlatformSupported = WlcPlatformSupported;
      disp->SetDefaultDisplay = WlcSetDefaultDisplay;
      disp->GetDefaultDisplay = WlcGetDefaultDisplay;
      disp->GetClientExtensions = WlcGetClientExtensions;
      disp->GetDisplayExtensions = WlcGetDisplayExtensions;
      disp->WindowPlatformStateCreate = WlcWindowPlatformStateCreate;
      disp->WindowPlatformStateDestroy = WlcWindowPlatformStateDestroy;
   }
   return disp;
}

void DestroyDisplayInterfaceWaylandClient(
      BEGL_DisplayInterface *disp)
{
   free(disp);
}
