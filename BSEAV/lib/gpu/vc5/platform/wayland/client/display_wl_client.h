/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_WAYLAND_CLIENT_H__
#define __DISPLAY_WAYLAND_CLIENT_H__

#include "default_wl_client.h"

#include <EGL/begl_displayplatform.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

struct BEGL_DisplayInterface *CreateDisplayInterfaceWaylandClient(
      WaylandClientPlatform *platform);

void DestroyDisplayInterfaceWaylandClient(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_WAYLAND_CLIENT_H__ */
