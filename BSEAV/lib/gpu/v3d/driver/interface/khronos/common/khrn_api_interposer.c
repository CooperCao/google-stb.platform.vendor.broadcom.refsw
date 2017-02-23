/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  External interface

FILE DESCRIPTION
API re-entrancy protection (for multi-threading) and logging over
a remote interface.
=============================================================================*/

#if defined(REMOTE_API_LOGGING) || defined(BCG_MULTI_THREADED)

#include "interface/khronos/common/khrn_client_mangle.h" /* Mangle the names when we include the headers */

#include "interface/khronos/common/khrn_int_common.h"
#include "khrn_api_interposer.h"

#include "interface/khronos/common/khrn_client_unmangle.h" /* Now use the unmangled names */
#include "interface/khronos/common/khrn_client.h"
#include "interface/khronos/common/khrn_api_command_ids.h"

#include "interface/vcos/vcos.h"
#include "vcos_platform.h"

#include <stdio.h>
#include <string.h>

/* Set to true to capture event log data for all APIs - this will be slow */
static bool sEventLogAllAPIs = false;

#define QUEUE_LEN 16
static VCOS_MUTEX_T s_eventMutex;
static EventData    s_lastEvent;
static uint32_t     s_queueLen = 0;
static EventData    s_queue[QUEUE_LEN];

#define REMOTE_API_MAJOR_VER 1
#define REMOTE_API_MINOR_VER 10

#define FOURCC(a,b,c,d)		(((a) << 24) | ((b) << 16) | ((c) << 8) | (d))

#if defined(ANDROID)
#include <cutils/properties.h>
#endif

void khrn_init_api_interposer(void);

extern BEGL_DriverInterfaces* BEGLint_GetDriverInterfaces(void);
static void GetTime(uint64_t *now)
{
   BEGL_HWInfo info;
   BEGLint_GetDriverInterfaces()->hwInterface->GetInfo(BEGLint_GetDriverInterfaces()->hwInterface->context, &info);

   *now = info.time;
}

static uint32_t TimeDiffNano(uint64_t *start, uint64_t *end)
{
   return (uint32_t)(end - start);
}

typedef struct {
   bool     (*remote_inVarSet)(uint32_t, uint32_t);
   uint32_t (*remote_inBottleneckMode)(void);
   uint32_t (*remote_getBottleneckMode)(void);

   void (*remote_error)(GLenum error);

   void (*remote_return_void)(uint32_t ns);
   void (*remote_return_GLboolean)(GLboolean b, uint32_t ns);
   void (*remote_return_GLbitfield)(GLbitfield b, uint32_t ns);
   void (*remote_return_GLenum)(GLenum b, uint32_t ns);
   void (*remote_return_GLint)(GLint b, uint32_t ns);
   void (*remote_return_GLuint)(GLuint b, uint32_t ns);

   void (*remote_return_EGLBoolean)(EGLBoolean b, uint32_t ns);
   void (*remote_return_EGLDisplay)(EGLDisplay b, uint32_t ns);
   void (*remote_return_EGLSurface)(EGLSurface b, uint32_t ns);
   void (*remote_return_EGLEnum)(EGLenum b, uint32_t ns);
   void (*remote_return_EGLContext)(EGLContext b, uint32_t ns);
   void (*remote_return_EGLint)(EGLint b, uint32_t ns);
   void (*remote_return_EGLenum)(EGLenum b, uint32_t ns);
   void (*remote_return_EGLImageKHR)(EGLImageKHR b, uint32_t ns);
   void (*remote_return_EGLSyncKHR)(EGLSyncKHR b, uint32_t ns);
   void (*remote_return_string)(const char *s, uint32_t ns);

   void (*remote_event_notify)(EventData *ev);
   void (*remote_thread_change)(uint32_t threadID, EGLContext context);

   bool (*remote_glIsBuffer)(GLuint buffer);
   bool (*remote_glIsEnabled)(GLenum cap);
   bool (*remote_glIsFramebuffer)(GLuint framebuffer);
   bool (*remote_glIsProgram)(GLuint program);
   bool (*remote_glIsRenderbuffer)(GLuint renderbuffer);
   bool (*remote_glIsShader)(GLuint shader);
   bool (*remote_glIsTexture)(GLuint texture);
   bool (*remote_glCheckFramebufferStatus)(GLenum target);
   bool (*remote_glGetError)(void);
   bool (*remote_glCreateProgram)(void);
   bool (*remote_glCreateShader)(GLenum type);
   bool (*remote_glGetString)(GLenum name);
   bool (*remote_glGetAttribLocation)(GLuint program, const GLchar* name);
   bool (*remote_glGetUniformLocation)(GLuint program, const GLchar* name);
   bool (*remote_glActiveTexture)(GLenum texture);
   bool (*remote_glAlphaFunc)(GLenum func, GLclampf ref);
   bool (*remote_glAlphaFuncx)(GLenum func, GLclampx ref);
   bool (*remote_glAttachShader)(GLuint program, GLuint shader);
   bool (*remote_glBindAttribLocation)(GLuint program, GLuint index, const GLchar* name);
   bool (*remote_glBindBuffer)(GLenum target, GLuint buffer);
   bool (*remote_glBindFramebuffer)(GLenum target, GLuint framebuffer);
   bool (*remote_glBindRenderbuffer)(GLenum target, GLuint renderbuffer);
   bool (*remote_glBindTexture)(GLenum target, GLuint texture);
   bool (*remote_glBlendColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   bool (*remote_glBlendEquation)( GLenum mode );
   bool (*remote_glBlendEquationSeparate)(GLenum modeRGB, GLenum modeAlpha);
   bool (*remote_glBlendFunc)(GLenum sfactor, GLenum dfactor);
   bool (*remote_glBlendFuncSeparate)(GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha);
   bool (*remote_glBufferData)(GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage);
   bool (*remote_glBufferSubData)(GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data);
   bool (*remote_glClear)(GLbitfield mask);
   bool (*remote_glClearColor)(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
   bool (*remote_glClearColorx)(GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha);
   bool (*remote_glClearDepthf)(GLclampf depth);
   bool (*remote_glClearDepthx)(GLclampx depth);
   bool (*remote_glClearStencil)(GLint s);
   bool (*remote_glClientActiveTexture)(GLenum texture);
   bool (*remote_glClipPlanef)(GLenum plane, const GLfloat *equation);
   bool (*remote_glClipPlanex)(GLenum plane, const GLfixed *equation);
   bool (*remote_glColor4f)(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha);
   bool (*remote_glColor4ub)(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha);
   bool (*remote_glColor4x)(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha);
   bool (*remote_glColorMask)(GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha);
   bool (*remote_glColorPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   bool (*remote_glCompileShader)(GLuint shader);
   bool (*remote_glCompressedTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data);
   bool (*remote_glCompressedTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data);
   bool (*remote_glCopyTexImage2D)(GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border);
   bool (*remote_glCopyTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height);
   bool (*remote_glCullFace)(GLenum mode);
   bool (*remote_glDeleteBuffers)(GLsizei n, const GLuint *buffers);
   bool (*remote_glDeleteFramebuffers)(GLsizei n, const GLuint* framebuffers);
   bool (*remote_glDeleteProgram)(GLuint program);
   bool (*remote_glDeleteRenderbuffers)(GLsizei n, const GLuint* renderbuffers);
   bool (*remote_glDeleteShader)(GLuint shader);
   bool (*remote_glDeleteTextures)(GLsizei n, const GLuint* textures);
   bool (*remote_glDepthFunc)(GLenum func);
   bool (*remote_glDepthMask)(GLboolean flag);
   bool (*remote_glDepthRangef)(GLclampf zNear, GLclampf zFar);
   bool (*remote_glDepthRangex)(GLclampx zNear, GLclampx zFar);
   bool (*remote_glDetachShader)(GLuint program, GLuint shader);
   bool (*remote_glDisable)(GLenum cap);
   bool (*remote_glDisableClientState)(GLenum array);
   bool (*remote_glDisableVertexAttribArray)(GLuint index);
   bool (*remote_glDrawArrays)(GLenum mode, GLint first, GLsizei count);
   bool (*remote_glDrawElements)(GLenum mode, GLsizei count, GLenum type, const GLvoid* indices);
   bool (*remote_glEnable)(GLenum cap);
   bool (*remote_glEnableClientState)(GLenum array);
   bool (*remote_glEnableVertexAttribArray)(GLuint index);
   bool (*remote_glFinish)(void);
   bool (*remote_glFlush)(void);
   bool (*remote_glFogf)(GLenum pname, GLfloat param);
   bool (*remote_glFogfv)(GLenum pname, const GLfloat *params);
   bool (*remote_glFogx)(GLenum pname, GLfixed param);
   bool (*remote_glFogxv)(GLenum pname, const GLfixed *params);
   bool (*remote_glFramebufferRenderbuffer)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
   bool (*remote_glFramebufferTexture2D)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
   bool (*remote_glFrontFace)(GLenum mode);
   bool (*remote_glFrustumf)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   bool (*remote_glFrustumx)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
   bool (*remote_glGenBuffers)(GLsizei n, GLuint* buffers);
   bool (*remote_glGenFramebuffers)(GLsizei n, GLuint* framebuffers);
   bool (*remote_glGenRenderbuffers)(GLsizei n, GLuint* renderbuffers);
   bool (*remote_glGenTextures)(GLsizei n, GLuint *textures);
   bool (*remote_glGenerateMipmap)(GLenum target);
   bool (*remote_glGetActiveAttrib)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
   bool (*remote_glGetActiveUniform)(GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name);
   bool (*remote_glGetAttachedShaders)(GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders);
   bool (*remote_glGetBooleanv)(GLenum pname, GLboolean* params);
   bool (*remote_glGetBufferParameteriv)(GLenum target, GLenum pname, GLint* params);
   bool (*remote_glGetClipPlanef)(GLenum pname, GLfloat eqn[4]);
   bool (*remote_glGetClipPlanex)(GLenum pname, GLfixed eqn[4]);
   bool (*remote_glGetFixedv)(GLenum pname, GLfixed *params);
   bool (*remote_glGetFloatv)(GLenum pname, GLfloat *params);
   bool (*remote_glGetFramebufferAttachmentParameteriv)(GLenum target, GLenum attachment, GLenum pname, GLint* params);
   bool (*remote_glGetIntegerv)(GLenum pname, GLint *params);
   bool (*remote_glGetLightfv)(GLenum light, GLenum pname, GLfloat *params);
   bool (*remote_glGetLightxv)(GLenum light, GLenum pname, GLfixed *params);
   bool (*remote_glGetMaterialfv)(GLenum face, GLenum pname, GLfloat *params);
   bool (*remote_glGetMaterialxv)(GLenum face, GLenum pname, GLfixed *params);
   bool (*remote_glGetPointerv)(GLenum pname, GLvoid **params);
   bool (*remote_glGetProgramInfoLog)(GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog);
   bool (*remote_glGetProgramiv)(GLuint program, GLenum pname, GLint* params);
   bool (*remote_glGetRenderbufferParameteriv)(GLenum target, GLenum pname, GLint* params);
   bool (*remote_glGetShaderInfoLog)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog);
   bool (*remote_glGetShaderPrecisionFormat)(GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision);
   bool (*remote_glGetShaderSource)(GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source);
   bool (*remote_glGetShaderiv)(GLuint shader, GLenum pname, GLint* params);
   bool (*remote_glGetTexEnvfv)(GLenum env, GLenum pname, GLfloat *params);
   bool (*remote_glGetTexEnviv)(GLenum env, GLenum pname, GLint *params);
   bool (*remote_glGetTexEnvxv)(GLenum env, GLenum pname, GLfixed *params);
   bool (*remote_glGetTexParameterfv)(GLenum target, GLenum pname, GLfloat *params);
   bool (*remote_glGetTexParameteriv)(GLenum target, GLenum pname, GLint *params);
   bool (*remote_glGetTexParameterxv)(GLenum target, GLenum pname, GLfixed *params);
   bool (*remote_glGetUniformfv)(GLuint program, GLint location, GLfloat* params);
   bool (*remote_glGetUniformiv)(GLuint program, GLint location, GLint* params);
   bool (*remote_glGetVertexAttribPointerv)(GLuint index, GLenum pname, GLvoid** pointer);
   bool (*remote_glGetVertexAttribfv)(GLuint index, GLenum pname, GLfloat* params);
   bool (*remote_glGetVertexAttribiv)(GLuint index, GLenum pname, GLint* params);
   bool (*remote_glHint)(GLenum target, GLenum mode);
   bool (*remote_glLightModelf)(GLenum pname, GLfloat param);
   bool (*remote_glLightModelfv)(GLenum pname, const GLfloat *params);
   bool (*remote_glLightModelx)(GLenum pname, GLfixed param);
   bool (*remote_glLightModelxv)(GLenum pname, const GLfixed *params);
   bool (*remote_glLightf)(GLenum light, GLenum pname, GLfloat param);
   bool (*remote_glLightfv)(GLenum light, GLenum pname, const GLfloat *params);
   bool (*remote_glLightx)(GLenum light, GLenum pname, GLfixed param);
   bool (*remote_glLightxv)(GLenum light, GLenum pname, const GLfixed *params);
   bool (*remote_glLineWidth)(GLfloat width);
   bool (*remote_glLineWidthx)(GLfixed width);
   bool (*remote_glLinkProgram)(GLuint program);
   bool (*remote_glLoadIdentity)(void);
   bool (*remote_glLoadMatrixf)(const GLfloat *m);
   bool (*remote_glLoadMatrixx)(const GLfixed *m);
   bool (*remote_glLogicOp)(GLenum opcode);
   bool (*remote_glMaterialf)(GLenum face, GLenum pname, GLfloat param);
   bool (*remote_glMaterialfv)(GLenum face, GLenum pname, const GLfloat *params);
   bool (*remote_glMaterialx)(GLenum face, GLenum pname, GLfixed param);
   bool (*remote_glMaterialxv)(GLenum face, GLenum pname, const GLfixed *params);
   bool (*remote_glMatrixMode)(GLenum mode);
   bool (*remote_glMultMatrixf)(const GLfloat *m);
   bool (*remote_glMultMatrixx)(const GLfixed *m);
   bool (*remote_glMultiTexCoord4f)(GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q);
   bool (*remote_glMultiTexCoord4x)(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q);
   bool (*remote_glNormal3f)(GLfloat nx, GLfloat ny, GLfloat nz);
   bool (*remote_glNormal3x)(GLfixed nx, GLfixed ny, GLfixed nz);
   bool (*remote_glNormalPointer)(GLenum type, GLsizei stride, const GLvoid *pointer);
   bool (*remote_glOrthof)(GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar);
   bool (*remote_glOrthox)(GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar);
   bool (*remote_glPixelStorei)(GLenum pname, GLint param);
   bool (*remote_glPointParameterf)(GLenum pname, GLfloat param);
   bool (*remote_glPointParameterfv)(GLenum pname, const GLfloat *params);
   bool (*remote_glPointParameterx)(GLenum pname, GLfixed param);
   bool (*remote_glPointParameterxv)(GLenum pname, const GLfixed *params);
   bool (*remote_glPointSize)(GLfloat size);
   bool (*remote_glPointSizex)(GLfixed size);
   bool (*remote_glPolygonOffset)(GLfloat factor, GLfloat units);
   bool (*remote_glPolygonOffsetx)(GLfixed factor, GLfixed units);
   bool (*remote_glPopMatrix)(void);
   bool (*remote_glPushMatrix)(void);
   bool (*remote_glReadPixels)(GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels);
   bool (*remote_glReleaseShaderCompiler)(void);
   bool (*remote_glRenderbufferStorage)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
   bool (*remote_glRotatef)(GLfloat angle, GLfloat x, GLfloat y, GLfloat z);
   bool (*remote_glRotatex)(GLfixed angle, GLfixed x, GLfixed y, GLfixed z);
   bool (*remote_glSampleCoverage)(GLclampf value, GLboolean invert);
   bool (*remote_glSampleCoveragex)(GLclampx value, GLboolean invert);
   bool (*remote_glScalef)(GLfloat x, GLfloat y, GLfloat z);
   bool (*remote_glScalex)(GLfixed x, GLfixed y, GLfixed z);
   bool (*remote_glScissor)(GLint x, GLint y, GLsizei width, GLsizei height);
   bool (*remote_glShadeModel)(GLenum mode);
   bool (*remote_glShaderBinary)(GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length);
   bool (*remote_glShaderSource)(GLuint shader, GLsizei count, const GLchar** string, const GLint* length);
   bool (*remote_glStencilFunc)(GLenum func, GLint ref, GLuint mask);
   bool (*remote_glStencilFuncSeparate)(GLenum face, GLenum func, GLint ref, GLuint mask);
   bool (*remote_glStencilMask)(GLuint mask);
   bool (*remote_glStencilMaskSeparate)(GLenum face, GLuint mask);
   bool (*remote_glStencilOp)(GLenum fail, GLenum zfail, GLenum zpass);
   bool (*remote_glStencilOpSeparate)(GLenum face, GLenum fail, GLenum zfail, GLenum zpass);
   bool (*remote_glTexCoordPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   bool (*remote_glTexEnvf)(GLenum target, GLenum pname, GLfloat param);
   bool (*remote_glTexEnvfv)(GLenum target, GLenum pname, const GLfloat *params);
   bool (*remote_glTexEnvi)(GLenum target, GLenum pname, GLint param);
   bool (*remote_glTexEnviv)(GLenum target, GLenum pname, const GLint *params);
   bool (*remote_glTexEnvx)(GLenum target, GLenum pname, GLfixed param);
   bool (*remote_glTexEnvxv)(GLenum target, GLenum pname, const GLfixed *params);
   bool (*remote_glTexImage2D)(GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels);
   bool (*remote_glTexParameterf)(GLenum target, GLenum pname, GLfloat param);
   bool (*remote_glTexParameterfv)(GLenum target, GLenum pname, const GLfloat *params);
   bool (*remote_glTexParameteri)(GLenum target, GLenum pname, GLint param);
   bool (*remote_glTexParameteriv)(GLenum target, GLenum pname, const GLint *params);
   bool (*remote_glTexParameterx)(GLenum target, GLenum pname, GLfixed param);
   bool (*remote_glTexParameterxv)(GLenum target, GLenum pname, const GLfixed *params);
   bool (*remote_glTexSubImage2D)(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels);
   bool (*remote_glTranslatef)(GLfloat x, GLfloat y, GLfloat z);
   bool (*remote_glTranslatex)(GLfixed x, GLfixed y, GLfixed z);
   bool (*remote_glUniform1f)(GLint location, GLfloat x);
   bool (*remote_glUniform1fv)(GLint location, GLsizei count, const GLfloat* v);
   bool (*remote_glUniform1i)(GLint location, GLint x);
   bool (*remote_glUniform1iv)(GLint location, GLsizei count, const GLint* v);
   bool (*remote_glUniform2f)(GLint location, GLfloat x, GLfloat y);
   bool (*remote_glUniform2fv)(GLint location, GLsizei count, const GLfloat* v);
   bool (*remote_glUniform2i)(GLint location, GLint x, GLint y);
   bool (*remote_glUniform2iv)(GLint location, GLsizei count, const GLint* v);
   bool (*remote_glUniform3f)(GLint location, GLfloat x, GLfloat y, GLfloat z);
   bool (*remote_glUniform3fv)(GLint location, GLsizei count, const GLfloat* v);
   bool (*remote_glUniform3i)(GLint location, GLint x, GLint y, GLint z);
   bool (*remote_glUniform3iv)(GLint location, GLsizei count, const GLint* v);
   bool (*remote_glUniform4f)(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   bool (*remote_glUniform4fv)(GLint location, GLsizei count, const GLfloat* v);
   bool (*remote_glUniform4i)(GLint location, GLint x, GLint y, GLint z, GLint w);
   bool (*remote_glUniform4iv)(GLint location, GLsizei count, const GLint* v);
   bool (*remote_glUniformMatrix2fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
   bool (*remote_glUniformMatrix3fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
   bool (*remote_glUniformMatrix4fv)(GLint location, GLsizei count, GLboolean transpose, const GLfloat* value);
   bool (*remote_glUseProgram)(GLuint program);
   bool (*remote_glValidateProgram)(GLuint program);
   bool (*remote_glVertexAttrib1f)(GLuint indx, GLfloat x);
   bool (*remote_glVertexAttrib1fv)(GLuint indx, const GLfloat* values);
   bool (*remote_glVertexAttrib2f)(GLuint indx, GLfloat x, GLfloat y);
   bool (*remote_glVertexAttrib2fv)(GLuint indx, const GLfloat* values);
   bool (*remote_glVertexAttrib3f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
   bool (*remote_glVertexAttrib3fv)(GLuint indx, const GLfloat* values);
   bool (*remote_glVertexAttrib4f)(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
   bool (*remote_glVertexAttrib4fv)(GLuint indx, const GLfloat* values);
   bool (*remote_glVertexAttribPointer)(GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr);
   bool (*remote_glVertexPointer)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   bool (*remote_glViewport)(GLint x, GLint y, GLsizei width, GLsizei height);
   bool (*remote_glEGLImageTargetTexture2DOES)(GLenum target, GLeglImageOES image);
   bool (*remote_glEGLImageTargetRenderbufferStorageOES)(GLenum target, GLeglImageOES image);
   bool (*remote_glDrawTexsOES)(GLshort x, GLshort y, GLshort z, GLshort width, GLshort height);
   bool (*remote_glDrawTexiOES)(GLint x, GLint y, GLint z, GLint width, GLint height);
   bool (*remote_glDrawTexxOES)(GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height);
   bool (*remote_glDrawTexsvOES)(const GLshort *coords);
   bool (*remote_glDrawTexivOES)(const GLint *coords);
   bool (*remote_glDrawTexxvOES)(const GLfixed *coords);
   bool (*remote_glDrawTexfOES)(GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height);
   bool (*remote_glDrawTexfvOES)(const GLfloat *coords);
   bool (*remote_glIsRenderbufferOES)(GLuint renderbuffer);
   bool (*remote_glBindRenderbufferOES)(GLenum target, GLuint renderbuffer);
   bool (*remote_glDeleteRenderbuffersOES)(GLsizei n, const GLuint* renderbuffers);
   bool (*remote_glGenRenderbuffersOES)(GLsizei n, GLuint* renderbuffers);
   bool (*remote_glRenderbufferStorageOES)(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
   bool (*remote_glGetRenderbufferParameterivOES)(GLenum target, GLenum pname, GLint* params);
   bool (*remote_glIsFramebufferOES)(GLuint framebuffer);
   bool (*remote_glBindFramebufferOES)(GLenum target, GLuint framebuffer);
   bool (*remote_glDeleteFramebuffersOES)(GLsizei n, const GLuint* framebuffers);
   bool (*remote_glGenFramebuffersOES)(GLsizei n, GLuint* framebuffers);
   bool (*remote_glCheckFramebufferStatusOES)(GLenum target);
   bool (*remote_glFramebufferRenderbufferOES)(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
   bool (*remote_glFramebufferTexture2DOES)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level);
   bool (*remote_glGetFramebufferAttachmentParameterivOES)(GLenum target, GLenum attachment, GLenum pname, GLint* params);
   bool (*remote_glGenerateMipmapOES)(GLenum target);
   bool (*remote_glQueryMatrixxOES)(GLfixed mantissa[16], GLint exponent[16]);
   bool (*remote_glPointSizePointerOES)(GLenum type, GLsizei stride, const GLvoid *pointer);
   bool (*remote_glDiscardFramebufferEXT)(GLenum target, GLsizei numAttachments, const GLenum *attachments);
   bool (*remote_glInsertEventMarkerEXT)(GLsizei length, const GLchar *marker);
   bool (*remote_glPushGroupMarkerEXT)(GLsizei length, const GLchar *marker);
   bool (*remote_glPopGroupMarkerEXT)(void);
   bool (*remote_glCurrentPaletteMatrixOES)(GLuint matrixpaletteindex);
   bool (*remote_glLoadPaletteFromModelViewMatrixOES)(void);
   bool (*remote_glMatrixIndexPointerOES)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);
   bool (*remote_glWeightPointerOES)(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer);

   bool (*remote_glRenderbufferStorageMultisampleEXT)(GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height);
   bool (*remote_glFramebufferTexture2DMultisampleEXT)(GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples);

   bool (*remote_eglGetError)(void);
   bool (*remote_eglGetDisplay)(EGLNativeDisplayType display_id);
   bool (*remote_eglInitialize)(EGLDisplay dpy, EGLint *major, EGLint *minor);
   bool (*remote_eglTerminate)(EGLDisplay dpy);
   bool (*remote_eglQueryString)(EGLDisplay dpy, EGLint name);
   bool (*remote_eglGetConfigs)(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config);
   bool (*remote_eglChooseConfig)(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config);
   bool (*remote_eglGetConfigAttrib)(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value);
   bool (*remote_eglCreateWindowSurface)(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list);
   bool (*remote_eglCreatePbufferSurface)(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list);
   bool (*remote_eglCreatePixmapSurface)(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list);
   bool (*remote_eglDestroySurface)(EGLDisplay dpy, EGLSurface surface);
   bool (*remote_eglQuerySurface)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value);
   bool (*remote_eglBindAPI)(EGLenum api);
   bool (*remote_eglQueryAPI)(void);
   bool (*remote_eglWaitClient)(void);
   bool (*remote_eglReleaseThread)(void);
   bool (*remote_eglCreatePbufferFromClientBuffer)(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list);
   bool (*remote_eglSurfaceAttrib)(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value);
   bool (*remote_eglBindTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
   bool (*remote_eglReleaseTexImage)(EGLDisplay dpy, EGLSurface surface, EGLint buffer);
   bool (*remote_eglSwapInterval)(EGLDisplay dpy, EGLint interval);
   bool (*remote_eglCreateContext)(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list);
   bool (*remote_eglDestroyContext)(EGLDisplay dpy, EGLContext ctx);
   bool (*remote_eglMakeCurrent)(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx);
   bool (*remote_eglGetCurrentContext)(void);
   bool (*remote_eglGetCurrentSurface)(EGLint readdraw);
   bool (*remote_eglGetCurrentDisplay)(void);
   bool (*remote_eglQueryContext)(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value);
   bool (*remote_eglWaitGL)(void);
   bool (*remote_eglWaitNative)(EGLint engine);
   bool (*remote_eglSwapBuffers)(EGLDisplay dpy, EGLSurface surface);
   bool (*remote_eglCopyBuffers)(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target);
#if EGL_KHR_lock_surface
   bool (*remote_eglLockSurfaceKHR)(EGLDisplay display, EGLSurface surface, const EGLint *attrib_list);
   bool (*remote_eglUnlockSurfaceKHR)(EGLDisplay display, EGLSurface surface);
#endif
#if EGL_KHR_image
   bool (*remote_eglCreateImageKHR)(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list);
   bool (*remote_eglDestroyImageKHR)(EGLDisplay dpy, EGLImageKHR image);
#endif
#if EGL_KHR_sync
   bool (*remote_eglCreateSyncKHR)(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list);
   bool (*remote_eglDestroySyncKHR)(EGLDisplay dpy, EGLSyncKHR sync);
   bool (*remote_eglClientWaitSyncKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout);
   bool (*remote_eglGetSyncAttribKHR)(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value);
#endif
#if EGL_BRCM_image_update_control
   bool (*remote_eglImageUpdateParameterivBRCM)(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, const EGLint *params);
   bool (*remote_eglImageUpdateParameteriBRCM)(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, EGLint param);
#endif

} REMOTE_API_TABLE;

static CLIENT_THREAD_STATE_T *locked_thread = NULL;

static REMOTE_API_TABLE logger;

#ifdef TIMELINE_EVENT_LOGGING
static void EventLog(uint32_t t, uint32_t c, uint32_t r, uint32_t d, char *desc)
{
   EventData ev;
   ev.eventType = t;
   ev.eventCode = c;
   ev.eventRow  = r;
   ev.eventData = d;
   ev.desc = desc;
   ev.eventSecs = 0;
   ev.eventNanosecs = 0;
   khrn_remote_event_log(&ev);
}

static uint32_t sEventCPUThreads[eEVENT_CPU4 - eEVENT_CPU + 1];

eEventRow khrn_event_cpu_row(void)
{
   uint32_t i;
   uint32_t tid = vcos_thread_get_id();

   for (i = eEVENT_CPU; i <= eEVENT_CPU4; i++)
   {
      if (sEventCPUThreads[i - eEVENT_CPU] == 0xFFFFFFFF)
      {
         sEventCPUThreads[i - eEVENT_CPU] = tid;
         return (eEventRow)i;
      }
      else if (sEventCPUThreads[i - eEVENT_CPU] == tid)
         return (eEventRow)i;
   }

   return eEVENT_CPU;
}
#else
#define EventLog(t, c, r, d, desc)
#define khrn_event_cpu_row() eEVENT_CPU
#endif

#define Real(func) mangled_##func

/* This function and macro wrap the hook functions inside a mutex in
 *  order to serialise the messages from the target device.  The extra faff is because
 *  some API functions ultimately call other hooked functions.  In such a case we must
 *  not emit tracing information as that will upset the analyser frontend.
 */

#ifdef BCG_MULTI_THREADED

static EGLContext s_lastSentContext = 0;

bool send_remote_thread_change(void)
{
   EGLContext        curContext = 0;

   CLIENT_PROCESS_STATE_T *process = CLIENT_GET_PROCESS_STATE();

   // We can only use the thread state once the process has been initialized
   if (process->inited)
   {
      CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();

      // Ensure the correct thread context is current for the API call
      if (thread != NULL && thread->bound_api != 0 && thread->opengl.context)
         curContext = thread->opengl.context->name;
   }

   if (curContext != s_lastSentContext)
   {
      logger.remote_thread_change(vcos_thread_get_id(), curContext);
      s_lastSentContext = curContext;
   }

   return true;
}

#define API_LOCK        { locked_thread = client_lock_api(); }
#define API_UNLOCK      { client_unlock_api(); locked_thread = NULL; }
#define COMMLOCK        send_remote_thread_change()
#define COMMUNLOCK

#else

#define API_LOCK
#define API_UNLOCK
#define COMMLOCK        (0 != commlock())
#define COMMUNLOCK	   vcos_mutex_unlock( &mu_commlock );

static VCOS_MUTEX_T mu_commlock;

static int commlock( void )
{
   int isLocked = 10; /* Try up to 10 times, so max delay = 10ms */

   while ( isLocked )
   {
      if ( VCOS_SUCCESS == vcos_mutex_trylock ( &mu_commlock ) )
         break;
      isLocked--;
      vcos_sleep( 1 );
   }

   return isLocked;
}

#endif

#define IsDebugMode() (logger.remote_inBottleneckMode && !logger.remote_inBottleneckMode())
#define IsPerfMode() (logger.remote_inBottleneckMode && logger.remote_inBottleneckMode())

static void LogError(void)
{
   if (IsDebugMode())
   {
      GLenum err = Real(glGetError)();
      if (err != GL_NO_ERROR)
      {
         if (locked_thread != NULL)
         {
            if (locked_thread->cached_gl_error == GL_NO_ERROR)
               locked_thread->cached_gl_error = err;
         }
         else
            vcos_assert(0);

         /* Tell remote logger that we have an error */
         logger.remote_error(err);
      }
   }
}

static void LogEGLError(void)
{
   if (IsDebugMode())
   {
      EGLint err = Real(eglGetError)();
      if (err != EGL_SUCCESS)
      {
         if (locked_thread != NULL)
         {
            if (locked_thread->cached_egl_error == EGL_SUCCESS)
               locked_thread->cached_egl_error = err;
         }
         else
            vcos_assert(0);

         /* Tell remote logger that we have an error */
         logger.remote_error(err);
      }
   }
}

#define ReturningFunc(retType, func, args) \
{ \
   API_LOCK\
   if (logger.remote_##func && COMMLOCK) \
   { \
      if (logger.remote_##func args) \
      { \
         uint64_t start, end; \
         uint32_t  ns = 0; \
         retType   ret; \
         bool dbg = IsDebugMode(); \
         eEventRow cpuRow = eEVENT_CPU; \
         if (dbg) \
            GetTime(&start); \
         if (sEventLogAllAPIs) \
         { \
            cpuRow = khrn_event_cpu_row(); \
            EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_##func, 0); \
            ret = Real(func)args; \
            EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_##func, 0); \
         } \
         else \
            ret = Real(func)args; \
         if (dbg) \
         {\
            GetTime(&end); \
            ns = TimeDiffNano(&start, &end); \
         }\
         LogError(); \
         logger.remote_return_##retType(ret, ns); \
         COMMUNLOCK \
         API_UNLOCK \
         return ret; \
      }\
      else \
      { \
         COMMUNLOCK \
         API_UNLOCK \
         return (retType)0; \
      } \
   } \
   else \
   { \
      retType ret = Real(func)args; \
      API_UNLOCK \
      return ret; \
   } \
}

#define EGLReturningFunc(retType, func, args) \
{ \
   API_LOCK\
   if (logger.remote_##func && COMMLOCK) \
   { \
      if (logger.remote_##func args) \
      {\
         uint64_t start, end; \
         uint32_t  ns = 0; \
         retType   ret; \
         bool dbg = IsDebugMode(); \
         eEventRow cpuRow = eEVENT_CPU; \
         if (dbg) \
            GetTime(&start); \
         if (sEventLogAllAPIs) \
         { \
            cpuRow = khrn_event_cpu_row(); \
            EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_##func, 0); \
            ret = Real(func)args; \
            EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_##func, 0); \
         } \
         else \
            ret = Real(func)args; \
         if (dbg) \
         {\
            GetTime(&end); \
            ns = TimeDiffNano(&start, &end); \
         }\
         LogEGLError(); \
         logger.remote_return_##retType(ret, ns); \
         COMMUNLOCK \
         API_UNLOCK \
         return ret; \
      }\
      else \
      { \
         COMMUNLOCK \
         API_UNLOCK \
         return (retType)0; \
      } \
   } \
   else \
   { \
      retType ret = Real(func)args; \
      API_UNLOCK \
      return ret; \
   } \
}

#define voidFunc(func, args) \
{ \
   API_LOCK\
   if (logger.remote_##func && COMMLOCK) \
   { \
      if (logger.remote_##func args) \
      {\
         uint64_t start, end; \
         uint32_t  ns = 0; \
         bool dbg = IsDebugMode(); \
         eEventRow cpuRow = eEVENT_CPU; \
         if (dbg) \
            GetTime(&start); \
         if (sEventLogAllAPIs) \
         { \
            cpuRow = khrn_event_cpu_row(); \
            EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_##func, 0); \
            Real(func)args; \
            EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_##func, 0); \
         } \
         else \
            Real(func)args; \
         if (dbg) \
         {\
            GetTime(&end); \
            ns = TimeDiffNano(&start, &end); \
         }\
         LogError(); \
         logger.remote_return_void(ns); \
      }\
      COMMUNLOCK \
   } \
   else \
      Real(func)args; \
   \
   API_UNLOCK \
}

#define voidVGFunc(func, args) \
{ \
   API_LOCK\
   Real(func)args; \
   API_UNLOCK \
}

#define returningVGFunc(retType, func, args) \
{ \
   retType ret; \
   API_LOCK\
   ret = Real(func)args; \
   API_UNLOCK \
   return ret; \
}

#define GLbitfieldFunc(func, args) ReturningFunc(GLbitfield, func, args)
#define GLBooleanFunc(func, args)  ReturningFunc(GLboolean,  func, args)
#define GLenumFunc(func, args)     ReturningFunc(GLenum,     func, args)
#define GLintFunc(func, args)      ReturningFunc(GLint,      func, args)
#define GLuintFunc(func, args)     ReturningFunc(GLuint,     func, args)

#define EGLBooleanFunc(func, args) EGLReturningFunc(EGLBoolean, func, args)
#define EGLDisplayFunc(func, args) EGLReturningFunc(EGLDisplay, func, args)
#define EGLSurfaceFunc(func, args) EGLReturningFunc(EGLSurface, func, args)
#define EGLContextFunc(func, args) EGLReturningFunc(EGLContext, func, args)
#define EGLintFunc(func, args)     EGLReturningFunc(EGLint,     func, args)
#define EGLenumFunc(func, args)    EGLReturningFunc(EGLenum,    func, args)
#define EGLSyncKHRFunc(func, args) EGLReturningFunc(EGLSyncKHR, func, args)
#define EGLImageKHRFunc(func, args) EGLReturningFunc(EGLImageKHR, func, args)

GL_API GLboolean GL_APIENTRY glIsBuffer (GLuint buffer) GLBooleanFunc(glIsBuffer, (buffer))
GL_API GLboolean GL_APIENTRY glIsEnabled (GLenum cap) GLBooleanFunc(glIsEnabled, (cap))
GL_API GLboolean GL_APIENTRY glIsFramebuffer (GLuint framebuffer) GLBooleanFunc(glIsFramebuffer, (framebuffer))
GL_API GLboolean GL_APIENTRY glIsProgram (GLuint program) GLBooleanFunc(glIsProgram, (program))
GL_API GLboolean GL_APIENTRY glIsRenderbuffer (GLuint renderbuffer) GLBooleanFunc(glIsRenderbuffer, (renderbuffer))
GL_API GLboolean GL_APIENTRY glIsShader (GLuint shader) GLBooleanFunc(glIsShader, (shader))
GL_API GLboolean GL_APIENTRY glIsTexture (GLuint texture) GLBooleanFunc(glIsTexture, (texture))
GL_API GLenum GL_APIENTRY glCheckFramebufferStatus (GLenum target) GLenumFunc(glCheckFramebufferStatus , (target))
GL_API GLenum GL_APIENTRY glGetError (void)
{
   API_LOCK
   if (logger.remote_glGetError && COMMLOCK)
   {
      GLenum err = locked_thread != NULL ? locked_thread->cached_gl_error : GL_NO_ERROR;
      if (logger.remote_glGetError())
      {
         uint32_t ns = 0;
         bool dbg = IsDebugMode();
         if (dbg)
         {
            // Make a call to eglGetError just for timing info
            uint64_t start, end;
            GetTime(&start);
            Real(glGetError)();
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }

         logger.remote_return_GLenum(err, ns);
         if (locked_thread != NULL)
            locked_thread->cached_gl_error = GL_NO_ERROR;
         COMMUNLOCK
         API_UNLOCK
         return err;
      }
      else
      {
         COMMUNLOCK
         API_UNLOCK
         return GL_NO_ERROR;
      }
   }
   else
   {
      GLenum ret = Real(glGetError)();
      API_UNLOCK
      return ret;
   }
}
GL_API GLuint GL_APIENTRY glCreateProgram (void) GLuintFunc(glCreateProgram, ())
GL_API GLuint GL_APIENTRY glCreateShader (GLenum type) GLuintFunc(glCreateShader ,(type))
GL_API const GLubyte* GL_APIENTRY glGetString (GLenum name)
{
   API_LOCK
   if (logger.remote_glGetString && COMMLOCK)
   {
      if (logger.remote_glGetString(name))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         const GLubyte *ret;
         bool dbg = IsDebugMode();
         if (dbg)
            GetTime(&start);
         ret = Real(glGetString)(name);
         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_GLuint((GLuint)ret, ns);
         COMMUNLOCK
         API_UNLOCK
         return ret;
      }
      else
      {
         COMMUNLOCK
         API_UNLOCK
         return 0;
      }
   }
   else
   {
      const GLubyte *ret = Real(glGetString)(name);
      API_UNLOCK
      return ret;
   }
}

GL_API int GL_APIENTRY glGetAttribLocation (GLuint program, const GLchar* name) GLintFunc(glGetAttribLocation , (program, name))
GL_API int GL_APIENTRY glGetUniformLocation (GLuint program, const GLchar* name) GLintFunc(glGetUniformLocation , (program, name))
GL_API void GL_APIENTRY glActiveTexture (GLenum texture) voidFunc(glActiveTexture, (texture))
GL_API void GL_APIENTRY glAlphaFunc (GLenum func, GLclampf ref) voidFunc(glAlphaFunc, (func, ref))
GL_API void GL_APIENTRY glAlphaFuncx (GLenum func, GLclampx ref) voidFunc(glAlphaFuncx, (func, ref))
GL_API void GL_APIENTRY glAttachShader (GLuint program, GLuint shader) voidFunc(glAttachShader, (program, shader))
GL_API void GL_APIENTRY glBindAttribLocation (GLuint program, GLuint index, const GLchar* name) voidFunc(glBindAttribLocation, (program, index, name))
GL_API void GL_APIENTRY glBindBuffer (GLenum target, GLuint buffer) voidFunc(glBindBuffer, (target, buffer))
GL_API void GL_APIENTRY glBindFramebuffer (GLenum target, GLuint framebuffer)
{
   API_LOCK
   if (logger.remote_glBindFramebuffer && COMMLOCK)
   {
      if (logger.remote_glBindFramebuffer(target, framebuffer))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();
         if (dbg)
            GetTime(&start);
         Real(glBindFramebuffer)(target, framebuffer);
         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();

         if (logger.remote_getBottleneckMode() & 2) /* Tiny viewport */
            Real(glViewport)(0, 0, 1, 1);

         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glBindFramebuffer)(target, framebuffer);
   API_UNLOCK
}
GL_API void GL_APIENTRY glBindRenderbuffer (GLenum target, GLuint renderbuffer) voidFunc(glBindRenderbuffer, (target, renderbuffer))
GL_API void GL_APIENTRY glBindTexture (GLenum target, GLuint texture) voidFunc(glBindTexture, (target, texture))
GL_API void GL_APIENTRY glBlendColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha) voidFunc(glBlendColor, (red, green, blue, alpha))
GL_API void GL_APIENTRY glBlendEquation (GLenum mode)
{
   API_LOCK
   if (logger.remote_glBlendEquation && COMMLOCK)
   {
      if (logger.remote_glBlendEquation(mode))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();

         if (logger.remote_getBottleneckMode() & 16) /* Overdraw */
            mode = GL_FUNC_ADD;

         if (dbg)
            GetTime(&start);
         Real(glBlendEquation)(mode);
         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glBlendEquation)(mode);

   API_UNLOCK
}
GL_API void GL_APIENTRY glBlendEquationSeparate (GLenum modeRGB, GLenum modeAlpha)
{
   API_LOCK
   if (logger.remote_glBlendEquationSeparate && COMMLOCK)
   {
      if (logger.remote_glBlendEquationSeparate(modeRGB, modeAlpha))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();

         if (logger.remote_getBottleneckMode() & 16) /* Overdraw */
         {
            modeRGB = GL_FUNC_ADD;
            modeAlpha = GL_FUNC_ADD;
         }

         if (dbg)
            GetTime(&start);
         Real(glBlendEquationSeparate)(modeRGB, modeAlpha);
         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glBlendEquationSeparate)(modeRGB, modeAlpha);

   API_UNLOCK
}
GL_API void GL_APIENTRY glBlendFunc (GLenum sfactor, GLenum dfactor)
{
   API_LOCK
   if (logger.remote_glBlendFunc && COMMLOCK)
   {
      if (logger.remote_glBlendFunc(sfactor, dfactor))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();

         if (logger.remote_getBottleneckMode() & 16) /* Overdraw */
         {
            sfactor = GL_ONE;
            dfactor = GL_ONE;
         }

         if (dbg)
            GetTime(&start);
         Real(glBlendFunc)(sfactor, dfactor);
         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glBlendFunc)(sfactor, dfactor);
   API_UNLOCK
}
GL_API void GL_APIENTRY glBlendFuncSeparate (GLenum srcRGB, GLenum dstRGB, GLenum srcAlpha, GLenum dstAlpha)
{
   API_LOCK
   if (logger.remote_glBlendFuncSeparate && COMMLOCK)
   {
      if (logger.remote_glBlendFuncSeparate(srcRGB, dstRGB, srcAlpha, dstAlpha))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();

         if (logger.remote_getBottleneckMode() & 16) /* Overdraw */
         {
            srcRGB = GL_ONE;
            dstRGB = GL_ONE;
            srcAlpha = GL_ONE;
            dstAlpha = GL_ZERO;
         }

         if (dbg)
            GetTime(&start);
         Real(glBlendFuncSeparate)(srcRGB, dstRGB, srcAlpha, dstAlpha);
         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glBlendFuncSeparate)(srcRGB, dstRGB, srcAlpha, dstAlpha);

   API_UNLOCK
}
GL_API void GL_APIENTRY glBufferData (GLenum target, GLsizeiptr size, const GLvoid *data, GLenum usage)
{
   API_LOCK

   if (logger.remote_glBufferData && COMMLOCK)
   {
      if (logger.remote_glBufferData(target, size, data, usage))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();
         eEventRow cpuRow = khrn_event_cpu_row();
         if (dbg)
            GetTime(&start);

         EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_glBufferData, 0);
         Real(glBufferData)(target, size, data, usage);
         EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_glBufferData, 0);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glBufferData)(target, size, data, usage);

   API_UNLOCK
}

GL_API void GL_APIENTRY glBufferSubData (GLenum target, GLintptr offset, GLsizeiptr size, const GLvoid *data)
{
   API_LOCK

   if (logger.remote_glBufferSubData && COMMLOCK)
   {
      if (logger.remote_glBufferSubData(target, offset, size, data))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();
         eEventRow cpuRow = khrn_event_cpu_row();
         if (dbg)
            GetTime(&start);

         EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_glBufferSubData, 0);
         Real(glBufferSubData)(target, offset, size, data);
         EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_glBufferSubData, 0);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glBufferSubData)(target, offset, size, data);

   API_UNLOCK
}

GL_API void GL_APIENTRY glClear (GLbitfield mask)
{
   API_LOCK

   if (logger.remote_glClear && COMMLOCK)
   {
      if (logger.remote_glClear(mask))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();
         eEventRow cpuRow = eEVENT_CPU;
         if (dbg)
            GetTime(&start);

         cpuRow = khrn_event_cpu_row();
         EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_glClear, 0);
         Real(glClear)(mask);
         EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_glClear, 0);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glClear)(mask);

   API_UNLOCK
}

GL_API void GL_APIENTRY glClearColor (GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha)
{
   API_LOCK
      if (logger.remote_glClearColor && COMMLOCK)
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();

         if (logger.remote_glClearColor(red, green, blue, alpha))
         {
            if (logger.remote_getBottleneckMode() & 16) /* Overdraw */
            {
               red = green = blue = 0.0f;
               alpha = 1.0f;
            }

            if (dbg)
               GetTime(&start);
            Real(glClearColor)(red, green, blue, alpha);
            if (dbg)
            {
               GetTime(&end);
               ns = TimeDiffNano(&start, &end);
            }
            LogError();
            logger.remote_return_void(ns);
         }
         COMMUNLOCK
      }
      else
         Real(glClearColor)(red, green, blue, alpha);

   API_UNLOCK
}
GL_API void GL_APIENTRY glClearColorx (GLclampx red, GLclampx green, GLclampx blue, GLclampx alpha) voidFunc(glClearColorx, (red, green, blue, alpha))
GL_API void GL_APIENTRY glClearDepthf (GLclampf depth) voidFunc(glClearDepthf, (depth))
GL_API void GL_APIENTRY glClearDepthx (GLclampx depth) voidFunc(glClearDepthx, (depth))
GL_API void GL_APIENTRY glClearStencil (GLint s) voidFunc(glClearStencil, (s))
GL_API void GL_APIENTRY glClientActiveTexture (GLenum texture) voidFunc(glClientActiveTexture, (texture))
GL_API void GL_APIENTRY glClipPlanef (GLenum plane, const GLfloat *equation) voidFunc(glClipPlanef, (plane, equation))
GL_API void GL_APIENTRY glClipPlanex (GLenum plane, const GLfixed *equation) voidFunc(glClipPlanex, (plane, equation))
GL_API void GL_APIENTRY glColor4f (GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) voidFunc(glColor4f, (red, green, blue, alpha))
GL_API void GL_APIENTRY glColor4ub (GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha) voidFunc(glColor4ub, (red, green, blue, alpha))
GL_API void GL_APIENTRY glColor4x (GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha) voidFunc(glColor4x, (red, green, blue, alpha))
GL_API void GL_APIENTRY glColorMask (GLboolean red, GLboolean green, GLboolean blue, GLboolean alpha) voidFunc(glColorMask, (red, green, blue, alpha))
GL_API void GL_APIENTRY glColorPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) voidFunc(glColorPointer, (size, type, stride, pointer))
GL_API void GL_APIENTRY glCompileShader (GLuint shader) voidFunc(glCompileShader, (shader))
GL_API void GL_APIENTRY glCompressedTexImage2D (GLenum target, GLint level, GLenum internalformat, GLsizei width, GLsizei height, GLint border, GLsizei imageSize, const GLvoid *data)
{
   API_LOCK
   if (logger.remote_glCompressedTexImage2D && COMMLOCK)
   {
      uint64_t start, end;
      uint32_t  ns = 0;
      bool dbg = IsDebugMode();
      if (dbg)
         GetTime(&start);

      if (data != NULL && logger.remote_getBottleneckMode() & 1) /* Tiny textures */
      {
         static uint8_t tiny[] = { 80, 180, 80 };

         if (level == 0)
         {
            if (logger.remote_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data))
            {
               Real(glTexImage2D)(target, level, GL_RGB, 1, 1, 0, GL_RGB, GL_UNSIGNED_BYTE, tiny);
               if (dbg)
               {
                  GetTime(&end);
                  ns = TimeDiffNano(&start, &end);
               }
               LogError();
               logger.remote_return_void(ns);
            }
         }
      }
      else
      {
         if (logger.remote_glCompressedTexImage2D(target, level, internalformat, width, height, border, imageSize, data))
         {
            Real(glCompressedTexImage2D)(target, level, internalformat, width, height, border, imageSize, data);
            if (dbg)
            {
               GetTime(&end);
               ns = TimeDiffNano(&start, &end);
            }
            LogError();
            logger.remote_return_void(ns);
         }
      }
      COMMUNLOCK
   }
   else
      Real(glCompressedTexImage2D)(target, level, internalformat, width, height, border, imageSize, data);

   API_UNLOCK
}
GL_API void GL_APIENTRY glCompressedTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLsizei imageSize, const GLvoid *data)
{
   API_LOCK
   if (logger.remote_glCompressedTexSubImage2D && COMMLOCK)
   {
      uint64_t start, end;
      uint32_t  ns = 0;
      bool dbg = IsDebugMode();
      if (dbg)
         GetTime(&start);

      if (data != NULL && logger.remote_getBottleneckMode() & 1) /* Tiny textures */
      {
         static uint8_t tiny[] = { 80, 180, 80 };

         if (level == 0)
         {
            if (logger.remote_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data))
            {
               Real(glTexSubImage2D)(target, level, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, tiny);
               if (dbg)
               {
                  GetTime(&end);
                  ns = TimeDiffNano(&start, &end);
               }
               LogError();
               logger.remote_return_void(ns);
            }
         }
      }
      else
      {
         if (logger.remote_glCompressedTexSubImage2D(target, level, xoffset, yoffset, width, height, format, imageSize, data))
         {
            Real(glCompressedTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, imageSize, data);
            if (dbg)
            {
               GetTime(&end);
               ns = TimeDiffNano(&start, &end);
            }
            LogError();
            logger.remote_return_void(ns);
         }
      }
      COMMUNLOCK
   }
   else
      Real(glCompressedTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, imageSize, data);

   API_UNLOCK
}
GL_API void GL_APIENTRY glCopyTexImage2D (GLenum target, GLint level, GLenum internalformat, GLint x, GLint y, GLsizei width, GLsizei height, GLint border) voidFunc(glCopyTexImage2D, (target, level, internalformat, x, y, width, height, border))
GL_API void GL_APIENTRY glCopyTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint x, GLint y, GLsizei width, GLsizei height) voidFunc(glCopyTexSubImage2D, (target, level, xoffset, yoffset, x, y, width, height))
GL_API void GL_APIENTRY glCullFace (GLenum mode) voidFunc(glCullFace, (mode))
GL_API void GL_APIENTRY glDeleteBuffers (GLsizei n, const GLuint *buffers) voidFunc(glDeleteBuffers, (n, buffers))
GL_API void GL_APIENTRY glDeleteFramebuffers (GLsizei n, const GLuint* framebuffers) voidFunc(glDeleteFramebuffers, (n, framebuffers))
GL_API void GL_APIENTRY glDeleteProgram (GLuint program) voidFunc(glDeleteProgram, (program))
GL_API void GL_APIENTRY glDeleteRenderbuffers (GLsizei n, const GLuint* renderbuffers) voidFunc(glDeleteRenderbuffers, (n, renderbuffers))
GL_API void GL_APIENTRY glDeleteShader (GLuint shader) voidFunc(glDeleteShader, (shader))
GL_API void GL_APIENTRY glDeleteTextures (GLsizei n, const GLuint* textures) voidFunc(glDeleteTextures, (n, textures))
GL_API void GL_APIENTRY glDepthFunc (GLenum func) voidFunc(glDepthFunc, (func))
GL_API void GL_APIENTRY glDepthMask (GLboolean flag) voidFunc(glDepthMask, (flag))
GL_API void GL_APIENTRY glDepthRangef (GLclampf zNear, GLclampf zFar) voidFunc(glDepthRangef, (zNear, zFar))
GL_API void GL_APIENTRY glDepthRangex (GLclampx zNear, GLclampx zFar) voidFunc(glDepthRangex, (zNear, zFar))
GL_API void GL_APIENTRY glDetachShader (GLuint program, GLuint shader) voidFunc(glDetachShader, (program, shader))
GL_API void GL_APIENTRY glDisable (GLenum cap)
{
   API_LOCK
      if (logger.remote_glDisable && COMMLOCK)
      {
         if (logger.remote_glDisable(cap))
         {
            uint64_t start, end;
            uint32_t  ns = 0;
            bool dbg = IsDebugMode();

            if (!(cap == GL_BLEND && (logger.remote_getBottleneckMode() & 16))) /* Overdraw */
            {
               if (dbg)
                  GetTime(&start);
               Real(glDisable)(cap);
               if (dbg)
               {
                  GetTime(&end);
                  ns = TimeDiffNano(&start, &end);
               }
            }
            LogError();
            logger.remote_return_void(ns);
         }
         COMMUNLOCK
      }
      else
         Real(glDisable)(cap);

   API_UNLOCK
}

GL_API void GL_APIENTRY glDisableClientState (GLenum array) voidFunc(glDisableClientState, (array))
GL_API void GL_APIENTRY glDisableVertexAttribArray (GLuint index) voidFunc(glDisableVertexAttribArray, (index))
GL_API void GL_APIENTRY glDrawArrays (GLenum mode, GLint first, GLsizei count)
{
   API_LOCK

   if (logger.remote_glDrawArrays && COMMLOCK)
   {
      if (logger.remote_getBottleneckMode() & 8) /* Null draw calls */
      {
         /* Do nothing */
      }
      else
      {
         if (logger.remote_glDrawArrays(mode, first, count))
         {
            uint64_t start, end;
            uint32_t  ns = 0;
            bool dbg = IsDebugMode();
            eEventRow cpuRow = khrn_event_cpu_row();
            if (dbg)
               GetTime(&start);

            EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_glDrawArrays, 0);
            Real(glDrawArrays)(mode, first, count);
            EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_glDrawArrays, 0);

            if (dbg)
            {
               GetTime(&end);
               ns = TimeDiffNano(&start, &end);
            }
            LogError();
            logger.remote_return_void(ns);
         }
      }
      COMMUNLOCK
   }
   else
      Real(glDrawArrays)(mode, first, count);

   API_UNLOCK
}
GL_API void GL_APIENTRY glDrawElements (GLenum mode, GLsizei count, GLenum type, const GLvoid* indices)
{
   API_LOCK

   if (logger.remote_glDrawElements && COMMLOCK)
   {
      if (logger.remote_getBottleneckMode() & 8) /* Null draw calls */
      {
         /* Do nothing */
      }
      else
      {
         if (logger.remote_glDrawElements(mode, count, type, indices))
         {
            uint64_t start, end;
            uint32_t  ns = 0;
            bool dbg = IsDebugMode();
            eEventRow cpuRow = khrn_event_cpu_row();
            if (dbg)
               GetTime(&start);

            EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_glDrawElements, 0);
            Real(glDrawElements)(mode, count, type, indices);
            EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_glDrawElements, 0);

            if (dbg)
            {
               GetTime(&end);
               ns = TimeDiffNano(&start, &end);
            }
            LogError();
            logger.remote_return_void(ns);
         }
      }
      COMMUNLOCK
   }
   else
      Real(glDrawElements)(mode, count, type, indices);

   API_UNLOCK
}
GL_API void GL_APIENTRY glEnable (GLenum cap) voidFunc(glEnable, (cap))
GL_API void GL_APIENTRY glEnableClientState (GLenum array) voidFunc(glEnableClientState, (array))
GL_API void GL_APIENTRY glEnableVertexAttribArray (GLuint index) voidFunc(glEnableVertexAttribArray, (index))
GL_API void GL_APIENTRY glFinish (void) voidFunc(glFinish, ())
GL_API void GL_APIENTRY glFlush (void) voidFunc(glFlush, ())
GL_API void GL_APIENTRY glFogf (GLenum pname, GLfloat param) voidFunc(glFogf, (pname, param))
GL_API void GL_APIENTRY glFogfv (GLenum pname, const GLfloat *params) voidFunc(glFogfv, (pname, params))
GL_API void GL_APIENTRY glFogx (GLenum pname, GLfixed param) voidFunc(glFogx, (pname, param))
GL_API void GL_APIENTRY glFogxv (GLenum pname, const GLfixed *params) voidFunc(glFogxv, (pname, params))
GL_API void GL_APIENTRY glFramebufferRenderbuffer (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) voidFunc(glFramebufferRenderbuffer, (target, attachment, renderbuffertarget, renderbuffer))
GL_API void GL_APIENTRY glFramebufferTexture2D (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) voidFunc(glFramebufferTexture2D, (target, attachment, textarget, texture, level))
GL_API void GL_APIENTRY glFrontFace (GLenum mode) voidFunc(glFrontFace, (mode))
GL_API void GL_APIENTRY glFrustumf (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) voidFunc(glFrustumf, (left, right, bottom, top, zNear, zFar))
GL_API void GL_APIENTRY glFrustumx (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar) voidFunc(glFrustumx, (left, right, bottom, top, zNear, zFar))
GL_API void GL_APIENTRY glGenBuffers (GLsizei n, GLuint* buffers) voidFunc(glGenBuffers, (n, buffers))
GL_API void GL_APIENTRY glGenFramebuffers (GLsizei n, GLuint* framebuffers) voidFunc(glGenFramebuffers, (n, framebuffers))
GL_API void GL_APIENTRY glGenRenderbuffers (GLsizei n, GLuint* renderbuffers) voidFunc(glGenRenderbuffers, (n, renderbuffers))
GL_API void GL_APIENTRY glGenTextures (GLsizei n, GLuint *textures) voidFunc(glGenTextures, (n, textures))
GL_API void GL_APIENTRY glGenerateMipmap (GLenum target) voidFunc(glGenerateMipmap, (target))
GL_API void GL_APIENTRY glGetActiveAttrib (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) voidFunc(glGetActiveAttrib, (program, index, bufsize, length, size, type, name))
GL_API void GL_APIENTRY glGetActiveUniform (GLuint program, GLuint index, GLsizei bufsize, GLsizei* length, GLint* size, GLenum* type, GLchar* name) voidFunc(glGetActiveUniform, (program, index, bufsize, length, size, type, name))
GL_API void GL_APIENTRY glGetAttachedShaders (GLuint program, GLsizei maxcount, GLsizei* count, GLuint* shaders) voidFunc(glGetAttachedShaders, (program, maxcount, count, shaders))
GL_API void GL_APIENTRY glGetBooleanv (GLenum pname, GLboolean* params) voidFunc(glGetBooleanv, (pname, params))
GL_API void GL_APIENTRY glGetBufferParameteriv (GLenum target, GLenum pname, GLint* params) voidFunc(glGetBufferParameteriv, (target, pname, params))
GL_API void GL_APIENTRY glGetClipPlanef (GLenum pname, GLfloat eqn[4]) voidFunc(glGetClipPlanef, (pname, eqn))
GL_API void GL_APIENTRY glGetClipPlanex (GLenum pname, GLfixed eqn[4]) voidFunc(glGetClipPlanex, (pname, eqn))
GL_API void GL_APIENTRY glGetFixedv (GLenum pname, GLfixed *params) voidFunc(glGetFixedv, (pname, params))
GL_API void GL_APIENTRY glGetFloatv (GLenum pname, GLfloat *params) voidFunc(glGetFloatv, (pname, params))
GL_API void GL_APIENTRY glGetFramebufferAttachmentParameteriv (GLenum target, GLenum attachment, GLenum pname, GLint* params) voidFunc(glGetFramebufferAttachmentParameteriv, (target, attachment, pname, params))
GL_API void GL_APIENTRY glGetIntegerv (GLenum pname, GLint *params) voidFunc(glGetIntegerv, (pname, params))
GL_API void GL_APIENTRY glGetLightfv (GLenum light, GLenum pname, GLfloat *params) voidFunc(glGetLightfv, (light, pname, params))
GL_API void GL_APIENTRY glGetLightxv (GLenum light, GLenum pname, GLfixed *params) voidFunc(glGetLightxv, (light, pname, params))
GL_API void GL_APIENTRY glGetMaterialfv (GLenum face, GLenum pname, GLfloat *params) voidFunc(glGetMaterialfv, (face, pname, params))
GL_API void GL_APIENTRY glGetMaterialxv (GLenum face, GLenum pname, GLfixed *params) voidFunc(glGetMaterialxv, (face, pname, params))
GL_API void GL_APIENTRY glGetPointerv (GLenum pname, GLvoid **params) voidFunc(glGetPointerv, (pname, params))
GL_API void GL_APIENTRY glGetProgramInfoLog (GLuint program, GLsizei bufsize, GLsizei* length, GLchar* infolog) voidFunc(glGetProgramInfoLog, (program, bufsize, length, infolog))
GL_API void GL_APIENTRY glGetProgramiv (GLuint program, GLenum pname, GLint* params) voidFunc(glGetProgramiv, (program, pname, params))
GL_API void GL_APIENTRY glGetRenderbufferParameteriv (GLenum target, GLenum pname, GLint* params) voidFunc(glGetRenderbufferParameteriv, (target, pname, params))
GL_API void GL_APIENTRY glGetShaderInfoLog (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* infolog) voidFunc(glGetShaderInfoLog, (shader, bufsize, length, infolog))
GL_API void GL_APIENTRY glGetShaderPrecisionFormat (GLenum shadertype, GLenum precisiontype, GLint* range, GLint* precision) voidFunc(glGetShaderPrecisionFormat, (shadertype, precisiontype, range, precision))
GL_API void GL_APIENTRY glGetShaderSource (GLuint shader, GLsizei bufsize, GLsizei* length, GLchar* source) voidFunc(glGetShaderSource, (shader, bufsize, length, source))
GL_API void GL_APIENTRY glGetShaderiv (GLuint shader, GLenum pname, GLint* params) voidFunc(glGetShaderiv, (shader, pname, params))
GL_API void GL_APIENTRY glGetTexEnvfv (GLenum env, GLenum pname, GLfloat *params) voidFunc(glGetTexEnvfv, (env, pname, params))
GL_API void GL_APIENTRY glGetTexEnviv (GLenum env, GLenum pname, GLint *params) voidFunc(glGetTexEnviv, (env, pname, params))
GL_API void GL_APIENTRY glGetTexEnvxv (GLenum env, GLenum pname, GLfixed *params) voidFunc(glGetTexEnvxv, (env, pname, params))
GL_API void GL_APIENTRY glGetTexParameterfv (GLenum target, GLenum pname, GLfloat *params) voidFunc(glGetTexParameterfv, (target, pname, params))
GL_API void GL_APIENTRY glGetTexParameteriv (GLenum target, GLenum pname, GLint *params) voidFunc(glGetTexParameteriv, (target, pname, params))
GL_API void GL_APIENTRY glGetTexParameterxv (GLenum target, GLenum pname, GLfixed *params) voidFunc(glGetTexParameterxv, (target, pname, params))
GL_API void GL_APIENTRY glGetUniformfv (GLuint program, GLint location, GLfloat* params) voidFunc(glGetUniformfv, (program, location, params))
GL_API void GL_APIENTRY glGetUniformiv (GLuint program, GLint location, GLint* params) voidFunc(glGetUniformiv, (program, location, params))
GL_API void GL_APIENTRY glGetVertexAttribPointerv (GLuint index, GLenum pname, GLvoid** pointer) voidFunc(glGetVertexAttribPointerv, (index, pname, pointer))
GL_API void GL_APIENTRY glGetVertexAttribfv (GLuint index, GLenum pname, GLfloat* params) voidFunc(glGetVertexAttribfv, (index, pname, params))
GL_API void GL_APIENTRY glGetVertexAttribiv (GLuint index, GLenum pname, GLint* params) voidFunc(glGetVertexAttribiv, (index, pname, params))
GL_API void GL_APIENTRY glHint (GLenum target, GLenum mode) voidFunc(glHint, (target, mode))
GL_API void GL_APIENTRY glLightModelf (GLenum pname, GLfloat param) voidFunc(glLightModelf, (pname, param))
GL_API void GL_APIENTRY glLightModelfv (GLenum pname, const GLfloat *params) voidFunc(glLightModelfv, (pname, params))
GL_API void GL_APIENTRY glLightModelx (GLenum pname, GLfixed param) voidFunc(glLightModelx, (pname, param))
GL_API void GL_APIENTRY glLightModelxv (GLenum pname, const GLfixed *params) voidFunc(glLightModelxv, (pname, params))
GL_API void GL_APIENTRY glLightf (GLenum light, GLenum pname, GLfloat param) voidFunc(glLightf, (light, pname, param))
GL_API void GL_APIENTRY glLightfv (GLenum light, GLenum pname, const GLfloat *params) voidFunc(glLightfv, (light, pname, params))
GL_API void GL_APIENTRY glLightx (GLenum light, GLenum pname, GLfixed param) voidFunc(glLightx, (light, pname, param))
GL_API void GL_APIENTRY glLightxv (GLenum light, GLenum pname, const GLfixed *params) voidFunc(glLightxv, (light, pname, params))
GL_API void GL_APIENTRY glLineWidth (GLfloat width) voidFunc(glLineWidth, (width))
GL_API void GL_APIENTRY glLineWidthx (GLfixed width) voidFunc(glLineWidthx, (width))
GL_API void GL_APIENTRY glLinkProgram (GLuint program)
{
   API_LOCK

   if (logger.remote_glLinkProgram && COMMLOCK)
   {
      if (logger.remote_glLinkProgram(program))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();
         eEventRow cpuRow = khrn_event_cpu_row();
         if (dbg)
            GetTime(&start);

         EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_glLinkProgram, 0);
         Real(glLinkProgram)(program);
         EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_glLinkProgram, 0);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glLinkProgram)(program);

   API_UNLOCK
}
GL_API void GL_APIENTRY glLoadIdentity (void) voidFunc(glLoadIdentity, ())
GL_API void GL_APIENTRY glLoadMatrixf (const GLfloat *m) voidFunc(glLoadMatrixf, (m))
GL_API void GL_APIENTRY glLoadMatrixx (const GLfixed *m) voidFunc(glLoadMatrixx, (m))
GL_API void GL_APIENTRY glLogicOp (GLenum opcode) voidFunc(glLogicOp, ( opcode))
GL_API void GL_APIENTRY glMaterialf (GLenum face, GLenum pname, GLfloat param) voidFunc(glMaterialf, (face, pname, param))
GL_API void GL_APIENTRY glMaterialfv (GLenum face, GLenum pname, const GLfloat *params) voidFunc(glMaterialfv, (face, pname, params))
GL_API void GL_APIENTRY glMaterialx (GLenum face, GLenum pname, GLfixed param) voidFunc(glMaterialx, (face, pname, param))
GL_API void GL_APIENTRY glMaterialxv (GLenum face, GLenum pname, const GLfixed *params) voidFunc(glMaterialxv, (face, pname, params))
GL_API void GL_APIENTRY glMatrixMode (GLenum mode) voidFunc(glMatrixMode, (mode))
GL_API void GL_APIENTRY glMultMatrixf (const GLfloat *m) voidFunc(glMultMatrixf, (m))
GL_API void GL_APIENTRY glMultMatrixx (const GLfixed *m) voidFunc(glMultMatrixx, (m))
GL_API void GL_APIENTRY glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q) voidFunc(glMultiTexCoord4f, (target, s, t, r, q))
GL_API void GL_APIENTRY glMultiTexCoord4x (GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q) voidFunc(glMultiTexCoord4x, (target, s, t, r, q))
GL_API void GL_APIENTRY glNormal3f (GLfloat nx, GLfloat ny, GLfloat nz) voidFunc(glNormal3f, (nx, ny, nz))
GL_API void GL_APIENTRY glNormal3x (GLfixed nx, GLfixed ny, GLfixed nz) voidFunc(glNormal3x, (nx, ny, nz))
GL_API void GL_APIENTRY glNormalPointer (GLenum type, GLsizei stride, const GLvoid *pointer) voidFunc(glNormalPointer, (type, stride, pointer))
GL_API void GL_APIENTRY glOrthof (GLfloat left, GLfloat right, GLfloat bottom, GLfloat top, GLfloat zNear, GLfloat zFar) voidFunc(glOrthof, (left, right, bottom, top, zNear, zFar))
GL_API void GL_APIENTRY glOrthox (GLfixed left, GLfixed right, GLfixed bottom, GLfixed top, GLfixed zNear, GLfixed zFar) voidFunc(glOrthox, (left, right, bottom, top, zNear, zFar))
GL_API void GL_APIENTRY glPixelStorei (GLenum pname, GLint param) voidFunc(glPixelStorei, (pname, param))
GL_API void GL_APIENTRY glPointParameterf (GLenum pname, GLfloat param) voidFunc(glPointParameterf, (pname, param))
GL_API void GL_APIENTRY glPointParameterfv (GLenum pname, const GLfloat *params) voidFunc(glPointParameterfv, (pname, params))
GL_API void GL_APIENTRY glPointParameterx (GLenum pname, GLfixed param) voidFunc(glPointParameterx, (pname, param))
GL_API void GL_APIENTRY glPointParameterxv (GLenum pname, const GLfixed *params) voidFunc(glPointParameterxv, (pname, params))
GL_API void GL_APIENTRY glPointSize (GLfloat size) voidFunc(glPointSize, (size))
GL_API void GL_APIENTRY glPointSizex (GLfixed size) voidFunc(glPointSizex, (size))
GL_API void GL_APIENTRY glPolygonOffset (GLfloat factor, GLfloat units) voidFunc(glPolygonOffset, (factor, units))
GL_API void GL_APIENTRY glPolygonOffsetx (GLfixed factor, GLfixed units) voidFunc(glPolygonOffsetx, (factor, units))
GL_API void GL_APIENTRY glPopMatrix (void) voidFunc(glPopMatrix, ())
GL_API void GL_APIENTRY glPushMatrix (void) voidFunc(glPushMatrix, ())
GL_API void GL_APIENTRY glReadPixels (GLint x, GLint y, GLsizei width, GLsizei height, GLenum format, GLenum type, GLvoid* pixels) voidFunc(glReadPixels, (x, y, width, height, format, type, pixels))
GL_API void GL_APIENTRY glReleaseShaderCompiler (void) voidFunc(glReleaseShaderCompiler, ())
GL_API void GL_APIENTRY glRenderbufferStorage (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) voidFunc(glRenderbufferStorage, (target, internalformat, width, height))
GL_API void GL_APIENTRY glRotatef (GLfloat angle, GLfloat x, GLfloat y, GLfloat z) voidFunc(glRotatef, (angle, x, y, z))
GL_API void GL_APIENTRY glRotatex (GLfixed angle, GLfixed x, GLfixed y, GLfixed z) voidFunc(glRotatex, (angle, x, y, z))
GL_API void GL_APIENTRY glSampleCoverage (GLclampf value, GLboolean invert) voidFunc(glSampleCoverage, (value, invert))
GL_API void GL_APIENTRY glSampleCoveragex (GLclampx value, GLboolean invert) voidFunc(glSampleCoveragex, (value, invert))
GL_API void GL_APIENTRY glScalef (GLfloat x, GLfloat y, GLfloat z) voidFunc(glScalef, (x, y, z))
GL_API void GL_APIENTRY glScalex (GLfixed x, GLfixed y, GLfixed z) voidFunc(glScalex, (x, y, z))
GL_API void GL_APIENTRY glScissor (GLint x, GLint y, GLsizei width, GLsizei height) voidFunc(glScissor, (x, y, width, height))
GL_API void GL_APIENTRY glShadeModel (GLenum mode) voidFunc(glShadeModel, (mode))
GL_API void GL_APIENTRY glShaderBinary (GLsizei n, const GLuint* shaders, GLenum binaryformat, const GLvoid* binary, GLsizei length) voidFunc(glShaderBinary, (n, shaders, binaryformat, binary, length))
GL_API void GL_APIENTRY glShaderSource (GLuint shader, GLsizei count, const GLchar** string, const GLint* length)
{
   API_LOCK
   if (logger.remote_glShaderSource && logger.remote_getBottleneckMode && COMMLOCK)
   {
      GLint   type;
      static  uint32_t frag_shader = 0;
      Real(glGetShaderiv)(shader, GL_SHADER_TYPE, &type);

      if (type == GL_FRAGMENT_SHADER &&
         ((logger.remote_getBottleneckMode() & 4 /* Minimal fragment shader */) ||
          (logger.remote_getBottleneckMode() & 16 /* Overdraw */)))
      {
         int32_t  i;
         uint32_t c;
         uint32_t state = 0;  /* Awaiting main */
         uint32_t totLen = 0;
         GLchar   *buf = 0;

         if (logger.remote_glShaderSource(shader, count, string, length))
         {
            for (i = 0; i < count; i++)
            {
               if (string[i])
                  totLen += ((length && length[i]) ? length[i] : (int)strlen(string[i]));
            }

            buf = (GLchar*)malloc(totLen + 1024);

            memset(buf, 0, totLen + 1024);

            /* Minimize the shader */
            for (i = 0; i < count; i++)
            {
               uint32_t len;

               if (string[i] == NULL)
                  continue;

               len = ((length && length[i]) ? length[i] : (int)strlen(string[i]));

               for (c = 0; c < len; c++)
               {
                  const GLchar *s = (string[i] + c);
                  GLchar ch = *(string[i] + c);
                  GLchar *d = (buf + c);

                  if (state == 0) /* Awaiting main */
                  {
                     if (!strncmp(s, "main", 4))
                     {
                        state++;
                        c += 3;
                        memcpy(d, s, 4);
                     }
                     else
                        *d = ch;
                  }
                  else if (state == 1) /* Check ( */
                  {
                     if (!isspace(ch) && ch != '(')
                        state--;
                     else if (!isspace(ch) && ch == '(')
                        state++;
                     *d = ch;
                  }
                  else if (state == 2) /* ...) */
                  {
                     if (ch == ')')
                        state++;
                     *d = ch;
                  }
                  else if (state == 3) /*  Drop our main in now */
                  {
                     if (logger.remote_getBottleneckMode() & 16) /* Overdraw */
                     {
                        Real(glEnable)(GL_BLEND);
                        Real(glBlendFunc)(GL_ONE, GL_ONE);
                        Real(glBlendEquation)(GL_FUNC_ADD);
                        vcos_snprintf(d, (totLen + 1024) - (d - buf), "{ gl_FragColor = vec4(0.0, 0.1, 0.0, 1.0); }\n\n");
                        buf[(totLen + 1024) - 1] = '\0';
                     }
                     else if (logger.remote_getBottleneckMode() & 4) /* Minimal fragment shader */
                     {
                        vcos_snprintf(d, (totLen + 1024) - (d - buf), "{ gl_FragColor = vec4(0.5, 0.5, %f, 1.0); }\n\n", (float)(shader % 255) / 255.0f);
                        buf[(totLen + 1024) - 1] = '\0';
                     }
                     state++;
                  }
               }
            }
            Real(glShaderSource)(shader, 1, (const GLchar **)&buf, NULL);
            LogError();
            logger.remote_return_void(0);

            free(buf);
         }
      }
      else
      {
         if (logger.remote_glShaderSource(shader, count, string, length))
         {
            uint64_t start, end;
            uint32_t  ns = 0;
            bool dbg = IsDebugMode();
            if (dbg)
               GetTime(&start);

            Real(glShaderSource)(shader, count, string, length);

            if (dbg)
            {
               GetTime(&end);
               ns = TimeDiffNano(&start, &end);
            }
            LogError();
            logger.remote_return_void(ns);
         }
      }

      if (type == GL_FRAGMENT_SHADER)
         frag_shader++;
      COMMUNLOCK
   }
   else
      Real(glShaderSource)(shader, count, string, length);
   API_UNLOCK
}
GL_API void GL_APIENTRY glStencilFunc (GLenum func, GLint ref, GLuint mask) voidFunc(glStencilFunc, (func, ref, mask))
GL_API void GL_APIENTRY glStencilFuncSeparate (GLenum face, GLenum func, GLint ref, GLuint mask) voidFunc(glStencilFuncSeparate, (face, func, ref, mask))
GL_API void GL_APIENTRY glStencilMask (GLuint mask) voidFunc(glStencilMask, (mask))
GL_API void GL_APIENTRY glStencilMaskSeparate (GLenum face, GLuint mask) voidFunc(glStencilMaskSeparate, (face, mask))
GL_API void GL_APIENTRY glStencilOp (GLenum fail, GLenum zfail, GLenum zpass) voidFunc(glStencilOp, (fail, zfail, zpass))
GL_API void GL_APIENTRY glStencilOpSeparate (GLenum face, GLenum fail, GLenum zfail, GLenum zpass) voidFunc(glStencilOpSeparate, (face, fail, zfail, zpass))
GL_API void GL_APIENTRY glTexCoordPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) voidFunc(glTexCoordPointer, (size, type, stride, pointer))
GL_API void GL_APIENTRY glTexEnvf (GLenum target, GLenum pname, GLfloat param) voidFunc(glTexEnvf, (target, pname, param))
GL_API void GL_APIENTRY glTexEnvfv (GLenum target, GLenum pname, const GLfloat *params) voidFunc(glTexEnvfv, (target, pname, params))
GL_API void GL_APIENTRY glTexEnvi (GLenum target, GLenum pname, GLint param) voidFunc(glTexEnvi, (target, pname, param))
GL_API void GL_APIENTRY glTexEnviv (GLenum target, GLenum pname, const GLint *params) voidFunc(glTexEnviv, (target, pname, params))
GL_API void GL_APIENTRY glTexEnvx (GLenum target, GLenum pname, GLfixed param) voidFunc(glTexEnvx, (target, pname, param))
GL_API void GL_APIENTRY glTexEnvxv (GLenum target, GLenum pname, const GLfixed *params) voidFunc(glTexEnvxv, (target, pname, params))
GL_API void GL_APIENTRY glTexImage2D (GLenum target, GLint level, GLint internalformat, GLsizei width, GLsizei height, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   API_LOCK
   if (logger.remote_glTexImage2D && COMMLOCK)
   {
      if (pixels != NULL && logger.remote_getBottleneckMode() & 1) /* Tiny textures */
      {
         static uint8_t tiny[] = { 180, 80, 80, 255 };

         if (level == 0)
         {
            if (logger.remote_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels))
            {
               Real(glTexImage2D)(target, level, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, tiny);
               LogError();
               logger.remote_return_void(0);
            }
         }
      }
      else
      {
         if (logger.remote_glTexImage2D(target, level, internalformat, width, height, border, format, type, pixels))
         {
            uint64_t start, end;
            uint32_t  ns = 0;
            bool dbg = IsDebugMode();
            eEventRow cpuRow = khrn_event_cpu_row();

            if (dbg)
               GetTime(&start);

            EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_glTexImage2D, 0);
            Real(glTexImage2D)(target, level, internalformat, width, height, border, format, type, pixels);
            EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_glTexImage2D, 0);

            if (dbg)
            {
               GetTime(&end);
               ns = TimeDiffNano(&start, &end);
            }
            LogError();
            logger.remote_return_void(ns);
         }
      }
      COMMUNLOCK
   }
   else
      Real(glTexImage2D)(target, level, internalformat, width, height, border, format, type, pixels);
   API_UNLOCK
}
GL_API void GL_APIENTRY glTexParameterf (GLenum target, GLenum pname, GLfloat param) voidFunc(glTexParameterf, (target, pname, param))
GL_API void GL_APIENTRY glTexParameterfv (GLenum target, GLenum pname, const GLfloat *params) voidFunc(glTexParameterfv, (target, pname, params))
GL_API void GL_APIENTRY glTexParameteri (GLenum target, GLenum pname, GLint param) voidFunc(glTexParameteri, (target, pname, param))
GL_API void GL_APIENTRY glTexParameteriv (GLenum target, GLenum pname, const GLint *params) voidFunc(glTexParameteriv, (target, pname, params))
GL_API void GL_APIENTRY glTexParameterx (GLenum target, GLenum pname, GLfixed param) voidFunc(glTexParameterx, (target, pname, param))
GL_API void GL_APIENTRY glTexParameterxv (GLenum target, GLenum pname, const GLfixed *params) voidFunc(glTexParameterxv, (target, pname, params))
GL_API void GL_APIENTRY glTexSubImage2D (GLenum target, GLint level, GLint xoffset, GLint yoffset, GLsizei width, GLsizei height, GLenum format, GLenum type, const GLvoid* pixels)
{
   API_LOCK
   if (logger.remote_glTexSubImage2D && COMMLOCK)
   {
      if (logger.remote_getBottleneckMode() & 1) /* Tiny textures */
      {
         static uint8_t tiny[] = { 255, 0, 0, 255 };

         if (level == 0)
         {
            if (logger.remote_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels))
            {
               Real(glTexSubImage2D)(target, level, 0, 0, 1, 1, GL_RGBA, GL_UNSIGNED_BYTE, tiny);
               LogError();
               logger.remote_return_void(0);
            }
         }
      }
      else
      {
         if (logger.remote_glTexSubImage2D(target, level, xoffset, yoffset, width, height, format, type, pixels))
         {
            uint64_t start, end;
            uint32_t  ns = 0;
            bool dbg = IsDebugMode();
            eEventRow cpuRow = khrn_event_cpu_row();

            if (dbg)
               GetTime(&start);

            EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_glTexSubImage2D, 0);
            Real(glTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, type, pixels);
            EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_glTexSubImage2D, 0);

            if (dbg)
            {
               GetTime(&end);
               ns = TimeDiffNano(&start, &end);
            }
            LogError();
            logger.remote_return_void(ns);
         }
      }
      COMMUNLOCK
   }
   else
      Real(glTexSubImage2D)(target, level, xoffset, yoffset, width, height, format, type, pixels);
   API_UNLOCK
}
GL_API void GL_APIENTRY glTranslatef (GLfloat x, GLfloat y, GLfloat z) voidFunc(glTranslatef, (x, y, z))
GL_API void GL_APIENTRY glTranslatex (GLfixed x, GLfixed y, GLfixed z) voidFunc(glTranslatex, (x, y, z))
GL_API void GL_APIENTRY glUniform1f (GLint location, GLfloat x) voidFunc(glUniform1f, (location, x))
GL_API void GL_APIENTRY glUniform1fv (GLint location, GLsizei count, const GLfloat* v) voidFunc(glUniform1fv, (location, count, v))
GL_API void GL_APIENTRY glUniform1i (GLint location, GLint x) voidFunc(glUniform1i, (location, x))
GL_API void GL_APIENTRY glUniform1iv (GLint location, GLsizei count, const GLint* v) voidFunc(glUniform1iv, (location, count, v))
GL_API void GL_APIENTRY glUniform2f (GLint location, GLfloat x, GLfloat y) voidFunc(glUniform2f, (location, x, y))
GL_API void GL_APIENTRY glUniform2fv (GLint location, GLsizei count, const GLfloat* v) voidFunc(glUniform2fv, (location, count, v))
GL_API void GL_APIENTRY glUniform2i (GLint location, GLint x, GLint y) voidFunc(glUniform2i, (location, x, y))
GL_API void GL_APIENTRY glUniform2iv (GLint location, GLsizei count, const GLint* v) voidFunc(glUniform2iv, (location, count, v))
GL_API void GL_APIENTRY glUniform3f (GLint location, GLfloat x, GLfloat y, GLfloat z) voidFunc(glUniform3f, (location, x, y, z))
GL_API void GL_APIENTRY glUniform3fv (GLint location, GLsizei count, const GLfloat* v) voidFunc(glUniform3fv, (location, count, v))
GL_API void GL_APIENTRY glUniform3i (GLint location, GLint x, GLint y, GLint z) voidFunc(glUniform3i, (location, x, y, z))
GL_API void GL_APIENTRY glUniform3iv (GLint location, GLsizei count, const GLint* v) voidFunc(glUniform3iv, (location, count, v))
GL_API void GL_APIENTRY glUniform4f (GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w) voidFunc(glUniform4f, (location, x, y, z, w))
GL_API void GL_APIENTRY glUniform4fv (GLint location, GLsizei count, const GLfloat* v) voidFunc(glUniform4fv, (location, count, v))
GL_API void GL_APIENTRY glUniform4i (GLint location, GLint x, GLint y, GLint z, GLint w) voidFunc(glUniform4i, (location, x, y, z, w))
GL_API void GL_APIENTRY glUniform4iv (GLint location, GLsizei count, const GLint* v) voidFunc(glUniform4iv, (location, count, v))
GL_API void GL_APIENTRY glUniformMatrix2fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) voidFunc(glUniformMatrix2fv, (location, count, transpose, value))
GL_API void GL_APIENTRY glUniformMatrix3fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) voidFunc(glUniformMatrix3fv, (location, count, transpose, value))
GL_API void GL_APIENTRY glUniformMatrix4fv (GLint location, GLsizei count, GLboolean transpose, const GLfloat* value) voidFunc(glUniformMatrix4fv, (location, count, transpose, value))
GL_API void GL_APIENTRY glUseProgram (GLuint program) voidFunc(glUseProgram, (program))
GL_API void GL_APIENTRY glValidateProgram (GLuint program) voidFunc(glValidateProgram, (program))
GL_API void GL_APIENTRY glVertexAttrib1f (GLuint indx, GLfloat x) voidFunc(glVertexAttrib1f, (indx, x))
GL_API void GL_APIENTRY glVertexAttrib1fv (GLuint indx, const GLfloat* values) voidFunc(glVertexAttrib1fv, (indx, values))
GL_API void GL_APIENTRY glVertexAttrib2f (GLuint indx, GLfloat x, GLfloat y) voidFunc(glVertexAttrib2f, (indx, x, y))
GL_API void GL_APIENTRY glVertexAttrib2fv (GLuint indx, const GLfloat* values) voidFunc(glVertexAttrib2fv, (indx, values))
GL_API void GL_APIENTRY glVertexAttrib3f (GLuint indx, GLfloat x, GLfloat y, GLfloat z) voidFunc(glVertexAttrib3f, (indx, x, y, z))
GL_API void GL_APIENTRY glVertexAttrib3fv (GLuint indx, const GLfloat* values) voidFunc(glVertexAttrib3fv, (indx, values))
GL_API void GL_APIENTRY glVertexAttrib4f (GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w) voidFunc(glVertexAttrib4f, (indx, x, y, z, w))
GL_API void GL_APIENTRY glVertexAttrib4fv (GLuint indx, const GLfloat* values) voidFunc(glVertexAttrib4fv, (indx, values))
GL_API void GL_APIENTRY glVertexAttribPointer (GLuint indx, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const GLvoid* ptr)  voidFunc(glVertexAttribPointer, (indx, size, type, normalized, stride, ptr))
GL_API void GL_APIENTRY glVertexPointer (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) voidFunc(glVertexPointer, (size, type, stride, pointer))
GL_API void GL_APIENTRY glViewport (GLint x, GLint y, GLsizei width, GLsizei height)
{
   API_LOCK
   if (logger.remote_glViewport && COMMLOCK)
   {
      if (logger.remote_getBottleneckMode() & 2) /* Tiny viewport */
      {
         if (logger.remote_glViewport(x, y, width, height))
         {
            Real(glViewport)(0, 0, 1, 1);
            LogError();
            logger.remote_return_void(0);
         }
      }
      else
      {
         if (logger.remote_glViewport(x, y, width, height))
         {
            uint64_t start, end;
            uint32_t  ns = 0;
            bool dbg = IsDebugMode();
            if (dbg)
               GetTime(&start);

            Real(glViewport)(x, y, width, height);

            if (dbg)
            {
               GetTime(&end);
               ns = TimeDiffNano(&start, &end);
            }
            LogError();
            logger.remote_return_void(ns);
         }
      }
      COMMUNLOCK
   }
   else
      Real(glViewport)(x, y, width, height);
   API_UNLOCK
}
/* GL Extensions */
GL_API void GL_APIENTRY glEGLImageTargetTexture2DOES (GLenum target, GLeglImageOES image) voidFunc(glEGLImageTargetTexture2DOES, (target, image))
GL_API void GL_APIENTRY glEGLImageTargetRenderbufferStorageOES (GLenum target, GLeglImageOES image) voidFunc(glEGLImageTargetRenderbufferStorageOES, (target, image))

GL_API void GL_APIENTRY glDrawTexsOES (GLshort x, GLshort y, GLshort z, GLshort width, GLshort height) voidFunc(glDrawTexsOES, (x, y, z, width, height))
GL_API void GL_APIENTRY glDrawTexiOES (GLint x, GLint y, GLint z, GLint width, GLint height) voidFunc(glDrawTexiOES, (x, y, z, width, height))
GL_API void GL_APIENTRY glDrawTexxOES (GLfixed x, GLfixed y, GLfixed z, GLfixed width, GLfixed height) voidFunc(glDrawTexxOES, (x, y, z, width, height))
GL_API void GL_APIENTRY glDrawTexsvOES (const GLshort *coords) voidFunc(glDrawTexsvOES, (coords))
GL_API void GL_APIENTRY glDrawTexivOES (const GLint *coords) voidFunc(glDrawTexivOES, (coords))
GL_API void GL_APIENTRY glDrawTexxvOES (const GLfixed *coords) voidFunc(glDrawTexxvOES, (coords))
GL_API void GL_APIENTRY glDrawTexfOES (GLfloat x, GLfloat y, GLfloat z, GLfloat width, GLfloat height) voidFunc(glDrawTexfOES, (x, y, z, width, height))
GL_API void GL_APIENTRY glDrawTexfvOES (const GLfloat *coords) voidFunc(glDrawTexfvOES, (coords))

GL_API GLboolean GL_APIENTRY glIsRenderbufferOES (GLuint renderbuffer) GLBooleanFunc(glIsRenderbufferOES, (renderbuffer))
GL_API void GL_APIENTRY glBindRenderbufferOES (GLenum target, GLuint renderbuffer) voidFunc(glBindRenderbufferOES, (target, renderbuffer))
GL_API void GL_APIENTRY glDeleteRenderbuffersOES (GLsizei n, const GLuint* renderbuffers) voidFunc(glDeleteRenderbuffersOES, (n, renderbuffers))
GL_API void GL_APIENTRY glGenRenderbuffersOES (GLsizei n, GLuint* renderbuffers) voidFunc(glGenRenderbuffersOES, (n, renderbuffers))
GL_API void GL_APIENTRY glRenderbufferStorageOES (GLenum target, GLenum internalformat, GLsizei width, GLsizei height) voidFunc(glRenderbufferStorageOES, (target, internalformat, width, height))
GL_API void GL_APIENTRY glGetRenderbufferParameterivOES (GLenum target, GLenum pname, GLint* params) voidFunc(glGetRenderbufferParameterivOES, (target, pname, params))
GL_API GLboolean GL_APIENTRY glIsFramebufferOES (GLuint framebuffer) GLBooleanFunc(glIsFramebufferOES, (framebuffer))
GL_API void GL_APIENTRY glBindFramebufferOES (GLenum target, GLuint framebuffer) voidFunc(glBindFramebufferOES, (target, framebuffer))
GL_API void GL_APIENTRY glDeleteFramebuffersOES (GLsizei n, const GLuint* framebuffers) voidFunc(glDeleteFramebuffersOES, (n, framebuffers))
GL_API void GL_APIENTRY glGenFramebuffersOES (GLsizei n, GLuint* framebuffers) voidFunc(glGenFramebuffersOES, (n, framebuffers))
GL_API GLenum GL_APIENTRY glCheckFramebufferStatusOES (GLenum target) GLenumFunc(glCheckFramebufferStatusOES , (target))
GL_API void GL_APIENTRY glFramebufferRenderbufferOES (GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer) voidFunc(glFramebufferRenderbufferOES, (target, attachment, renderbuffertarget, renderbuffer))
GL_API void GL_APIENTRY glFramebufferTexture2DOES (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level) voidFunc(glFramebufferTexture2DOES, (target, attachment, textarget, texture, level))
GL_API void GL_APIENTRY glGetFramebufferAttachmentParameterivOES (GLenum target, GLenum attachment, GLenum pname, GLint* params) voidFunc(glGetFramebufferAttachmentParameterivOES, (target, attachment, pname, params))
GL_API void GL_APIENTRY glGenerateMipmapOES (GLenum target) voidFunc(glGenerateMipmapOES, (target))
GL_API void GL_APIENTRY glRenderbufferStorageMultisampleEXT (GLenum target, GLsizei samples, GLenum internalformat, GLsizei width, GLsizei height) voidFunc(glRenderbufferStorageMultisampleEXT, (target, samples, internalformat, width, height))
GL_API void GL_APIENTRY glFramebufferTexture2DMultisampleEXT (GLenum target, GLenum attachment, GLenum textarget, GLuint texture, GLint level, GLsizei samples) voidFunc(glFramebufferTexture2DMultisampleEXT, (target, attachment, textarget, texture, level, samples))

/*
GL_API GLbitfield GL_APIENTRY glQueryMatrixxOES (GLfixed mantissa[16], GLint exponent[16]) GLbitfieldFunc(glQueryMatrixxOES , (mantissa, exponent))
*/

GL_API void GL_APIENTRY glPointSizePointerOES (GLenum type, GLsizei stride, const GLvoid *pointer) voidFunc(glPointSizePointerOES, (type, stride, pointer))
GL_API void GL_APIENTRY glDiscardFramebufferEXT(GLenum target, GLsizei numAttachments, const GLenum *attachments) voidFunc(glDiscardFramebufferEXT, (target, numAttachments, attachments))
GL_API void GL_APIENTRY glInsertEventMarkerEXT(GLsizei length, const GLchar *marker)
{
   API_LOCK
   if (logger.remote_glInsertEventMarkerEXT && COMMLOCK)
   {
      if (logger.remote_glInsertEventMarkerEXT(length, marker))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();
         eEventRow cpuRow = khrn_event_cpu_row();

         if (dbg)
            GetTime(&start);

         EventLog(eEVENT_ONE_SHOT, eEVENT_API, cpuRow, cmd_glInsertEventMarkerEXT, (char*)marker);
         Real(glInsertEventMarkerEXT)(length, marker);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogError();
         logger.remote_return_void(ns);
      }
      COMMUNLOCK
   }
   else
      Real(glInsertEventMarkerEXT)(length, marker);
   API_UNLOCK
}

GL_API void GL_APIENTRY glPushGroupMarkerEXT(GLsizei length, const GLchar *marker) voidFunc(glPushGroupMarkerEXT, (length, marker))
GL_API void GL_APIENTRY glPopGroupMarkerEXT(void) voidFunc(glPopGroupMarkerEXT, ())

GL_API void GL_APIENTRY glCurrentPaletteMatrixOES(GLuint matrixpaletteindex) voidFunc(glCurrentPaletteMatrixOES, (matrixpaletteindex))
GL_API void GL_APIENTRY glLoadPaletteFromModelViewMatrixOES(void) voidFunc(glLoadPaletteFromModelViewMatrixOES, ())
GL_API void GL_APIENTRY glMatrixIndexPointerOES (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) voidFunc(glMatrixIndexPointerOES, (size, type, stride, pointer))
GL_API void GL_APIENTRY glWeightPointerOES (GLint size, GLenum type, GLsizei stride, const GLvoid *pointer) voidFunc(glWeightPointerOES, (size, type, stride, pointer))

/* EGL */
EGLAPI EGLint EGLAPIENTRY eglGetError(void)
{
   API_LOCK
   if (logger.remote_eglGetError && COMMLOCK)
   {
      EGLint err = locked_thread != NULL ? locked_thread->cached_egl_error : EGL_SUCCESS;
      if (logger.remote_eglGetError())
      {
         uint32_t ns = 0;
         bool dbg = IsDebugMode();
         if (dbg)
         {
            // Make a call to eglGetError just for timing info
            uint64_t start, end;
            GetTime(&start);
            Real(eglGetError)();
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }

         logger.remote_return_EGLint(err, ns);
         if (locked_thread != NULL)
            locked_thread->cached_egl_error = EGL_SUCCESS;
         COMMUNLOCK
         API_UNLOCK
         return err;
      }
      else
      {
         COMMUNLOCK
         API_UNLOCK
         return EGL_SUCCESS;
      }
   }
   else
   {
      EGLint ret = Real(eglGetError)();
      API_UNLOCK
      return ret;
   }
}

EGLAPI EGLDisplay EGLAPIENTRY eglGetDisplay(EGLNativeDisplayType display_id)
{
   API_LOCK
   if (logger.remote_eglGetDisplay && COMMLOCK)
   {
      if (logger.remote_eglGetDisplay(display_id))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         EGLDisplay ret;
         bool dbg = IsDebugMode();
         if (dbg)
            GetTime(&start);

         ret = Real(eglGetDisplay)(display_id);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogEGLError();
         logger.remote_return_EGLDisplay(ret, ns);
         COMMUNLOCK
         API_UNLOCK
         return ret;
      }
      else
      {
         COMMUNLOCK
         API_UNLOCK
         return 0;
      }
   }
   else
   {
      EGLDisplay ret = Real(eglGetDisplay)(display_id);
      API_UNLOCK
      return ret;
   }
}
EGLAPI EGLBoolean EGLAPIENTRY eglInitialize(EGLDisplay dpy, EGLint *major, EGLint *minor)
{
   API_LOCK
   if (logger.remote_eglInitialize && COMMLOCK)
   {
      if (logger.remote_eglInitialize(dpy, major, minor))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         EGLBoolean ret;
         bool dbg = IsDebugMode();
         if (dbg)
            GetTime(&start);

         ret = Real(eglInitialize)(dpy, major, minor);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogEGLError();
         logger.remote_return_EGLBoolean(ret, ns);
         COMMUNLOCK
         API_UNLOCK
         return ret;
      }
      else
      {
         COMMUNLOCK
         API_UNLOCK
         return 0;
      }
   }
   else
   {
      EGLBoolean ret = Real(eglInitialize)(dpy, major, minor);
      API_UNLOCK
      return ret;
   }
}
EGLAPI EGLBoolean EGLAPIENTRY eglTerminate(EGLDisplay dpy) EGLBooleanFunc(eglTerminate, (dpy))
EGLAPI const char * EGLAPIENTRY eglQueryString(EGLDisplay dpy, EGLint name)
{
   API_LOCK
   if (logger.remote_eglQueryString && COMMLOCK)
   {
      if (logger.remote_eglQueryString(dpy, name))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         const char *ret;
         bool dbg = IsDebugMode();
         if (dbg)
            GetTime(&start);

         ret = Real(eglQueryString)(dpy, name);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogEGLError();
         logger.remote_return_string(ret, ns);
         COMMUNLOCK
         API_UNLOCK
         return ret;
      }
      else
      {
         COMMUNLOCK
         API_UNLOCK
         return 0;
      }
   }
   else
   {
      const char *ret = Real(eglQueryString)(dpy, name);
      API_UNLOCK
      return ret;
   }
}
EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigs(EGLDisplay dpy, EGLConfig *configs, EGLint config_size, EGLint *num_config) EGLBooleanFunc(eglGetConfigs, (dpy, configs, config_size, num_config))
EGLAPI EGLBoolean EGLAPIENTRY eglChooseConfig(EGLDisplay dpy, const EGLint *attrib_list, EGLConfig *configs, EGLint config_size, EGLint *num_config) EGLBooleanFunc(eglChooseConfig, (dpy, attrib_list, configs, config_size, num_config))
EGLAPI EGLBoolean EGLAPIENTRY eglGetConfigAttrib(EGLDisplay dpy, EGLConfig config, EGLint attribute, EGLint *value) EGLBooleanFunc(eglGetConfigAttrib, (dpy, config, attribute, value))
EGLAPI EGLSurface EGLAPIENTRY eglCreateWindowSurface(EGLDisplay dpy, EGLConfig config, EGLNativeWindowType win, const EGLint *attrib_list) EGLSurfaceFunc(eglCreateWindowSurface, (dpy, config, win, attrib_list))
EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferSurface(EGLDisplay dpy, EGLConfig config, const EGLint *attrib_list) EGLSurfaceFunc(eglCreatePbufferSurface, (dpy, config, attrib_list))
EGLAPI EGLSurface EGLAPIENTRY eglCreatePixmapSurface(EGLDisplay dpy, EGLConfig config, EGLNativePixmapType pixmap, const EGLint *attrib_list) EGLSurfaceFunc(eglCreatePixmapSurface, (dpy, config, pixmap, attrib_list))
EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy, EGLSurface surface) EGLBooleanFunc(eglDestroySurface, (dpy, surface))
EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint *value) EGLBooleanFunc(eglQuerySurface, (dpy, surface, attribute, value))
EGLAPI EGLBoolean EGLAPIENTRY eglBindAPI(EGLenum api) EGLBooleanFunc(eglBindAPI, (api))
EGLAPI EGLenum    EGLAPIENTRY eglQueryAPI(void) EGLenumFunc(eglQueryAPI, ())
EGLAPI EGLBoolean EGLAPIENTRY eglWaitClient(void) EGLBooleanFunc(eglWaitClient, ())
EGLAPI EGLBoolean EGLAPIENTRY eglReleaseThread(void) EGLBooleanFunc(eglReleaseThread, ())
EGLAPI EGLSurface EGLAPIENTRY eglCreatePbufferFromClientBuffer(EGLDisplay dpy, EGLenum buftype, EGLClientBuffer buffer, EGLConfig config, const EGLint *attrib_list) EGLSurfaceFunc(eglCreatePbufferFromClientBuffer, (dpy, buftype, buffer, config, attrib_list))
EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surface, EGLint attribute, EGLint value) EGLBooleanFunc(eglSurfaceAttrib, (dpy, surface, attribute, value))
EGLAPI EGLBoolean EGLAPIENTRY eglBindTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) EGLBooleanFunc(eglBindTexImage, (dpy, surface, buffer))
EGLAPI EGLBoolean EGLAPIENTRY eglReleaseTexImage(EGLDisplay dpy, EGLSurface surface, EGLint buffer) EGLBooleanFunc(eglReleaseTexImage, (dpy, surface, buffer))
EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy, EGLint interval) EGLBooleanFunc(eglSwapInterval, (dpy, interval))
EGLAPI EGLContext EGLAPIENTRY eglCreateContext(EGLDisplay dpy, EGLConfig config, EGLContext share_context, const EGLint *attrib_list) EGLContextFunc(eglCreateContext, (dpy, config, share_context, attrib_list))
EGLAPI EGLBoolean EGLAPIENTRY eglDestroyContext(EGLDisplay dpy, EGLContext ctx) EGLBooleanFunc(eglDestroyContext, (dpy, ctx))
EGLAPI EGLBoolean EGLAPIENTRY eglMakeCurrent(EGLDisplay dpy, EGLSurface draw, EGLSurface read, EGLContext ctx)
{
   API_LOCK
   if (logger.remote_eglMakeCurrent && COMMLOCK)
   {
      EGLBoolean ret;
      if (logger.remote_eglMakeCurrent(dpy, draw, read, ctx))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         bool dbg = IsDebugMode();
         if (dbg)
            GetTime(&start);

         ret = Real(eglMakeCurrent)(dpy, draw, read, ctx);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogEGLError();

         if (logger.remote_getBottleneckMode() & 2) /* Tiny viewport */
            Real(glViewport(0, 0, 1, 1));

         logger.remote_return_EGLBoolean(ret, ns);
         COMMUNLOCK
         API_UNLOCK
         return ret;
      }
      else
      {
         COMMUNLOCK
         API_UNLOCK
         return 0;
      }
   }
   else
   {
      EGLBoolean ret = Real(eglMakeCurrent)(dpy, draw, read, ctx);
      API_UNLOCK
      return ret;
   }
}
EGLAPI EGLContext EGLAPIENTRY eglGetCurrentContext(void) EGLContextFunc(eglGetCurrentContext, ())
EGLAPI EGLSurface EGLAPIENTRY eglGetCurrentSurface(EGLint readdraw) EGLSurfaceFunc(eglGetCurrentSurface, (readdraw))
EGLAPI EGLDisplay EGLAPIENTRY eglGetCurrentDisplay(void) EGLDisplayFunc(eglGetCurrentDisplay, ())
EGLAPI EGLBoolean EGLAPIENTRY eglQueryContext(EGLDisplay dpy, EGLContext ctx, EGLint attribute, EGLint *value) EGLBooleanFunc(eglQueryContext, (dpy, ctx, attribute, value))
EGLAPI EGLBoolean EGLAPIENTRY eglWaitGL(void) EGLBooleanFunc(eglWaitGL, ())
EGLAPI EGLBoolean EGLAPIENTRY eglWaitNative(EGLint engine) EGLBooleanFunc(eglWaitNative, (engine))
EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surface)
{
   API_LOCK
   if (logger.remote_eglSwapBuffers && COMMLOCK)
   {
      eEventRow cpuRow = khrn_event_cpu_row();
      EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_eglSwapBuffers, "** GPUMonitor comms **");
      if (logger.remote_eglSwapBuffers(dpy, surface))
      {
         uint64_t start, end;
         uint32_t  ns = 0;
         EGLBoolean   ret;
         bool dbg = IsDebugMode();
         EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_eglSwapBuffers, "** GPUMonitor comms **");
         if (dbg)
            GetTime(&start);

         EventLog(eEVENT_START, eEVENT_API, cpuRow, cmd_eglSwapBuffers, 0);
         ret = Real(eglSwapBuffers)(dpy, surface);
         EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_eglSwapBuffers, 0);

         if (dbg)
         {
            GetTime(&end);
            ns = TimeDiffNano(&start, &end);
         }
         LogEGLError();
         logger.remote_return_EGLBoolean(ret, ns);
         COMMUNLOCK
         API_UNLOCK
         return ret;
      }
      else
      {
         EventLog(eEVENT_END, eEVENT_API, cpuRow, cmd_eglSwapBuffers, "** GPUMonitor comms **");
         COMMUNLOCK
         API_UNLOCK
         return (EGLBoolean)0;
      }
   }
   else
   {
      EGLBoolean ret = Real(eglSwapBuffers)(dpy, surface);
      API_UNLOCK
      return ret;
   }
}
EGLAPI EGLBoolean EGLAPIENTRY eglCopyBuffers(EGLDisplay dpy, EGLSurface surface, EGLNativePixmapType target) EGLBooleanFunc(eglCopyBuffers, (dpy, surface, target))
#if EGL_KHR_lock_surface
EGLAPI EGLBoolean EGLAPIENTRY eglLockSurfaceKHR(EGLDisplay dpy, EGLSurface surface, const EGLint *attrib_list) EGLBooleanFunc(eglLockSurfaceKHR, (dpy, surface, attrib_list))
EGLAPI EGLBoolean EGLAPIENTRY eglUnlockSurfaceKHR(EGLDisplay dpy, EGLSurface surface) EGLBooleanFunc(eglUnlockSurfaceKHR, (dpy, surface))
#endif
#if EGL_KHR_image
EGLAPI EGLImageKHR EGLAPIENTRY eglCreateImageKHR(EGLDisplay dpy, EGLContext ctx, EGLenum target, EGLClientBuffer buffer, const EGLint *attrib_list) EGLImageKHRFunc(eglCreateImageKHR, (dpy, ctx, target, buffer, attrib_list))
EGLAPI EGLBoolean EGLAPIENTRY eglDestroyImageKHR(EGLDisplay dpy, EGLImageKHR image) EGLBooleanFunc(eglDestroyImageKHR, (dpy, image))
#endif
#if EGL_KHR_sync
EGLAPI EGLSyncKHR EGLAPIENTRY eglCreateSyncKHR(EGLDisplay dpy, EGLenum type, const EGLint *attrib_list) EGLSyncKHRFunc(eglCreateSyncKHR, (dpy, type, attrib_list))
EGLAPI EGLBoolean EGLAPIENTRY eglDestroySyncKHR(EGLDisplay dpy, EGLSyncKHR sync) EGLBooleanFunc(eglDestroySyncKHR, (dpy, sync))
EGLAPI EGLint     EGLAPIENTRY eglClientWaitSyncKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint flags, EGLTimeKHR timeout) EGLintFunc(eglClientWaitSyncKHR, (dpy, sync, flags, timeout))
EGLAPI EGLBoolean EGLAPIENTRY eglGetSyncAttribKHR(EGLDisplay dpy, EGLSyncKHR sync, EGLint attribute, EGLint *value) EGLBooleanFunc(eglGetSyncAttribKHR, (dpy, sync, attribute, value))
#endif
#if EGL_BRCM_image_update_control
EGLAPI EGLBoolean EGLAPIENTRY eglImageUpdateParameterivBRCM(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, const EGLint *params) EGLBooleanFunc(eglImageUpdateParameterivBRCM, (dpy, image, pname, params))
EGLAPI EGLBoolean EGLAPIENTRY eglImageUpdateParameteriBRCM(EGLDisplay dpy, EGLImageKHR image, EGLenum pname, EGLint param) EGLBooleanFunc(eglImageUpdateParameteriBRCM, (dpy, image, pname, param))
#endif

EGLAPI __eglMustCastToProperFunctionPointerType EGLAPIENTRY eglGetProcAddress(const char *procname)
{
   __eglMustCastToProperFunctionPointerType ret;
   /* Bypass */
   API_LOCK
   ret = Real(eglGetProcAddress)(procname);
   LogEGLError();
   API_UNLOCK
   return ret;
}

#ifndef NO_OPENVG
VG_API_CALL VGErrorCode VG_API_ENTRY vgGetError(void) returningVGFunc(VGErrorCode, vgGetError, ())
VG_API_CALL void VG_API_ENTRY vgFlush(void) voidVGFunc(vgFlush, ())
VG_API_CALL void VG_API_ENTRY vgFinish(void) voidVGFunc(vgFinish, ())

/* Getters and Setters */
VG_API_CALL void VG_API_ENTRY vgSetf (VGParamType type, VGfloat value) voidVGFunc(vgSetf , (type, value))
VG_API_CALL void VG_API_ENTRY vgSeti (VGParamType type, VGint value) voidVGFunc(vgSeti , (type, value))
VG_API_CALL void VG_API_ENTRY vgSetfv(VGParamType type, VGint count, const VGfloat * values) voidVGFunc(vgSetfv, (type, count, values))
VG_API_CALL void VG_API_ENTRY vgSetiv(VGParamType type, VGint count, const VGint * values) voidVGFunc(vgSetiv, (type, count, values))

VG_API_CALL VGfloat VG_API_ENTRY vgGetf(VGParamType type) returningVGFunc(VGfloat, vgGetf, (type))
VG_API_CALL VGint VG_API_ENTRY vgGeti(VGParamType type) returningVGFunc(VGint, vgGeti, (type))
VG_API_CALL VGint VG_API_ENTRY vgGetVectorSize(VGParamType type) returningVGFunc(VGint, vgGetVectorSize, (type))
VG_API_CALL void VG_API_ENTRY vgGetfv(VGParamType type, VGint count, VGfloat * values) voidVGFunc(vgGetfv, (type, count, values))
VG_API_CALL void VG_API_ENTRY vgGetiv(VGParamType type, VGint count, VGint * values) voidVGFunc(vgGetiv, (type, count, values))

VG_API_CALL void VG_API_ENTRY vgSetParameterf(VGHandle object, VGint paramType, VGfloat value) voidVGFunc(vgSetParameterf, (object, paramType, value))
VG_API_CALL void VG_API_ENTRY vgSetParameteri(VGHandle object, VGint paramType, VGint value) voidVGFunc(vgSetParameteri, (object, paramType, value))
VG_API_CALL void VG_API_ENTRY vgSetParameterfv(VGHandle object, VGint paramType, VGint count, const VGfloat * values) voidVGFunc(vgSetParameterfv, (object, paramType, count, values))
VG_API_CALL void VG_API_ENTRY vgSetParameteriv(VGHandle object, VGint paramType, VGint count, const VGint * values) voidVGFunc(vgSetParameteriv, (object, paramType, count, values))

VG_API_CALL VGfloat VG_API_ENTRY vgGetParameterf(VGHandle object, VGint paramType) returningVGFunc(VGfloat, vgGetParameterf, (object, paramType))
VG_API_CALL VGint VG_API_ENTRY vgGetParameteri(VGHandle object, VGint paramType) returningVGFunc(VGint, vgGetParameteri, (object, paramType))
VG_API_CALL VGint VG_API_ENTRY vgGetParameterVectorSize(VGHandle object, VGint paramType)  returningVGFunc(VGint, vgGetParameterVectorSize, (object, paramType))
VG_API_CALL void VG_API_ENTRY vgGetParameterfv(VGHandle object, VGint paramType, VGint count, VGfloat * values) voidVGFunc(vgGetParameterfv, (object, paramType, count, values))
VG_API_CALL void VG_API_ENTRY vgGetParameteriv(VGHandle object, VGint paramType, VGint count, VGint * values) voidVGFunc(vgGetParameteriv, (object, paramType, count, values))

/* Matrix Manipulation */
VG_API_CALL void VG_API_ENTRY vgLoadIdentity(void) voidVGFunc(vgLoadIdentity, ())
VG_API_CALL void VG_API_ENTRY vgLoadMatrix(const VGfloat * m) voidVGFunc(vgLoadMatrix, (m))
VG_API_CALL void VG_API_ENTRY vgGetMatrix(VGfloat * m) voidVGFunc(vgGetMatrix, (m))
VG_API_CALL void VG_API_ENTRY vgMultMatrix(const VGfloat * m) voidVGFunc(vgMultMatrix, (m))
VG_API_CALL void VG_API_ENTRY vgTranslate(VGfloat tx, VGfloat ty) voidVGFunc(vgTranslate, (tx, ty))
VG_API_CALL void VG_API_ENTRY vgScale(VGfloat sx, VGfloat sy) voidVGFunc(vgScale, (sx, sy))
VG_API_CALL void VG_API_ENTRY vgShear(VGfloat shx, VGfloat shy) voidVGFunc(vgShear, (shx, shy))
VG_API_CALL void VG_API_ENTRY vgRotate(VGfloat angle) voidVGFunc(vgRotate, (angle))

/* Masking and Clearing */
VG_API_CALL void VG_API_ENTRY vgMask(VGHandle mask, VGMaskOperation operation, VGint x, VGint y, VGint width, VGint height) voidVGFunc(vgMask, (mask, operation, x, y, width, height))
VG_API_CALL void VG_API_ENTRY vgRenderToMask(VGPath path, VGbitfield paintModes, VGMaskOperation operation) voidVGFunc(vgRenderToMask, (path, paintModes, operation))
VG_API_CALL VGMaskLayer VG_API_ENTRY vgCreateMaskLayer(VGint width, VGint height) returningVGFunc(VGMaskLayer, vgCreateMaskLayer, (width, height))
VG_API_CALL void VG_API_ENTRY vgDestroyMaskLayer(VGMaskLayer maskLayer) voidVGFunc(vgDestroyMaskLayer, (maskLayer))
VG_API_CALL void VG_API_ENTRY vgFillMaskLayer(VGMaskLayer maskLayer, VGint x, VGint y, VGint width, VGint height, VGfloat value) voidVGFunc(vgFillMaskLayer, (maskLayer, x, y, width, height, value))
VG_API_CALL void VG_API_ENTRY vgCopyMask(VGMaskLayer maskLayer, VGint dx, VGint dy, VGint sx, VGint sy, VGint width, VGint height) voidVGFunc(vgCopyMask, (maskLayer, dx, dy, sx, sy, width, height))
VG_API_CALL void VG_API_ENTRY vgClear(VGint x, VGint y, VGint width, VGint height) voidVGFunc(vgClear, (x, y, width, height))

/* Paths */
VG_API_CALL VGPath VG_API_ENTRY vgCreatePath(VGint pathFormat, VGPathDatatype datatype, VGfloat scale, VGfloat bias, VGint segmentCapacityHint, VGint coordCapacityHint, VGbitfield capabilities) returningVGFunc(VGPath, vgCreatePath, (pathFormat, datatype, scale, bias, segmentCapacityHint, coordCapacityHint, capabilities))
VG_API_CALL void VG_API_ENTRY vgClearPath(VGPath path, VGbitfield capabilities) voidVGFunc(vgClearPath, (path, capabilities))
VG_API_CALL void VG_API_ENTRY vgDestroyPath(VGPath path) voidVGFunc(vgDestroyPath, (path))
VG_API_CALL void VG_API_ENTRY vgRemovePathCapabilities(VGPath path, VGbitfield capabilities) voidVGFunc(vgRemovePathCapabilities, (path, capabilities))
VG_API_CALL VGbitfield VG_API_ENTRY vgGetPathCapabilities(VGPath path) returningVGFunc(VGbitfield, vgGetPathCapabilities, (path))
VG_API_CALL void VG_API_ENTRY vgAppendPath(VGPath dstPath, VGPath srcPath) voidVGFunc(vgAppendPath, (dstPath, srcPath))
VG_API_CALL void VG_API_ENTRY vgAppendPathData(VGPath dstPath, VGint numSegments, const VGubyte * pathSegments, const void * pathData) voidVGFunc(vgAppendPathData, (dstPath, numSegments, pathSegments, pathData))
VG_API_CALL void VG_API_ENTRY vgModifyPathCoords(VGPath dstPath, VGint startIndex, VGint numSegments, const void * pathData) voidVGFunc(vgModifyPathCoords, (dstPath, startIndex, numSegments, pathData))
VG_API_CALL void VG_API_ENTRY vgTransformPath(VGPath dstPath, VGPath srcPath) voidVGFunc(vgTransformPath, (dstPath, srcPath))
VG_API_CALL VGboolean VG_API_ENTRY vgInterpolatePath(VGPath dstPath, VGPath startPath, VGPath endPath, VGfloat amount) returningVGFunc(VGboolean, vgInterpolatePath, (dstPath, startPath, endPath, amount))
VG_API_CALL VGfloat VG_API_ENTRY vgPathLength(VGPath path, VGint startSegment, VGint numSegments) returningVGFunc(VGfloat, vgPathLength, (path, startSegment, numSegments))
VG_API_CALL void VG_API_ENTRY vgPointAlongPath(VGPath path, VGint startSegment, VGint numSegments, VGfloat distance, VGfloat * x, VGfloat * y, VGfloat * tangentX, VGfloat * tangentY) voidVGFunc(vgPointAlongPath, (path, startSegment, numSegments, distance, x, y, tangentX, tangentY))
VG_API_CALL void VG_API_ENTRY vgPathBounds(VGPath path, VGfloat * minX, VGfloat * minY, VGfloat * width, VGfloat * height) voidVGFunc(vgPathBounds, (path, minX, minY, width, height))
VG_API_CALL void VG_API_ENTRY vgPathTransformedBounds(VGPath path, VGfloat * minX, VGfloat * minY, VGfloat * width, VGfloat * height) voidVGFunc(vgPathTransformedBounds, (path, minX, minY, width, height))
VG_API_CALL void VG_API_ENTRY vgDrawPath(VGPath path, VGbitfield paintModes) voidVGFunc(vgDrawPath, (path, paintModes))

/* Paint */
VG_API_CALL VGPaint VG_API_ENTRY vgCreatePaint(void) returningVGFunc(VGPaint, vgCreatePaint, ())
VG_API_CALL void VG_API_ENTRY vgDestroyPaint(VGPaint paint) voidVGFunc(vgDestroyPaint, (paint))
VG_API_CALL void VG_API_ENTRY vgSetPaint(VGPaint paint, VGbitfield paintModes) voidVGFunc(vgSetPaint, (paint, paintModes))
VG_API_CALL VGPaint VG_API_ENTRY vgGetPaint(VGPaintMode paintMode) returningVGFunc(VGPaint, vgGetPaint, (paintMode))
VG_API_CALL void VG_API_ENTRY vgSetColor(VGPaint paint, VGuint rgba) voidVGFunc(vgSetColor, (paint, rgba))
VG_API_CALL VGuint VG_API_ENTRY vgGetColor(VGPaint paint) returningVGFunc(VGuint, vgGetColor, (paint))
VG_API_CALL void VG_API_ENTRY vgPaintPattern(VGPaint paint, VGImage pattern) voidVGFunc(vgPaintPattern, (paint, pattern))

/* Images */
VG_API_CALL VGImage VG_API_ENTRY vgCreateImage(VGImageFormat format, VGint width, VGint height, VGbitfield allowedQuality) returningVGFunc(VGImage, vgCreateImage, (format, width, height, allowedQuality))
VG_API_CALL void VG_API_ENTRY vgDestroyImage(VGImage image) voidVGFunc(vgDestroyImage, (image))
VG_API_CALL void VG_API_ENTRY vgClearImage(VGImage image, VGint x, VGint y, VGint width, VGint height) voidVGFunc(vgClearImage, (image, x, y, width, height))
VG_API_CALL void VG_API_ENTRY vgImageSubData(VGImage image, const void * data, VGint dataStride, VGImageFormat dataFormat, VGint x, VGint y, VGint width, VGint height) voidVGFunc(vgImageSubData, (image, data, dataStride, dataFormat, x, y, width, height))
VG_API_CALL void VG_API_ENTRY vgGetImageSubData(VGImage image, void * data, VGint dataStride, VGImageFormat dataFormat, VGint x, VGint y, VGint width, VGint height) voidVGFunc(vgGetImageSubData, (image, data, dataStride, dataFormat, x, y, width, height))
VG_API_CALL VGImage VG_API_ENTRY vgChildImage(VGImage parent, VGint x, VGint y, VGint width, VGint height) returningVGFunc(VGImage, vgChildImage, (parent, x, y, width, height))
VG_API_CALL VGImage VG_API_ENTRY vgGetParent(VGImage image) returningVGFunc(VGImage, vgGetParent, (image))
VG_API_CALL void VG_API_ENTRY vgCopyImage(VGImage dst, VGint dx, VGint dy, VGImage src, VGint sx, VGint sy, VGint width, VGint height, VGboolean dither) voidVGFunc(vgCopyImage, (dst, dx, dy, src, sx, sy, width, height, dither))
VG_API_CALL void VG_API_ENTRY vgDrawImage(VGImage image) voidVGFunc(vgDrawImage, (image))
VG_API_CALL void VG_API_ENTRY vgSetPixels(VGint dx, VGint dy, VGImage src, VGint sx, VGint sy, VGint width, VGint height) voidVGFunc(vgSetPixels, (dx, dy, src, sx, sy, width, height))
VG_API_CALL void VG_API_ENTRY vgWritePixels(const void * data, VGint dataStride, VGImageFormat dataFormat, VGint dx, VGint dy, VGint width, VGint height) voidVGFunc(vgWritePixels, (data, dataStride, dataFormat, dx, dy, width, height))
VG_API_CALL void VG_API_ENTRY vgGetPixels(VGImage dst, VGint dx, VGint dy, VGint sx, VGint sy, VGint width, VGint height) voidVGFunc(vgGetPixels, (dst, dx, dy, sx, sy, width, height))
VG_API_CALL void VG_API_ENTRY vgReadPixels(void * data, VGint dataStride, VGImageFormat dataFormat, VGint sx, VGint sy, VGint width, VGint height) voidVGFunc(vgReadPixels, (data, dataStride, dataFormat, sx, sy, width, height))
VG_API_CALL void VG_API_ENTRY vgCopyPixels(VGint dx, VGint dy, VGint sx, VGint sy, VGint width, VGint height) voidVGFunc(vgCopyPixels, (dx, dy, sx, sy, width, height))

/* Text */
VG_API_CALL VGFont VG_API_ENTRY vgCreateFont(VGint glyphCapacityHint) returningVGFunc(VGFont, vgCreateFont, (glyphCapacityHint))
VG_API_CALL void VG_API_ENTRY vgDestroyFont(VGFont font) voidVGFunc(vgDestroyFont, (font))
VG_API_CALL void VG_API_ENTRY vgSetGlyphToPath(VGFont font, VGuint glyphIndex, VGPath path, VGboolean isHinted, VGfloat glyphOrigin [2], VGfloat escapement[2]) voidVGFunc(vgSetGlyphToPath, (font, glyphIndex, path, isHinted, glyphOrigin, escapement))
VG_API_CALL void VG_API_ENTRY vgSetGlyphToImage(VGFont font, VGuint glyphIndex, VGImage image, VGfloat glyphOrigin [2], VGfloat escapement[2]) voidVGFunc(vgSetGlyphToImage, (font, glyphIndex, image, glyphOrigin, escapement))
VG_API_CALL void VG_API_ENTRY vgClearGlyph(VGFont font,VGuint glyphIndex) voidVGFunc(vgClearGlyph, (font, glyphIndex))
VG_API_CALL void VG_API_ENTRY vgDrawGlyph(VGFont font, VGuint glyphIndex, VGbitfield paintModes, VGboolean allowAutoHinting) voidVGFunc(vgDrawGlyph, (font, glyphIndex, paintModes, allowAutoHinting))
VG_API_CALL void VG_API_ENTRY vgDrawGlyphs(VGFont font, VGint glyphCount, const VGuint *glyphIndices, const VGfloat *adjustments_x, const VGfloat *adjustments_y, VGbitfield paintModes, VGboolean allowAutoHinting) voidVGFunc(vgDrawGlyphs, (font, glyphCount, glyphIndices, adjustments_x, adjustments_y, paintModes, allowAutoHinting))

/* Image Filters */
VG_API_CALL void VG_API_ENTRY vgColorMatrix(VGImage dst, VGImage src, const VGfloat * matrix) voidVGFunc(vgColorMatrix, (dst, src, matrix))
VG_API_CALL void VG_API_ENTRY vgConvolve(VGImage dst, VGImage src, VGint kernelWidth, VGint kernelHeight, VGint shiftX, VGint shiftY, const VGshort * kernel, VGfloat scale, VGfloat bias, VGTilingMode tilingMode) voidVGFunc(vgConvolve, (dst, src, kernelWidth, kernelHeight, shiftX, shiftY, kernel, scale, bias, tilingMode))
VG_API_CALL void VG_API_ENTRY vgSeparableConvolve(VGImage dst, VGImage src, VGint kernelWidth, VGint kernelHeight, VGint shiftX, VGint shiftY, const VGshort * kernelX, const VGshort * kernelY, VGfloat scale, VGfloat bias, VGTilingMode tilingMode) voidVGFunc(vgSeparableConvolve, (dst, src, kernelWidth, kernelHeight, shiftX, shiftY, kernelX, kernelY, scale, bias, tilingMode))
VG_API_CALL void VG_API_ENTRY vgGaussianBlur(VGImage dst, VGImage src, VGfloat stdDeviationX, VGfloat stdDeviationY, VGTilingMode tilingMode) voidVGFunc(vgGaussianBlur, (dst, src, stdDeviationX, stdDeviationY, tilingMode))
VG_API_CALL void VG_API_ENTRY vgLookup(VGImage dst, VGImage src, const VGubyte * redLUT, const VGubyte * greenLUT, const VGubyte * blueLUT, const VGubyte * alphaLUT, VGboolean outputLinear, VGboolean outputPremultiplied) voidVGFunc(vgLookup, (dst, src, redLUT, greenLUT, blueLUT, alphaLUT, outputLinear, outputPremultiplied))
VG_API_CALL void VG_API_ENTRY vgLookupSingle(VGImage dst, VGImage src, const VGuint * lookupTable, VGImageChannel sourceChannel, VGboolean outputLinear, VGboolean outputPremultiplied) voidVGFunc(vgLookupSingle, (dst, src, lookupTable, sourceChannel, outputLinear, outputPremultiplied))

/* Hardware Queries */
VG_API_CALL VGHardwareQueryResult VG_API_ENTRY vgHardwareQuery(VGHardwareQueryType key, VGint setting) returningVGFunc(VGHardwareQueryResult, vgHardwareQuery, (key, setting))

/* Renderer and Extension Information */
VG_API_CALL const VGubyte * VG_API_ENTRY vgGetString(VGStringID name) returningVGFunc(const VGubyte *, vgGetString, (name))

VGU_API_CALL VGUErrorCode VGU_API_ENTRY vguLine(VGPath path,VGfloat x0, VGfloat y0,VGfloat x1, VGfloat y1) returningVGFunc(VGUErrorCode, vguLine, (path, x0, y0, x1, y1))
VGU_API_CALL VGUErrorCode VGU_API_ENTRY vguPolygon(VGPath path, const VGfloat * points, VGint count, VGboolean closed) returningVGFunc(VGUErrorCode, vguPolygon, (path, points, count, closed))
VGU_API_CALL VGUErrorCode VGU_API_ENTRY vguRect(VGPath path, VGfloat x, VGfloat y, VGfloat width, VGfloat height) returningVGFunc(VGUErrorCode, vguRect, (path, x, y, width, height))
VGU_API_CALL VGUErrorCode VGU_API_ENTRY vguRoundRect(VGPath path, VGfloat x, VGfloat y, VGfloat width, VGfloat height, VGfloat arcWidth, VGfloat arcHeight) returningVGFunc(VGUErrorCode, vguRoundRect, (path, x, y, width, height, arcWidth, arcHeight))
VGU_API_CALL VGUErrorCode VGU_API_ENTRY vguEllipse(VGPath path, VGfloat cx, VGfloat cy, VGfloat width, VGfloat height) returningVGFunc(VGUErrorCode, vguEllipse, (path, cx, cy, width, height))
VGU_API_CALL VGUErrorCode VGU_API_ENTRY vguArc(VGPath path, VGfloat x, VGfloat y, VGfloat width, VGfloat height, VGfloat startAngle, VGfloat angleExtent, VGUArcType arcType) returningVGFunc(VGUErrorCode, vguArc, (path, x, y, width, height, startAngle, angleExtent, arcType))
VGU_API_CALL VGUErrorCode VGU_API_ENTRY vguComputeWarpQuadToSquare(VGfloat sx0, VGfloat sy0, VGfloat sx1, VGfloat sy1, VGfloat sx2, VGfloat sy2, VGfloat sx3, VGfloat sy3, VGfloat * matrix) returningVGFunc(VGUErrorCode, vguComputeWarpQuadToSquare, (sx0, sy0, sx1, sy1, sx2, sy2, sx3, sy3, matrix))
VGU_API_CALL VGUErrorCode VGU_API_ENTRY vguComputeWarpSquareToQuad(VGfloat dx0, VGfloat dy0, VGfloat dx1, VGfloat dy1, VGfloat dx2, VGfloat dy2, VGfloat dx3, VGfloat dy3, VGfloat * matrix) returningVGFunc(VGUErrorCode, vguComputeWarpSquareToQuad, (dx0, dy0, dx1, dy1, dx2, dy2, dx3, dy3, matrix))
VGU_API_CALL VGUErrorCode VGU_API_ENTRY vguComputeWarpQuadToQuad(VGfloat dx0, VGfloat dy0,  VGfloat dx1, VGfloat dy1, VGfloat dx2, VGfloat dy2,  VGfloat dx3, VGfloat dy3,  VGfloat sx0, VGfloat sy0, VGfloat sx1, VGfloat sy1, VGfloat sx2, VGfloat sy2, VGfloat sx3, VGfloat sy3, VGfloat * matrix) returningVGFunc(VGUErrorCode, vguComputeWarpQuadToQuad, (dx0, dy0,  dx1, dy1, dx2, dy2, dx3, dy3, sx0, sy0, sx1, sy1, sx2, sy2, sx3, sy3, matrix))

VG_API_CALL VGImage VG_API_ENTRY vgCreateEGLImageTargetKHR(VGeglImageKHR image) returningVGFunc(VGImage, vgCreateEGLImageTargetKHR, (image))
#endif /* NO_OPENVG */

#ifdef WIN32
#include <windows.h>

typedef HMODULE LibHandle;
typedef FARPROC FuncPtr;

static LibHandle OpenDll(const char *libName)
{
   return LoadLibraryA(libName);
}

static FuncPtr GetFunc(LibHandle h, const char *symbol)
{
   if (h != NULL)
      return GetProcAddress(h, symbol);

   return NULL;
}

static void CloseDll(LibHandle h)
{
   if (h != NULL)
      FreeLibrary(h);
}

#else

#include <dlfcn.h>

typedef void * LibHandle;
typedef void (*FuncPtr)(void);

static LibHandle OpenDll(const char *libName)
{
   return dlopen(libName, RTLD_LAZY);
}

static FuncPtr GetFunc(LibHandle h, const char *symbol)
{
   if (h != NULL)
      return dlsym(h, symbol);

    return NULL;
}

static void CloseDll(LibHandle h)
{
   if (h != NULL)
      dlclose(h);
}

#endif

static LibHandle logLibHandle = NULL;

/* Grim */
#define HOOK(f) \
   *(FuncPtr*)(&(logger.remote_##f)) = GetFunc(logLibHandle, "remote_"#f); \
   realFuncPtrs.real_##f = mangled_##f

#define HOOK_REM_ONLY(f) \
   *(FuncPtr*)(&(logger.remote_##f)) = GetFunc(logLibHandle, "remote_"#f);

#define HOOK_REAL_ONLY(f) \
   realFuncPtrs.real_##f = mangled_##f

void khrn_init_api_interposer(void)
{
   memset(&logger, 0, sizeof(REMOTE_API_TABLE));
   memset(&s_lastEvent, 0, sizeof(EventData));

#ifdef TIMELINE_EVENT_LOGGING
   memset(&sEventCPUThreads, 0xFF, sizeof(sEventCPUThreads));
#endif

   vcos_mutex_create(&s_eventMutex, "timeline_async_mutex");

#ifdef REMOTE_API_LOGGING

   /* Attach to the logging dll */
#ifdef WIN32
   logLibHandle = OpenDll("gpumonitor.dll");
#else
   logLibHandle = OpenDll("libgpumonitor.so");
#endif

   if (logLibHandle != NULL)
   {
      REAL_GL_API_TABLE realFuncPtrs;
      memset(&realFuncPtrs, 0, sizeof(realFuncPtrs));

      HOOK_REM_ONLY(inVarSet);
      HOOK_REM_ONLY(inBottleneckMode);
      HOOK_REM_ONLY(getBottleneckMode);
      HOOK_REM_ONLY(error);
      HOOK_REM_ONLY(return_void);
      HOOK_REM_ONLY(return_GLboolean);
      HOOK_REM_ONLY(return_GLbitfield);
      HOOK_REM_ONLY(return_GLenum);
      HOOK_REM_ONLY(return_GLint);
      HOOK_REM_ONLY(return_GLuint);
      HOOK_REM_ONLY(return_EGLBoolean);
      HOOK_REM_ONLY(return_EGLDisplay);
      HOOK_REM_ONLY(return_EGLSurface);
      HOOK_REM_ONLY(return_EGLEnum);
      HOOK_REM_ONLY(return_EGLContext);
      HOOK_REM_ONLY(return_EGLint);
      HOOK_REM_ONLY(return_EGLenum);
      HOOK_REM_ONLY(return_EGLImageKHR);
      HOOK_REM_ONLY(return_EGLSyncKHR);
      HOOK_REM_ONLY(return_string);
      HOOK_REM_ONLY(event_notify);
      HOOK_REM_ONLY(thread_change);
      HOOK_REAL_ONLY(eglGetProcAddress);

      HOOK(glIsBuffer);
      HOOK(glIsEnabled);
      HOOK(glIsFramebuffer);
      HOOK(glIsProgram);
      HOOK(glIsRenderbuffer);
      HOOK(glIsShader);
      HOOK(glIsTexture);
      HOOK(glCheckFramebufferStatus);
      HOOK(glGetError);
      HOOK(glCreateProgram);
      HOOK(glCreateShader);
      HOOK(glGetString);
      HOOK(glGetAttribLocation);
      HOOK(glGetUniformLocation);
      HOOK(glActiveTexture);
      HOOK(glAlphaFunc);
      HOOK(glAlphaFuncx);
      HOOK(glAttachShader);
      HOOK(glBindAttribLocation);
      HOOK(glBindBuffer);
      HOOK(glBindFramebuffer);
      HOOK(glBindRenderbuffer);
      HOOK(glBindTexture);
      HOOK(glBlendColor);
      HOOK(glBlendEquation);
      HOOK(glBlendEquationSeparate);
      HOOK(glBlendFunc);
      HOOK(glBlendFuncSeparate);
      HOOK(glBufferData);
      HOOK(glBufferSubData);
      HOOK(glClear);
      HOOK(glClearColor);
      HOOK(glClearColorx);
      HOOK(glClearDepthf);
      HOOK(glClearDepthx);
      HOOK(glClearStencil);
      HOOK(glClientActiveTexture);
      HOOK(glClipPlanef);
      HOOK(glClipPlanex);
      HOOK(glColor4f);
      HOOK(glColor4ub);
      HOOK(glColor4x);
      HOOK(glColorMask);
      HOOK(glColorPointer);
      HOOK(glCompileShader);
      HOOK(glCompressedTexImage2D);
      HOOK(glCompressedTexSubImage2D);
      HOOK(glCopyTexImage2D);
      HOOK(glCopyTexSubImage2D);
      HOOK(glCullFace);
      HOOK(glDeleteBuffers);
      HOOK(glDeleteFramebuffers);
      HOOK(glDeleteProgram);
      HOOK(glDeleteRenderbuffers);
      HOOK(glDeleteShader);
      HOOK(glDeleteTextures);
      HOOK(glDepthFunc);
      HOOK(glDepthMask);
      HOOK(glDepthRangef);
      HOOK(glDepthRangex);
      HOOK(glDetachShader);
      HOOK(glDisable);
      HOOK(glDisableClientState);
      HOOK(glDisableVertexAttribArray);
      HOOK(glDrawArrays);
      HOOK(glDrawElements);
      HOOK(glEnable);
      HOOK(glEnableClientState);
      HOOK(glEnableVertexAttribArray);
      HOOK(glFinish);
      HOOK(glFlush);
      HOOK(glFogf);
      HOOK(glFogfv);
      HOOK(glFogx);
      HOOK(glFogxv);
      HOOK(glFramebufferRenderbuffer);
      HOOK(glFramebufferTexture2D);
      HOOK(glFrontFace);
      HOOK(glFrustumf);
      HOOK(glFrustumx);
      HOOK(glGenBuffers);
      HOOK(glGenFramebuffers);
      HOOK(glGenRenderbuffers);
      HOOK(glGenTextures);
      HOOK(glGenerateMipmap);
      HOOK(glGetActiveAttrib);
      HOOK(glGetActiveUniform);
      HOOK(glGetAttachedShaders);
      HOOK(glGetBooleanv);
      HOOK(glGetBufferParameteriv);
      HOOK(glGetClipPlanef);
      HOOK(glGetClipPlanex);
      HOOK(glGetFixedv);
      HOOK(glGetFloatv);
      HOOK(glGetFramebufferAttachmentParameteriv);
      HOOK(glGetIntegerv);
      HOOK(glGetLightfv);
      HOOK(glGetLightxv);
      HOOK(glGetMaterialfv);
      HOOK(glGetMaterialxv);
      HOOK(glGetPointerv);
      HOOK(glGetProgramInfoLog);
      HOOK(glGetProgramiv);
      HOOK(glGetRenderbufferParameteriv);
      HOOK(glGetShaderInfoLog);
      HOOK(glGetShaderPrecisionFormat);
      HOOK(glGetShaderSource);
      HOOK(glGetShaderiv);
      HOOK(glGetTexEnvfv);
      HOOK(glGetTexEnviv);
      HOOK(glGetTexEnvxv);
      HOOK(glGetTexParameterfv);
      HOOK(glGetTexParameteriv);
      HOOK(glGetTexParameterxv);
      HOOK(glGetUniformfv);
      HOOK(glGetUniformiv);
      HOOK(glGetVertexAttribPointerv);
      HOOK(glGetVertexAttribfv);
      HOOK(glGetVertexAttribiv);
      HOOK(glHint);
      HOOK(glLightModelf);
      HOOK(glLightModelfv);
      HOOK(glLightModelx);
      HOOK(glLightModelxv);
      HOOK(glLightf);
      HOOK(glLightfv);
      HOOK(glLightx);
      HOOK(glLightxv);
      HOOK(glLineWidth);
      HOOK(glLineWidthx);
      HOOK(glLinkProgram);
      HOOK(glLoadIdentity);
      HOOK(glLoadMatrixf);
      HOOK(glLoadMatrixx);
      HOOK(glLogicOp);
      HOOK(glMaterialf);
      HOOK(glMaterialfv);
      HOOK(glMaterialx);
      HOOK(glMaterialxv);
      HOOK(glMatrixMode);
      HOOK(glMultMatrixf);
      HOOK(glMultMatrixx);
      HOOK(glMultiTexCoord4f);
      HOOK(glMultiTexCoord4x);
      HOOK(glNormal3f);
      HOOK(glNormal3x);
      HOOK(glNormalPointer);
      HOOK(glOrthof);
      HOOK(glOrthox);
      HOOK(glPixelStorei);
      HOOK(glPointParameterf);
      HOOK(glPointParameterfv);
      HOOK(glPointParameterx);
      HOOK(glPointParameterxv);
      HOOK(glPointSize);
      HOOK(glPointSizex);
      HOOK(glPolygonOffset);
      HOOK(glPolygonOffsetx);
      HOOK(glPopMatrix);
      HOOK(glPushMatrix);
      HOOK(glReadPixels);
      HOOK(glReleaseShaderCompiler);
      HOOK(glRenderbufferStorage);
      HOOK(glRotatef);
      HOOK(glRotatex);
      HOOK(glSampleCoverage);
      HOOK(glSampleCoveragex);
      HOOK(glScalef);
      HOOK(glScalex);
      HOOK(glScissor);
      HOOK(glShadeModel);
      HOOK(glShaderBinary);
      HOOK(glShaderSource);
      HOOK(glStencilFunc);
      HOOK(glStencilFuncSeparate);
      HOOK(glStencilMask);
      HOOK(glStencilMaskSeparate);
      HOOK(glStencilOp);
      HOOK(glStencilOpSeparate);
      HOOK(glTexCoordPointer);
      HOOK(glTexEnvf);
      HOOK(glTexEnvfv);
      HOOK(glTexEnvi);
      HOOK(glTexEnviv);
      HOOK(glTexEnvx);
      HOOK(glTexEnvxv);
      HOOK(glTexImage2D);
      HOOK(glTexParameterf);
      HOOK(glTexParameterfv);
      HOOK(glTexParameteri);
      HOOK(glTexParameteriv);
      HOOK(glTexParameterx);
      HOOK(glTexParameterxv);
      HOOK(glTexSubImage2D);
      HOOK(glTranslatef);
      HOOK(glTranslatex);
      HOOK(glUniform1f);
      HOOK(glUniform1fv);
      HOOK(glUniform1i);
      HOOK(glUniform1iv);
      HOOK(glUniform2f);
      HOOK(glUniform2fv);
      HOOK(glUniform2i);
      HOOK(glUniform2iv);
      HOOK(glUniform3f);
      HOOK(glUniform3fv);
      HOOK(glUniform3i);
      HOOK(glUniform3iv);
      HOOK(glUniform4f);
      HOOK(glUniform4fv);
      HOOK(glUniform4i);
      HOOK(glUniform4iv);
      HOOK(glUniformMatrix2fv);
      HOOK(glUniformMatrix3fv);
      HOOK(glUniformMatrix4fv);
      HOOK(glUseProgram);
      HOOK(glValidateProgram);
      HOOK(glVertexAttrib1f);
      HOOK(glVertexAttrib1fv);
      HOOK(glVertexAttrib2f);
      HOOK(glVertexAttrib2fv);
      HOOK(glVertexAttrib3f);
      HOOK(glVertexAttrib3fv);
      HOOK(glVertexAttrib4f);
      HOOK(glVertexAttrib4fv);
      HOOK(glVertexAttribPointer);
      HOOK(glVertexPointer);
      HOOK(glViewport);
      HOOK(glEGLImageTargetTexture2DOES);
      HOOK(glEGLImageTargetRenderbufferStorageOES);
      HOOK(glDrawTexsOES);
      HOOK(glDrawTexiOES);
      HOOK(glDrawTexxOES);
      HOOK(glDrawTexsvOES);
      HOOK(glDrawTexivOES);
      HOOK(glDrawTexxvOES);
      HOOK(glDrawTexfOES);
      HOOK(glDrawTexfvOES);
      HOOK(glIsRenderbufferOES);
      HOOK(glBindRenderbufferOES);
      HOOK(glDeleteRenderbuffersOES);
      HOOK(glGenRenderbuffersOES);
      HOOK(glRenderbufferStorageOES);
      HOOK(glGetRenderbufferParameterivOES);
      HOOK(glIsFramebufferOES);
      HOOK(glBindFramebufferOES);
      HOOK(glDeleteFramebuffersOES);
      HOOK(glGenFramebuffersOES);
      HOOK(glCheckFramebufferStatusOES);
      HOOK(glFramebufferRenderbufferOES);
      HOOK(glFramebufferTexture2DOES);
      HOOK(glGetFramebufferAttachmentParameterivOES);
      HOOK(glGenerateMipmapOES);
      HOOK_REM_ONLY(glQueryMatrixxOES);
      HOOK(glPointSizePointerOES);
      HOOK(glDiscardFramebufferEXT);
      HOOK(glInsertEventMarkerEXT);
      HOOK(glPushGroupMarkerEXT);
      HOOK(glPopGroupMarkerEXT);
      HOOK(eglGetError);
      HOOK(eglGetDisplay);
      HOOK(eglInitialize);
      HOOK(eglTerminate);
      HOOK(eglQueryString);
      HOOK(eglGetConfigs);
      HOOK(eglChooseConfig);
      HOOK(eglGetConfigAttrib);
      HOOK(eglCreateWindowSurface);
      HOOK(eglCreatePbufferSurface);
      HOOK(eglCreatePixmapSurface);
      HOOK(eglDestroySurface);
      HOOK(eglQuerySurface);
      HOOK(eglBindAPI);
      HOOK(eglQueryAPI);
      HOOK(eglWaitClient);
      HOOK(eglReleaseThread);
      HOOK(eglCreatePbufferFromClientBuffer);
      HOOK(eglSurfaceAttrib);
      HOOK(eglBindTexImage);
      HOOK(eglReleaseTexImage);
      HOOK(eglSwapInterval);
      HOOK(eglCreateContext);
      HOOK(eglDestroyContext);
      HOOK(eglMakeCurrent);
      HOOK(eglGetCurrentContext);
      HOOK(eglGetCurrentSurface);
      HOOK(eglGetCurrentDisplay);
      HOOK(eglQueryContext);
      HOOK(eglWaitGL);
      HOOK(eglWaitNative);
      HOOK(eglSwapBuffers);
      HOOK(eglCopyBuffers);
#if EGL_KHR_lock_surface
      HOOK(eglLockSurfaceKHR);
      HOOK(eglUnlockSurfaceKHR);
#endif
#if EGL_KHR_image
      HOOK(eglCreateImageKHR);
      HOOK(eglDestroyImageKHR);
#endif
#if EGL_KHR_sync
      HOOK(eglCreateSyncKHR);
      HOOK(eglDestroySyncKHR);
      HOOK(eglClientWaitSyncKHR);
      HOOK(eglGetSyncAttribKHR);
#endif
#if EGL_BRCM_image_update_control
      HOOK(eglImageUpdateParameterivBRCM);
      HOOK(eglImageUpdateParameteriBRCM);
#endif
#if GL_EXT_multisampled_render_to_texture
      HOOK(glRenderbufferStorageMultisampleEXT);
      HOOK(glFramebufferTexture2DMultisampleEXT);
#endif

#ifndef BCG_MULTI_THREADED
      vcos_mutex_create( &mu_commlock, "LogCommLock" );
#endif
      {
         typedef bool (*InitFuncPtr)(void *, int32_t, uint32_t, uint32_t, uint32_t);
         InitFuncPtr init = (InitFuncPtr)GetFunc(logLibHandle, "remote_log_initialize");
         uint32_t debugBuild = 1;
#ifdef NDEBUG
         debugBuild = 0;
#endif

         if (init)
         {
            if (!init(&realFuncPtrs, sizeof(realFuncPtrs), debugBuild, REMOTE_API_MAJOR_VER, REMOTE_API_MINOR_VER))
            {
               // Version mismatch, or disabled, disable spying
               memset(&logger, 0, sizeof(REMOTE_API_TABLE));
            }
         }
      }
   }
   else
   {
      ANDROID_LOGD("GPU monitor inactive - could not find libgpumonitor.so [%s]", dlerror());
   }

#endif /* REMOTE_API_LOGGING */
}

#ifdef REMOTE_API_LOGGING

bool khrn_remote_event_logging(void)
{
   uint32_t captureEvents = 0;

   if (logger.remote_inVarSet)
      captureEvents = !logger.remote_inVarSet(FOURCC('E','V','T','D'), 1);

   return captureEvents;
}

void khrn_remote_event_log(EventData *data)
{
   uint32_t i;

   if (logger.remote_event_notify)
   {
      // Find the context
      data->context = 0;

#ifdef BCG_MULTI_THREADED
      data->context = (uint32_t)s_lastSentContext;
#endif

      logger.remote_event_notify(data);

      vcos_mutex_lock(&s_eventMutex);

      if (s_lastEvent.desc != 0)
         free(s_lastEvent.desc);

      s_lastEvent = *data;
      s_lastEvent.desc = 0;
      if (data->desc != 0)
         s_lastEvent.desc = strdup(data->desc);

      // Send any events we may have queued asynchronously
      for (i = 0; i < s_queueLen; i++)
         logger.remote_event_notify(&s_queue[i]);

      s_queueLen = 0;
      vcos_mutex_unlock(&s_eventMutex);
   }
}

EventData *khrn_remote_get_last_event(void)
{
   static EventData ret;
   vcos_mutex_lock(&s_eventMutex);
   ret = s_lastEvent;
   vcos_mutex_unlock(&s_eventMutex);
   return &ret;
}

void khrn_remote_buffer_display_stamp(void)
{
#ifdef TIMELINE_EVENT_LOGGING
   if (logger.remote_event_notify)
   {
      // Note: this is called asynchronously from the display thread
      uint64_t now;
      GetTime(&now);

      vcos_mutex_lock(&s_eventMutex);

      if (s_queueLen < QUEUE_LEN)
      {
         EventData *d = &s_queue[s_queueLen];

         d->context = 0;
         d->desc = 0;
         d->eventCode = eEVENT_MISC;
         d->eventData = 0;
         d->eventRow = eEVENT_CPU;
         d->eventType = eEVENT_ONE_SHOT;
         d->eventSecs = (uint32_t)(now / 1000000);
         d->eventNanosecs = (uint32_t)((now - ((uint64_t)d->eventSecs * 1000000)) * 1000);

         s_queueLen++;
      }

      vcos_mutex_unlock(&s_eventMutex);
   }
#endif
}

#else

bool khrn_remote_event_logging(void)
{
   return false;
}

void khrn_remote_event_log(EventData *data)
{
}

EventData *khrn_remote_get_last_event(void)
{
   return &s_lastEvent;
}

void khrn_remote_buffer_display_stamp(void)
{
}

#endif

#else

void khrn_init_api_interposer() {}

#endif /* REMOTE_API_LOGGING */
