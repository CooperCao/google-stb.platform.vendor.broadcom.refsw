/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 1.1 and 2.0 server-side implementation functions.
=============================================================================*/

#ifndef GLXX_INT_IMPL_H
#define GLXX_INT_IMPL_H

#include "interface/khronos/include/GLES/gl.h"
#include "interface/khronos/include/GLES/glext.h"
#include "interface/khronos/glxx/glxx_int_attrib.h"

extern void glActiveTexture_impl(GLenum texture);
extern void glBindBuffer_impl(GLenum target, GLuint buffer);
extern void glBindTexture_impl(GLenum target, GLuint texture);
extern void glBlendFuncSeparate_impl(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
extern void glBufferData_impl(GLenum target, GLsizeiptr size, GLenum usage, const GLvoid *data);
extern void glBufferSubData_impl(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
extern void glClear_impl(GLbitfield mask);
extern void glClearColor_impl(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void glClearDepthf_impl(GLclampf depth);
extern void glClearStencil_impl(GLint s);
extern void glColorMask_impl(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);

extern GLboolean glCompressedTexImage2D_impl(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const void *data);
extern void glCopyTexImage2D_impl(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
extern void glCopyTexSubImage2D_impl(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
extern void glCullFace_impl(GLenum mode);
extern void glDeleteBuffers_impl(GLsizei n, const GLuint *buffers);
extern void glDeleteTextures_impl(GLsizei n, const GLuint *textures);
extern void glDepthFunc_impl(GLenum func);
extern void glDepthMask_impl(GLboolean flag);
extern void glDepthRangef_impl(GLclampf zNear, GLclampf zFar);
extern void glDisable_impl(GLenum cap);
extern void glDrawElements_impl(GLenum mode, GLsizei count, GLenum type, const void *indices_pointer,
   GLuint indices_buffer, GLXX_ATTRIB_T *attribs, int *keys, int keys_count, bool secure);
extern void glEnable_impl(GLenum cap);
extern GLuint glFinish_impl(void);
extern void glFlush_impl(void);
extern void glFrontFace_impl(GLenum mode);
extern void glGenBuffers_impl(GLsizei n, GLuint *buffers);
extern void glGenTextures_impl(GLsizei n, GLuint *textures);
extern GLenum glGetError_impl(void);
extern int glGetBooleanv_impl(GLenum pname, GLboolean *params);
extern int glGetBufferParameteriv_impl(GLenum target, GLenum pname, GLint *params);
extern int glGetFloatv_impl(GLenum pname, GLfloat *params);
extern int glGetIntegerv_impl(GLenum pname, GLint *params);
extern int glGetTexParameteriv_impl(GLenum target, GLenum pname, GLint *params);
extern int glGetTexParameterfv_impl(GLenum target, GLenum pname, GLfloat *params);
extern void glHint_impl(GLenum target, GLenum mode);
extern GLboolean glIsBuffer_impl(GLuint buffer);
extern GLboolean glIsEnabled_impl(GLenum cap);
extern GLboolean glIsTexture_impl(GLuint texture);
extern void glLineWidth_impl(GLfloat width);
extern void glPolygonOffset_impl(GLfloat factor, GLfloat units);
extern void glReadPixels_impl(GLint x, GLint y,
   GLsizei width, GLsizei height, GLenum format, GLenum type, GLint alignment, void *pixels);
extern void glSampleCoverage_impl(GLclampf value, GLboolean invert);
extern void glScissor_impl(GLint x, GLint y, GLsizei width, GLsizei height);
extern void glStencilFuncSeparate_impl(GLenum face, GLenum func, GLint ref, GLuint mask);
extern void glStencilMaskSeparate_impl(GLenum face, GLuint mask);
extern void glStencilOpSeparate_impl(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
extern GLboolean glTexImage2D_impl(GLenum target, GLint level, GLint internalformat,
   GLsizei width, GLsizei height, GLint border, GLenum format,
   GLenum type, GLint alignment, const GLvoid *pixels, bool secure);
extern void glTexParameteri_impl(GLenum target, GLenum pname, GLint param);
extern void glTexParameterf_impl(GLenum target, GLenum pname, GLfloat param);
extern void glTexParameteriv_impl(GLenum target, GLenum pname, const GLint *params);
extern void glTexParameterfv_impl(GLenum target, GLenum pname, const GLfloat *params);
extern void glTexSubImage2D_impl(GLenum target, GLint level, GLint xoffset, GLint yoffset,
   GLsizei width, GLsizei height, GLenum format,
   GLenum type, GLint alignment, const void *pixels);
extern void glViewport_impl(GLint x, GLint y, GLsizei width, GLsizei height);
extern void glintGetCoreRevision_impl(GLsizei bufSize, GLubyte *revisionStr);

/*****************************************************************************************/
/*                                 EXT extension functions                               */
/*****************************************************************************************/
extern void glDiscardFramebufferEXT_impl(GLenum target, GLsizei numAttachments, const GLenum *attachments);
extern void glxx_RenderbufferStorageMultisampleEXT_impl(GLenum target, GLsizei samples,
   GLenum internalformat, GLsizei width, GLsizei height, bool secure);
extern void glxx_FramebufferTexture2DMultisampleEXT_impl(GLenum target, GLenum attachment,
   GLenum textarget, GLuint texture, GLint level, GLsizei samples, bool secure);

/*****************************************************************************************/
/*                                 OES extension functions                               */
/*****************************************************************************************/

extern int glintFindMax_impl(GLsizei count, GLenum type, const void *indices_pointer);
extern void glintCacheCreate_impl(GLsizei offset);
extern void glintCacheDelete_impl(GLsizei offset);
extern void glintCacheData_impl(GLsizei offset, GLsizei length, const GLvoid *data);
extern GLboolean glintCacheGrow_impl(void);

#if GL_OES_EGL_image
extern void glEGLImageTargetTexture2DOES_impl(GLenum target, GLeglImageOES image);
#endif

/* OES_framebuffer_object for ES 1.1 and core in ES 2.0 */
extern GLboolean glIsRenderbuffer_impl(GLuint renderbuffer);
extern void glBindRenderbuffer_impl(GLenum target, GLuint renderbuffer);
extern void glDeleteRenderbuffers_impl(GLsizei n, const GLuint *renderbuffers);
extern void glGenRenderbuffers_impl(GLsizei n, GLuint *renderbuffers);
extern void glRenderbufferStorage_impl(GLenum target, GLenum internalformat, GLsizei width, GLsizei height, bool secure);
extern int glGetRenderbufferParameteriv_impl(GLenum target, GLenum pname, GLint* params);
extern GLboolean glIsFramebuffer_impl(GLuint framebuffer);
extern void glBindFramebuffer_impl(GLenum target, GLuint framebuffer);
extern void glDeleteFramebuffers_impl(GLsizei n, const GLuint *framebuffers);
extern void glGenFramebuffers_impl(GLsizei n, GLuint *framebuffers);
extern GLenum glCheckFramebufferStatus_impl(GLenum target);
extern void glFramebufferTexture2D_impl(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
extern void glFramebufferRenderbuffer_impl(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
extern int glGetFramebufferAttachmentParameteriv_impl(GLenum target, GLenum attachment, GLenum pname, GLint *params);
extern void glGenerateMipmap_impl(GLenum target);

#endif /* GLXX_INT_IMPL_H */
