/*=============================================================================
 * Broadcom Proprietary and Confidential. (c)2015 Broadcom.
 * All rights reserved.
 *
 * Project  :  khronos
 * Module   :  glxx
 *
 * FILE DESCRIPTION
 * Stub functions for ES3.2
 * =============================================================================*/

#include "gl_public_api.h"
#include "glxx_int_config.h"
#include "libs/util/log/log.h"
#include "vcos.h"
#include <stdio.h>

#if KHRN_GLES32_DRIVER || GLXX_HAS_TNG

LOG_DEFAULT_CAT("gl32_stubs")

static inline void not_implemented(const char *func)
{
   log_error("ES3.2 function '%s' not implemented yet", func);
}

#define NOT_IMPLEMENTED not_implemented(VCOS_FUNCTION)

#endif

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
#endif

#if GLXX_HAS_TNG
GL_APICALL void GL_APIENTRY glFramebufferTextureOES(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glFramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
#endif
