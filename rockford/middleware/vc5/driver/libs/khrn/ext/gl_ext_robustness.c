/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
=============================================================================*/
#include "../glxx/glxx_server.h"

GL_APICALL GLenum GL_APIENTRY glGetGraphicsResetStatusEXT(void)
{
   /* v3d doesnt loose context on GPU reset.  return GL_NO_ERROR always.*/

   GLXX_SERVER_STATE_T *state = glxx_lock_server_state_unchanged(OPENGL_ES_ANY);
   GLenum result = GL_NO_ERROR;
   if (!state) return GL_NO_ERROR;

   glxx_unlock_server_state();
   return result;
}
