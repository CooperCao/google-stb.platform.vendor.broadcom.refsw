/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
GL-functions which do something on both skin and fruit
=============================================================================*/
#include "../common/khrn_int_common.h"
#include "libs/util/dglenum/dglenum.h"
#include "../common/khrn_int_util.h"

#include "../glxx/glxx_server.h"
#include "../glxx/glxx_draw.h"
#include "../glxx/glxx_server_internal.h"

static bool is_vertex_size(GLint size);
static bool is_vertex_type(GLenum type);
static bool is_color_size(GLint size);
static bool is_color_type(GLenum type);
static bool is_normal_type(GLenum type);
static bool is_texture_coord_size(GLint size);
static bool is_texture_coord_type(GLenum type);
static bool is_point_size_type(GLenum type);

static int get_attr_11(GLenum array) {
   switch(array) {
   case GL_VERTEX_ARRAY:           return GL11_IX_VERTEX;
   case GL_NORMAL_ARRAY:           return GL11_IX_NORMAL;
   case GL_COLOR_ARRAY:            return GL11_IX_COLOR;
   case GL_POINT_SIZE_ARRAY_OES:   return GL11_IX_POINT_SIZE;
#if GL_OES_matrix_palette
   case GL_MATRIX_INDEX_ARRAY_OES: return GL11_IX_MATRIX_INDEX;
   case GL_WEIGHT_ARRAY_OES:       return GL11_IX_MATRIX_WEIGHT;
#endif
   case GL_TEXTURE_COORD_ARRAY:    return GL11_IX_CLIENT_ACTIVE_TEXTURE;
   default:                        return -1;
   }
}

static void enable_client_state(GLenum array, bool enabled)
{
   int attrib;
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   attrib = get_attr_11(array);
   if (attrib == -1) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (attrib == GL11_IX_CLIENT_ACTIVE_TEXTURE)
      attrib = GL11_IX_TEXTURE_COORD + state->gl11.client_active_texture - GL_TEXTURE0;

   glintAttribEnable(state, attrib, enabled);

end:
   GL11_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glDisableClientState(GLenum array)
{
   enable_client_state(array, false);
}

GL_API void GL_APIENTRY glEnableClientState(GLenum array)
{
   enable_client_state(array, true);
}

/*
   VERTEX ARRAY POINTER GetPointerv
   NORMAL ARRAY POINTER GetPointerv
   COLOR ARRAY POINTER GetPointerv
   TEXTURE COORD ARRAY POINTER GetPointerv
   POINT SIZE ARRAY POINTER OES GetPointerv
   MATRIX_INDEX_ARRAY_POINTER_OES GetPointerv
   WEIGHT_ARRAY_POINTER_OES GetPointerv
*/

static int pointer_to_attr_11(GLenum pname) {
   switch (pname)
   {
   case GL_VERTEX_ARRAY_POINTER:           return GL11_IX_VERTEX;
   case GL_NORMAL_ARRAY_POINTER:           return GL11_IX_NORMAL;
   case GL_COLOR_ARRAY_POINTER:            return GL11_IX_COLOR;
   case GL_TEXTURE_COORD_ARRAY_POINTER:    return GL11_IX_CLIENT_ACTIVE_TEXTURE;
   case GL_POINT_SIZE_ARRAY_POINTER_OES:   return GL11_IX_POINT_SIZE;
#if GL_OES_matrix_palette
   case GL_MATRIX_INDEX_ARRAY_POINTER_OES: return GL11_IX_MATRIX_INDEX;
   case GL_WEIGHT_ARRAY_POINTER_OES:       return GL11_IX_MATRIX_WEIGHT;
#endif
   default:                                return -1;
   }
}

GL_API void GL_APIENTRY glGetPointerv(GLenum pname, GLvoid **params)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   int attrib = pointer_to_attr_11(pname);
   if (attrib == -1) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (attrib == GL11_IX_CLIENT_ACTIVE_TEXTURE)
      attrib = GL11_IX_TEXTURE_COORD + state->gl11.client_active_texture - GL_TEXTURE0;

   assert(attrib < GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

   params[0] = glintAttribGetPointer(state, attrib);

end:
   GL11_UNLOCK_SERVER_STATE();
}

/* GLES 1.1 attrib functions: vertex */
static bool is_vertex_size(GLint size)
{
   return size == 2 ||
          size == 3 ||
          size == 4;
}

static bool is_vertex_type(GLenum type)
{
   return type == GL_BYTE  ||
          type == GL_SHORT ||
          type == GL_FIXED ||
          type == GL_FLOAT;
}

GL_API void GL_APIENTRY glVertexPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_vertex_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (is_vertex_size(size) && glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0)
      glintAttribPointer_GL11(state, GL11_IX_VERTEX, size, type, GL_FALSE, stride, (GLintptr)pointer);
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GL11_UNLOCK_SERVER_STATE();
}

/* GLES 1.1 attrib functions: color */
static bool is_color_size(GLint size)
{
   return size == 4;
}

static bool is_color_type(GLenum type)
{
   return type == GL_UNSIGNED_BYTE ||
          type == GL_FIXED         ||
          type == GL_FLOAT;
}

GL_API void GL_APIENTRY glColorPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_color_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (is_color_size(size) && glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0)
      glintAttribPointer_GL11(state, GL11_IX_COLOR, size, type, GL_TRUE, stride, (GLintptr)pointer);
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GL11_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glColor4f(GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha)
{
   glintColor(
      clampf(red,   0.0f, 1.0f),
      clampf(green, 0.0f, 1.0f),
      clampf(blue,  0.0f, 1.0f),
      clampf(alpha, 0.0f, 1.0f));
}

GL_API void GL_APIENTRY glColor4ub(GLubyte red, GLubyte green, GLubyte blue, GLubyte alpha)
{
   glintColor(
      (float)red   / 255.0f,
      (float)green / 255.0f,
      (float)blue  / 255.0f,
      (float)alpha / 255.0f);
}

GL_API void GL_APIENTRY glColor4x(GLfixed red, GLfixed green, GLfixed blue, GLfixed alpha)
{
   glintColor(
      clampf(fixed_to_float(red),   0.0f, 1.0f),
      clampf(fixed_to_float(green), 0.0f, 1.0f),
      clampf(fixed_to_float(blue),  0.0f, 1.0f),
      clampf(fixed_to_float(alpha), 0.0f, 1.0f));
}

/* GLES 1.1 attrib functions: normal */
static bool is_normal_type(GLenum type)
{
   return type == GL_BYTE  ||
          type == GL_SHORT ||
          type == GL_FIXED ||
          type == GL_FLOAT;
}

GL_API void GL_APIENTRY glNormalPointer(GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_normal_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0)
      glintAttribPointer_GL11(state, GL11_IX_NORMAL, 3, type, GL_TRUE, stride, (GLintptr)pointer);
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GL11_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glNormal3f(GLfloat nx, GLfloat ny, GLfloat nz)
{
   glintAttrib(OPENGL_ES_11, GL11_IX_NORMAL, nx, ny, nz, 0.0f);
}

GL_API void GL_APIENTRY glNormal3x(GLfixed nx, GLfixed ny, GLfixed nz)
{
   glintAttrib(OPENGL_ES_11, GL11_IX_NORMAL, fixed_to_float(nx), fixed_to_float(ny), fixed_to_float(nz), 0.0f);
}

/* GLES 1.1 attrib functions: texture_coord */

static bool is_texture_coord_size(GLint size)
{
   return size == 2 ||
          size == 3 ||
          size == 4;
}

static bool is_texture_coord_type(GLenum type)
{
   return type == GL_BYTE  ||
          type == GL_SHORT ||
          type == GL_FIXED ||
          type == GL_FLOAT;
}

GL_API void GL_APIENTRY glTexCoordPointer(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_texture_coord_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (is_texture_coord_size(size) && glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0) {
      unsigned index = GL11_IX_TEXTURE_COORD + state->gl11.client_active_texture - GL_TEXTURE0;
      glintAttribPointer_GL11(state, index, size, type, GL_FALSE, stride, (GLintptr)pointer);
   } else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GL11_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glMultiTexCoord4f (GLenum target, GLfloat s, GLfloat t, GLfloat r, GLfloat q)
{
   if (target >= GL_TEXTURE0 && target < GL_TEXTURE0 + GL11_CONFIG_MAX_TEXTURE_UNITS)
   {
      uint32_t indx = GL11_IX_TEXTURE_COORD + target - GL_TEXTURE0;
      glintAttrib(OPENGL_ES_11, indx, s, t, r, q);
   }
   else
      glxx_set_error_api(OPENGL_ES_11, GL_INVALID_ENUM);
}

GL_API void GL_APIENTRY glMultiTexCoord4x(GLenum target, GLfixed s, GLfixed t, GLfixed r, GLfixed q)
{
   if (target >= GL_TEXTURE0 && target < GL_TEXTURE0 + GL11_CONFIG_MAX_TEXTURE_UNITS)
   {
      uint32_t indx = GL11_IX_TEXTURE_COORD + target - GL_TEXTURE0;
      glintAttrib(OPENGL_ES_11, indx, fixed_to_float(s), fixed_to_float(t), fixed_to_float(r), fixed_to_float(q));
   }
   else
      glxx_set_error_api(OPENGL_ES_11, GL_INVALID_ENUM);
}

/* GLES 1.1 attrib functions: point_size */
static bool is_point_size_type(GLenum type)
{
   return type == GL_FIXED ||
          type == GL_FLOAT;
}

GL_API void GL_APIENTRY glPointSizePointerOES(GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_point_size_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0)
      glintAttribPointer_GL11(state, GL11_IX_POINT_SIZE, 1, type, GL_FALSE, stride, (GLintptr)pointer);
   else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GL11_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glPointSize(GLfloat size)
{
   size = clean_float(size);

   if (size > 0.0f)
      glintAttrib(OPENGL_ES_11, GL11_IX_POINT_SIZE, size, 0.0f, 0.0f, 0.0f);
   else
      glxx_set_error_api(OPENGL_ES_11, GL_INVALID_VALUE);
}

GL_API void GL_APIENTRY glPointSizex(GLfixed size)
{
   if (size > 0)
      glintAttrib(OPENGL_ES_11, GL11_IX_POINT_SIZE, fixed_to_float(size), 0.0f, 0.0f, 0.0f);
   else
      glxx_set_error_api(OPENGL_ES_11, GL_INVALID_VALUE);
}
