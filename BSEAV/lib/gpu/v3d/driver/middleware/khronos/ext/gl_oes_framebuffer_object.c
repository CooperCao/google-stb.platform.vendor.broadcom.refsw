/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define GL_GLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_int_common.h"
#include <GLES/gl.h>
#include <GLES/glext.h>

#include "middleware/khronos/glxx/glxx_server.h"

GL_API GLboolean GL_APIENTRY glIsRenderbufferOES (GLuint renderbuffer)
{
   return is_renderbuffer_internal(renderbuffer);
}

GL_API void GL_APIENTRY glBindRenderbufferOES (GLenum target, GLuint renderbuffer)
{
   bind_renderbuffer_internal(target, renderbuffer);
}

GL_API void GL_APIENTRY glDeleteRenderbuffersOES (GLsizei n, const GLuint* renderbuffers)
{
   delete_renderbuffers_internal(n, renderbuffers);
}

GL_API void GL_APIENTRY glGenRenderbuffersOES (GLsizei n, GLuint* renderbuffers)
{
   gen_renderbuffers_internal(n, renderbuffers);
}

GL_API void GL_APIENTRY glRenderbufferStorageOES (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
   renderbuffer_storage_multisample_internal(target, 0, internalformat, width, height);
}

GL_API void GL_APIENTRY glGetRenderbufferParameterivOES (GLenum target, GLenum pname, GLint* params)
{
   get_renderbuffer_parameteriv_internal(target, pname, params);
}

GL_API GLboolean GL_APIENTRY glIsFramebufferOES (GLuint framebuffer)
{
   return is_framebuffer_internal(framebuffer);
}

GL_API void GL_APIENTRY glBindFramebufferOES (GLenum target, GLuint framebuffer)
{
   bind_framebuffer_internal(target, framebuffer);
}

GL_API void GL_APIENTRY glDeleteFramebuffersOES (GLsizei n, const GLuint* framebuffers)
{
   delete_framebuffers_internal(n, framebuffers);
}

GL_API void GL_APIENTRY glGenFramebuffersOES (GLsizei n, GLuint* framebuffers)
{
   gen_framebuffers_internal(n, framebuffers);
}

GL_API GLenum GL_APIENTRY glCheckFramebufferStatusOES (GLenum target)
{
   return check_framebuffer_status_internal(target);
}

GL_API void GL_APIENTRY glFramebufferRenderbufferOES (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
   framebuffer_renderbuffer_internal(target, attachment, renderbuffertarget, renderbuffer);
}

GL_API void GL_APIENTRY glFramebufferTexture2DOES (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
   framebuffer_texture2D_multisample_internal(target, attachment, textarget, texture, level, 0, false);
}

GL_API void GL_APIENTRY glGetFramebufferAttachmentParameterivOES (GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
   get_framebuffer_attachment_parameteriv_internal(target, attachment, pname, params);
}

GL_API void GL_APIENTRY glGenerateMipmapOES (GLenum target)
{
   generate_mipmap_internal(target);
}
