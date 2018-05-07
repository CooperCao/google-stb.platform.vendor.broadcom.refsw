/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "default_nexus.h"
#include "../common/perf_event.h"

#include <EGL/begl_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

BEGL_MemoryInterface  *NXPL_CreateMemInterface(BEGL_HWInterface *hwIface);
BEGL_HWInterface      *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks,
                                              EventContext *eventContext);
BEGL_DisplayInterface *NXPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
                                                   BEGL_HWInterface     *hwIface,
                                                   NEXUS_DISPLAYHANDLE display,
                                                   EventContext *eventContext);

void NXPL_DestroyMemInterface(BEGL_MemoryInterface *iface);
void NXPL_DestroyHWInterface(BEGL_HWInterface *iface);
void NXPL_DestroyDisplayInterface(BEGL_DisplayInterface *iface);

NEXUS_HeapHandle NXPL_MemHeap(BEGL_MemoryInterface *mem);
NEXUS_HeapHandle NXPL_MemHeapSecure(BEGL_MemoryInterface *mem);

#ifdef __cplusplus
}
#endif
