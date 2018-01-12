/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "surface_interface.h"

#include "wl_client.h"
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* A surface interface implementation that creates Wayland client surfaces */

typedef struct WlWindowBuffer
{
   WlSharedBuffer buffer;
   int dx;
   int dy;
} WlWindowBuffer;

bool SurfaceInterface_InitWayland(SurfaceInterface *si, WlClient *client,
      void (*release)(void *data, struct wl_buffer *wl_buffer), void *data);

#ifdef __cplusplus
}
#endif
