/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../common/khrn_int_common.h"
#include "../glxx/glxx_server.h"

#include "../glxx/gl_public_api.h"
#include "../gl11/gl11_int_config.h"

static GLboolean is_matrix_index_type(GLenum type) {
   return (type == GL_UNSIGNED_BYTE);
}

static GLboolean is_matrix_palette_size(GLint size) {
   /* TODO: Should size 0 be allowed or not? */
   return size > 0 && size <= GL11_CONFIG_MAX_VERTEX_UNITS_OES;
}

GL_API void GL_APIENTRY glMatrixIndexPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
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
   glxx_unlock_server_state();
}

static GLboolean is_matrix_weight_type(GLenum type) {
   return type == GL_FIXED ||
          type == GL_FLOAT;
}

GL_API void GL_APIENTRY glWeightPointerOES(GLint size, GLenum type, GLsizei stride, const GLvoid *pointer)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);
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
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glCurrentPaletteMatrixOES(GLuint index) {
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);

   /* The index has already been verified by the client */
   if (index >= GL11_CONFIG_MAX_PALETTE_MATRICES_OES) {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   state->gl11.current_palette_matrix = index;

end:
   glxx_unlock_server_state();
}

GL_API void GL_APIENTRY glLoadPaletteFromModelViewMatrixOES() {
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_11);

   assert(state->gl11.current_palette_matrix < GL11_CONFIG_MAX_PALETTE_MATRICES_OES);
   gl11_matrix_load(state->gl11.palette_matrices[state->gl11.current_palette_matrix], state->gl11.current_modelview);

   glxx_unlock_server_state();
}
