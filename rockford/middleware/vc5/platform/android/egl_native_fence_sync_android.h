/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :  egl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef EGL_NATIVE_FENCE_SYNC_ANDROID_H
#define EGL_NATIVE_FENCE_SYNC_ANDROID_H

#include "middleware/khronos/egl/egl_types.h"
#include "middleware/khronos/egl/egl_attrib_list.h"

extern EGL_SYNC_T *egl_native_fence_sync_android_new(EGL_CONTEXT_T *context,
      EGLenum type, const void *attrib_list, EGL_AttribType attrib_type);
#endif
