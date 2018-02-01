/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define GL_GLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES/glext.h"

#include "middleware/khronos/glxx/glxx_server.h"

#if GL_EXT_multisampled_render_to_texture

GL_API void GL_APIENTRY glRenderbufferStorageMultisampleEXT(GLenum target, GLsizei samples,
   GLenum internalformat, GLsizei width, GLsizei height)
{
   renderbuffer_storage_multisample_internal(target, samples, internalformat, width, height);
}

GL_API void GL_APIENTRY glFramebufferTexture2DMultisampleEXT(GLenum target, GLenum attachment,
   GLenum textarget, GLuint texture, GLint level, GLsizei samples)
{
   framebuffer_texture2D_multisample_internal(target, attachment, textarget, texture, level, samples, false);
}

#endif