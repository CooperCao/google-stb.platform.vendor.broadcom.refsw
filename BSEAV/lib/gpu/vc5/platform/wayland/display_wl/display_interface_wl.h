/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "display_interface.h"
#include "wl_client.h"
#include "fence_interface.h"
#include "wayland-egl.h"
#include <stdbool.h>
#include "windowinfo.h"

#ifdef __cplusplus
extern "C" {
#endif

/* An implementation of display interface that uses provided Wayland client
 * and fence interface do show WlWindowBuffer surfaces.
 */

bool DisplayInterface_InitWayland(DisplayInterface *di, WlClient *client,
      const FenceInterface *fi, struct wl_egl_window *window, unsigned buffers);

void DisplayInterface_WlBufferRelease(DisplayInterface *di,
      struct wl_buffer *wl_buffer);

#ifdef __cplusplus
}
#endif
