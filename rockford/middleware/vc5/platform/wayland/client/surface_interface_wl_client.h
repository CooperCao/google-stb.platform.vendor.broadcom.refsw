/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __SURFACE_INTERFACE_WL_CLIENT_H__
#define __SURFACE_INTERFACE_WL_CLIENT_H__

#include "surface_interface.h"
#include "private_nexus.h" /* NXPL_Surface */
#include "default_wl_client.h"
#include "wayland_nexus_client.h"

#include <stdbool.h>

/* A surface interface implementation that creates Wayland client surfaces */

typedef struct WaylandClientBuffer
{
   NXPL_Surface surface;
   bool created;
   struct wl_buffer *buffer;
   void (*buffer_release_callback)(void *data, struct wl_buffer *wl_buffer);
} WaylandClientBuffer;

void SurfaceInterface_InitWlClient(SurfaceInterface *si,
      const WaylandClient *client);

#endif /* __SURFACE_INTERFACE_WL_CLIENT_H__ */
