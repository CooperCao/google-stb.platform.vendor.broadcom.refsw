/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#include "../glxx/gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "../glxx/glxx_server.h"

GL_APICALL void GL_APIENTRY glProvokingVertexBRCM(GLenum mode)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_ANY);
   if (!state)
      return;

   /* Otherwise we shouldn't have reported the extension */
   assert(V3D_VER_AT_LEAST(3,3,0,0));

   if ((mode != GL_FIRST_VERTEX_CONVENTION_BRCM) &&
      (mode != GL_LAST_VERTEX_CONVENTION_BRCM))
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else
   {
      state->provoking_vtx = mode;
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
   }

   glxx_unlock_server_state();
}
