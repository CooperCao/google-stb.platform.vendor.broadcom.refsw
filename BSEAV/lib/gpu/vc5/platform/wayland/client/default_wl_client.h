/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DEFAULT_WAYLAND_CLIENT_H__
#define __DEFAULT_WAYLAND_CLIENT_H__

#include "wl_client.h"
#include "display_helpers.h"
#include "fence_interface.h"
#include "surface_interface.h"

#include <EGL/begl_memplatform.h>
#include <EGL/begl_schedplatform.h>
#include <EGL/begl_displayplatform.h>

#include <stdbool.h>

typedef struct WaylandClientPlatform
{
   struct WaylandClient    client;
   FenceInterface          fence_interface;
   SurfaceInterface        surface_interface;
   bool                    joined;
   bool                    drm;
   BEGL_MemoryInterface    *memoryInterface;
   BEGL_SchedInterface     *schedInterface;
   BEGL_DisplayInterface   *displayInterface;
} WaylandClientPlatform;

#endif /* __DEFAULT_WAYLAND_CLIENT_H__ */
