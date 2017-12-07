/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../common/khrn_int_common.h"
#include "libs/util/dglenum/dglenum.h"
#include "../common/khrn_int_util.h"

#include "glxx_server.h"
#include "glxx_draw.h"
#include "glxx_server_internal.h"
#include "../common/khrn_process.h"

#include "libs/util/snprintf.h"
#include "libs/core/v3d/v3d_ident.h"

static bool is_precision_type(GLenum type);
static bool is_float_type(GLenum type);
static void attrib_enable(uint32_t index, bool enabled) {
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   glintAttribEnable(state, index, enabled);

   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glDisableVertexAttribArray(GLuint index)
{
   attrib_enable(index, false);
}

GL_API void GL_APIENTRY glEnableVertexAttribArray(GLuint index)
{
   attrib_enable(index, true);
}

GL_API const GLubyte * GL_APIENTRY glGetString(GLenum name)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   const char *result = NULL;

   if (!state) return NULL;

   if (IS_GL_11(state))
   {
      switch (name)
      {
      case GL_VENDOR:
#ifndef NDEBUG
         result = "Broadcom DEBUG";
#else
         result = "Broadcom";
#endif
         break;
      case GL_RENDERER:
         result = khrn_get_device_name();
         break;
      case GL_VERSION:
         result = "OpenGL ES-CM 1.1";
         break;
      case GL_EXTENSIONS:
         result = khrn_get_gl11_exts_str();
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
   }
   else
   {
      switch (name)
      {
      case GL_VENDOR:
#ifndef NDEBUG
         result = "Broadcom DEBUG";
#else
         result = "Broadcom";
#endif
         break;
      case GL_RENDERER:
         result = khrn_get_device_name();
         break;
      case GL_VERSION:
         if (KHRN_GLES32_DRIVER)
            result = "OpenGL ES 3.2";
         else if (V3D_VER_AT_LEAST(3,3,0,0))
            result = "OpenGL ES 3.1";
         else
            result = "OpenGL ES 3.0";
         break;
      case GL_SHADING_LANGUAGE_VERSION:
         if (KHRN_GLES32_DRIVER)
            result = "OpenGL ES GLSL ES 3.20";
         else if (V3D_VER_AT_LEAST(3,3,0,0))
            result = "OpenGL ES GLSL ES 3.10";
         else
            result = "OpenGL ES GLSL ES 3.00";
         break;
      case GL_EXTENSIONS:
         result = khrn_get_gl3x_exts_str();
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
   }

   glxx_unlock_server_state();
   return (const GLubyte *)result;
}

GL_API const GLubyte* GL_APIENTRY glGetStringi(GLenum name, GLuint index)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return NULL;

   const GLubyte* res = NULL;
   if (name != GL_EXTENSIONS)
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else if (index >= khrn_get_num_gl3x_exts())
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
   else
      res = (const GLubyte *)khrn_get_gl3x_ext(index);

   glxx_unlock_server_state();
   return res;
}

/*
   GetVertexAttribPointer

   VERTEX ATTRIB ARRAY POINTER NULL GetVertexAttribPointer
*/

GL_API void GL_APIENTRY glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state) return;

   if (pname != GL_VERTEX_ATTRIB_ARRAY_POINTER) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (index >= GLXX_CONFIG_MAX_VERTEX_ATTRIBS) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   *pointer = glintAttribGetPointer(state, index);

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glReleaseShaderCompiler(void)
{
}

GL_API void GL_APIENTRY glShaderBinary(GLint n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLint length)
{
   unused(n);
   unused(shaders);
   unused(binaryformat);
   unused(binary);
   unused(length);

   glxx_set_error_api(OPENGL_ES_3X, GL_INVALID_ENUM);
}

/* OES_shader_source + OES_shader_binary */

static bool is_precision_type(GLenum type)
{
   return type == GL_LOW_FLOAT    ||
          type == GL_MEDIUM_FLOAT ||
          type == GL_HIGH_FLOAT   ||
          type == GL_LOW_INT      ||
          type == GL_MEDIUM_INT   ||
          type == GL_HIGH_INT;
}

static bool is_float_type(GLenum type)
{
   return type == GL_LOW_FLOAT    ||
          type == GL_MEDIUM_FLOAT ||
          type == GL_HIGH_FLOAT;
}

GL_API void GL_APIENTRY glGetShaderPrecisionFormat(GLenum shadertype, GLenum precisiontype, GLint *range, GLint *precision)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   const int ranges[2][2] = { { 127, 127 }, { 31, 30 } };
   const int precisions[2] = { 23, 0 };
   int idx;

   if (!state) return;

   if (!glxx_is_shader_type(shadertype) || !is_precision_type(precisiontype))
   {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (is_float_type(precisiontype)) idx = 0;
   else                              idx = 1;

   if (range) {
      range[0] = ranges[idx][0];
      range[1] = ranges[idx][1];
   }

   if (precision) *precision = precisions[idx];

end:
   glxx_unlock_server_state();
}


/* GLES 2.0 attrib functions: vertex_attrib */
GL_API void GL_APIENTRY glVertexAttrib1f(GLuint indx, GLfloat x)
{
   glintAttrib(OPENGL_ES_3X, indx, x, 0.0f, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
   glintAttrib(OPENGL_ES_3X, indx, x, y, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
   glintAttrib(OPENGL_ES_3X, indx, x, y, z, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   glintAttrib(OPENGL_ES_3X, indx, x, y, z, w);
}

GL_API void GL_APIENTRY glVertexAttrib1fv(GLuint indx, const GLfloat *values)
{
   glintAttrib(OPENGL_ES_3X, indx, values[0], 0.0f, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib2fv(GLuint indx, const GLfloat *values)
{
   glintAttrib(OPENGL_ES_3X, indx, values[0], values[1], 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib3fv(GLuint indx, const GLfloat *values)
{
   glintAttrib(OPENGL_ES_3X, indx, values[0], values[1], values[2], 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib4fv(GLuint indx, const GLfloat *values)
{
   glintAttrib(OPENGL_ES_3X, indx, values[0], values[1], values[2], values[3]);
}


GL_API void GL_APIENTRY glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
   glintAttribI(OPENGL_ES_3X, index, x, y, z, w, true);
}

GL_API void GL_APIENTRY glVertexAttribI4iv(GLuint index, const GLint* v)
{
   glintAttribI(OPENGL_ES_3X, index, v[0], v[1], v[2], v[3], true);
}

GL_API void GL_APIENTRY glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
   glintAttribI(OPENGL_ES_3X, index, x, y, z, w, false);
}

GL_API void GL_APIENTRY glVertexAttribI4uiv(GLuint index, const GLuint* v)
{
   glintAttribI(OPENGL_ES_3X, index, v[0], v[1], v[2], v[3], false);
}

GL_API void GL_APIENTRY glBindVertexArrayOES(GLuint array)
{
   glintBindVertexArray(array);
}

GL_API void GL_APIENTRY glDeleteVertexArraysOES(GLsizei n, const GLuint* arrays)
{
   glintDeleteVertexArrays(n, arrays);
}

GL_API void GL_APIENTRY glGenVertexArraysOES(GLsizei n, GLuint* arrays)
{
   glintGenVertexArrays(n, arrays);
}

GL_API GLboolean GL_APIENTRY glIsVertexArrayOES(GLuint array)
{
   return glintIsVertexArray(array);
}
