/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

/* The 10 supported TexImage* calls have a lot in common, so they use the same
 * sanity-checking routine.
 * The exact list of checks required depends on the call. This structure
 * describes the supported checks.
 */
struct glxx_teximage_sanity_checks {
   int dimensions;
   bool compressed;
   bool respecify;
   // Additional checks that happen everytime:
   //   palette_compressed level flip: this always just happens
   //   check level valid: always just happens
   //   hwd_limits; always just happens
   //   border is 0: always just happens
   //   bool yuv: always just happens
};

extern bool glxx_teximage_internal_checks(GLXX_SERVER_STATE_T *state,
      const struct glxx_teximage_sanity_checks *checks,
      GLenum target, GLenum format, GLenum type, GLenum internalformat,
      GLsizei level, GLsizei width, GLsizei height, GLsizei depth,
      GLint xoffset, GLint yoffset, GLint zoffset, GLint border,
      GLXX_TEXTURE_T **tex_out, GLenum *error);

extern void glxx_texparamter_iv_common(GLenum target, GLenum pname, const GLint *params);
extern void glxx_get_texparameter_iv_common(GLenum target, GLenum pname, GLint *params);
extern void glxx_sampler_parameter_iv_common(GLuint sampler, GLenum pname, const GLint *param);
extern void glxx_get_sampler_parameter_iv_common(GLuint sampler, GLenum pname, GLint *params);

extern void glxx_texparameterf_sampler_internal(GLXX_SERVER_STATE_T *state,
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler, GLenum pname, const GLfloat *f);

extern void glxx_texparameter_sampler_internal(GLXX_SERVER_STATE_T *state,
      GLenum target, GLXX_TEXTURE_SAMPLER_STATE_T *sampler, GLenum pname, const
      GLint *i);
uint32_t glxx_get_texparameter_sampler_internal(GLXX_SERVER_STATE_T *state,
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler, GLenum pname, GLint *params);
uint32_t glxx_get_texparameterf_sampler_internal(GLXX_SERVER_STATE_T *state,
      GLXX_TEXTURE_SAMPLER_STATE_T *sampler, GLenum pname, GLfloat *params);
