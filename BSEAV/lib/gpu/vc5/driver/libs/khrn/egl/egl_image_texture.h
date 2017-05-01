/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_IMAGE_TEXTURE_H
#define EGL_IMAGE_TEXTURE_H
#include <EGL/egl.h>
#include "egl_types.h"
#include "egl_attrib_list.h"

extern EGL_IMAGE_T *egl_image_texture_new(EGL_CONTEXT_T *context,
      EGLenum target, EGLClientBuffer buffer, const void *attrib_list,
      EGL_AttribType attrib_type);

#endif /* EGL_IMAGE_TEXTURE_H */
