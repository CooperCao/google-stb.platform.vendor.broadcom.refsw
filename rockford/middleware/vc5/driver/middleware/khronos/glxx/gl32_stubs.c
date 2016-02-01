/*=============================================================================
 * Copyright (c) 2015 Broadcom Europe Limited.
 * All rights reserved.
 *
 * Project  :  khronos
 * Module   :  glxx
 *
 * FILE DESCRIPTION
 * Stub functions for ES3.2
 * =============================================================================*/

#include "interface/khronos/glxx/gl_public_api.h"
#include "vcos.h"

#define VCOS_LOG_CATEGORY (&gl32_stubs)
static VCOS_LOG_CAT_T gl32_stubs = VCOS_LOG_INIT("gl32_stubs", VCOS_LOG_NEVER);

#if GL_ES_VERSION_3_2

#include <stdio.h>

static inline void NotImplemented(const char *func)
{
   vcos_log_error("ES3.2 function '%s' not implemented yet\n", func);
}

#ifdef WIN32
#undef __func__
#define __func__ __FUNCTION__
#endif

#define NOT_IMPLEMENTED NotImplemented(__func__);

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
GL_APICALL void GL_APIENTRY glEnablei(GLenum target, GLuint index) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glDisablei(GLenum target, GLuint index) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glBlendEquationi(GLuint buf, GLenum mode) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glBlendEquationSeparatei(GLuint buf, GLenum modeRGB, GLenum modeAlpha) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glBlendFunci(GLuint buf, GLenum src, GLenum dst) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glBlendFuncSeparatei(GLuint buf, GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glColorMaski(GLuint index, GLboolean r, GLboolean g, GLboolean b, GLboolean a) { NOT_IMPLEMENTED; }
GL_APICALL GLboolean GL_APIENTRY glIsEnabledi(GLenum target, GLuint index) { NOT_IMPLEMENTED; return GL_FALSE; }
GL_APICALL void GL_APIENTRY glDrawElementsBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLint basevertex) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glDrawRangeElementsBaseVertex(GLenum mode, GLuint start, GLuint end, GLsizei count, GLenum type, const void *indices, GLint basevertex) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertex(GLenum mode, GLsizei count, GLenum type, const void *indices, GLsizei instancecount, GLint basevertex) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glPrimitiveBoundingBox(GLfloat minX, GLfloat minY, GLfloat minZ, GLfloat minW, GLfloat maxX, GLfloat maxY, GLfloat maxZ, GLfloat maxW) { NOT_IMPLEMENTED; }
GL_APICALL GLenum GL_APIENTRY glGetGraphicsResetStatus(void) { NOT_IMPLEMENTED; return GL_NO_ERROR; }
GL_APICALL void GL_APIENTRY glReadnPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLsizei bufSize, void *data) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetnUniformfv(GLuint program, GLint location, GLsizei bufSize, GLfloat *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetnUniformiv(GLuint program, GLint location, GLsizei bufSize, GLint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetnUniformuiv(GLuint program, GLint location, GLsizei bufSize, GLuint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glMinSampleShading(GLfloat value) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glPatchParameteri(GLenum pname, GLint value) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glTexParameterIiv(GLenum target, GLenum pname, const GLint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glTexParameterIuiv(GLenum target, GLenum pname, const GLuint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetTexParameterIiv(GLenum target, GLenum pname, GLint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetTexParameterIuiv(GLenum target, GLenum pname, GLuint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glSamplerParameterIiv(GLuint sampler, GLenum pname, const GLint *param) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glSamplerParameterIuiv(GLuint sampler, GLenum pname, const GLuint *param) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetSamplerParameterIiv(GLuint sampler, GLenum pname, GLint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glGetSamplerParameterIuiv(GLuint sampler, GLenum pname, GLuint *params) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glTexBuffer(GLenum target, GLenum internalformat, GLuint buffer) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glTexBufferRange(GLenum target, GLenum internalformat, GLuint buffer, GLintptr offset, GLsizeiptr size) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glTexStorage3DMultisample(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth, GLboolean fixedsamplelocations) { NOT_IMPLEMENTED; }

#endif // GL_ES_VERSION_3_2
