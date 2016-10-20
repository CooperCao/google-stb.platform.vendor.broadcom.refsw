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
   return GL_NO_ERROR;
}
