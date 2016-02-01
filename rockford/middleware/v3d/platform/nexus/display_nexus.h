/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
This is a default implementation of a Nexus platform layer used by V3D.
This illustrates one way in which the abstract display interface might be
implemented. You can replace this with your own custom version if preferred.
=============================================================================*/

#ifndef __DISPLAY_NEXUS_H__
#define __DISPLAY_NEXUS_H__

#include "default_nexus.h"

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum NXPL_DisplayType
{
   NXPL_2D = 0,
   NXPL_3D_LEFT_RIGHT,
   NXPL_3D_OVER_UNDER
} NXPL_DisplayType;

BEGL_MemoryInterface  *NXPL_CreateMemInterface(BEGL_HWInterface *hwIface);
BEGL_HWInterface      *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks);
#ifdef SINGLE_PROCESS
BEGL_DisplayInterface *NXPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
                                                   BEGL_HWInterface     *hwIface,
                                                   NEXUS_DISPLAYHANDLE display,
                                                   BEGL_DisplayCallbacks *displayCallbacks);
#else
BEGL_DisplayInterface *NXPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
                                                   BEGL_HWInterface     *hwIface,
                                                   BEGL_DisplayCallbacks *displayCallbacks);
#endif

void NXPL_SetDisplayType(NXPL_PlatformHandle handle, NXPL_DisplayType type);
void NXPL_DestroyMemInterface(BEGL_MemoryInterface *iface);
void NXPL_DestroyHWInterface(BEGL_HWInterface *iface);
void NXPL_DestroyDisplayInterface(BEGL_DisplayInterface *iface);

NEXUS_HEAPHANDLE NXPL_MemHeap(BEGL_MemoryInterface *mem);

typedef struct
{
   BEGL_BufferSettings     settings;
   NEXUS_SURFACEHANDLE     surface;
} NXPL_BufferData;

#ifdef __cplusplus
}
#endif

#endif
