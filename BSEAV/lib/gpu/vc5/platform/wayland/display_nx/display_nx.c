/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
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
   BEGL_Error (*GetNativeSurface)(void *context, uint32_t eglTarget,
         void *eglClientBuffer, void **opaqueNativeSurface);
   BEGL_Error (*SurfaceChangeRefCount)(void *context, void *opaqueNativeSurface,
         BEGL_RefCountMode inOrDec);
   BEGL_Error (*SurfaceGetInfo)(void *context, void *opaqueNativeSurface,
         BEGL_SurfaceInfo *info);

   FenceInterface fence_interface;
   SurfaceInterface surface_interface;
   WlDisplayBinding binding;
} WLPL_NexusDisplay;

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

      if (flags & BEGL_WindowInfoBackBufferAge)
         info->backBufferAge = nw->ageOfLastDequeuedSurface;

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_Error NxGetNextSurface(void *context, void *nativeWindow,
      BEGL_BufferFormat format, BEGL_BufferFormat *actualFormat,
      void **nativeSurface, bool secure, int *fence)
{
   UNUSED(context);
   WLPL_NexusWindow *window = (WLPL_NexusWindow *)nativeWindow;

   if (window == NULL || nativeSurface == NULL || actualFormat == NULL)
      return BEGL_Fail;

   *nativeSurface = DisplayFramework_GetNextSurface(&window->display_framework,
            format, secure, fence, &window->native_window->ageOfLastDequeuedSurface);

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

static BEGL_Error NxDisplaySurface(void *context, void *nativeWindow,
      void *nativeSurface, int fence, int interval)
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
      void *nativeSurface, int fence)
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

WLPL_NexusDisplay *ToWlplNexusDisplay(NXPL_DisplayContext *context)
{
   WLPL_NexusDisplay *display =
         context ? wl_container_of(context, display, parent) : NULL;
   return display;
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

static BEGL_Error NxGetNativeSurface(void *context, uint32_t eglTarget,
      void *eglSurface, void **nativeSurface)
{
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(context);

   if (eglTarget == EGL_WAYLAND_BUFFER_WL)
   {
      *nativeSurface = GetWlBufferMemory((struct wl_resource*)eglSurface);
      return BEGL_Success;
   }
   else /* call inherited function */
   {
      return display->GetNativeSurface(context, eglTarget, eglSurface,
            nativeSurface);
   }
}

static BEGL_Error NxSurfaceChangeRefCount(void *context,
      void *opaqueNativeSurface, BEGL_RefCountMode incOrDec)
{
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(context);

   if (isNXPL_Surface(opaqueNativeSurface))
   {
      return display->SurfaceChangeRefCount(context, opaqueNativeSurface,
            incOrDec);
   }
   else
   {
      WlBufferMemory *memory = (WlBufferMemory *)opaqueNativeSurface;
      if (incOrDec == BEGL_Increment)
         WlBufferMemoryRefInc(memory);
      else
         WlBufferMemoryRefDec(memory);
      return BEGL_Success;
   }
}

static BEGL_Error NxSurfaceGetInfo(void *context,
      void *opaqueNativeSurface, BEGL_SurfaceInfo *info)
{
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(context);

   NXPL_Surface *surface;
   if (isNXPL_Surface(opaqueNativeSurface))
   {
      surface = (NXPL_Surface *)opaqueNativeSurface;
      return display->SurfaceGetInfo(context, surface, info);
   }
   else if (opaqueNativeSurface)
   {
      WlBufferMemory *memory = (WlBufferMemory *)opaqueNativeSurface;
      *info = memory->settings;
      return BEGL_Success;
   }
   else
   {
      return BEGL_Fail;
   }
}

static bool NxBindWaylandDisplay(void *context, void *egl_display,
      void *wl_display)
{
   WLPL_NexusDisplay *display = ToWlplNexusDisplay(context);
   BSTD_UNUSED(egl_display);
   NEXUS_HeapHandle heap = GetDefaultHeap();
   NEXUS_HeapHandle secure_heap = GetSecureHeap();
   return BindWlDisplay(&display->binding, wl_display, heap, secure_heap);

   WlBufferMemory *GetWlBufferMemory(struct wl_resource *buffer);

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
      iface->GetNativeSurface             = NxGetNativeSurface;
      iface->GetNextSurface               = NxGetNextSurface;
      iface->DisplaySurface               = NxDisplaySurface;
      iface->CancelSurface                = NxCancelSurface;
      iface->GetDisplayExtensions         = NxGetDisplayExtensions;
      iface->WindowPlatformStateCreate    = NxWindowPlatformStateCreate;
      iface->WindowPlatformStateDestroy   = NxWindowPlatformStateDestroy;
      iface->BindWaylandDisplay           = NxBindWaylandDisplay;
      iface->UnbindWaylandDisplay         = NxUnbindWaylandDisplay;
      iface->QueryBuffer                  = NxQueryBuffer;

      /* override but allow calling the inherited function */
      display->GetNativeSurface = iface->GetNativeSurface;
      iface->GetNativeSurface = NxGetNativeSurface;

      display->SurfaceGetInfo = iface->SurfaceGetInfo;
      iface->SurfaceGetInfo = NxSurfaceGetInfo;

      display->SurfaceChangeRefCount = iface->SurfaceChangeRefCount;
      iface->SurfaceChangeRefCount = NxSurfaceChangeRefCount;

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
