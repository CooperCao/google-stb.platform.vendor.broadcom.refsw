/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "libs/core/v3d/v3d_ver.h"

#if V3D_VER_AT_LEAST(3,3,0,0)

#include "gl_public_api.h"
#include "libs/core/lfmt/lfmt.h"
#include "../common/khrn_image.h"
#include "../common/khrn_int_util.h"
#include "glxx_server.h"

static bool is_int_texlevel_param(GLenum pname)
{
   switch(pname)
   {
   case GL_TEXTURE_WIDTH:
   case GL_TEXTURE_HEIGHT:
   case GL_TEXTURE_DEPTH:
   case GL_TEXTURE_SAMPLES:
   case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
   case GL_TEXTURE_INTERNAL_FORMAT:
   case GL_TEXTURE_RED_SIZE:
   case GL_TEXTURE_GREEN_SIZE:
   case GL_TEXTURE_BLUE_SIZE:
   case GL_TEXTURE_ALPHA_SIZE:
   case GL_TEXTURE_DEPTH_SIZE:
   case GL_TEXTURE_STENCIL_SIZE:
   case GL_TEXTURE_SHARED_SIZE:
   case GL_TEXTURE_RED_TYPE:
   case GL_TEXTURE_GREEN_TYPE:
   case GL_TEXTURE_BLUE_TYPE:
   case GL_TEXTURE_ALPHA_TYPE:
   case GL_TEXTURE_DEPTH_TYPE:
   case GL_TEXTURE_COMPRESSED:
   case GL_TEXTURE_BUFFER_DATA_STORE_BINDING:
   case GL_TEXTURE_BUFFER_OFFSET:
   case GL_TEXTURE_BUFFER_SIZE:
      return true;
   default:
      return false;
   }
}

typedef uint32_t (*comp_size_fct_t)(GFX_LFMT_T lfmt);
static int component_size(GLenum pname, GFX_LFMT_T api_fmt)
{
   if (gfx_lfmt_is_compressed(api_fmt))
   {
      /* not implemented yet */
      /* we need to report the component resolution of a uncompressed image
       * format that produces an image roughly the same quality as the
       * compressed image in question */
      assert(0);
   }

   comp_size_fct_t fct = NULL;
   switch(pname)
   {
   case GL_TEXTURE_RED_SIZE:
      fct = gfx_lfmt_red_bits;
      break;
   case GL_TEXTURE_GREEN_SIZE:
      fct = gfx_lfmt_green_bits;
      break;
   case GL_TEXTURE_BLUE_SIZE:
      fct = gfx_lfmt_blue_bits;
      break;
   case GL_TEXTURE_ALPHA_SIZE:
      fct = gfx_lfmt_alpha_bits;
      break;
   case GL_TEXTURE_DEPTH_SIZE:
      fct = gfx_lfmt_depth_bits;
      break;
   case GL_TEXTURE_STENCIL_SIZE:
      fct = gfx_lfmt_stencil_bits;
      break;
   default:
      unreachable();
      break;
   }
   return fct(api_fmt);
}

static GLenum lfmt_type_to_gl_type(GFX_LFMT_TYPE_T lfmt_type)
{
   switch(lfmt_type)
   {
   case GFX_LFMT_TYPE_UFLOAT:
   case GFX_LFMT_TYPE_FLOAT:
      return GL_FLOAT;
   case GFX_LFMT_TYPE_UINT:
      return GL_UNSIGNED_INT;
   case GFX_LFMT_TYPE_INT:
      return GL_INT;
   case GFX_LFMT_TYPE_UNORM:
   case GFX_LFMT_TYPE_SRGB:
      return GL_UNSIGNED_NORMALIZED;
   case GFX_LFMT_TYPE_SNORM:
      return GL_SIGNED_NORMALIZED;
   default:
      return GL_NONE;
   }
}

typedef GFX_LFMT_TYPE_T (*comp_type_fct_t)(GFX_LFMT_T lfmt);
static GLenum component_type(GLenum pname, GFX_LFMT_T api_fmt)
{
   comp_type_fct_t fct;
   switch(pname)
   {
   case GL_TEXTURE_RED_TYPE:
      fct = gfx_lfmt_red_type;
      break;
   case GL_TEXTURE_GREEN_TYPE:
      fct = gfx_lfmt_green_type;
      break;
   case GL_TEXTURE_BLUE_TYPE:
      fct = gfx_lfmt_blue_type;
      break;
   case GL_TEXTURE_ALPHA_TYPE:
      fct = gfx_lfmt_alpha_type;
      break;
   case GL_TEXTURE_DEPTH_TYPE:
      fct = gfx_lfmt_depth_type;
      break;
   default:
      unreachable();
      break;
   }

   GFX_LFMT_TYPE_T type = fct(api_fmt);
   return lfmt_type_to_gl_type(type);
}

static unsigned get_texlevel_param(GLXX_SERVER_STATE_T *state,
      GLenum target, GLint level, GLenum pname, GLint *params)
{
   int result = 0;
   GLenum error = GL_NO_ERROR;

   if (target == GL_TEXTURE_CUBE_MAP)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }

   GLenum tex_target = target;
   if (glxx_texture_is_cube_face(target))
      tex_target = GL_TEXTURE_CUBE_MAP;
   GLXX_TEXTURE_T* texture = glxx_server_state_get_texture(state, tex_target);
   if (!texture)
      goto end;

   if (level < 0 || !glxx_texture_is_legal_level(texture->target, level))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   unsigned face =  glxx_texture_get_face(target);
   const khrn_image *img = NULL;
   const struct tex_buffer_s *tex_buffer = NULL;

   /* texture buffer is treated differently  because we create an image only at
    * draw time */
   if (texture->target == GL_TEXTURE_BUFFER)
      tex_buffer = &texture->tex_buffer;
   else
      img = texture->img[face][level];

   unsigned scale = glxx_ms_mode_get_scale(texture->ms_mode);
   GFX_LFMT_T api_fmt = (tex_buffer && tex_buffer->buffer) ? tex_buffer->api_fmt :
                           (img ? img->api_fmt : GL_NONE);

   switch(pname)
   {
   case GL_TEXTURE_WIDTH:
#if V3D_VER_AT_LEAST(4,1,34,0)
      /* None of the tex_buffer things in this function are needed if
       * !has_large_1d_texture but this is the only thing that actually breaks */
      if (tex_buffer)
         params[0] = glxx_texture_buffer_get_width(texture);
      else
#endif
         params[0] = img ? khrn_image_get_width(img) / scale : 0;
      result = 1;
      break;
   case GL_TEXTURE_HEIGHT:
      if (tex_buffer)
         params[0] = tex_buffer->buffer ? 1 : 0;
      else
         params[0] = img ? khrn_image_get_height(img) / scale : 0;
      result = 1;
      break;
   case GL_TEXTURE_DEPTH:
      if (tex_buffer)
         params[0] = tex_buffer->buffer ? 1 : 0;
      else  if (img)
         params[0] = khrn_image_get_depth(img) * khrn_image_get_num_elems(img);
      else
         params[0] = 0;
      result = 1;
      break;
   case GL_TEXTURE_SAMPLES:
      params[0] = (int)texture->ms_mode;
      result = 1;
      break;
   case GL_TEXTURE_FIXED_SAMPLE_LOCATIONS:
      params[0] = texture->fixed_sample_locations;
      result = 1;
      break;
   case GL_TEXTURE_INTERNAL_FORMAT:
      if (api_fmt != GL_NONE)
      {
         params[0] = gfx_internalformat_from_api_fmt_maybe(api_fmt);
         if (params[0] == GL_NONE)
         {
            params[0] = gfx_compressed_internalformat_from_api_fmt_maybe(api_fmt);
            assert(params[0] != GL_NONE);
         }
      }
      else
         params[0] = GL_RGBA;
      result = 1;
      break;
   case GL_TEXTURE_RED_SIZE:
   case GL_TEXTURE_GREEN_SIZE:
   case GL_TEXTURE_BLUE_SIZE:
   case GL_TEXTURE_ALPHA_SIZE:
   case GL_TEXTURE_DEPTH_SIZE:
   case GL_TEXTURE_STENCIL_SIZE:
      params[0] = api_fmt != GL_NONE ? component_size(pname, api_fmt) : 0;
      result = 1;
      break;
   case GL_TEXTURE_SHARED_SIZE:
      params[0] = 0;
      if (api_fmt != GL_NONE && (gfx_lfmt_get_base(&api_fmt) == GFX_LFMT_BASE_C9C9C9SHAREDEXP5))
         params[0] = 5;
      result = 1;
      break;
   case GL_TEXTURE_RED_TYPE:
   case GL_TEXTURE_GREEN_TYPE:
   case GL_TEXTURE_BLUE_TYPE:
   case GL_TEXTURE_ALPHA_TYPE:
   case GL_TEXTURE_DEPTH_TYPE:
      params[0] = api_fmt != GL_NONE ? component_type(pname, api_fmt) : 0;
      result = 1;
      break;
   case GL_TEXTURE_COMPRESSED:
      params[0] = api_fmt != GL_NONE ? gfx_lfmt_is_compressed(api_fmt) : 0;
      result = 1;
      break;
   case GL_TEXTURE_BUFFER_DATA_STORE_BINDING:
      params[0] = (tex_buffer && tex_buffer->buffer) ? tex_buffer->buffer->name : 0;
      result = 1;
      break;
   case GL_TEXTURE_BUFFER_OFFSET:
      params[0] = tex_buffer ? tex_buffer->offset : 0;
      result = 1;
      break;
   case GL_TEXTURE_BUFFER_SIZE:
      params[0] = 0;
      if(tex_buffer && tex_buffer->buffer)
      {
         params[0] = tex_buffer->size != SIZE_MAX ? tex_buffer->size :
            glxx_buffer_get_size(tex_buffer->buffer);
      }
      result = 1;
      break;
   default:
      unreachable();
      break;
   }

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);
   return result;
}

GL_APICALL void GL_APIENTRY glGetTexLevelParameteriv (GLenum target, GLint level,
      GLenum pname, GLint *params)
{
   if (!params)   return;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)  return;

   if (is_int_texlevel_param(pname))
      get_texlevel_param(state, target, level, pname, params);
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glGetTexLevelParameterfv (GLenum target, GLint level,
      GLenum pname, GLfloat *params)
{
   if (!params)   return;
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)  return;

   if (is_int_texlevel_param(pname))
   {
      GLint iparams[4];
      unsigned count = get_texlevel_param(state, target, level, pname, iparams);
      for (unsigned i = 0; i < count; i++)
         params[i] = (GLfloat) iparams[i];
   }
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   glxx_unlock_server_state();
}

#endif
