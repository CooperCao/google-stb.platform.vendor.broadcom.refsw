/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_NATIVE_FENCE_SYNC_ANDROID_H
#define EGL_NATIVE_FENCE_SYNC_ANDROID_H

#include "../../egl_types.h"
#include "../../egl_attrib_list.h"

extern EGL_SYNC_T *egl_native_fence_sync_android_new(EGL_CONTEXT_T *context,
      EGLenum type, const void *attrib_list, EGL_AttribType attrib_type);
#endif
