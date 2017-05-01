/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "gl_public_api.h"
#include "glxx_int_config.h"
#include "libs/util/log/log.h"
#include <stdio.h>

#if KHRN_GLES32_DRIVER || GLXX_HAS_TNG

LOG_DEFAULT_CAT("gl32_stubs")

static inline void not_implemented(const char *func)
{
   log_error("ES3.2 function '%s' not implemented yet", func);
}

#define NOT_IMPLEMENTED not_implemented(__FUNCTION__)

#endif

#if KHRN_GLES32_DRIVER
GL_APICALL void GL_APIENTRY glFramebufferTexture(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
#endif

#if GLXX_HAS_TNG
GL_APICALL void GL_APIENTRY glFramebufferTextureOES(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
GL_APICALL void GL_APIENTRY glFramebufferTextureEXT(GLenum target, GLenum attachment, GLuint texture, GLint level) { NOT_IMPLEMENTED; }
#endif
