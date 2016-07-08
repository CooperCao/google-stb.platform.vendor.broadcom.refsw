/*=============================================================================
  Broadcom Proprietary and Confidential. (c)2013 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
=============================================================================*/
#include "../glxx/gl_public_api.h"
#include "../common/khrn_int_common.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_texture.h"
#include "../glxx/glxx_server_texture.h"

GL_APICALL void GL_APIENTRY glTexImage1DBRCM(GLenum target, GLint level, GLint internalformat, GLsizei
      width, GLint border, GLenum format, GLenum type, const GLvoid *pixels)
{
   texImageX(target, level, internalformat, width, 1, 1, border, format, type, pixels, 1);
}
