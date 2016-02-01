/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION

EGLImages from GL Textures
=============================================================================*/

#ifndef EGL_IMAGE_TEXTURE_H
#define EGL_IMAGE_TEXTURE_H
#include "interface/khronos/include/EGL/egl.h"
#include "middleware/khronos/egl/egl_types.h"
#include "middleware/khronos/egl/egl_attrib_list.h"

extern EGL_IMAGE_T *egl_image_texture_new(EGL_CONTEXT_T *context,
      EGLenum target, EGLClientBuffer buffer, const void *attrib_list,
      EGL_AttribType attrib_type);

#endif /* EGL_IMAGE_TEXTURE_H */
