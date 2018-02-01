/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "EGL/egl.h"
#include "wl_server.h"
#include "wl_client.h"

#ifdef __cplusplus
extern "C"
{
#endif

typedef void (*BufferGetRequirementsFunc)(
      BEGL_PixmapInfoEXT const *bufferRequirements,
      BEGL_BufferSettings *bufferConstrainedRequirements);

typedef struct
{
   BufferGetRequirementsFunc bufferGetRequirementsFunc;
   BEGL_MemoryInterface *memInterface;
   BEGL_HWInterface *hwInterface;
   WlClient client;
   WlDisplayBinding binding;
} WLPL_WaylandDisplay;

extern NEXUS_HeapHandle NXPL_MemHeap(BEGL_MemoryInterface *mem);
extern NEXUS_HeapHandle NXPL_MemHeapSecure(BEGL_MemoryInterface *mem);

#ifdef __cplusplus
}
#endif
