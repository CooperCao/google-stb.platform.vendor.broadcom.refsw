/******************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
******************************************************************************/
#pragma once

#include <EGL/begl_platform.h>
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

#define UNUSED(X) X

#ifdef __cplusplus
}
#endif
