/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "display_wl_server.h"

#include "bind_wl_display.h"
#include "private_nexus.h"
#include "display_nexus.h"
#include "memory_nexus.h"
#include "sched_nexus.h"
#include "display_interface.h"
#include "display_framework.h"
#include "display_nexus_multi.h"
#include "display_surface.h"
#include "platform_common.h"
#include "fence_queue.h"

#include "nexus_base_types.h"
#include "nexus_display.h"

#include <EGL/eglext_wayland.h>
#include <wayland-util.h>
#include <wayland-server.h>

#include "../../common/fence_interface_nexus.h"
#include "../../common/surface_interface_nexus.h"

#ifdef NXPL_PLATFORM_EXCLUSIVE
#error Wayland platform requires multi-process mode!
#endif

typedef struct PlatformState
{
   SurfaceInterface surface_interface;
   FenceInterface fence_interface;
   DisplayInterface display_interface;
   NXPL_NativeWindow *native_window;
   DisplayFramework display_framework;
} PlatformState;


static BEGL_Error WLSWindowGetInfo(void *context, void *nativeWindow,
      BEGL_WindowInfoFlags flags, BEGL_WindowInfo *info)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;
   NXPL_NativeWindow *nw = state->native_window;

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

static BEGL_Error WLSGetNextSurface(void *context, void *nativeWindow,
      BEGL_BufferFormat format, BEGL_BufferFormat *actualFormat,
      void **nativeSurface, bool secure, int *fence)
{
   UNUSED(context);
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

static BEGL_Error WLSDisplaySurface(void *context, void *nativeWindow,
      void *nativeSurface, int fence)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;
   if (state && nativeSurface)
   {
      DisplayFramework_DisplaySurface(&state->display_framework,
            nativeSurface, fence, state->native_window->swapInterval);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error WLSCancelSurface(void *context, void *nativeWindow,
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

static void WLSSetSwapInterval(void *context, void *nativeWindow, int interval)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;
   if (interval >= 0)
      state->native_window->swapInterval = interval;
}

static BEGL_Error WLSWindowPlatformStateDestroy(void *context, void *windowState)
{
   NXPL_DisplayContext *ctx = (NXPL_DisplayContext *) context;
   PlatformState *state = (PlatformState *) windowState;

   if (state)
   {
      DisplayFramework_Stop(&state->display_framework);
      Interface_Destroy(&state->display_interface.base);
      Interface_Destroy(&state->surface_interface.base);
      Interface_Destroy(&state->fence_interface.base);
#ifndef NDEBUG
      /* catch some cases of use after free */
      memset(state, 0, sizeof(*state));
#endif
      free(state);
   }
   return BEGL_Success;
}

static void *WLSWindowPlatformStateCreate(void *context, void *native)
{
   NXPL_DisplayContext *ctx = (NXPL_DisplayContext *) context;
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*) native;
   PlatformState *state = NULL;

   if (ctx && nw)
   {

      nw->numSurfaces = 2;
      nw->swapInterval = 1;

      state = calloc(1, sizeof(*state));
      if (state)
      {
         FenceInteraface_InitNexus(&state->fence_interface, ctx->schedIface);
         SurfaceInterface_InitNexus(&state->surface_interface);
         DisplayInterface_InitNexusMulti(&state->display_interface,
               &state->fence_interface, &nw->windowInfo, ctx->displayType,
               nw->numSurfaces, nw->clientID, nw->surfaceClient);

         state->native_window = nw;

         if (!DisplayFramework_Start(&state->display_framework,
               &state->display_interface,
               &state->fence_interface,
               &state->surface_interface,
               state->native_window->windowInfo.width,
               state->native_window->windowInfo.height,
               state->native_window->numSurfaces))
         {
            WLSWindowPlatformStateDestroy(context, state);
            state = NULL;
         }
      }
   }
   return state;
}

static const char *WLSGetClientExtensions(void *context)
{
   BSTD_UNUSED(context);
   return NULL;
}

static const char *WLSGetDisplayExtensions(void *context)
{
   BSTD_UNUSED(context);
   return "EGL_WL_bind_wayland_display";
}

WLServer_DisplayContext *ToWlServerDisplayContext(
      NXPL_DisplayContext *context)
{
   WLServer_DisplayContext *wl_context = wl_container_of(context,
         wl_context, parent);
   return wl_context;
}

static BEGL_Error WLSSurfaceGetInfo(void *context, void *opaqueNativeSurface, BEGL_SurfaceInfo *info)
{
   WLServer_DisplayContext *wl_context = ToWlServerDisplayContext(context);
   if (!isNXPL_Surface(opaqueNativeSurface))
   {
      /* assume wl_buffer */
      return GetWlSurfaceInfo(wl_context,
            (struct wl_resource*) opaqueNativeSurface, info) ?
                  BEGL_Success : BEGL_Fail;
   }
   else /* call inherited function */
   {
      return wl_context->SurfaceGetInfo(context, opaqueNativeSurface, info);
   }
}

static BEGL_Error WLSSurfaceVerifyImageTarget(void *context,
      void *nativeSurface, uint32_t eglTarget)
{
   WLServer_DisplayContext *wl_context = ToWlServerDisplayContext(context);

   if (eglTarget == EGL_WAYLAND_BUFFER_WL)
   {
      return  BEGL_Success;
   }
   else /* call inherited function */
   {
      return wl_context->SurfaceVerifyImageTarget(context,
                        nativeSurface, eglTarget);
   }
}

static bool WLSBindWaylandDisplay(void *context, void *egl_display, void *wl_display)
{
   WLServer_DisplayContext *wl_context = ToWlServerDisplayContext(context);
   BSTD_UNUSED(egl_display);
   return BindWlDisplay(wl_context, wl_display);
}

static bool WLSUnbindWaylandDisplay(void *context, void *egl_display, void *wl_display)
{
   WLServer_DisplayContext *wl_context = ToWlServerDisplayContext(context);
   BSTD_UNUSED(egl_display);
   return UnbindWlDisplay(wl_context, wl_display);
}

static bool WLSQueryBuffer(void *context, void *display, void* buffer,
      int32_t attribute, int32_t *value)
{
   WLServer_DisplayContext *wl_context = ToWlServerDisplayContext(context);
   BSTD_UNUSED(display);
   return QueryWlBuffer(wl_context, (struct wl_resource*) buffer, attribute,
         value);
}

BEGL_DisplayInterface *CreateDisplayInterfaceWaylandServer(NEXUS_DISPLAYHANDLE display, struct BEGL_SchedInterface *schedIface)
{
   WLServer_DisplayContext *context = calloc(1, sizeof(*context));
   BEGL_DisplayInterface *disp = CreateDisplayInterface(display, &context->parent, schedIface);
   if (disp)
   {
      disp->WindowGetInfo = WLSWindowGetInfo;
      /* disp->SurfaceChangeRefCount      = use the Nexus platform function */
      disp->GetNextSurface = WLSGetNextSurface;
      disp->DisplaySurface = WLSDisplaySurface;
      disp->CancelSurface = WLSCancelSurface;
      disp->SetSwapInterval = WLSSetSwapInterval;
      /* disp->PlatformSupported          = use the Nexus platform function */
      /* disp->SetDefaultDisplay          = use the Nexus platform function */
      /* disp->GetDefaultDisplay          = use the Nexus platform function */
      disp->GetDisplayExtensions = WLSGetDisplayExtensions;
      disp->GetClientExtensions = WLSGetClientExtensions;
      disp->WindowPlatformStateCreate = WLSWindowPlatformStateCreate;
      disp->WindowPlatformStateDestroy = WLSWindowPlatformStateDestroy;
      disp->BindWaylandDisplay = WLSBindWaylandDisplay;
      disp->UnbindWaylandDisplay = WLSUnbindWaylandDisplay;
      disp->QueryBuffer = WLSQueryBuffer;

      /* override but allow calling the inherited function */
      context->SurfaceGetInfo = disp->SurfaceGetInfo;
      disp->SurfaceGetInfo = WLSSurfaceGetInfo;

      context->SurfaceVerifyImageTarget = disp->SurfaceVerifyImageTarget;
      disp->SurfaceVerifyImageTarget = WLSSurfaceVerifyImageTarget;
   }
   return disp;
}

void DestroyDisplayInterfaceWaylandServer(BEGL_DisplayInterface *disp)
{
   /* nothing custom to destroy here */
   DestroyDisplayInterface(disp);
}
