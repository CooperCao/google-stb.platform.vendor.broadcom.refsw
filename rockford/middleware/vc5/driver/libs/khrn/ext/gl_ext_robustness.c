/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
=============================================================================*/
#include "../glxx/glxx_server.h"

#if GL_EXT_robustness
GL_APICALL GLenum GL_APIENTRY glGetGraphicsResetStatusEXT(void)
{
   /* v3d doesnt loose context on GPU reset.  return GL_NO_ERROR always.*/

   GLXX_SERVER_STATE_T *state = GLXX_LOCK_SERVER_STATE_UNCHANGED();
   GLenum result = GL_NO_ERROR;
   if (!state) return GL_NO_ERROR;

   GLXX_UNLOCK_SERVER_STATE();
   return result;
}
#endif