/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_IMAGE_RENDERBUFFER_H
#define EGL_IMAGE_RENDERBUFFER_H
#include <EGL/egl.h>
#include "egl_types.h"
#include "egl_context.h"

extern EGL_IMAGE_T *egl_image_renderbuffer_new(EGL_CONTEXT_T *context,
      EGLenum target, EGLClientBuffer buffer);

#endif /* EGL_IMAGE_RENDERBUFFER_H */
