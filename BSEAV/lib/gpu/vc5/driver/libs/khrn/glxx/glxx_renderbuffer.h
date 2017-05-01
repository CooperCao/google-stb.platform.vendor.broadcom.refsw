/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef GLXX_RENDERBUFFER_H
#define GLXX_RENDERBUFFER_H

#include "gl_public_api.h"
#include "../common/khrn_image.h"
#include "../egl/egl_types.h"
#include "glxx_utils.h"

typedef struct GLXX_RENDERBUFFER_ {
   uint32_t name;
   khrn_image   *image;        // floating khrn_image
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
