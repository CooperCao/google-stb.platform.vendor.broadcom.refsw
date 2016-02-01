/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/ext/gl_brcm_provoking_vertex.h"

#if GL_BRCM_provoking_vertex

bool gl_brcm_provoking_vertex_supported(void)
{
   return khrn_get_v3d_version() >= V3D_MAKE_VER(3,3);
}

GL_APICALL void GL_APIENTRY glProvokingVertexBRCM(GLenum mode)
{
   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE();
   if (!state)
      return;

   /* Otherwise we shouldn't have reported the extension */
   assert(gl_brcm_provoking_vertex_supported());

   if ((mode != GL_FIRST_VERTEX_CONVENTION_BRCM) &&
      (mode != GL_LAST_VERTEX_CONVENTION_BRCM))
      glxx_server_state_set_error(state, GL_INVALID_ENUM);
   else
   {
      state->provoking_vtx = mode;
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
   }

   GLXX_UNLOCK_SERVER_STATE();
}

#endif
