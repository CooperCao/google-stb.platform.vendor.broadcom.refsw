/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __SURFACE_INTERFACE_WL_CLIENT_H__
#define __SURFACE_INTERFACE_WL_CLIENT_H__

#include "surface_interface.h"

#include <EGL/begl_displayplatform.h>
#include "wl_client.h"
#include "list.h"
#include <stdbool.h>

/* A surface interface implementation that creates Wayland client surfaces */

typedef struct WaylandClientBuffer
{
   BEGL_SurfaceInfo info;
   struct wl_buffer *buffer;
   void (*buffer_release_callback)(void *data, struct wl_buffer *wl_buffer);
   struct list link;
} WaylandClientBuffer;

bool SurfaceInterface_InitWlClient(SurfaceInterface *si,
      const WaylandClient *client);

#endif /* __SURFACE_INTERFACE_WL_CLIENT_H__ */
