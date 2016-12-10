/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DEFAULT_WAYLAND_CLIENT_H__
#define __DEFAULT_WAYLAND_CLIENT_H__

#include "wayland_nexus_client.h"
#include "display_helpers.h"

#include <EGL/begl_memplatform.h>
#include <EGL/begl_schedplatform.h>
#include <EGL/begl_displayplatform.h>

#include <stdbool.h>

typedef struct WaylandClientDisplayPlatform
{
   BEGL_MemoryInterface    *memoryInterface;
   BEGL_SchedInterface     *schedInterface;
   BEGL_DisplayInterface   *displayInterface;
} WaylandClientDisplayPlatform;

typedef struct WaylandClient
{
   struct wl_display *display;
   struct wl_registry *registry;
   struct wl_event_queue *events;
   struct wl_nexus *nexus;
   bool joined;

   WaylandClientDisplayPlatform platform;
} WaylandClient;

#endif /* __DEFAULT_WAYLAND_CLIENT_H__ */
