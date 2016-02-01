/*=============================================================================
Copyright (c) 2012 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "interface/khronos/tools/dglenum/dglenum.h"

#if GL_BRCM_polygon_mode

#define VCOS_LOG_CATEGORY (&log_cat)
static VCOS_LOG_CAT_T log_cat = VCOS_LOG_INIT("glxx_brcm_polygon_mode", VCOS_LOG_WARN);

GL_APICALL void GL_APIENTRY glPolygonModeBRCM(GLenum mode)
{
   GLXX_SERVER_STATE_T *state   = GLXX_LOCK_SERVER_STATE();
   GLenum               error   = GL_NO_ERROR;

   vcos_log_trace("glPolygonModeBRCM");

   if (!state)
      return;

   if(!(mode == GL_FILL_BRCM || mode == GL_LINE_BRCM || mode == GL_POINT_BRCM))
      error = GL_INVALID_VALUE;
   else
   {
      state->fill_mode = mode;
      state->dirty.cfg = KHRN_RENDER_STATE_SET_ALL;
      state->dirty.stuff = KHRN_RENDER_STATE_SET_ALL;
   }

   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   GLXX_UNLOCK_SERVER_STATE();
}

#endif
