/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
=============================================================================*/
#include "middleware/khronos/glxx/glxx_server.h"

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