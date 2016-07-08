/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#ifndef EGL_IMAGE_RENDERBUFFER_H
#define EGL_IMAGE_RENDERBUFFER_H
#include <EGL/egl.h>
#include "egl_types.h"
#include "egl_context.h"

extern EGL_IMAGE_T *egl_image_renderbuffer_new(EGL_CONTEXT_T *context,
      EGLenum target, EGLClientBuffer buffer);

#endif /* EGL_IMAGE_RENDERBUFFER_H */
