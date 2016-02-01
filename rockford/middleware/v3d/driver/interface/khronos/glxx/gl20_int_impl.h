/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
OpenGL ES 2.0 server-side implementation functions.
=============================================================================*/

#ifndef GL20_INT_IMPL_H
#define GL20_INT_IMPL_H

//gl 2.0 specific
extern void glAttachShader_impl_20(GLuint program, GLuint shader);
extern void glBindAttribLocation_impl_20(GLuint program, GLuint index, const char *name);
extern void glBlendColor_impl_20(GLclampf red, GLclampf green, GLclampf blue, GLclampf alpha);
extern void glBlendEquationSeparate_impl_20(GLenum modeRGB, GLenum modeAlpha);
extern GLuint glCreateProgram_impl_20(void);
extern GLuint glCreateShader_impl_20(GLenum type);
extern void glDeleteProgram_impl_20(GLuint program);
extern void glDeleteShader_impl_20(GLuint shader);
extern void glDetachShader_impl_20(GLuint program, GLuint shader);
//extern void glDisableVertexAttribArray_impl_20(GLuint index);
//extern void glEnableVertexAttribArray_impl_20(GLuint index);
extern void glGetActiveAttrib_impl_20(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, char *name);
extern void glGetActiveUniform_impl_20(GLuint program, GLuint index, GLsizei bufsize, GLsizei *length, GLint *size, GLenum *type, char *name);
extern void glGetAttachedShaders_impl_20(GLuint program, GLsizei maxcount, GLsizei *count, GLuint *shaders);
extern int glGetAttribLocation_impl_20(GLuint program, const char *name);
extern int glGetProgramiv_impl_20(GLuint program, GLenum pname, GLint *params);
extern void glGetProgramInfoLog_impl_20(GLuint program, GLsizei bufsize, GLsizei *length, char *infolog);
extern int glGetUniformfv_impl_20(GLuint program, GLint location, GLfloat *params);
extern int glGetUniformiv_impl_20(GLuint program, GLint location, GLint *params);
extern int glGetUniformLocation_impl_20(GLuint program, const char *name);
//extern void glGetVertexAttribfv_impl_20(GLuint index, GLenum pname, GLfloat *params);
//extern void glGetVertexAttribiv_impl_20(GLuint index, GLenum pname, GLint *params);
//extern void glGetVertexAttribPointerv_impl_20(GLuint index, GLenum pname, void **pointer);
extern GLboolean glIsProgram_impl_20(GLuint program);
extern GLboolean glIsShader_impl_20(GLuint shader);
extern void glLinkProgram_impl_20(GLuint program);
extern void glPointSize_impl_20(GLfloat size);
extern void glUniform1i_impl_20(GLint location, GLint x);
extern void glUniform2i_impl_20(GLint location, GLint x, GLint y);
extern void glUniform3i_impl_20(GLint location, GLint x, GLint y, GLint z);
extern void glUniform4i_impl_20(GLint location, GLint x, GLint y, GLint z, GLint w);
extern void glUniform1f_impl_20(GLint location, GLfloat x);
extern void glUniform2f_impl_20(GLint location, GLfloat x, GLfloat y);
extern void glUniform3f_impl_20(GLint location, GLfloat x, GLfloat y, GLfloat z);
extern void glUniform4f_impl_20(GLint location, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
extern void glUniform1iv_impl_20(GLint location, GLsizei count, int size, const GLint *v);
extern void glUniform2iv_impl_20(GLint location, GLsizei count, int size, const GLint *v);
extern void glUniform3iv_impl_20(GLint location, GLsizei count, int size, const GLint *v);
extern void glUniform4iv_impl_20(GLint location, GLsizei count, int size, const GLint *v);
extern void glUniform1fv_impl_20(GLint location, GLsizei count, int size, const GLfloat *v);
extern void glUniform2fv_impl_20(GLint location, GLsizei count, int size, const GLfloat *v);
extern void glUniform3fv_impl_20(GLint location, GLsizei count, int size, const GLfloat *v);
extern void glUniform4fv_impl_20(GLint location, GLsizei count, int size, const GLfloat *v);
extern void glUniformMatrix2fv_impl_20(GLint location, GLsizei count, GLboolean transpose, int size, const GLfloat *value);
extern void glUniformMatrix3fv_impl_20(GLint location, GLsizei count, GLboolean transpose, int size, const GLfloat *value);
extern void glUniformMatrix4fv_impl_20(GLint location, GLsizei count, GLboolean transpose, int size, const GLfloat *value);
extern void glUseProgram_impl_20(GLuint program);
extern void glValidateProgram_impl_20(GLuint program);
//extern void glVertexAttrib1f_impl_20(GLuint indx, GLfloat x);
//extern void glVertexAttrib2f_impl_20(GLuint indx, GLfloat x, GLfloat y);
//extern void glVertexAttrib3f_impl_20(GLuint indx, GLfloat x, GLfloat y, GLfloat z);
//extern void glVertexAttrib4f_impl_20(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
//extern void glVertexAttrib1fv_impl_20(GLuint indx, const GLfloat *values);
//extern void glVertexAttrib2fv_impl_20(GLuint indx, const GLfloat *values);
//extern void glVertexAttrib3fv_impl_20(GLuint indx, const GLfloat *values);
//extern void glVertexAttrib4fv_impl_20(GLuint indx, const GLfloat *values);


/* OES_shader_source */
extern void glCompileShader_impl_20(GLuint shader);
extern int glGetShaderiv_impl_20(GLuint shader, GLenum pname, GLint *params);
extern void glGetShaderInfoLog_impl_20(GLuint shader, GLsizei bufsize, GLsizei *length, char *infolog);
extern void glGetShaderSource_impl_20(GLuint shader, GLsizei bufsize, GLsizei *length, char *source);
extern void glShaderSource_impl_20(GLuint shader, GLsizei count, const char **string, const GLint *length);
//extern void glGetShaderPrecisionFormat_impl_20(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision);
extern void glGetShaderPrecisionFormat_impl_20(GLenum shadertype, GLenum precisiontype, GLint *result);

/*****************************************************************************************/
/*                                 OES extension functions                               */
/*****************************************************************************************/

//gl 2.0 specific

extern void glVertexAttribPointer_impl_20(GLuint indx);

#if GL_OES_EGL_image
extern void glEGLImageTargetRenderbufferStorageOES_impl_20(GLenum target, GLeglImageOES image);
#endif

#endif /* GL20_INT_IMPL_H */