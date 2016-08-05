/*=============================================================================
 * Broadcom Proprietary and Confidential. (c)2015 Broadcom.
 * All rights reserved.
 *
 * Project  :  khronos
 * Module   :  glxx
 *
 * FILE DESCRIPTION
 * Stub functions for ES3.2
 * =============================================================================*/

#include "gl_public_api.h"
#include "glxx_int_config.h"
#include "libs/util/log/log.h"
#include "vcos.h"
#include <stdio.h>

#if KHRN_GLES32_DRIVER || GLXX_HAS_TNG

LOG_DEFAULT_CAT("gl32_stubs")

static inline void not_implemented(const char *func)
{
   log_error("ES3.2 function '%s' not implemented yet", func);
}

#define NOT_IMPLEMENTED not_implemented(VCOS_FUNCTION)

#endif

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glBlendBarrier(void) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glCopyImageSubData(GLuint srcName, GLenum srcTarget, GLint srcLevel, GLint srcX, GLint srcY, GLint srcZ, GLuint dstName, GLenum dstTarget, GLint dstLevel, GLint dstX, GLint dstY, GLint dstZ, GLsizei srcWidth, GLsizei srcHeight, GLsizei srcDepth) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glDebugMessageControl(GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glDebugMessageInsert(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *buf) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glDebugMessageCallback(GLDEBUGPROC callback, const void *userParam) { NOT_IMPLEMENTED; }
GL_APICALL GLuint GL_APIENTRY glGetDebugMessageLog(GLuint count, GLsizei bufSize, GLenum *sources, GLenum *types, GLuint *ids, GLenum *severities, GLsizei *lengths, GLchar *messageLog) { NOT_IMPLEMENTED; return 0; }
GL_APICALL void GL_APIENTRY glPushDebugGroup(GLenum source, GLuint id, GLsizei length, const GLchar *message) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glPopDebugGroup(void) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glObjectLabel(GLenum identifier, GLuint name, GLsizei length, const GLchar *label) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetObjectLabel(GLenum identifier, GLuint name, GLsizei bufSize, GLsizei *length, GLchar *label) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glObjectPtrLabel(const void *ptr, GLsizei length, const GLchar *label) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetObjectPtrLabel(const void *ptr, GLsizei bufSize, GLsizei *length, GLchar *label) { NOT_IMPLEMENTED; }

// This function existed in ES1.1, but in 3.1 it also needs to include stuff from glGetPointervKHR
//GL_APICALL void GL_APIENTRY glGetPointerv(GLenum pname, void **params) { NOT_IMPLEMENTED; }
//
GL_APICALL void GL_APIENTRY glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
GL_APICALL GLenum GL_APIENTRY glGetGraphicsResetStatus(void) { NOT_IMPLEMENTED; return GL_NO_ERROR; }
GL_APICALL void GL_APIENTRY glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glMinSampleShading(GLfloat value) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) { NOT_IMPLEMENTED; }
#endif

#if GLXX_HAS_TNG
GL_APICALL void GL_APIENTRY glFramebufferTextureOES(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glFramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
#endif
