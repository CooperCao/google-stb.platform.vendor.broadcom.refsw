/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.
=============================================================================*/

#include "../glxx/gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "../glxx/glxx_server.h"
#include "libs/util/dglenum/dglenum.h"
#include "../glxx/glxx_draw.h"

#if GL_BRCM_base_instance

GL_APICALL void GL_APIENTRY glDrawArraysInstancedBaseInstanceBRCM(
   GLenum mode,
   GLint first,
   GLsizei count,
   GLsizei instanceCount,
   GLuint baseinstance
)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
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

   GL30_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseInstanceBRCM(
   GLenum mode,
   GLsizei count,
   GLenum index_type,
   const void *indices,
   GLsizei instanceCount,
   GLuint baseinstance
)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
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

   GL31_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glDrawElementsInstancedBaseVertexBaseInstanceBRCM(
   GLenum mode,
   GLsizei count,
   GLenum index_type,
   const void *indices,
   GLsizei instanceCount,
   GLint basevertex,
   GLuint baseinstance
)
{
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
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

   GL30_UNLOCK_SERVER_STATE();
}

#endif
