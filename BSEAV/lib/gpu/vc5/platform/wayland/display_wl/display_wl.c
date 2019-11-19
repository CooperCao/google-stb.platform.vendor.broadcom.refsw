/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "wl_server.h"
#include "wl_priv.h"
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
   BEGL_SchedInterface    *schedIface;
   WlClient client;
   WlDisplayBinding binding;
} WLPL_WaylandDisplay;

typedef struct WindowState
{
   struct wl_egl_window *wl_egl_window;
   DisplayFramework display_framework;
   uint32_t width;
   uint32_t height;
} WindowState;

static BEGL_NativeBuffer WlSurfaceAcquire(void *context, uint32_t target,
      void *eglObject, uint32_t plane, BEGL_SurfaceInfo *info)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(plane);

   switch (target)
   {
   case EGL_WAYLAND_BUFFER_WL:
   {
      struct wl_resource *resource = (struct wl_resource *)eglObject;
      NEXUS_MemoryBlockHandle memory = AcquireWlBufferMemory(resource);
      if (memory)
         GetWlBufferSettings(resource, info);
      return (BEGL_NativeBuffer)memory;
   }
   case BEGL_SWAPCHAIN_BUFFER:
   {
      WlWindowBuffer *buffer = (WlWindowBuffer *)eglObject;
      if (buffer)
         *info = buffer->buffer.settings;
      /* TODO: implement reference counting */
      return (BEGL_NativeBuffer)buffer;
   }
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_PIXMAP_BUFFER:
   default:
      return NULL;
   }
}

static BEGL_Error WlSurfaceRelease(void *context, uint32_t target,
      uint32_t plane, BEGL_NativeBuffer buffer)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(plane);

   switch (target)
   {
   case EGL_WAYLAND_BUFFER_WL:
   {
      ReleaseWlBufferMemory((NEXUS_MemoryBlockHandle)buffer);
      return BEGL_Success;
   }
   case BEGL_SWAPCHAIN_BUFFER:
   {
      /* TODO: implement reference counting */
      return BEGL_Success;
   }
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_PIXMAP_BUFFER:
   default:
      return BEGL_Fail;
   }
}

static BEGL_SwapchainBuffer WlGetNextSurface(void *context, void *nativeWindow,
      BEGL_BufferFormat format, bool secure,
      int *age, int *fence)
{
   UNUSED(context);
   UNUSED(secure);

   WindowState *state = (WindowState *)nativeWindow;

   if (state == NULL)
      return NULL;

   WindowInfo windowInfo = { 0 };
   windowInfo.width = state->width;
   windowInfo.height = state->height;
   windowInfo.magic = NATIVE_WINDOW_INFO_MAGIC;

   WlWindowBuffer *buffer = DisplayFramework_GetNextSurface(&state->display_framework,
         format, secure, age, fence, &windowInfo);

   return (BEGL_SwapchainBuffer)buffer;
}

static BEGL_Error WlDisplaySurface(void *context, void *nativeWindow,
      BEGL_SwapchainBuffer nativeSurface, int fence, int interval)
{
   UNUSED(context);
   WindowState *state = (WindowState *)nativeWindow;
   if (state && nativeSurface)
   {
      WindowInfo windowInfo = { 0 };
      windowInfo.width = state->wl_egl_window->width;
      windowInfo.height = state->wl_egl_window->height;
      windowInfo.magic = NATIVE_WINDOW_INFO_MAGIC;

      state->wl_egl_window->attached_width = state->wl_egl_window->width;
      state->wl_egl_window->attached_height = state->wl_egl_window->height;

      DisplayFramework_DisplaySurface(&state->display_framework, nativeSurface,
            fence, interval, &windowInfo);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error WlCancelSurface(void *context, void *nativeWindow,
      BEGL_SwapchainBuffer nativeSurface, int fence)
{
   UNUSED(context);
   WindowState *state = (WindowState *)nativeWindow;
   if (state && nativeSurface)
   {
      DisplayFramework_CancelSurface(&state->display_framework, nativeSurface,
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
   WindowState *state = (WindowState *)context;
   state->width = wl_egl_window->width;
   state->height = wl_egl_window->height;
}

static void WlReleaseCallback(void *data, struct wl_buffer *wl_buffer)
{
   WindowState *state = (WindowState *)data;
   DisplayFramework *df = (DisplayFramework *)&state->display_framework;
   DisplayInterface_WlBufferRelease(&df->display_interface, wl_buffer);
}

static void *WlWindowPlatformStateCreate(void *context, void *native)
{
   WLPL_WaylandDisplay *display = (WLPL_WaylandDisplay *)context;
   struct wl_egl_window *wl_egl_window = (struct wl_egl_window *)native;

   WindowState *state = NULL;

   if (display && wl_egl_window && wl_egl_window->surface)
   {
      state = calloc(1, sizeof(*state));
      if (state)
      {
         DisplayFramework *df = (DisplayFramework *)&state->display_framework;
         InitFenceInterface(&df->fence_interface, display->schedIface);

         DisplayInterface_InitWayland(&df->display_interface,
               &display->client, &df->fence_interface,
               wl_egl_window, SWAPCHAIN_COUNT);

         SurfaceInterface_InitWayland(&df->surface_interface,
               &display->client, WlReleaseCallback, state);

         state->width = wl_egl_window->width;
         state->height = wl_egl_window->height;

         if (DisplayFramework_Start(df, SWAPCHAIN_COUNT))
         {
            state->wl_egl_window = wl_egl_window;
            state->wl_egl_window->callback_private = state;
            state->wl_egl_window->resize_callback = WlResizeCallback;
            state->wl_egl_window->destroy_window_callback = NULL;
         }
         else
         {
            Interface_Destroy(&df->display_interface.base);
            Interface_Destroy(&df->surface_interface.base);
            Interface_Destroy(&df->fence_interface.base);
            free(state);
            state = NULL;
         }

      }
   }
   return state;
}

static BEGL_Error WlWindowPlatformStateDestroy(void *context, void *windowState)
{
   UNUSED(context);
   WindowState *state = (WindowState *)windowState;

   if (state)
   {
      state->wl_egl_window->callback_private = NULL;
      state->wl_egl_window->get_attached_size_callback = NULL;
      state->wl_egl_window->destroy_window_callback = NULL;
      state->wl_egl_window = NULL;

      DisplayFramework *df = (DisplayFramework *)&state->display_framework;

      DisplayFramework_Stop(df);

      Interface_Destroy(&df->display_interface.base);
      Interface_Destroy(&df->surface_interface.base);
      Interface_Destroy(&df->fence_interface.base);

#ifndef NDEBUG
      memset(state, 0, sizeof(*state));
#endif
      free(state);
   }
   return BEGL_Success;
}

static bool WlBindWaylandDisplay(void *context, void *egl_display,
      void *wl_display)
{
   WLPL_WaylandDisplay *display = (WLPL_WaylandDisplay *)context;
   BSTD_UNUSED(egl_display);
   return BindWlDisplay(&display->binding, wl_display);
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

   struct wl_resource *resource = (struct wl_resource*)buffer;
   BEGL_SurfaceInfo settings = {};
   if (!GetWlBufferSettings(resource, &settings))
      return false;

   switch (attribute)
   {
   case EGL_WIDTH:
      *value = settings.width;
      break;

   case EGL_HEIGHT:
      *value = settings.height;
      break;

   case EGL_TEXTURE_FORMAT:
      switch (settings.format)
      {
      case BEGL_BufferFormat_eA8B8G8R8:
      case BEGL_BufferFormat_eR8G8B8A8:
      case BEGL_BufferFormat_eR4G4B4A4:
      case BEGL_BufferFormat_eA4B4G4R4:
      case BEGL_BufferFormat_eA1B5G5R5:
      case BEGL_BufferFormat_eR5G5B5A1:
      case BEGL_BufferFormat_eBSTC:
      case BEGL_BufferFormat_eTILED:
         *value = EGL_TEXTURE_RGBA;
         break;
      case BEGL_BufferFormat_eX8B8G8R8:
      case BEGL_BufferFormat_eR8G8B8X8:
      case BEGL_BufferFormat_eR5G6B5:
         *value = EGL_TEXTURE_RGB;
         break;
      /* we don't render into any of the formats below */
      case BEGL_BufferFormat_eYUV422:
      case BEGL_BufferFormat_eYV12:
      case BEGL_BufferFormat_eY8:
      case BEGL_BufferFormat_eCr8Cb8:
      case BEGL_BufferFormat_eCb8Cr8:
      case BEGL_BufferFormat_eY10:
      case BEGL_BufferFormat_eCr10Cb10:
      case BEGL_BufferFormat_eCb10Cr10:
      default:
         return false;
      }
      break;

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
      display->schedIface                 = schedIface;

      iface->context = display;
      iface->GetPixmapFormat              = NULL; /* Wayland doesn't have pixmaps */
      iface->SurfaceAcquire               = WlSurfaceAcquire;
      iface->SurfaceRelease               = WlSurfaceRelease;
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
      DestroyWlClient(&display->client);
#ifndef NDEBUG
      memset(display, 0, sizeof(*display));
      memset(iface, 0, sizeof(*iface));
#endif
      free(display);
      free(iface);
   }
}
