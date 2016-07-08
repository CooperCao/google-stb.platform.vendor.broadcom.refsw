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

#include "interface/khronos/include/GLES2/gl2.h"

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
   MEM_HANDLE_T mh_storage;      /* floating KHRN_IMAGE_T */
   MEM_HANDLE_T mh_ms_storage;   /* floating KHRN_IMAGE_T */
} GLXX_RENDERBUFFER_T;

extern void glxx_renderbuffer_init(GLXX_RENDERBUFFER_T *renderbuffer, int32_t name);
extern void glxx_renderbuffer_term(void *v, uint32_t size);

extern bool glxx_renderbuffer_storage(GLXX_RENDERBUFFER_T *renderbuffer,
   GLenum internalformat, GLuint width, GLuint height, bool secure);

extern bool glxx_renderbuffer_unmerge(GLXX_RENDERBUFFER_T *renderbuffer);
extern void glxx_renderbuffer_attempt_merge(GLXX_RENDERBUFFER_T *depth, GLXX_RENDERBUFFER_T *stencil);

extern bool glxx_renderbuffer_bind_image(GLXX_RENDERBUFFER_T *renderbuffer, MEM_HANDLE_T himage);

extern bool glxx_renderbuffer_storage_multisample(GLXX_RENDERBUFFER_T *renderbuffer, GLsizei samples,
   GLenum internalformat, GLuint width, GLuint height, bool secure);
#endif
