/*=============================================================================
  Copyright (c) 2015 Broadcom Europe Limited.
  All rights reserved.

Project  :  glxx
Module   :  Header file

FILE DESCRIPTION
Implement glTexLevelParam
=============================================================================*/
#include "interface/khronos/glxx/gl_public_api.h"
#include "helpers/gfx/gfx_lfmt.h"
#include "middleware/khronos/common/khrn_image.h"
#include "interface/khronos/common/khrn_int_util.h"
#include "middleware/khronos/glxx/glxx_server.h"

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
      return true;
   default:
      return false;
   }
}

typedef uint32_t (*comp_size_fct_t)(GFX_LFMT_T lfmt);
static int component_size(GLenum pname, const KHRN_IMAGE_T *img)
{
   if (gfx_lfmt_is_compressed(img->api_fmt))
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
   return fct(img->api_fmt);
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
static GLenum component_type(GLenum pname, const KHRN_IMAGE_T *img)
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

   GFX_LFMT_TYPE_T type = fct(img->api_fmt);
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

   GLXX_TEXTURE_T* texture = glxx_server_state_get_texture(state, target, true);
   if (!texture)
   {
      error = GL_INVALID_ENUM;
      goto end;
   }
   if (level < 0 || !glxx_texture_is_legal_level(texture->target, level))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   unsigned face =  glxx_texture_get_face(target);
   const KHRN_IMAGE_T *img = texture->img[face][level];

   unsigned scale = glxx_ms_mode_get_scale(texture->ms_mode);

   switch(pname)
   {
   case GL_TEXTURE_WIDTH:
      params[0] = img ? khrn_image_get_width(img) / scale : 0;
      result = 1;
      break;
   case GL_TEXTURE_HEIGHT:
      params[0] = img ? khrn_image_get_height(img) / scale : 0;
      result = 1;
      break;
   case GL_TEXTURE_DEPTH:
      if (img)
      {
         if (texture->target == GL_TEXTURE_2D_ARRAY)
            params[0] = khrn_image_get_num_elems(img);
         else
            params[0] = khrn_image_get_depth(img);
      }
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
      if (img)
      {
         params[0] = gfx_internalformat_from_api_fmt_maybe(img->api_fmt);
         if (params[0] == GL_NONE)
         {
            params[0] = gfx_compressed_internalformat_from_api_fmt_maybe(img->api_fmt);
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
      params[0] = img ? component_size(pname, img) : 0;
      result = 1;
      break;
   case GL_TEXTURE_SHARED_SIZE:
      params[0] = 0;
      if (img && (gfx_lfmt_get_base(&img->api_fmt) == GFX_LFMT_BASE_C9C9C9SHAREDEXP5))
         params[0] = 5;
      result = 1;
      break;
   case GL_TEXTURE_RED_TYPE:
   case GL_TEXTURE_GREEN_TYPE:
   case GL_TEXTURE_BLUE_TYPE:
   case GL_TEXTURE_ALPHA_TYPE:
   case GL_TEXTURE_DEPTH_TYPE:
      params[0] = img ? component_type(pname, img) : 0;
      result = 1;
      break;
   case GL_TEXTURE_COMPRESSED:
      params[0] = img ? gfx_lfmt_is_compressed(img->api_fmt) : 0;
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
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
   if (!state)  return;

   if (is_int_texlevel_param(pname))
      get_texlevel_param(state, target, level, pname, params);
   else
      glxx_server_state_set_error(state, GL_INVALID_ENUM);

   GL31_UNLOCK_SERVER_STATE();
}

GL_APICALL void GL_APIENTRY glGetTexLevelParameterfv (GLenum target, GLint level,
      GLenum pname, GLfloat *params)
{
   if (!params)   return;
   GLXX_SERVER_STATE_T *state = GL31_LOCK_SERVER_STATE();
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

   GL31_UNLOCK_SERVER_STATE();
}
