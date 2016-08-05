/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION

=============================================================================*/
#include "../glxx/gl_public_api.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_server_texture.h"

#if V3D_HAS_NEW_TMU_CFG
GL_APICALL void GL_APIENTRY glTexParameterIivOES (GLenum target, GLenum pname, const GLint *params)
{
   glxx_texparamter_iv_common(target, pname, params);
}

GL_APICALL void GL_APIENTRY glTexParameterIuivOES (GLenum target, GLenum pname, const GLuint *params)
{
   glxx_texparamter_iv_common(target, pname, (GLint *) params);
}

GL_APICALL void GL_APIENTRY glGetTexParameterIivOES (GLenum target, GLenum pname, GLint *params)
{
   glxx_get_texparameter_iv_common(target, pname, params);
}

GL_APICALL void GL_APIENTRY glGetTexParameterIuivOES (GLenum target, GLenum pname, GLuint *params)
{
   glxx_get_texparameter_iv_common(target, pname, (GLint *) params);
}

GL_APICALL void GL_APIENTRY glSamplerParameterIivOES (GLuint sampler, GLenum pname, const GLint *param)
{
   glxx_sampler_parameter_iv_common(sampler, pname, param);
}

GL_APICALL void GL_APIENTRY glSamplerParameterIuivOES (GLuint sampler, GLenum pname, const GLuint *param)
{
   glxx_sampler_parameter_iv_common(sampler, pname, (GLint *) param);
}

GL_APICALL void GL_APIENTRY glGetSamplerParameterIivOES (GLuint sampler, GLenum pname, GLint *params)
{
   glxx_get_sampler_parameter_iv_common(sampler, pname, params);
}

GL_APICALL void GL_APIENTRY glGetSamplerParameterIuivOES (GLuint sampler, GLenum pname, GLuint *params)
{
   glxx_get_sampler_parameter_iv_common(sampler, pname, (GLint *) params);
}
#endif
