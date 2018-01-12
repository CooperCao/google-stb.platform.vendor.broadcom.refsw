/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "wl_server.h"
#include "display_interface.h"
#include "display_framework.h"
#include "private_nexus.h"
#include "platform_common.h"
#include "fence_queue.h"
#include "display_surface.h"
#include "display_helpers.h"
#include "wayland_nexus_client.h"
#include "wayland_egl_priv.h"
#include "sched_nexus.h"
#include "nexus_heap_selection.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <wayland-util.h>
#include <string.h>

#include "../../common/surface_interface_nexus.h"
#include "display_wl.h"
#include "display_interface_wl.h"
#include "surface_interface_wl.h"

#ifdef SINGLE_PROCESS
#error Wayland platform requires multi-process mode!
#endif

#define SWAPCHAIN_COUNT 3

typedef struct WLPL_WaylandDisplay
{
   WlClient client;
   FenceInterface fence_interface;
   WlDisplayBinding binding;
} WLPL_WaylandDisplay;

typedef struct WLPL_WaylandWindow
{
   struct wl_egl_window *wl_egl_window;
   SurfaceInterface surface_interface;
   DisplayInterface display_interface;
   DisplayFramework display_framework;
} WLPL_WaylandWindow;

static BEGL_Error WlWindowGetInfo(void *context, void *nativeWindow,
      BEGL_WindowInfoFlags flags, BEGL_WindowInfo *info)
{
   UNUSED(context);
   WLPL_WaylandWindow *window = (WLPL_WaylandWindow *)nativeWindow;

   if (window)
   {
      uint32_t width, height;
      DisplayFramework_GetSize(&window->display_framework, &width, &height);
      if (flags & BEGL_WindowInfoWidth)
         info->width = width;
      if (flags & BEGL_WindowInfoHeight)
         info->height = height;
      if (flags & BEGL_WindowInfoSwapChainCount)
         info->swapchain_count = SWAPCHAIN_COUNT;
      return BEGL_Success;
   }
   return BEGL_Fail;
}

static BEGL_Error WlGetNativeSurface(void *context, uint32_t eglTarget,
      void *eglSurface, void **nativeSurface)
{
   BSTD_UNUSED(context);

   if (eglTarget != EGL_NATIVE_PIXMAP_KHR)
      return BEGL_Fail;
   *nativeSurface = eglSurface;
   return BEGL_Success;
}

static BEGL_Error WlSurfaceGetInfo(void *context,
      void *surface, BEGL_SurfaceInfo *info)
{
   BSTD_UNUSED(context);

   if (surface != NULL && surface != (void*)0xFFFFFFFF && info != NULL)
   {
      WlWindowBuffer *buffer = (WlWindowBuffer *)surface;
      *info = buffer->buffer.settings;
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error WlSurfaceChangeRefCount(void *context, void *nativeSurface,
      BEGL_RefCountMode incOrDec)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(nativeSurface);
   BSTD_UNUSED(incOrDec);

   /* Nothing to do in Wayland client platform */
   return BEGL_Success;
}

static BEGL_Error WlGetNextSurface(void *context, void *nativeWindow,
      BEGL_BufferFormat format, BEGL_BufferFormat *actualFormat,
      void **nativeSurface, bool secure, int *fence)
{
   UNUSED(context);
   UNUSED(secure);

   WLPL_WaylandWindow *window = (WLPL_WaylandWindow *)nativeWindow;

   if (window == NULL || nativeSurface == NULL || actualFormat == NULL)
      return BEGL_Fail;

   *nativeSurface = DisplayFramework_GetNextSurface(&window->display_framework,
         format, secure, fence);

   if (*nativeSurface)
   {
      return BEGL_Success;
   }
   else
   {
      *actualFormat = BEGL_BufferFormat_INVALID;
      return BEGL_Fail;
   }
}

static BEGL_Error WlDisplaySurface(void *context, void *nativeWindow,
      void *nativeSurface, int fence, int interval)
{
   UNUSED(context);
   WLPL_WaylandWindow *window = (WLPL_WaylandWindow *)nativeWindow;
   if (window && nativeSurface)
   {
      DisplayFramework_DisplaySurface(&window->display_framework, nativeSurface,
            fence, interval);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error WlCancelSurface(void *context, void *nativeWindow,
      void *nativeSurface, int fence)
{
   UNUSED(context);
   WLPL_WaylandWindow *window = (WLPL_WaylandWindow *)nativeWindow;
   if (window && nativeSurface)
   {
      DisplayFramework_CancelSurface(&window->display_framework, nativeSurface,
            fence);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static const char *WlGetDisplayExtensions(void *context)
{
   BSTD_UNUSED(context);
   return "EGL_WL_bind_wayland_display";
}

static void WlResizeCallback(struct wl_egl_window *wl_egl_window, void *context)
{
   WLPL_WaylandWindow *window = (WLPL_WaylandWindow *)context;
   DisplayFramework_SetSize(&window->display_framework,
         wl_egl_window->width, wl_egl_window->height);
}

static void WlGetAttachedCallback(struct wl_egl_window *wl_egl_window, void *context)
{
   WLPL_WaylandWindow *window = (WLPL_WaylandWindow *)context;
   uint32_t width, height;
   DisplayFramework_GetSize(&window->display_framework, &width, &height);
   wl_egl_window->attached_width = width;
   wl_egl_window->attached_height = height;
}

static void WlReleaseCallback(void *data, struct wl_buffer *wl_buffer)
{
   WLPL_WaylandWindow *window = (WLPL_WaylandWindow *)data;
   DisplayInterface_WlBufferRelease(&window->display_interface, wl_buffer);
}

static void *WlWindowPlatformStateCreate(void *context, void *native)
{
   WLPL_WaylandDisplay *display = (WLPL_WaylandDisplay *)context;
   struct wl_egl_window *wl_egl_window = (struct wl_egl_window *)native;

   WLPL_WaylandWindow *window = NULL;

   if (display && wl_egl_window && wl_egl_window->surface)
   {
      window = calloc(1, sizeof(*window));
      if (window)
      {
         DisplayInterface_InitWayland(&window->display_interface,
               &display->client, &display->fence_interface,
               wl_egl_window, SWAPCHAIN_COUNT);

         SurfaceInterface_InitWayland(&window->surface_interface,
               &display->client, WlReleaseCallback, window);

         if (DisplayFramework_Start(&window->display_framework,
               &window->display_interface, &display->fence_interface,
               &window->surface_interface, wl_egl_window->width,
               wl_egl_window->height, SWAPCHAIN_COUNT))
         {
            window->wl_egl_window = wl_egl_window;
            window->wl_egl_window->callback_private = window;
            window->wl_egl_window->resize_callback = WlResizeCallback;
            window->wl_egl_window->get_attached_size_callback = WlGetAttachedCallback;
            window->wl_egl_window->destroy_window_callback = NULL;
         }
         else
         {
            Interface_Destroy(&window->display_interface.base);
            Interface_Destroy(&window->surface_interface.base);
            free(window);
            window = NULL;
         }

      }
   }
   return window;
}

static BEGL_Error WlWindowPlatformStateDestroy(void *context, void *windowState)
{
   UNUSED(context);
   WLPL_WaylandWindow *window = (WLPL_WaylandWindow *)windowState;

   if (window)
   {
      window->wl_egl_window->callback_private = NULL;
      window->wl_egl_window->get_attached_size_callback = NULL;
      window->wl_egl_window->destroy_window_callback = NULL;
      window->wl_egl_window = NULL;

      DisplayFramework_Stop(&window->display_framework);

      Interface_Destroy(&window->display_interface.base);
      Interface_Destroy(&window->surface_interface.base);

#ifndef NDEBUG
      memset(window, 0, sizeof(*window));
#endif
      free(window);
   }
   return BEGL_Success;
}

static bool WlBindWaylandDisplay(void *context, void *egl_display,
      void *wl_display)
{
   WLPL_WaylandDisplay *display = (WLPL_WaylandDisplay *)context;
   BSTD_UNUSED(egl_display);
   NEXUS_HeapHandle heap = GetDefaultHeap();
   NEXUS_HeapHandle secure_heap = GetSecureHeap();
   return BindWlDisplay(&display->binding, wl_display, heap, secure_heap);
}

static bool WlUnbindWaylandDisplay(void *context, void *egl_display,
      void *wl_display)
{
   WLPL_WaylandDisplay *display = (WLPL_WaylandDisplay *)context;
   BSTD_UNUSED(egl_display);
   return UnbindWlDisplay(&display->binding, wl_display);
}

static bool WlQueryBuffer(void *context, void *display, void* buffer,
      int32_t attribute, int32_t *value)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(display);

   WlBufferMemory *memory = GetWlBufferMemory((struct wl_resource*)buffer);
   if (!memory)
      return false;

   switch (attribute)
   {
   case EGL_WIDTH:
      *value = memory->settings.width;
      break;

   case EGL_HEIGHT:
      *value = memory->settings.height;
      break;

   case EGL_TEXTURE_FORMAT:
   {
      NEXUS_PixelFormat result = NEXUS_PixelFormat_eUnknown;
      BeglToNexusFormat(&result, memory->settings.format);
      switch (result)
      {
      case NEXUS_PixelFormat_eA8_B8_G8_R8:
         *value = EGL_TEXTURE_RGBA;
         break;

      case NEXUS_PixelFormat_eR5_G6_B5:
         *value = EGL_TEXTURE_RGB;
         break;

      default:
         return false;
      }
      break;
   }
   case EGL_WAYLAND_Y_INVERTED_WL:
      *value = EGL_TRUE;
      break;

   default:
      return false;
   }
   return true;
}

struct BEGL_DisplayInterface *WLPL_CreateWaylandDisplayInterface(
      struct wl_display *wl_display, struct BEGL_SchedInterface *schedIface)
{
   WLPL_WaylandDisplay *display = calloc(1, sizeof(*display));
   BEGL_DisplayInterface *iface = calloc(1, sizeof(*iface));
   if (display && iface && CreateWlClient(&display->client, wl_display))
   {
      InitFenceInterface(&display->fence_interface, schedIface);

      iface->context = display;
      iface->WindowGetInfo                = WlWindowGetInfo;
      iface->GetNativeSurface             = WlGetNativeSurface;
      iface->SurfaceGetInfo               = WlSurfaceGetInfo;
      iface->SurfaceChangeRefCount        = WlSurfaceChangeRefCount;
      iface->GetNextSurface               = WlGetNextSurface;
      iface->DisplaySurface               = WlDisplaySurface;
      iface->CancelSurface                = WlCancelSurface;
      iface->GetDisplayExtensions         = WlGetDisplayExtensions;
      iface->WindowPlatformStateCreate    = WlWindowPlatformStateCreate;
      iface->WindowPlatformStateDestroy   = WlWindowPlatformStateDestroy;
      iface->BindWaylandDisplay           = WlBindWaylandDisplay;
      iface->UnbindWaylandDisplay         = WlUnbindWaylandDisplay;
      iface->QueryBuffer                  = WlQueryBuffer;
   }
   else
   {
      free(display);
      display = NULL;
      free(iface);
      iface = NULL;
   }
   return iface;
}

void WLPL_DestroyWaylandDisplayInterface(BEGL_DisplayInterface *iface)
{
   if (iface)
   {
      WLPL_WaylandDisplay *display = (WLPL_WaylandDisplay *)iface->context;
      Interface_Destroy(&display->fence_interface.base);
      DestroyWlClient(&display->client);
#ifndef NDEBUG
      memset(display, 0, sizeof(*display));
      memset(iface, 0, sizeof(*iface));
#endif
      free(display);
      free(iface);
   }
}
