/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#if defined(ANDROID)

#ifndef EGLEXT_ANDROID_H
#define EGLEXT_ANDROID_H

#ifdef __cplusplus
extern "C" {
#endif

#ifndef EGL_ANDROID_recordable
#define EGL_ANDROID_recordable   1
#define EGL_RECORDABLE_ANDROID   0x3142
#endif

/* EGL_ANDROID_framebuffer_target */
#ifndef EGL_ANDROID_framebuffer_target
#define EGL_ANDROID_framebuffer_target   1
#define EGL_FRAMEBUFFER_TARGET_ANDROID                0x3147
#endif

#ifdef __cplusplus
}
#endif

#endif /* EGLEXT_ANDROID_H */

#endif /* defined(ANDROID) */
