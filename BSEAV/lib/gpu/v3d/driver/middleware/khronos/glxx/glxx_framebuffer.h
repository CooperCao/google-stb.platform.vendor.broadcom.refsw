/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/include/GLES2/gl2.h"
#include "interface/khronos/include/GLES2/gl2ext.h"

#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/glxx/glxx_renderbuffer.h"

typedef struct {
   GLenum type;
   GLenum target;
   GLint level;
   GLsizei samples;

   void *object;
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
extern void glxx_framebuffer_term(void *p);
extern KHRN_IMAGE_T *glxx_attachment_info_get_images(GLXX_ATTACHMENT_INFO_T *attachment, KHRN_IMAGE_T **ms_image);
extern GLenum glxx_framebuffer_check_status(GLXX_FRAMEBUFFER_T *framebuffer);
extern bool glxx_framebuffer_hw_support(KHRN_IMAGE_FORMAT_T format);
