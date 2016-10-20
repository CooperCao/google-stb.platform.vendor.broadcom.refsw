/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 2.0 / Open GL ES 1.1 OES_framebuffer_object renderbuffer structure declaration.
=============================================================================*/

#ifndef GLXX_RENDERBUFFER_H
#define GLXX_RENDERBUFFER_H

#include "gl_public_api.h"
#include "../common/khrn_image.h"
#include "../egl/egl_types.h"
#include "glxx_utils.h"

typedef struct GLXX_RENDERBUFFER_ {
   uint32_t name;
   KHRN_IMAGE_T   *image;        // floating KHRN_IMAGE_T
   glxx_ms_mode   ms_mode;

   unsigned width_pixels;
   unsigned height_pixels;

   char           *debug_label;

   /*
    * If we are an EGLImage target sibling (because of a call to
    * glEGLImageTargetRenderbufferStorageOES) the source is stored here.
    */
   EGL_IMAGE_T    *source;
} GLXX_RENDERBUFFER_T;

extern GLXX_RENDERBUFFER_T *glxx_renderbuffer_create(uint32_t name);

extern bool glxx_renderbuffer_storage(GLXX_RENDERBUFFER_T *renderbuffer,
      glxx_ms_mode ms_mode, GLenum internalformat,
      unsigned width_pixels, unsigned height_pixels,
      bool secure);

extern bool glxx_renderbuffer_sub_image(GLXX_RENDERBUFFER_T *rb, int xoffset, int yoffset,
      unsigned int width, unsigned int height, GLenum format, GLenum type, const void *pixels);
#endif
