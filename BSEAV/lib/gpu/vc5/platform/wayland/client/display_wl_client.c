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
#include "surface_interface.h"
#include "surface_interface_wl_client.h"
#include "display_surface.h"
#include "wayland_nexus_client.h"
#include "wayland_egl_priv.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-util.h>

#include "../../common/fence_interface_nexus.h"
#include "../../common/surface_interface_nexus.h"

#ifdef NXPL_PLATFORM_EXCLUSIVE
#error Wayland platform requires multi-process mode!
#endif

#define SWAPCHAIN_COUNT 3

typedef struct PlatformState
{
   WaylandClientPlatform *platform;

   int swap_interval;
   struct wl_egl_window *wl_egl_window;

   SurfaceInterface surface_interface;
   FenceInterface fence_interface;
   DisplayInterface display_interface;

   DisplayFramework display_framework;
} PlatformState;

static BEGL_Error WlcWindowGetInfo(void *context, void *nativeWindow,
      BEGL_WindowInfoFlags flags, BEGL_WindowInfo *info)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;

   if (state)
   {
      if (flags & BEGL_WindowInfoWidth)
         info->width = state->wl_egl_window->width;
      if (flags & BEGL_WindowInfoHeight)
         info->height = state->wl_egl_window->height;
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

   PlatformState *state = (PlatformState *) nativeWindow;

   if (state == NULL || nativeSurface == NULL || actualFormat == NULL)
      return BEGL_Fail;

   *nativeSurface = DisplayFramework_GetNextSurface(
         &state->display_framework, format, secure, fence);

   if (!*nativeSurface)
   {
      *actualFormat = BEGL_BufferFormat_INVALID;
      return BEGL_Fail;
   }
   return BEGL_Success;
}

static BEGL_Error WlcDisplaySurface(void *context, void *nativeWindow,
      void *nativeSurface, int fence)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;
   if (state && nativeSurface)
   {
      DisplayFramework_DisplaySurface(&state->display_framework,
            nativeSurface, fence, state->swap_interval);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error WlcCancelSurface(void *context, void *nativeWindow,
      void *nativeSurface, int fence)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;
   if (state && nativeSurface)
   {
      DisplayFramework_CancelSurface(&state->display_framework, nativeSurface,
            fence);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static void WlcSetSwapInterval(void *context, void *nativeWindow, int interval)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;
   if (interval >= 0)
      state->swap_interval = interval;
}

static bool WlcPlatformSupported(void *context, uint32_t platform)
{
   UNUSED(context);
   return platform == EGL_PLATFORM_WAYLAND_KHR;
}

static bool WlcSetDefaultDisplay(void *context, void *display)
{
   WaylandClientPlatform *platform = (WaylandClientPlatform *)context;
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
   PlatformState *state = (PlatformState *) context;

   state->display_framework.window_info.width = wl_egl_window->width;
   state->display_framework.window_info.height = wl_egl_window->height;
}

static void *WlcWindowPlatformStateCreate(void *context, void *native)
{
   WaylandClientPlatform *platform = (WaylandClientPlatform *)context;
   struct wl_egl_window *wl_egl_window = (struct wl_egl_window *)native;
   PlatformState *state = NULL;

   if (platform && wl_egl_window && wl_egl_window->surface)
   {
      state = calloc(1, sizeof(*state));
      if (state)
      {
         state->platform = platform;
         state->swap_interval = 1;
         state->wl_egl_window = wl_egl_window;
         state->wl_egl_window->resize.context = state;
         state->wl_egl_window->resize.callback = WlcResizeCallback;

         FenceInteraface_InitNexus(&state->fence_interface, platform->schedInterface);
         SurfaceInterface_InitWlClient(&state->surface_interface, &platform->client);
         DisplayInterface_InitWlClient(&state->display_interface, &platform->client,
               &state->fence_interface, state->wl_egl_window, SWAPCHAIN_COUNT);

         if (!DisplayFramework_Start(&state->display_framework,
               &state->display_interface,
               &state->fence_interface,
               &state->surface_interface,
               state->wl_egl_window->width,
               state->wl_egl_window->height,
               SWAPCHAIN_COUNT))
         {
            Interface_Destroy(&state->display_interface.base);
            Interface_Destroy(&state->surface_interface.base);
            Interface_Destroy(&state->fence_interface.base);
            free(state);
            state = NULL;
         }
      }
   }
   return state;
}

static BEGL_Error WlcWindowPlatformStateDestroy(void *context, void *windowState)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) windowState;

   if (state)
   {
      DisplayFramework_Stop(&state->display_framework);

      Interface_Destroy(&state->display_interface.base);
      Interface_Destroy(&state->surface_interface.base);
      Interface_Destroy(&state->fence_interface.base);

      state->wl_egl_window->resize.callback = NULL;
      state->wl_egl_window->resize.context = NULL;
      state->wl_egl_window = NULL;

#ifndef NDEBUG
      /* catch some cases of use after free */
      memset(state, 0, sizeof(*state));
#endif
      free(state);
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
      disp->WindowGetInfo = WlcWindowGetInfo;
      disp->GetNativeSurface = WlcGetNativeSurface;
      disp->SurfaceGetInfo = WlcSurfaceGetInfo;
      disp->SurfaceChangeRefCount = WlcSurfaceChangeRefCount;
      disp->GetNextSurface = WlcGetNextSurface;
      disp->DisplaySurface = WlcDisplaySurface;
      disp->CancelSurface = WlcCancelSurface;
      disp->SetSwapInterval = WlcSetSwapInterval;
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
