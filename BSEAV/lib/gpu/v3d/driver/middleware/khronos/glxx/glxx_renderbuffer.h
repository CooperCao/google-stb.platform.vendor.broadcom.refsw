/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <GLES2/gl2.h>
#include "middleware/khronos/common/khrn_image.h"

typedef enum {
   RB_NEW_T,
   RB_COLOR_T,
   RB_DEPTH16_T,
   RB_DEPTH24_T,
   RB_STENCIL_T,
   RB_DEPTH24_STENCIL8_T
} GLXX_RENDERBUFFER_TYPE_T;

typedef struct {
   int32_t name;

   GLXX_RENDERBUFFER_TYPE_T type;
   bool merged;      //storage points to combined depth+stencil. Can be no external references except renderer
   GLsizei samples;  // Number of samples for multisample render buffer
   KHRN_IMAGE_T *storage;
   KHRN_IMAGE_T *ms_storage;
} GLXX_RENDERBUFFER_T;

extern void glxx_renderbuffer_init(GLXX_RENDERBUFFER_T *renderbuffer, int32_t name);
extern void glxx_renderbuffer_term(void *p);

extern bool glxx_renderbuffer_unmerge(GLXX_RENDERBUFFER_T *renderbuffer);
extern void glxx_renderbuffer_attempt_merge(GLXX_RENDERBUFFER_T *depth, GLXX_RENDERBUFFER_T *stencil);

extern bool glxx_renderbuffer_bind_image(GLXX_RENDERBUFFER_T *renderbuffer, KHRN_IMAGE_T *image);

extern bool glxx_renderbuffer_storage_multisample(GLXX_RENDERBUFFER_T *renderbuffer, GLsizei samples,
   GLenum internalformat, GLuint width, GLuint height, bool secure);
