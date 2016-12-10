/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __BIND_WL_DISPLAY_H__
#define __BIND_WL_DISPLAY_H__

#include "display_nexus.h"

#include "libs/khrn/egl/egl_types.h"
#include <wayland-util.h>

typedef struct WLServer_DisplayContext
{
   /* inherited from Nexus platform */
   NXPL_DisplayContext parent; /* for functions inherited from Nexus platform */
   BEGL_Error (*SurfaceGetInfo)(void *context, void *opaqueNativeSurface, BEGL_SurfaceInfo *info);
   BEGL_Error (*SurfaceVerifyImageTarget)(void *context,
         void *opaqueNativeSurface, uint32_t eglTarget);

   struct wl_display *display;
   struct wl_global *global;
   struct wl_resource *nexus;

   struct wl_list clients;
} WLServer_DisplayContext;

bool BindWlDisplay(WLServer_DisplayContext *wl_context,
      struct wl_display *wl_display);

bool UnbindWlDisplay(WLServer_DisplayContext *wl_context,
      struct wl_display *wl_display);

bool GetWlSurfaceInfo(WLServer_DisplayContext *wl_context,
      struct wl_resource *bufferResource, BEGL_SurfaceInfo *info);

bool QueryWlBuffer(WLServer_DisplayContext *wl_context,
      struct wl_resource *bufferResource, int32_t attribute,
      int32_t *value);

#endif /* __BIND_WL_DISPLAY_H__ */
