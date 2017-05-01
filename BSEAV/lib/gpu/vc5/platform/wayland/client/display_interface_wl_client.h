/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_INTERFACE_WL_CLIENT_H__
#define __DISPLAY_INTERFACE_WL_CLIENT_H__

#include "display_interface.h"
#include "wl_client.h"
#include "fence_interface.h"
#include "wayland-egl.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

/* An implementation of display interface that uses provided Wayland client
 * and fence interface do show WaylandClientSurface surfaces.
 */

bool DisplayInterface_InitWlClient(DisplayInterface *di,
      WaylandClient *client, const FenceInterface *fi,
      struct wl_egl_window *window, int buffers);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_INTERFACE_WL_CLIENT_H__ */
