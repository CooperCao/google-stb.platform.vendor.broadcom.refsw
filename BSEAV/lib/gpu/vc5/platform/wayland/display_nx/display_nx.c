/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "display_nx.h"
#include "default_wayland.h"

#include "wl_server.h"
#include "private_nexus.h"
#include "memory_nexus.h"
#include "sched_nexus.h"
#include "display_interface.h"
#include "display_framework.h"
#include "display_nexus_multi.h"
#include "display_surface.h"
#include "display_helpers.h"
#include "platform_common.h"
#include "fence_queue.h"
#include "nexus_heap_selection.h"

#include "nexus_base_types.h"
#include "nexus_display.h"

#include <EGL/eglext.h>
#include <EGL/eglext_wayland.h>
#include <wayland-util.h>
#include <wayland-server.h>
#include <string.h>

#include "../../common/surface_interface_nexus.h"

#ifdef SINGLE_PROCESS
#error Wayland platform requires multi-process mode!
#endif

typedef struct WLPL_NexusWindow
{
   DisplayInterface display_interface;
   NXPL_NativeWindow *native_window;
   DisplayFramework display_framework;
} WLPL_NexusWindow;

typedef struct WLPL_NexusDisplay
{
   /* inherited from Nexus platform */
   NXPL_DisplayContext parent; /* for functions inherited from Nexus platform */
   BEGL_NativeBuffer (*SurfaceAcquire)(void *context, uint32_t target,
         void *eglObject, uint32_t plane, BEGL_SurfaceInfo *info);
   BEGL_Error (*SurfaceRelease)(void *context, uint32_t target,
         uint32_t plane, BEGL_NativeBuffer buffer);

   FenceInterface fence_interface;
   SurfaceInterface surface_interface;
   WlDisplayBinding binding;
} WLPL_NexusDisplay;

static WLPL_NexusDisplay *ToWlplNexusDisplay(NXPL_DisplayContext *context)
{
   WLPL_NexusDisplay *display =
         context ? wl_container_of(context, display, parent) : NULL;
   return display;
}

static BEGL_Error NxWindowGetInfo(void *context, void *nativeWindow,
      BEGL_WindowInfoFlags flags, BEGL_WindowInfo *info)
{
   UNUSED(context);
   WLPL_NexusWindow *window = (WLPL_NexusWindow *)nativeWindow;
   NXPL_NativeWindow *nw = window->native_window;

   if (nw != NULL)
   {
      if (flags & BEGL_WindowInfoWidth)
         info->width = nw->windowInfo.width;
      if (flags & BEGL_WindowInfoHeight)
         info->height = nw->windowInfo.height;

      if (flags & BEGL_WindowInfoSwapChainCount)
         info->swapchain_count = nw->numSurfaces;

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_SwapchainBuffer NxGetNextSurface(void *context, void *nativeWindow,
      BEGL_BufferFormat format, bool secure,
      int *age, int *fence)
{
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(context);
   WLPL_NexusWindow *window = (WLPL_NexusWindow *)nativeWindow;

   if (window == NULL)
      return NULL;

   NXPL_Surface *nxpl_surface = DisplayFramework_GetNextSurface(
         &window->display_framework, format, secure, age, fence);

   return (BEGL_SwapchainBuffer)nxpl_surface;
}

static BEGL_Error NxDisplaySurface(void *context, void *nativeWindow,
      BEGL_SwapchainBuffer nativeSurface, int fence, int interval)
{
   UNUSED(context);
   WLPL_NexusWindow *window = (WLPL_NexusWindow *)nativeWindow;
   if (window && nativeSurface)
   {
      DisplayFramework_DisplaySurface(&window->display_framework, nativeSurface,
            fence, interval);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error NxCancelSurface(void *context, void *nativeWindow,
      BEGL_SwapchainBuffer nativeSurface, int fence)
{
   UNUSED(context);
   WLPL_NexusWindow *state = (WLPL_NexusWindow *)nativeWindow;
   if (state && nativeSurface)
   {
      DisplayFramework_CancelSurface(&state->display_framework, nativeSurface,
            fence);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error NxWindowPlatformStateDestroy(void *context, void *windowState)
{
   NXPL_DisplayContext *ctx = (NXPL_DisplayContext *)context;
   WLPL_NexusWindow *window = (WLPL_NexusWindow *)windowState;

   if (window)
   {
      DisplayFramework_Stop(&window->display_framework);
      Interface_Destroy(&window->display_interface.base);
#ifndef NDEBUG
      /* catch some cases of use after free */
      memset(window, 0, sizeof(*window));
#endif
      free(window);
   }
   return BEGL_Success;
}

static void *NxWindowPlatformStateCreate(void *context, void *native)
{
   NXPL_DisplayContext *ctx = (NXPL_DisplayContext *)context;
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(ctx);
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;

   WLPL_NexusWindow *window = NULL;

   if (display && nw)
   {
      window = calloc(1, sizeof(*window));
      if (window)
      {
         DisplayInterface_InitNexusMulti(&window->display_interface,
               &display->fence_interface, &nw->windowInfo, ctx->displayType,
               nw->numSurfaces, nw->clientID, nw->surfaceClient, ctx->eventContext);

         window->native_window = nw;

         if (!DisplayFramework_Start(&window->display_framework,
               &window->display_interface, &display->fence_interface,
               &display->surface_interface,
               window->native_window->windowInfo.width,
               window->native_window->windowInfo.height,
               window->native_window->numSurfaces))
         {
            NxWindowPlatformStateDestroy(context, window);
            window = NULL;
         }
      }
   }
   return window;
}

static const char *NxGetDisplayExtensions(void *context)
{
   BSTD_UNUSED(context);
   return "EGL_WL_bind_wayland_display";
}

static BEGL_NativeBuffer NxSurfaceAcquire(void *context, uint32_t target,
      void *eglObject, uint32_t plane, BEGL_SurfaceInfo *info)
{
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(context);

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
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_SWAPCHAIN_BUFFER:
   case BEGL_PIXMAP_BUFFER:
   default:
      /* call inherited function */
      return display->SurfaceAcquire(context, target, eglObject, plane, info);
   }
}

static BEGL_Error NxSurfaceRelease(void *context, uint32_t target,
      uint32_t plane, BEGL_NativeBuffer buffer)
{
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(context);

   switch (target)
   {
   case EGL_WAYLAND_BUFFER_WL:
   {
      ReleaseWlBufferMemory((NEXUS_MemoryBlockHandle)buffer);
      return BEGL_Success;
   }
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_SWAPCHAIN_BUFFER:
   case BEGL_PIXMAP_BUFFER:
   default:
      /* call inherited function */
      return display->SurfaceRelease(context, target, plane, buffer);
   }
}

static bool NxBindWaylandDisplay(void *context, void *egl_display,
      void *wl_display)
{
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(context);
   BSTD_UNUSED(egl_display);
   return BindWlDisplay(&display->binding, wl_display);
}

static bool NxUnbindWaylandDisplay(void *context, void *egl_display,
      void *wl_display)
{
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(context);
   BSTD_UNUSED(egl_display);
   return UnbindWlDisplay(&display->binding, wl_display);
}

static bool NxQueryBuffer(void *context, void *display, void* buffer,
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

BEGL_DisplayInterface *WLPL_CreateNexusDisplayInterface(
      struct BEGL_SchedInterface *schedIface, EventContext *eventContext)
{
   WLPL_NexusDisplay *display = calloc(1, sizeof(*display));
   BEGL_DisplayInterface *iface = NULL;
   if (display)
      iface = CreateDisplayInterface(NULL, &display->parent, schedIface, eventContext);
   if (iface)
   {
      iface->WindowGetInfo                = NxWindowGetInfo;
      iface->GetNextSurface               = NxGetNextSurface;
      iface->DisplaySurface               = NxDisplaySurface;
      iface->CancelSurface                = NxCancelSurface;
      iface->GetDisplayExtensions         = NxGetDisplayExtensions;
      iface->WindowPlatformStateCreate    = NxWindowPlatformStateCreate;
      iface->WindowPlatformStateDestroy   = NxWindowPlatformStateDestroy;
      iface->BindWaylandDisplay           = NxBindWaylandDisplay;
      iface->UnbindWaylandDisplay         = NxUnbindWaylandDisplay;
      iface->QueryBuffer                  = NxQueryBuffer;

      /* iface->GetPixmapFormat           = use Nexus platform function */;

      /* override but allow calling the inherited function */
      display->SurfaceAcquire = iface->SurfaceAcquire;
      iface->SurfaceAcquire = NxSurfaceAcquire;

      display->SurfaceRelease = iface->SurfaceRelease;
      iface->SurfaceRelease = NxSurfaceRelease;

      InitFenceInterface(&display->fence_interface, schedIface);
      SurfaceInterface_InitNexus(&display->surface_interface);
   }
   else
   {
      free(display);
      display = NULL;
   }
   return iface;
}

void WLPL_DestroyNexusDisplayInterface(BEGL_DisplayInterface *iface)
{
   if (iface)
   {
      WLPL_NexusDisplay *display = ToWlplNexusDisplay(iface->context);

      Interface_Destroy(&display->surface_interface.base);
      Interface_Destroy(&display->fence_interface.base);
      DestroyDisplayInterface(iface); /* inherited from NXPL */
      free(display);
   }
}

void WLPL_GetDefaultNexusWindowInfoEXT(WLPL_NexusWindowInfoEXT *info)
{
   if (info != NULL)
   {
      NXPL_NativeWindowInfoEXT nxpl_info;
      NXPL_GetDefaultNativeWindowInfoEXT(&nxpl_info);
      info->alphaBlend = nxpl_info.alphaBlend;
      info->width = nxpl_info.width;
      info->height = nxpl_info.height;
      info->x = nxpl_info.x;
      info->y = nxpl_info.y;
      info->stretch = nxpl_info.stretch;
      info->clientID = nxpl_info.clientID;
      info->zOrder = nxpl_info.zOrder;
      info->colorBlend = nxpl_info.colorBlend;
      info->alphaBlend = nxpl_info.alphaBlend;
      info->magic = nxpl_info.magic;
   }
}

void *WLPL_CreateNexusWindowEXT(const WLPL_NexusWindowInfoEXT *info)
{
   NXPL_NativeWindowInfoEXT nxpl_info;
   nxpl_info.alphaBlend = info->alphaBlend;
   nxpl_info.width = info->width;
   nxpl_info.height = info->height;
   nxpl_info.x = info->x;
   nxpl_info.y = info->y;
   nxpl_info.stretch = info->stretch;
   nxpl_info.clientID = info->clientID;
   nxpl_info.zOrder = info->zOrder;
   nxpl_info.colorBlend = info->colorBlend;
   nxpl_info.alphaBlend = info->alphaBlend;
   nxpl_info.magic = info->magic;
   return NXPL_CreateNativeWindowEXT(&nxpl_info);
}

void WLPL_DestroyNexusWindow(void *native)
{
   NXPL_DestroyNativeWindow(native);
}
