/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 2.0 framebuffer / Open GL ES 1.1 OES_framebuffer_object structure declaration.
=============================================================================*/

#ifndef GLXX_FRAMEBUFFER_H
#define GLXX_FRAMEBUFFER_H

#include "interface/khronos/include/GLES2/gl2.h"
#include "interface/khronos/include/GLES2/gl2ext.h"

#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"

typedef struct {
   GLenum type;
   GLenum target;
   GLint level;
   GLsizei samples;

   MEM_HANDLE_T mh_object;
} GLXX_ATTACHMENT_INFO_T;

typedef struct {
   int32_t name;

   struct {
      GLXX_ATTACHMENT_INFO_T color;
      GLXX_ATTACHMENT_INFO_T depth;
      GLXX_ATTACHMENT_INFO_T stencil;
   } attachments;
} GLXX_FRAMEBUFFER_T;

extern void glxx_framebuffer_init(GLXX_FRAMEBUFFER_T *framebuffer, int32_t name);
extern void glxx_framebuffer_term(MEM_HANDLE_T handle);
extern MEM_HANDLE_T glxx_attachment_info_get_images(GLXX_ATTACHMENT_INFO_T *attachment, MEM_HANDLE_T *ms_image);
extern GLenum glxx_framebuffer_check_status(GLXX_FRAMEBUFFER_T *framebuffer);
extern bool glxx_framebuffer_hw_support(KHRN_IMAGE_FORMAT_T format);

#endif
