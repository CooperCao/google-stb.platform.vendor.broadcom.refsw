/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../glxx/gl_public_api.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_server_texture.h"

#if V3D_VER_AT_LEAST(4,1,34,0)

GL_APICALL void GL_APIENTRY glTexParameterIivEXT (GLenum target, GLenum pname, const GLint *params)
{
   glxx_texparamter_iv_common(target, pname, params);
}

GL_APICALL void GL_APIENTRY glTexParameterIuivEXT (GLenum target, GLenum pname, const GLuint *params)
{
   glxx_texparamter_iv_common(target, pname, (GLint *) params);
}

GL_APICALL void GL_APIENTRY glGetTexParameterIivEXT (GLenum target, GLenum pname, GLint *params)
{
   glxx_get_texparameter_iv_common(target, pname, params);
}

GL_APICALL void GL_APIENTRY glGetTexParameterIuivEXT (GLenum target, GLenum pname, GLuint *params)
{
   glxx_get_texparameter_iv_common(target, pname, (GLint *) params);
}

GL_APICALL void GL_APIENTRY glSamplerParameterIivEXT (GLuint sampler, GLenum pname, const GLint *param)
{
   glxx_sampler_parameter_iv_common(sampler, pname, param);
}

GL_APICALL void GL_APIENTRY glSamplerParameterIuivEXT (GLuint sampler, GLenum pname, const GLuint *param)
{
   glxx_sampler_parameter_iv_common(sampler, pname, (GLint *) param);
}

GL_APICALL void GL_APIENTRY glGetSamplerParameterIivEXT (GLuint sampler, GLenum pname, GLint *params)
{
   glxx_get_sampler_parameter_iv_common(sampler, pname, params);
}

GL_APICALL void GL_APIENTRY glGetSamplerParameterIuivEXT (GLuint sampler, GLenum pname, GLuint *params)
{
   glxx_get_sampler_parameter_iv_common(sampler, pname, (GLint *) params);
}

#endif
