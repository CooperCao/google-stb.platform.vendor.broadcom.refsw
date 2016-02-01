/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default rootx11 platform API for EGL driver
Module   :  rootx11 platform

FILE DESCRIPTION

WARNING! This only renders to the root window of the X11 project for Chromium
OS.  Do not expect to run multiclient or any other X apps via this.

DESC
=============================================================================*/

#ifndef __DISPLAY_ROOTX11_H__
#define __DISPLAY_ROOTX11_H__

#include "default_rootx11.h"

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

BEGL_MemoryInterface  *NXPL_CreateMemInterface(BEGL_HWInterface *hwIface);
BEGL_HWInterface      *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks);
BEGL_DisplayInterface *RXPL_CreateDisplayInterface(BEGL_MemoryInterface *memIface,
                                                   Display *display,
                                                   BEGL_DisplayCallbacks *displayCallbacks);

void NXPL_DestroyMemInterface(BEGL_MemoryInterface *iface);
void NXPL_DestroyHWInterface(BEGL_HWInterface *iface);
void RXPL_DestroyDisplayInterface(BEGL_DisplayInterface *iface);

#ifdef __cplusplus
}
#endif

#endif
