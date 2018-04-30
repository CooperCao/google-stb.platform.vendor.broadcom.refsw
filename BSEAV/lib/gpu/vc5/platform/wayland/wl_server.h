/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "nexus_memory.h"
#include <EGL/begl_displayplatform.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

typedef struct WlDisplayBinding
{
   struct wl_display *wl_display;
   struct wl_global *wl_global;
} WlDisplayBinding;

struct wl_resource;

bool BindWlDisplay(WlDisplayBinding *binding, struct wl_display *wl_display);

bool UnbindWlDisplay(WlDisplayBinding *binding, struct wl_display *wl_display);

bool GetWlBufferSettings(struct wl_resource *buffer,
      BEGL_SurfaceInfo *settings);

NEXUS_MemoryBlockHandle AcquireWlBufferMemory(struct wl_resource *buffer);

void ReleaseWlBufferMemory(NEXUS_MemoryBlockHandle handle);

#ifdef __cplusplus
}
#endif
