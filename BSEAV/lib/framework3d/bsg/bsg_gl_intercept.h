/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_GL_INTERCEPT_H__
#define __BSG_GL_INTERCEPT_H__

#include "bsg_gl.h"

#include <stdint.h>

// @cond
namespace bsg
{

class GLTextureTargetState
{
public:
   GLTextureTargetState();

public:
   GLuint    m_boundTexture;
};

class GLTextureUnit
{
public:
   GLTextureTargetState m_2d;
   GLTextureTargetState m_cube;
};

class GLStateMirror
{
public:
   GLStateMirror();

   void Reset();

   GLTextureTargetState &TargetState(GLenum target);
   void                 SetAttributeEnables();

public:
   uint32_t       m_activeProgram;
   GLenum         m_activeTextureUnit;
   GLTextureUnit  m_textureUnits[32];

   enum
   {
      NUM_ATTRIBUTES = 8
   };

   bool           m_vertexAttribEnableRequired[NUM_ATTRIBUTES];
   bool           m_vertexAttribEnableCurrent[NUM_ATTRIBUTES];
};

void GLStateMirrorReset();

}

extern "C"
{
   void intercept_glActiveTexture(GLenum texture);
   void intercept_glBindTexture(GLenum target, GLuint texture);
   void intercept_glUseProgram(GLuint program);
   void intercept_glEnableVertexAttribArray(GLuint index);
   void intercept_glDisableVertexAttribArray(GLuint index);
   void intercept_glDrawArrays(GLenum mode, GLint first, GLsizei count);
   void intercept_glDrawElements(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
   void intercept_glGetVertexAttribPointerv(GLuint index, GLenum pname, GLvoid** pointer);
   void intercept_glDeleteTextures(GLsizei n, const GLuint* textures);

/*
   void intercept_glVertexAttribPointer(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
   GLboolean intercept_glIsBuffer(GLuint buffer);
   GLboolean intercept_glIsEnabled(GLenum cap);
   GLboolean intercept_glIsFramebuffer(GLuint framebuffer);
   GLboolean intercept_glIsProgram(GLuint program);
   GLboolean intercept_glIsRenderbuffer(GLuint renderbuffer);
   GLboolean intercept_glIsShader(GLuint shader);
   GLboolean intercept_glIsTexture(GLuint texture);
   GLenum intercept_glCheckFramebufferStatus(GLenum target);
   GLenum intercept_glGetError(void);
   GLuint intercept_glCreateProgram(void);
   GLuint intercept_glCreateShader(GLenum type);
   const GLubyte* intercept_glGetString(GLenum name);
   int intercept_glGetAttribLocation(GLuint program, const GLchar* name);
   int intercept_glGetUniformLocation(GLuint program, const GLchar* name);
   void intercept_glAlphaFunc(GLenum func, GLclampf ref);
   void intercept_glAlphaFuncx(GLenum func, GLclampx ref);
   void intercept_glAttachShader(GLuint program, GLuint shader);
   void intercept_glBindAttribLocation(GLuint program, GLuint index, const GLchar* name);
   void intercept_glBindBuffer(GLenum target, GLuint buffer);
   void intercept_glBindFramebuffer(GLenum target, GLuint framebuffer);
   void intercept_glBindRenderbuffer(GLenum target, GLuint renderbuffer);
   void intercept_glBlendColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   void intercept_glBlendEquation( GLenum mode );
   void intercept_glBlendEquationSeparate(GLenum modeRGB, GLenum modeAlpha);
   void intercept_glBlendFunc(GLenum sfactor, GLenum dfactor);
   void intercept_glBlendFuncSeparate(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
   void intercept_glBufferData(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
   void intercept_glBufferSubData(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
   void intercept_glClear(GLbitfield mask);
   void intercept_glClearColor(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   void intercept_glClearColorx(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
   void intercept_glClearDepthf(GLclampf depth);
   void intercept_glClearDepthx(GLclampx depth);
   void intercept_glClearStencil(GLint s);
   void intercept_glClientActiveTexture(GLenum texture);
   void intercept_glClipPlanef(GLenum plane, const GLfloat *equation);
   void intercept_glClipPlanex(GLenum plane, const GLfixed *equation);
   void intercept_glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
   void intercept_glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
   void intercept_glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
   void intercept_glColorMask(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
   void intercept_glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void intercept_glCompileShader(GLuint shader);
   void intercept_glCompressedTexImage2D(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
   void intercept_glCompressedTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
   void intercept_glCopyTexImage2D(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
   void intercept_glCopyTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
   void intercept_glCullFace(GLenum mode);
   void intercept_glDeleteBuffers(GLsizei n, const GLuint *buffers);
   void intercept_glDeleteFramebuffers(GLsizei n, const GLuint* framebuffers);
   void intercept_glDeleteProgram(GLuint program);
   void intercept_glDeleteRenderbuffers(GLsizei n, const GLuint* renderbuffers);
   void intercept_glDeleteShader(GLuint shader);
   void intercept_glDepthFunc(GLenum func);
   void intercept_glDepthMask(GLboolean flag);
   void intercept_glDepthRangef(GLclampf zNear, GLclampf zFar);
   void intercept_glDepthRangex(GLclampx zNear, GLclampx zFar);
   void intercept_glDetachShader(GLuint program, GLuint shader);
   void intercept_glDisable(GLenum cap);
   void intercept_glDisableClientState(GLenum array);
   void intercept_glEnable(GLenum cap);
   void intercept_glEnableClientState(GLenum array);
   void intercept_glFinish(void);
   void intercept_glFlush(void);
   void intercept_glFogf(GLenum pname, GLfloat param);
   void intercept_glFogfv(GLenum pname, const GLfloat *params);
   void intercept_glFogx(GLenum pname, GLfixed param);
   void intercept_glFogxv(GLenum pname, const GLfixed *params);
   void intercept_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
   void intercept_glFramebufferTexture2D(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
   void intercept_glFrontFace(GLenum mode);
   void intercept_glFrustumf(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   void intercept_glFrustumx(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
   void intercept_glGenBuffers(GLsizei n, GLuint* buffers);
   void intercept_glGenFramebuffers(GLsizei n, GLuint* framebuffers);
   void intercept_glGenRenderbuffers(GLsizei n, GLuint* renderbuffers);
   void intercept_glGenTextures(GLsizei n, GLuint *textures);
   void intercept_glGenerateMipmap(GLenum target);
   void intercept_glGetActiveAttrib(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
   void intercept_glGetActiveUniform(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
   void intercept_glGetAttachedShaders(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
   void intercept_glGetBooleanv(GLenum pname, GLboolean* params);
   void intercept_glGetBufferParameteriv(GLenum target, GLenum pname, GLint* params);
   void intercept_glGetClipPlanef(GLenum pname, GLfloat eqn[4]);
   void intercept_glGetClipPlanex(GLenum pname, GLfixed eqn[4]);
   void intercept_glGetFixedv(GLenum pname, GLfixed *params);
   void intercept_glGetFloatv(GLenum pname, GLfloat *params);
   void intercept_glGetFramebufferAttachmentParameteriv(GLenum target, GLenum attachment, GLenum pname, GLint* params);
   void intercept_glGetIntegerv(GLenum pname, GLint *params);
   void intercept_glGetLightfv(GLenum light, GLenum pname, GLfloat *params);
   void intercept_glGetLightxv(GLenum light, GLenum pname, GLfixed *params);
   void intercept_glGetMaterialfv(GLenum face, GLenum pname, GLfloat *params);
   void intercept_glGetMaterialxv(GLenum face, GLenum pname, GLfixed *params);
   void intercept_glGetPointerv(GLenum pname, GLvoid **params);
   void intercept_glGetProgramInfoLog(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
   void intercept_glGetProgramiv(GLuint program, GLenum pname, GLint* params);
   void intercept_glGetRenderbufferParameteriv(GLenum target, GLenum pname, GLint* params);
   void intercept_glGetShaderInfoLog(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
   void intercept_glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
   void intercept_glGetShaderSource(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
   void intercept_glGetShaderiv(GLuint shader, GLenum pname, GLint* params);
   void intercept_glGetTexEnvfv(GLenum env, GLenum pname, GLfloat *params);
   void intercept_glGetTexEnviv(GLenum env, GLenum pname, GLint *params);
   void intercept_glGetTexEnvxv(GLenum env, GLenum pname, GLfixed *params);
   void intercept_glGetTexParameterfv(GLenum target, GLenum pname, GLfloat *params);
   void intercept_glGetTexParameteriv(GLenum target, GLenum pname, GLint *params);
   void intercept_glGetTexParameterxv(GLenum target, GLenum pname, GLfixed *params);
   void intercept_glGetUniformfv(GLuint program, GLint location, GLfloat* params);
   void intercept_glGetUniformiv(GLuint program, GLint location, GLint* params);
   void intercept_glGetVertexAttribfv(GLuint index, GLenum pname, GLfloat* params);
   void intercept_glGetVertexAttribiv(GLuint index, GLenum pname, GLint* params);
   void intercept_glHint(GLenum target, GLenum mode);
   void intercept_glLightModelf(GLenum pname, GLfloat param);
   void intercept_glLightModelfv(GLenum pname, const GLfloat *params);
   void intercept_glLightModelx(GLenum pname, GLfixed param);
   void intercept_glLightModelxv(GLenum pname, const GLfixed *params);
   void intercept_glLightf(GLenum light, GLenum pname, GLfloat param);
   void intercept_glLightfv(GLenum light, GLenum pname, const GLfloat *params);
   void intercept_glLightx(GLenum light, GLenum pname, GLfixed param);
   void intercept_glLightxv(GLenum light, GLenum pname, const GLfixed *params);
   void intercept_glLineWidth(GLfloat width);
   void intercept_glLineWidthx(GLfixed width);
   void intercept_glLinkProgram(GLuint program);
   void intercept_glLoadIdentity(void);
   void intercept_glLoadMatrixf(const GLfloat *m);
   void intercept_glLoadMatrixx(const GLfixed *m);
   void intercept_glLogicOp(GLenum opcode);
   void intercept_glMaterialf(GLenum face, GLenum pname, GLfloat param);
   void intercept_glMaterialfv(GLenum face, GLenum pname, const GLfloat *params);
   void intercept_glMaterialx(GLenum face, GLenum pname, GLfixed param);
   void intercept_glMaterialxv(GLenum face, GLenum pname, const GLfixed *params);
   void intercept_glMatrixMode(GLenum mode);
   void intercept_glMultMatrixf(const GLfloat *m);
   void intercept_glMultMatrixx(const GLfixed *m);
   void intercept_glMultiTexCoord4f(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
   void intercept_glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
   void intercept_glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz);
   void intercept_glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz);
   void intercept_glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer);
   void intercept_glOrthof(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   void intercept_glOrthox(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
   void intercept_glPixelStorei(GLenum pname, GLint param);
   void intercept_glPointParameterf(GLenum pname, GLfloat param);
   void intercept_glPointParameterfv(GLenum pname, const GLfloat *params);
   void intercept_glPointParameterx(GLenum pname, GLfixed param);
   void intercept_glPointParameterxv(GLenum pname, const GLfixed *params);
   void intercept_glPointSize(GLfloat size);
   void intercept_glPointSizex(GLfixed size);
   void intercept_glPolygonOffset(GLfloat factor, GLfloat units);
   void intercept_glPolygonOffsetx(GLfixed factor, GLfixed units);
   void intercept_glPopMatrix(void);
   void intercept_glPushMatrix(void);
   void intercept_glReadPixels(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
   void intercept_glReleaseShaderCompiler(void);
   void intercept_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
   void intercept_glRotatef(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
   void intercept_glRotatex(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
   void intercept_glSampleCoverage(GLclampf value, GLboolean invert);
   void intercept_glSampleCoveragex(GLclampx value, GLboolean invert);
   void intercept_glScalef(GLfloat x, GLfloat y, GLfloat z);
   void intercept_glScalex(GLfixed x, GLfixed y, GLfixed z);
   void intercept_glScissor(GLint x, GLint y, GLsizei width, GLsizei height);
   void intercept_glShadeModel(GLenum mode);
   void intercept_glShaderBinary(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
   void intercept_glShaderSource(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
   void intercept_glStencilFunc(GLenum func, GLint ref, GLuint mask);
   void intercept_glStencilFuncSeparate(GLenum face, GLenum func, GLint ref, GLuint mask);
   void intercept_glStencilMask(GLuint mask);
   void intercept_glStencilMaskSeparate(GLenum face, GLuint mask);
   void intercept_glStencilOp(GLenum fail, GLenum zfail, GLenum zpass);
   void intercept_glStencilOpSeparate(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
   void intercept_glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void intercept_glTexEnvf(GLenum target, GLenum pname, GLfloat param);
   void intercept_glTexEnvfv(GLenum target, GLenum pname, const GLfloat *params);
   void intercept_glTexEnvi(GLenum target, GLenum pname, GLint param);
   void intercept_glTexEnviv(GLenum target, GLenum pname, const GLint *params);
   void intercept_glTexEnvx(GLenum target, GLenum pname, GLfixed param);
   void intercept_glTexEnvxv(GLenum target, GLenum pname, const GLfixed *params);
   void intercept_glTexImage2D(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
   void intercept_glTexParameterf(GLenum target, GLenum pname, GLfloat param);
   void intercept_glTexParameterfv(GLenum target, GLenum pname, const GLfloat *params);
   void intercept_glTexParameteri(GLenum target, GLenum pname, GLint param);
   void intercept_glTexParameteriv(GLenum target, GLenum pname, const GLint *params);
   void intercept_glTexParameterx(GLenum target, GLenum pname, GLfixed param);
   void intercept_glTexParameterxv(GLenum target, GLenum pname, const GLfixed *params);
   void intercept_glTexSubImage2D(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
   void intercept_glTranslatef(GLfloat x, GLfloat y, GLfloat z);
   void intercept_glTranslatex(GLfixed x, GLfixed y, GLfixed z);
   void intercept_glUniform1f(GLint location, GLfloat x);
   void intercept_glUniform1fv(GLint location, GLsizei count, const GLfloat* v);
   void intercept_glUniform1i(GLint location, GLint x);
   void intercept_glUniform1iv(GLint location, GLsizei count, const GLint* v);
   void intercept_glUniform2f(GLint location, GLfloat x, GLfloat y);
   void intercept_glUniform2fv(GLint location, GLsizei count, const GLfloat* v);
   void intercept_glUniform2i(GLint location, GLint x, GLint y);
   void intercept_glUniform2iv(GLint location, GLsizei count, const GLint* v);
   void intercept_glUniform3f(GLint location, GLfloat x, GLfloat y, GLfloat z);
   void intercept_glUniform3fv(GLint location, GLsizei count, const GLfloat* v);
   void intercept_glUniform3i(GLint location, GLint x, GLint y, GLint z);
   void intercept_glUniform3iv(GLint location, GLsizei count, const GLint* v);
   void intercept_glUniform4f(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void intercept_glUniform4fv(GLint location, GLsizei count, const GLfloat* v);
   void intercept_glUniform4i(GLint location, GLint x, GLint y, GLint z, GLint w);
   void intercept_glUniform4iv(GLint location, GLsizei count, const GLint* v);
   void intercept_glUniformMatrix2fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
   void intercept_glUniformMatrix3fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
   void intercept_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
   void intercept_glValidateProgram(GLuint program);
   void intercept_glVertexAttrib1f(GLuint indx, GLfloat x);
   void intercept_glVertexAttrib1fv(GLuint indx, const GLfloat* values);
   void intercept_glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y);
   void intercept_glVertexAttrib2fv(GLuint indx, const GLfloat* values);
   void intercept_glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
   void intercept_glVertexAttrib3fv(GLuint indx, const GLfloat* values);
   void intercept_glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   void intercept_glVertexAttrib4fv(GLuint indx, const GLfloat* values);
   void intercept_glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   void intercept_glViewport(GLint x, GLint y, GLsizei width, GLsizei height);
   void intercept_glEGLImageTargetTexture2DOES(GLenum target, GLeglImageOES image);
   void intercept_glEGLImageTargetRenderbufferStorageOES(GLenum target, GLeglImageOES image);
   void intercept_glDrawTexsOES(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
   void intercept_glDrawTexiOES(GLint x, GLint y, GLint z, GLint width, GLint height);
   void intercept_glDrawTexxOES(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
   void intercept_glDrawTexsvOES(const GLshort *coords);
   void intercept_glDrawTexivOES(const GLint *coords);
   void intercept_glDrawTexxvOES(const GLfixed *coords);
   void intercept_glDrawTexfOES(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
   void intercept_glDrawTexfvOES(const GLfloat *coords);
   GLboolean intercept_glIsRenderbufferOES(GLuint renderbuffer);
   void intercept_glBindRenderbufferOES(GLenum target, GLuint renderbuffer);
   void intercept_glDeleteRenderbuffersOES(GLsizei n, const GLuint* renderbuffers);
   void intercept_glGenRenderbuffersOES(GLsizei n, GLuint* renderbuffers);
   void intercept_glRenderbufferStorageOES(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
   void intercept_glGetRenderbufferParameterivOES(GLenum target, GLenum pname, GLint* params);
   GLboolean intercept_glIsFramebufferOES(GLuint framebuffer);
   void intercept_glBindFramebufferOES(GLenum target, GLuint framebuffer);
   void intercept_glDeleteFramebuffersOES(GLsizei n, const GLuint* framebuffers);
   void intercept_glGenFramebuffersOES(GLsizei n, GLuint* framebuffers);
   GLenum intercept_glCheckFramebufferStatusOES(GLenum target);
   void intercept_glFramebufferRenderbufferOES(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
   void intercept_glFramebufferTexture2DOES(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
   void intercept_glGetFramebufferAttachmentParameterivOES(GLenum target, GLenum attachment, GLenum pname, GLint* params);
   void intercept_glGenerateMipmapOES(GLenum target);
   GLbitfield intercept_glQueryMatrixxOES(GLfixed mantissa[16], GLint exponent[16]);
   void intercept_glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid *pointer);
   void intercept_glDiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum *attachments);

   void intercept_glInsertEventMarkerEXT(GLsizei length, const GLchar *marker);
   void intercept_glPushGroupMarkerEXT(GLsizei length, const GLchar *marker);
   void intercept_glPopGroupMarkerEXT(void);
*/
}

#ifndef BSG_NO_NAME_MANGLING

#define glActiveTexture intercept_glActiveTexture
#define glBindTexture intercept_glBindTexture
#define glUseProgram intercept_glUseProgram

#ifndef BSG_USE_ES3
#define glEnableVertexAttribArray intercept_glEnableVertexAttribArray
#define glDisableVertexAttribArray intercept_glDisableVertexAttribArray
#define glDrawArrays intercept_glDrawArrays
#define glDrawElements intercept_glDrawElements
#endif

#define glDeleteTextures intercept_glDeleteTextures

/*
#define glVertexAttribPointer intercept_glVertexAttribPointer
#define glGetVertexAttribPointerv intercept_glGetVertexAttribPointerv
#define glTexParameteri intercept_glTexParameteri
#define glTexParameteriv intercept_glTexParameteriv
#define glTexParameterf intercept_glTexParameterf
#define glTexParameterfv intercept_glTexParameterfv
#define glAlphaFunc intercept_glAlphaFunc
#define glClearColor intercept_glClearColor
#define glClearDepthf intercept_glClearDepthf
#define glClipPlanef intercept_glClipPlanef
#define glColor4f intercept_glColor4f
#define glDepthRangef intercept_glDepthRangef
#define glFogf intercept_glFogf
#define glFogfv intercept_glFogfv
#define glFrustumf intercept_glFrustumf
#define glGetClipPlanef intercept_glGetClipPlanef
#define glGetFloatv intercept_glGetFloatv
#define glGetLightfv intercept_glGetLightfv
#define glGetMaterialfv intercept_glGetMaterialfv
#define glGetTexEnvfv intercept_glGetTexEnvfv
#define glGetTexParameterfv intercept_glGetTexParameterfv
#define glLightModelf intercept_glLightModelf
#define glLightModelfv intercept_glLightModelfv
#define glLightf intercept_glLightf
#define glLightfv intercept_glLightfv
#define glLineWidth intercept_glLineWidth
#define glLoadMatrixf intercept_glLoadMatrixf
#define glMaterialf intercept_glMaterialf
#define glMaterialfv intercept_glMaterialfv
#define glMultMatrixf intercept_glMultMatrixf
#define glMultiTexCoord4f intercept_glMultiTexCoord4f
#define glNormal3f intercept_glNormal3f
#define glOrthof intercept_glOrthof
#define glPointParameterf intercept_glPointParameterf
#define glPointParameterfv intercept_glPointParameterfv
#define glPointSize intercept_glPointSize
#define glPolygonOffset intercept_glPolygonOffset
#define glRotatef intercept_glRotatef
#define glScalef intercept_glScalef
#define glTexEnvf intercept_glTexEnvf
#define glTexEnvfv intercept_glTexEnvfv
#define glTranslatef intercept_glTranslatef
#define glAlphaFuncx intercept_glAlphaFuncx
#define glBindBuffer intercept_glBindBuffer
#define glBlendFunc intercept_glBlendFunc
#define glBlendColor intercept_glBlendColor
#define glBufferData intercept_glBufferData
#define glBlendEquation intercept_glBlendEquation
#define glBufferSubData intercept_glBufferSubData
#define glClear intercept_glClear
#define glClearColorx intercept_glClearColorx
#define glClearDepthx intercept_glClearDepthx
#define glClearStencil intercept_glClearStencil
#define glClientActiveTexture intercept_glClientActiveTexture
#define glClipPlanex intercept_glClipPlanex
#define glColor4ub intercept_glColor4ub
#define glColor4x intercept_glColor4x
#define glColorMask intercept_glColorMask
#define glColorPointer intercept_glColorPointer
#define glCompressedTexImage2D intercept_glCompressedTexImage2D
#define glCompressedTexSubImage2D intercept_glCompressedTexSubImage2D
#define glCopyTexImage2D intercept_glCopyTexImage2D
#define glCopyTexSubImage2D intercept_glCopyTexSubImage2D
#define glCullFace intercept_glCullFace
#define glDeleteBuffers intercept_glDeleteBuffers
#define glDepthFunc intercept_glDepthFunc
#define glDepthMask intercept_glDepthMask
#define glDepthRangex intercept_glDepthRangex
#define glDisable intercept_glDisable
#define glDisableClientState intercept_glDisableClientState
#define glEnable intercept_glEnable
#define glEnableClientState intercept_glEnableClientState
#define glFinish intercept_glFinish
#define glFlush intercept_glFlush
#define glFogx intercept_glFogx
#define glFogxv intercept_glFogxv
#define glFrontFace intercept_glFrontFace
#define glFrustumx intercept_glFrustumx
#define glGetBooleanv intercept_glGetBooleanv
#define glGetBufferParameteriv intercept_glGetBufferParameteriv
#define glGetClipPlanex intercept_glGetClipPlanex
#define glGenBuffers intercept_glGenBuffers
#define glGenTextures intercept_glGenTextures
#define glGetError intercept_glGetError
#define glGetFixedv intercept_glGetFixedv
#define glGetIntegerv intercept_glGetIntegerv
#define glGetLightxv intercept_glGetLightxv
#define glGetMaterialxv intercept_glGetMaterialxv
#define glGetPointerv intercept_glGetPointerv
#define glGetString intercept_glGetString
#define glGetTexEnviv intercept_glGetTexEnviv
#define glGetTexEnvxv intercept_glGetTexEnvxv
#define glGetTexParameteriv intercept_glGetTexParameteriv
#define glGetTexParameterxv intercept_glGetTexParameterxv
#define glHint intercept_glHint
#define glIsBuffer intercept_glIsBuffer
#define glIsEnabled intercept_glIsEnabled
#define glIsTexture intercept_glIsTexture
#define glLightModelx intercept_glLightModelx
#define glLightModelxv intercept_glLightModelxv
#define glLightx intercept_glLightx
#define glLightxv intercept_glLightxv
#define glLineWidthx intercept_glLineWidthx
#define glLoadIdentity intercept_glLoadIdentity
#define glLoadMatrixx intercept_glLoadMatrixx
#define glLogicOp intercept_glLogicOp
#define glMaterialx intercept_glMaterialx
#define glMaterialxv intercept_glMaterialxv
#define glMatrixMode intercept_glMatrixMode
#define glMultMatrixx intercept_glMultMatrixx
#define glMultiTexCoord4x intercept_glMultiTexCoord4x
#define glNormal3x intercept_glNormal3x
#define glNormalPointer intercept_glNormalPointer
#define glOrthox intercept_glOrthox
#define glPixelStorei intercept_glPixelStorei
#define glPointParameterx intercept_glPointParameterx
#define glPointParameterxv intercept_glPointParameterxv
#define glPointSizex intercept_glPointSizex
#define glPolygonOffsetx intercept_glPolygonOffsetx
#define glPopMatrix intercept_glPopMatrix
#define glPushMatrix intercept_glPushMatrix
#define glReadPixels intercept_glReadPixels
#define glRotatex intercept_glRotatex
#define glSampleCoverage intercept_glSampleCoverage
#define glSampleCoveragex intercept_glSampleCoveragex
#define glScalex intercept_glScalex
#define glScissor intercept_glScissor
#define glShadeModel intercept_glShadeModel
#define glStencilFunc intercept_glStencilFunc
#define glStencilMask intercept_glStencilMask
#define glStencilOp intercept_glStencilOp
#define glTexCoordPointer intercept_glTexCoordPointer
#define glTexEnvi intercept_glTexEnvi
#define glTexEnvx intercept_glTexEnvx
#define glTexEnviv intercept_glTexEnviv
#define glTexEnvxv intercept_glTexEnvxv
#define glTexImage2D intercept_glTexImage2D

#define glTexParameterx intercept_glTexParameterx
#define glTexParameterxv intercept_glTexParameterxv
#define glTexSubImage2D intercept_glTexSubImage2D
#define glTranslatex intercept_glTranslatex
#define glVertexPointer intercept_glVertexPointer
#define glViewport intercept_glViewport

#define glAttachShader intercept_glAttachShader
#define glBindAttribLocation intercept_glBindAttribLocation
#define glBlendEquationSeparate intercept_glBlendEquationSeparate
#define glBlendFuncSeparate intercept_glBlendFuncSeparate
#define glCreateProgram intercept_glCreateProgram
#define glCreateShader intercept_glCreateShader
#define glDeleteProgram intercept_glDeleteProgram
#define glDeleteShader intercept_glDeleteShader
#define glDetachShader intercept_glDetachShader
#define glGetActiveAttrib intercept_glGetActiveAttrib
#define glGetActiveUniform intercept_glGetActiveUniform
#define glGetAttachedShaders intercept_glGetAttachedShaders
#define glGetAttribLocation intercept_glGetAttribLocation
#define glGetProgramiv intercept_glGetProgramiv
#define glGetProgramInfoLog intercept_glGetProgramInfoLog
#define glGetUniformfv intercept_glGetUniformfv
#define glGetUniformiv intercept_glGetUniformiv
#define glGetUniformLocation intercept_glGetUniformLocation
#define glGetVertexAttribfv intercept_glGetVertexAttribfv
#define glGetVertexAttribiv intercept_glGetVertexAttribiv
#define glIsProgram intercept_glIsProgram
#define glIsShader intercept_glIsShader
#define glLinkProgram intercept_glLinkProgram
#define glStencilFuncSeparate intercept_glStencilFuncSeparate
#define glStencilMaskSeparate intercept_glStencilMaskSeparate
#define glStencilOpSeparate intercept_glStencilOpSeparate
#define glUniform1i intercept_glUniform1i
#define glUniform2i intercept_glUniform2i
#define glUniform3i intercept_glUniform3i
#define glUniform4i intercept_glUniform4i
#define glUniform1f intercept_glUniform1f
#define glUniform2f intercept_glUniform2f
#define glUniform3f intercept_glUniform3f
#define glUniform4f intercept_glUniform4f
#define glUniform1iv intercept_glUniform1iv
#define glUniform2iv intercept_glUniform2iv
#define glUniform3iv intercept_glUniform3iv
#define glUniform4iv intercept_glUniform4iv
#define glUniform1fv intercept_glUniform1fv
#define glUniform2fv intercept_glUniform2fv
#define glUniform3fv intercept_glUniform3fv
#define glUniform4fv intercept_glUniform4fv
#define glUniformMatrix2fv intercept_glUniformMatrix2fv
#define glUniformMatrix3fv intercept_glUniformMatrix3fv
#define glUniformMatrix4fv intercept_glUniformMatrix4fv
#define glValidateProgram intercept_glValidateProgram
#define glVertexAttrib1f intercept_glVertexAttrib1f
#define glVertexAttrib2f intercept_glVertexAttrib2f
#define glVertexAttrib3f intercept_glVertexAttrib3f
#define glVertexAttrib4f intercept_glVertexAttrib4f
#define glVertexAttrib1fv intercept_glVertexAttrib1fv
#define glVertexAttrib2fv intercept_glVertexAttrib2fv
#define glVertexAttrib3fv intercept_glVertexAttrib3fv
#define glVertexAttrib4fv intercept_glVertexAttrib4fv
#define glCompileShader intercept_glCompileShader
#define glGetShaderiv intercept_glGetShaderiv
#define glGetShaderInfoLog intercept_glGetShaderInfoLog
#define glGetShaderSource intercept_glGetShaderSource
#define glReleaseShaderCompiler intercept_glReleaseShaderCompiler
#define glShaderSource intercept_glShaderSource
#define glShaderBinary intercept_glShaderBinary
#define glGetShaderPrecisionFormat intercept_glGetShaderPrecisionFormat
#define glIsRenderbuffer intercept_glIsRenderbuffer
#define glBindRenderbuffer intercept_glBindRenderbuffer
#define glDeleteRenderbuffers intercept_glDeleteRenderbuffers
#define glGenRenderbuffers intercept_glGenRenderbuffers
#define glRenderbufferStorage intercept_glRenderbufferStorage
#define glGetRenderbufferParameteriv intercept_glGetRenderbufferParameteriv
#define glIsFramebuffer intercept_glIsFramebuffer
#define glBindFramebuffer intercept_glBindFramebuffer
#define glDeleteFramebuffers intercept_glDeleteFramebuffers
#define glGenFramebuffers intercept_glGenFramebuffers
#define glCheckFramebufferStatus intercept_glCheckFramebufferStatus
#define glFramebufferTexture2D intercept_glFramebufferTexture2D
#define glFramebufferRenderbuffer intercept_glFramebufferRenderbuffer
#define glGetFramebufferAttachmentParameteriv intercept_glGetFramebufferAttachmentParameteriv
#define glGenerateMipmap intercept_glGenerateMipmap

#define glPointSizePointerOES intercept_glPointSizePointerOES
#define glDiscardFramebufferEXT intercept_glDiscardFramebufferEXT
#define glInsertEventMarkerEXT intercept_glInsertEventMarkerEXT
#define glPushGroupMarkerEXT intercept_glPushGroupMarkerEXT
#define glPopGroupMarkerEXT intercept_glPopGroupMarkerEXT

#define glDrawTexfOES intercept_glDrawTexfOES
#define glDrawTexfvOES intercept_glDrawTexfvOES
#define glDrawTexiOES intercept_glDrawTexiOES
#define glDrawTexivOES intercept_glDrawTexivOES
#define glDrawTexsOES intercept_glDrawTexsOES
#define glDrawTexsvOES intercept_glDrawTexsvOES
#define glDrawTexxOES intercept_glDrawTexxOES
#define glDrawTexxvOES intercept_glDrawTexxvOES

#define glIsRenderbufferOES intercept_glIsRenderbufferOES
#define glBindRenderbufferOES intercept_glBindRenderbufferOES
#define glDeleteRenderbuffersOES intercept_glDeleteRenderbuffersOES
#define glGenRenderbuffersOES intercept_glGenRenderbuffersOES
#define glRenderbufferStorageOES intercept_glRenderbufferStorageOES
#define glGetRenderbufferParameterivOES intercept_glGetRenderbufferParameterivOES
#define glIsFramebufferOES intercept_glIsFramebufferOES
#define glBindFramebufferOES intercept_glBindFramebufferOES
#define glDeleteFramebuffersOES intercept_glDeleteFramebuffersOES
#define glGenFramebuffersOES intercept_glGenFramebuffersOES
#define glCheckFramebufferStatusOES intercept_glCheckFramebufferStatusOES
#define glFramebufferRenderbufferOES intercept_glFramebufferRenderbufferOES
#define glFramebufferTexture2DOES intercept_glFramebufferTexture2DOES
#define glGetFramebufferAttachmentParameterivOES intercept_glGetFramebufferAttachmentParameterivOES
#define glGenerateMipmapOES intercept_glGenerateMipmapOES
*/

#endif

// @endcond

#endif /* __BSG_GL_INTERCEPT_H__ */
