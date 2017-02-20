/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __WL_CLIENT_H__
#define __WL_CLIENT_H__

#include "wayland-client.h"
#include "wayland_nexus_client.h"
#include <stdbool.h>

typedef struct WaylandClient
{
   struct wl_display *display;
   struct wl_registry *registry;
   struct wl_event_queue *events;
   struct wl_nexus *nexus;
} WaylandClient;


extern bool InitWaylandClient(WaylandClient *wlc, struct wl_display *display);
extern void DestroyWaylandClient(WaylandClient *wlc);

#endif /* __WL_CLIENT_H__ */
