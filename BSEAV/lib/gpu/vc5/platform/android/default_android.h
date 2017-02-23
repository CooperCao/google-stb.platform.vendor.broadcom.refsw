/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  Default RSO Android platform API for EGL driver
Module   :  RSO Android platform on NEXUS

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef __DEFAULT_ANDROID_H__
#define __DEFAULT_ANDROID_H__

#include <EGL/egl.h>
#undef LOG_TAG
#define LOG_TAG "VC5"
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

#ifdef __cplusplus
}
#endif

#endif /* __DEFAULT_ANDROID_H__ */
