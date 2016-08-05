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

GL_APICALL void GL_APIENTRY glMultiDrawArraysIndirectEXT(
   GLenum mode,
   const void *indirect,
   GLsizei num_indirect,
   GLsizei indirect_stride
)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .is_draw_arrays = true,
      .is_indirect = true,
      .num_indirect = num_indirect,
      .indirect_stride = indirect_stride,
      .indirect = indirect};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glMultiDrawElementsIndirectEXT(
   GLenum mode,
   GLenum index_type,
   const void *indirect,
   GLsizei num_indirect,
   GLsizei indirect_stride
)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   GLXX_DRAW_RAW_T draw = {
      GLXX_DRAW_RAW_DEFAULTS,
      .mode = mode,
      .index_type = index_type,
      .is_indirect = true,
      .num_indirect = num_indirect,
      .indirect_stride = indirect_stride,
      .indirect = indirect};
   glintDrawArraysOrElements(state, &draw);

   glxx_unlock_server_state();
}

#endif
