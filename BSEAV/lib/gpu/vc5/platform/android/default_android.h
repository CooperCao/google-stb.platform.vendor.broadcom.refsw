/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DEFAULT_ANDROID_H__
#define __DEFAULT_ANDROID_H__

#include <EGL/egl.h>
#undef LOG_TAG
#define LOG_TAG "VC5"
#include <cutils/log.h>
#include <system/window.h>

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
