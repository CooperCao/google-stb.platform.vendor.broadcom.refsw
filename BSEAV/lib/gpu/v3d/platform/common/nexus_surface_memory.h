/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "nexus_begl_format.h"

#include "nexus_memory.h"
#include "nexus_surface.h"
#include <EGL/begl_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

NEXUS_MemoryBlockHandle AcquireNexusSurfaceMemory(
      NEXUS_SurfaceHandle surface, BEGL_BufferSettings *settings);

void ReleaseNexusSurfaceMemory(NEXUS_MemoryBlockHandle memory);

#ifdef __cplusplus
}
#endif
