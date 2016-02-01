/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#ifndef EGL_IMAGE_FRAMEBUFFER_H
#define EGL_IMAGE_FRAMEBUFFER_H
#include "interface/khronos/include/EGL/egl.h"
#include "middleware/khronos/egl/egl_types.h"
#include "middleware/khronos/egl/egl_context.h"
#include "middleware/khronos/egl/egl_attrib_list.h"

/*
 * Create an EGL Image from the currently bound framebuffer.
 *
 * The attrib_list is EGL_NONE terminated and says which framebuffer (read or
 * draw) and which attachment you want, so e.g.:
 *
 * EGL_GL_FRAMEBUFFER_TARGET_BRCM, GL_READ_FRAMEBUFFER,
 * EGL_GL_FRAMEBUFFER_ATTACHMENT_BRCM, GL_DEPTH_STENCIL_ATTACHMENT,
 * EGL_NONE
 *
 * You can create an image from any attachment point of either framebuffer.
 *
 * You can also set EGL_GL_FRAMEBUFFER_CONVERT_TO_COLOR_BRCM to true. This
 * changes the format of depth/stencil buffers to make them look like colour
 * buffers, so that they can be manipulated as if they were colour buffers
 * (for example blitted with shader programs).
 */
extern EGL_IMAGE_T *egl_image_framebuffer_new(EGL_CONTEXT_T *context,
      EGLenum target, EGLClientBuffer buffer, const void *attrib_list,
      EGL_AttribType attrib_type);

#endif /* EGL_IMAGE_FRAMEBUFFER_H */
