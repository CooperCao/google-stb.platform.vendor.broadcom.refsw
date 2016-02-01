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

#ifndef _RXPL_DEFAULT_ROOTX11_H__
#define _RXPL_DEFAULT_ROOTX11_H__

#include <X11/Xlib.h>

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *RXPL_PlatformHandle;

/* Register root X11 window against 3d renderer */
extern void RXPL_RegisterRootX11DisplayPlatform(RXPL_PlatformHandle *handle, Display *display);

/* Unregister a display for exclusive use. The client application can the use the root X11 display again. */
extern void RXPL_UnregisterRootX11DisplayPlatform(RXPL_PlatformHandle handle);

extern bool RXPL_BufferGetRequirements(RXPL_PlatformHandle handle, BEGL_PixmapInfo *bufferRequirements, BEGL_BufferSettings * bufferConstrainedRequirements);

#ifdef __cplusplus
}
#endif

#endif /* _RXPL_DEFAULT_ROOTX11_H__ */
