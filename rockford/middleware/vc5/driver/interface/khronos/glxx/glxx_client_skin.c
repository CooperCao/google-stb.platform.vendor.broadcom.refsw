/*=============================================================================
Copyright (c) 2011 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
GL-functions which do something on both skin and fruit
=============================================================================*/
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/tools/dglenum/dglenum.h"
#include "interface/khronos/common/khrn_int_util.h"

#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_draw.h"
#include "middleware/khronos/glxx/glxx_server_internal.h"
#include "middleware/khronos/glxx/glxx_log.h"
#include "middleware/khronos/glxx/glxx_extensions.h"
#include "middleware/khronos/common/khrn_process.h"

#include "helpers/snprintf.h"

#define OPENGL_ES_NOT_11 ( OPENGL_ES_30 | OPENGL_ES_31 )

static bool is_precision_type(GLenum type);
static bool is_float_type(GLenum type);
static void attrib_enable(uint32_t index, bool enabled) {
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
   if (!state) return;

   glintAttribEnable(state, index, enabled);

   GL30_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDisableVertexAttribArray(GLuint index)
{
   attrib_enable(index, false);
}

GL_API void GL_APIENTRY glEnableVertexAttribArray(GLuint index)
{
   attrib_enable(index, true);
}

static const char* get_string_renderer()
{
   // The format is "VideoCore V HW (V3D-<H><S><T>)" where...
   // <H> is "5" for v3dv3
   // <S> is the number of slices per core
   // <T> is (texture units per core - 1) * 5
   static char renderer[30] = { 0 };
   unsigned int slices = khrn_get_num_slices_per_core();
   unsigned int tmus = khrn_get_num_tmus_per_core();
   snprintf(renderer, sizeof(renderer), "VideoCore V HW (V3D-5%u%u)", slices, (tmus - 1) * 5);
   return renderer;
}

GL_API const GLubyte * GL_APIENTRY glGetString(GLenum name)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
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
         result = get_string_renderer();
         break;
      case GL_VERSION:
         result = "OpenGL ES-CM 1.1";
         break;
      case GL_EXTENSIONS:
         result =
            "GL_OES_point_size_array "
            "GL_OES_compressed_ETC1_RGB8_texture "
            "GL_OES_compressed_paletted_texture "
            "GL_OES_texture_npot "
            "GL_OES_EGL_image "
            "GL_OES_EGL_image_external "
            "GL_EXT_discard_framebuffer "
            "GL_OES_query_matrix "
            "GL_OES_framebuffer_object "
            "GL_OES_surfaceless_context "
            "GL_OES_rgb8_rgba8 "
            "GL_OES_depth24 "
            "GL_OES_stencil8 "
            "GL_OES_packed_depth_stencil "
            "GL_OES_EGL_sync "
            "GL_EXT_multisampled_render_to_texture "
            "GL_KHR_debug "
#if GL_OES_draw_texture
            "GL_OES_draw_texture "
#endif
            "GL_OES_mapbuffer "
#if GL_EXT_texture_format_BGRA8888
            "GL_EXT_texture_format_BGRA8888 "
#endif
#if GL_OES_matrix_palette && 0 /* TODO Matrix palette shaders don't compile yet */
            "GL_OES_matrix_palette "
#endif
#ifdef GL_EXT_debug_marker
            "GL_EXT_debug_marker "
#endif
#if GL_EXT_texture_filter_anisotropic
            "GL_EXT_texture_filter_anisotropic "
#endif
#if GL_EXT_robustness
            "GL_EXT_robustness "
#endif
            ;
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
         result = get_string_renderer();
         break;
      case GL_VERSION:
         if (KHRN_GLES31_DRIVER)
            result = "OpenGL ES 3.1";
         else
            result = "OpenGL ES 3.0";
         break;
      case GL_SHADING_LANGUAGE_VERSION:
         if (KHRN_GLES31_DRIVER)
            result = "OpenGL ES GLSL ES 3.10";
         else
            result = "OpenGL ES GLSL ES 3.00";
         break;
      case GL_EXTENSIONS:
         result = khrn_get_gl30_extensions();
         break;
      default:
         glxx_server_state_set_error(state, GL_INVALID_ENUM);
      }
   }

   GLXX_UNLOCK_SERVER_STATE();
   return (const GLubyte *)result;
}

GL_API const GLubyte* GL_APIENTRY glGetStringi(GLenum name, GLuint index)
{
   GLXX_SERVER_STATE_T *state;
   const GLubyte* res = NULL;

   state = GL30_LOCK_SERVER_STATE();
   if (!state) return NULL;

   if (name != GL_EXTENSIONS) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   res = (const GLubyte *)glxx_get_gl30_extension(index);
   if (res == NULL)
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GL30_UNLOCK_SERVER_STATE();
   return res;
}

/*
   GetVertexAttribPointer

   VERTEX ATTRIB ARRAY POINTER NULL GetVertexAttribPointer
*/

GL_API void GL_APIENTRY glGetVertexAttribPointerv(GLuint index, GLenum pname, void **pointer)
{
   GLXX_SERVER_STATE_T *state = GL20_LOCK_SERVER_STATE();
   if (!state) return;

   void *result = NULL;

   if (pname != GL_VERTEX_ATTRIB_ARRAY_POINTER) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   result = glintAttribGetPointer(state, index);

   if (pointer != NULL)
      *pointer = result;

end:
   GL20_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glReleaseShaderCompiler(void)
{
}

GL_API void GL_APIENTRY glShaderBinary(GLint n, const GLuint* shaders, GLenum binaryformat, const void* binary, GLint length)
{
   UNUSED(n);
   UNUSED(shaders);
   UNUSED(binaryformat);
   UNUSED(binary);
   UNUSED(length);

   glxx_set_error_api(OPENGL_ES_NOT_11, GL_INVALID_ENUM);
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
   GLXX_SERVER_STATE_T *state = GL30_LOCK_SERVER_STATE();
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
   GL30_UNLOCK_SERVER_STATE();
}


/* GLES 2.0 attrib functions: vertex_attrib */
GL_API void GL_APIENTRY glVertexAttrib1f(GLuint indx, GLfloat x)
{
   glintAttrib(OPENGL_ES_NOT_11, indx, x, 0.0f, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib2f(GLuint indx, GLfloat x, GLfloat y)
{
   glintAttrib(OPENGL_ES_NOT_11, indx, x, y, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib3f(GLuint indx, GLfloat x, GLfloat y, GLfloat z)
{
   glintAttrib(OPENGL_ES_NOT_11, indx, x, y, z, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib4f(GLuint indx, GLfloat x, GLfloat y, GLfloat z, GLfloat w)
{
   glintAttrib(OPENGL_ES_NOT_11, indx, x, y, z, w);
}

GL_API void GL_APIENTRY glVertexAttrib1fv(GLuint indx, const GLfloat *values)
{
   glintAttrib(OPENGL_ES_NOT_11, indx, values[0], 0.0f, 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib2fv(GLuint indx, const GLfloat *values)
{
   glintAttrib(OPENGL_ES_NOT_11, indx, values[0], values[1], 0.0f, 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib3fv(GLuint indx, const GLfloat *values)
{
   glintAttrib(OPENGL_ES_NOT_11, indx, values[0], values[1], values[2], 1.0f);
}

GL_API void GL_APIENTRY glVertexAttrib4fv(GLuint indx, const GLfloat *values)
{
   glintAttrib(OPENGL_ES_NOT_11, indx, values[0], values[1], values[2], values[3]);
}


GL_API void GL_APIENTRY glVertexAttribI4i(GLuint index, GLint x, GLint y, GLint z, GLint w)
{
   glintAttribI(OPENGL_ES_NOT_11, index, x, y, z, w, GL_INT);
}

GL_API void GL_APIENTRY glVertexAttribI4iv(GLuint index, const GLint* v)
{
   glintAttribI(OPENGL_ES_NOT_11, index, v[0], v[1], v[2], v[3], GL_INT);
}

GL_API void GL_APIENTRY glVertexAttribI4ui(GLuint index, GLuint x, GLuint y, GLuint z, GLuint w)
{
   glintAttribI(OPENGL_ES_NOT_11, index, x, y, z, w, GL_UNSIGNED_INT);
}

GL_API void GL_APIENTRY glVertexAttribI4uiv(GLuint index, const GLuint* v)
{
   glintAttribI(OPENGL_ES_NOT_11, index, v[0], v[1], v[2], v[3], GL_UNSIGNED_INT);
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

//TODO we need these to get the conformance test to build
#ifdef __cplusplus
extern "C" {
#endif

GL_API void GL_APIENTRY glTexImage3DOES(GLenum target, GLint level, GLenum internalformat,
   GLsizei width, GLsizei height, GLsizei depth, GLint border, GLenum format, GLenum type, const GLvoid* pixels)
{
   UNUSED(target);
   UNUSED(level);
   UNUSED(internalformat);
   UNUSED(width);
   UNUSED(height);
   UNUSED(depth);
   UNUSED(border);
   UNUSED(format);
   UNUSED(type);
   UNUSED(pixels);
}
GL_API void GL_APIENTRY glTexSubImage3DOES(GLenum target, GLint level, GLint xoffset, GLint yoffset, GLint zoffset,
   GLsizei width, GLsizei height, GLsizei depth, GLenum format, GLenum type, const GLvoid* pixels)
{
   UNUSED(target);
   UNUSED(level);
   UNUSED(xoffset);
   UNUSED(yoffset);
   UNUSED(zoffset);
   UNUSED(width);
   UNUSED(height);
   UNUSED(depth);
   UNUSED(format);
   UNUSED(type);
   UNUSED(pixels);
}

#ifdef __cplusplus
}
#endif
