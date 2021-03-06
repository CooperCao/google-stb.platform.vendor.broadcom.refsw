/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of the GL_OES_matrix_palette extension for GLES 1.1
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_server.h"

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/glxx/gl11_int_config.h"

#if GL_OES_matrix_palette

static GLboolean is_matrix_index_type(GLenum type) {
   return (type == GL_UNSIGNED_BYTE);
}

static GLboolean is_matrix_palette_size(GLint size) {
   /* TODO: Should size 0 be allowed or not? */
   return size > 0 && size <= GL11_CONFIG_MAX_VERTEX_UNITS_OES;
}

GL_API void GL_APIENTRY glMatrixIndexPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_matrix_index_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (is_matrix_palette_size(size) && glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0) {
      glintAttribPointer_GL11(state, GL11_IX_MATRIX_INDEX, size, type, GL_FALSE, stride, (GLintptr)pointer);
   } else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GL11_UNLOCK_SERVER_STATE();
}

static GLboolean is_matrix_weight_type(GLenum type) {
   return type == GL_FIXED ||
          type == GL_FLOAT;
}

GL_API void GL_APIENTRY glWeightPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();
   if (!state) return;

   if (!is_matrix_weight_type(type)) {
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
      goto end;
   }

   if (is_matrix_palette_size(size) && glxx_is_aligned(type, (size_t)pointer) && glxx_is_aligned(type, (size_t)stride) && stride >= 0) {
      glintAttribPointer_GL11(state, GL11_IX_MATRIX_WEIGHT, size, type, GL_FALSE, stride, (GLintptr)pointer);
   } else
      glxx_server_state_set_error(state, GL_INVALID_VALUE);

end:
   GL11_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glCurrentPaletteMatrixOES(GLuint index) {
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();

   /* The index has already been verified by the client */
   if (index >= GL11_CONFIG_MAX_PALETTE_MATRICES_OES) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   state->gl11.current_palette_matrix = index;

end:
   GL11_UNLOCK_SERVER_STATE();
}

GL_API void GL_APIENTRY glLoadPaletteFromModelViewMatrixOES() {
   GLXX_SERVER_STATE_T *state = GL11_LOCK_SERVER_STATE();

   assert(state->gl11.current_palette_matrix < GL11_CONFIG_MAX_PALETTE_MATRICES_OES);
   gl11_matrix_load(state->gl11.palette_matrices[state->gl11.current_palette_matrix], state->gl11.current_modelview);

   GL11_UNLOCK_SERVER_STATE();
}

#endif /* GL_OES_matrix_palette */
