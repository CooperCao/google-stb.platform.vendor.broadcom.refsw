/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_WAYLAND_SERVER_H__
#define __DISPLAY_WAYLAND_SERVER_H__

#include "default_wl_server.h"
#include "sched_abstract.h"

#ifdef __cplusplus
extern "C" {
#endif

struct BEGL_DisplayInterface;
struct BEGL_SchedInterface;

struct BEGL_DisplayInterface *CreateDisplayInterfaceWaylandServer(NEXUS_DISPLAYHANDLE display, struct BEGL_SchedInterface *schedIface);
void DestroyDisplayInterfaceWaylandServer(struct BEGL_DisplayInterface *disp);

#ifdef __cplusplus
}
#endif

#endif /* __DISPLAY_WAYLAND_SERVER_H__ */
