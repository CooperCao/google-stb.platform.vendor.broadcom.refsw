/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.
=============================================================================*/

#if KHRN_GLES31_DRIVER

#include "../glxx/gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "../glxx/glxx_server.h"
#include "libs/util/dglenum/dglenum.h"
#include "../glxx/glxx_draw.h"

GL_APICALL void GL_APIENTRY glDrawArraysInstancedBaseInstanceEXT(
   GLenum mode,
   GLint first,
   GLsizei count,
   GLsizei instanceCount,
   GLuint baseinstance
)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .count = count,
      .is_draw_arrays = true,
      .first = first,
      .instance_count = instanceCount,
      .baseinstance = baseinstance};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseInstanceEXT(
   GLenum mode,
   GLsizei count,
   GLenum index_type,
   const void *indices,
   GLsizei instanceCount,
   GLuint baseinstance
)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .count = count,
      .index_type = index_type,
      .indices = indices,
      .instance_count = instanceCount};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertexBaseInstanceEXT(
   GLenum mode,
   GLsizei count,
   GLenum index_type,
   const void *indices,
   GLsizei instanceCount,
   GLint basevertex,
   GLuint baseinstance
)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .count = count,
      .index_type = index_type,
      .indices = indices,
      .instance_count = instanceCount,
      .basevertex = basevertex,
      .baseinstance = baseinstance};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

#endif
