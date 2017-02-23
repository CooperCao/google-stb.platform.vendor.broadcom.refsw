/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Android EGL extensions header

FILE DESCRIPTION
This is included at the end of our version of EGL/eglext.h. If you're using a
different EGL/eglext.h, you can include this after it
=============================================================================*/
#if defined(ANDROID)

#ifndef EGLEXT_ANDROID_H
#define EGLEXT_ANDROID_H

#ifdef __cplusplus
extern "C" {
#endif

/* By default Android always define this internally, also due to a missing
** proper pending #define in the Android frameworks/base/opengl/libs/egl/egl.cpp
** module we cannot actually disable EGL_ANDROID_swap_rectangle support via build
** configuration (ie setting 'TARGET_GLOBAL_CPPFLAGS += -DEGL_ANDROID_swap_rectangle=0'
** in our BoardConfig.mk) which would be the preferred mechanism, instead we therefore
** have to match Android behavior and define by default what is expected to be supported,
** as well as provide an implementation for it (which implementation may be empty as
** long as it satisfies Android expectations).
*/
#ifndef EGL_ANDROID_swap_rectangle
#define EGL_ANDROID_swap_rectangle 1
#endif

#if EGL_ANDROID_swap_rectangle
#ifdef EGL_EGLEXT_PROTOTYPES
EGLAPI EGLBoolean EGLAPIENTRY eglSetSwapRectangleANDROID (EGLDisplay dpy, EGLSurface draw, EGLint left, EGLint top, EGLint width, EGLint height);
#endif /* EGL_EGLEXT_PROTOTYPES */
typedef EGLBoolean (EGLAPIENTRYP PFNEGLSETSWAPRECTANGLEANDROIDPROC) (EGLDisplay dpy, EGLSurface draw, EGLint left, EGLint top, EGLint width, EGLint height);
#endif /* EGL_ANDROID_swap_rectangle */

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
