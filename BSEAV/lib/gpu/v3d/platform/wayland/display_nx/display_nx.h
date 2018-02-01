/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <EGL/egl.h>

#include "default_wayland.h"

#ifdef __cplusplus
extern "C"
{
#endif

BEGL_DisplayInterface *WLPL_CreateNexusDisplayInterface(
      BEGL_MemoryInterface *memIface, BEGL_HWInterface *hwIface,
      BEGL_DisplayCallbacks *displayCallbacks);

void WLPL_DestroyNexusDisplayInterface(BEGL_DisplayInterface *iface);

#ifdef __cplusplus
}
#endif
