/*=============================================================================
Copyright (c) 2011 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  EGL Android extensions

FILE DESCRIPTION
Client-side implementation of:

       - EGL_ANDROID_swap_rectangle
=============================================================================*/

#if defined(ANDROID)

#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"

#if EGL_ANDROID_swap_rectangle

EGLAPI EGLBoolean EGLAPIENTRY eglSetSwapRectangleANDROID (EGLDisplay dpy, EGLSurface draw, EGLint left, EGLint top, EGLint width, EGLint height)
{
   // We do not support this extension, return EGL_FALSE to make Android
   // happy however (ie cleanly know that we do not support it).
   //
   return EGL_FALSE;
}

#endif /* EGL_ANDROID_swap_rectangle */

#if EGL_ANDROID_render_buffer

EGLAPI EGLClientBuffer EGLAPIENTRY eglGetRenderBufferANDROID (EGLDisplay dpy, EGLSurface sur)
{
   return 0;
}

#endif /* EGL_ANDROID_render_buffer */

#endif /* defined(ANDROID) */
