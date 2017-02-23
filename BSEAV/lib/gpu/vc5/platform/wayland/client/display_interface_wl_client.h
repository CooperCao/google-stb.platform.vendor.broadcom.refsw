/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_INTERFACE_WL_CLIENT_H__
#define __DISPLAY_INTERFACE_WL_CLIENT_H__

#include "display_interface.h"
#include "default_wl_client.h"
#include "fence_interface.h"
#include "wayland-egl.h"

/* An implementation of display interface that uses provided Wayland client
 * and fence interface do show WaylandClientSurface surfaces.
 */

bool DisplayInterface_InitWlClient(DisplayInterface *di,
      WaylandClient *client, const FenceInterface *fi,
      struct wl_egl_window *window, int buffers);

#endif /* __DISPLAY_INTERFACE_WL_CLIENT_H__ */
