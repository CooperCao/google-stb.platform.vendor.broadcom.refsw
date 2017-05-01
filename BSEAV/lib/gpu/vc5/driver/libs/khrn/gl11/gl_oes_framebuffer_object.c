/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../glxx/gl_public_api.h"

#include "../common/khrn_int_common.h"

GL_API GLboolean GL_APIENTRY glIsRenderbufferOES (GLuint renderbuffer)
{
   return glIsRenderbuffer(renderbuffer);
}

GL_API void GL_APIENTRY glBindRenderbufferOES (GLenum target, GLuint renderbuffer)
{
   glBindRenderbuffer(target, renderbuffer);
}

GL_API void GL_APIENTRY glDeleteRenderbuffersOES (GLsizei n, const GLuint* renderbuffers)
{
   glDeleteRenderbuffers(n, renderbuffers);
}

GL_API void GL_APIENTRY glGenRenderbuffersOES (GLsizei n, GLuint* renderbuffers)
{
   glGenRenderbuffers(n, renderbuffers);
}

GL_API void GL_APIENTRY glRenderbufferStorageOES (GLenum target, GLenum internalformat, GLsizei width, GLsizei height)
{
   glRenderbufferStorage(target, internalformat, width, height);
}

GL_API void GL_APIENTRY glGetRenderbufferParameterivOES (GLenum target, GLenum pname, GLint* params)
{
   glGetRenderbufferParameteriv(target, pname, params);
}

GL_API GLboolean GL_APIENTRY glIsFramebufferOES (GLuint framebuffer)
{
   return glIsFramebuffer(framebuffer);
}

GL_API void GL_APIENTRY glBindFramebufferOES (GLenum target, GLuint framebuffer)
{
   glBindFramebuffer(target, framebuffer);
}

GL_API void GL_APIENTRY glDeleteFramebuffersOES (GLsizei n, const GLuint* framebuffers)
{
   glDeleteFramebuffers(n, framebuffers);
}

GL_API void GL_APIENTRY glGenFramebuffersOES (GLsizei n, GLuint* framebuffers)
{
   glGenFramebuffers(n, framebuffers);
}

GL_API GLenum GL_APIENTRY glCheckFramebufferStatusOES (GLenum target)
{
   return glCheckFramebufferStatus(target);
}

GL_API void GL_APIENTRY glFramebufferRenderbufferOES (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer)
{
   glFramebufferRenderbuffer(target, attachment, renderbuffertarget, renderbuffer);
}

GL_API void GL_APIENTRY glFramebufferTexture2DOES (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level)
{
   glFramebufferTexture2D(target, attachment, textarget, texture, level);
}

GL_API void GL_APIENTRY glGetFramebufferAttachmentParameterivOES (GLenum target, GLenum attachment, GLenum pname, GLint* params)
{
   glGetFramebufferAttachmentParameteriv(target, attachment, pname, params);
}

GL_API void GL_APIENTRY glGenerateMipmapOES (GLenum target)
{
   glGenerateMipmap(target);
}
