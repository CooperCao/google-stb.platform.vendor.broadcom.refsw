/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  Default RSO Android platform API for EGL driver
Module   :  RSO Android platform on NEXUS

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef _RSOANPL_DEFAULT_RSOANDROID_H__
#define _RSOANPL_DEFAULT_RSOANDROID_H__

#include <EGL/egl.h>
#include <cutils/log.h>
#include <system/window.h>
#include <gralloc_priv.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *RSOANPL_PlatformHandle;

/* Register Android with the egl platform */
extern void RSOANPL_RegisterAndroidDisplayPlatform(RSOANPL_PlatformHandle *handle, ANativeWindow *awin);

/* Unregister Android */
extern void RSOANPL_UnregisterAndroidDisplayPlatform(RSOANPL_PlatformHandle handle);

extern bool RSOANPL_BufferGetRequirements(RSOANPL_PlatformHandle handle, BEGL_PixmapInfoEXT *bufferRequirements, BEGL_BufferSettings * bufferConstrainedRequirements);

#define UNUSED(X) X

#ifdef __cplusplus
}
#endif

#endif /* _RSOANPL_DEFAULT_RSOANDROID_H__ */
