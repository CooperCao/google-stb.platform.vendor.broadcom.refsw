/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_image_unit.h"
#include "glxx_texture.h"
#include "glxx_server.h"

static void reset_to_default(glxx_image_unit *image_unit)
{
   KHRN_MEM_ASSIGN(image_unit->texture, NULL);
   image_unit->level = 0;
   image_unit->layered = false;
   image_unit->layer = 0;
   image_unit->access = GL_READ_ONLY;
   image_unit->internalformat = GL_R32UI;
}

void glxx_image_unit_init_default(glxx_image_unit *image_unit)
{
   reset_to_default(image_unit);
}

void glxx_image_unit_deinit(glxx_image_unit *image_unit)
{
   reset_to_default(image_unit);
}

static bool is_allowed_image_unit_format(GLenum internalformat)
{
   switch(internalformat)
   {
   case GL_RGBA32F:
   case GL_RGBA16F:
   case GL_R32F:
   case GL_RGBA32UI:
   case GL_RGBA16UI:
   case GL_RGBA8UI:
   case GL_R32UI:
   case GL_RGBA32I:
   case GL_RGBA16I:
   case GL_RGBA8I:
   case GL_R32I:
   case GL_RGBA8:
   case GL_RGBA8_SNORM:

#if V3D_VER_AT_LEAST(4,2,13,0)
   case GL_RG32F:
   case GL_RG16F:
   case GL_R11F_G11F_B10F:
   case GL_R16F:
   case GL_RGBA16_EXT:
   case GL_RGB10_A2:
   case GL_RG16_EXT:
   case GL_RG8:
   case GL_R16_EXT:
   case GL_R8:
   case GL_RGBA16_SNORM_EXT:
   case GL_RG16_SNORM_EXT:
   case GL_RG8_SNORM:
   case GL_R16_SNORM_EXT:
   case GL_R8_SNORM:
   case GL_RG32I:
   case GL_RG16I:
   case GL_RG8I:
   case GL_R16I:
   case GL_R8I:
   case GL_RGB10_A2UI:
   case GL_RG32UI:
   case GL_RG16UI:
   case GL_RG8UI:
   case GL_R16UI:
   case GL_R8UI:
#endif
      return true;
   default:
      return false;
   }
}

static bool formats_compatible(glxx_image_unit_fmt unit_internalformat, GFX_LFMT_T tex_api_fmt)
{
   GLenum tex_internalformat = gfx_sized_internalformat_from_api_fmt_maybe(tex_api_fmt);

   if (tex_internalformat == GL_NONE)
      return false;
   if (!is_allowed_image_unit_format(tex_internalformat))
      return false;
   if (unit_internalformat == tex_internalformat)
      return true;

   /* see if the formats match in size */
   GFX_LFMT_T unit_fmt = gfx_api_fmt_from_sized_internalformat(unit_internalformat);

   unsigned unit_fmt_size = gfx_lfmt_bytes_per_block(unit_fmt) * 8;
   unsigned tex_fmt_size = gfx_lfmt_bytes_per_block(tex_api_fmt) * 8;

   if (unit_fmt_size == tex_fmt_size)
      return true;

   return false;
}

glxx_unit_access glxx_get_calc_image_unit(const glxx_image_unit *image_unit,
      const GLSL_IMAGE_T *info, glxx_calc_image_unit *calc_image_unit)
{
   glxx_unit_access acc = GLXX_ACC_INVALID;

   if (image_unit->texture == NULL)
      goto end;

   unsigned base_level, num_levels;
   GLXX_TEXTURE_T *texture = image_unit->texture;

   /* there is no imageStore/load for multisample textures */
   if (glxx_tex_target_is_multisample(texture->target))
      goto end;

   if (!glxx_texture_check_completeness(texture, false, &base_level,
            &num_levels))
      goto end;
   if (image_unit->level < base_level ||
         image_unit->level >= (base_level + num_levels))
      goto end;

   calc_image_unit->level = image_unit->level;
   calc_image_unit->use_face_layer = false;
   if (glxx_tex_target_has_layers_or_faces(texture->target) && !image_unit->layered)
   {
      switch (texture->target)
      {
      case GL_TEXTURE_CUBE_MAP:
         if (image_unit->layer >= 6)
            goto end;
         calc_image_unit->face = image_unit->layer;
         calc_image_unit->layer = 0;
         break;
      case GL_TEXTURE_CUBE_MAP_ARRAY:
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_3D:
         if (!glxx_texture_is_legal_layer(texture->target, image_unit->layer))
            goto end;
         calc_image_unit->face = 0;
         calc_image_unit->layer = image_unit->layer;
         break;
      default:
         unreachable();
         goto end;
      }
      calc_image_unit->use_face_layer = true;
   }

   if (!formats_compatible(image_unit->internalformat,
            texture->target == GL_TEXTURE_BUFFER ? texture->tex_buffer.api_fmt : texture->immutable_format))
      goto end;

    acc = GLXX_ACC_UNDEFINED;

   /* check if the fmtlayout qualifier used for the image variable in the shader matches
    * the one specified when for the image unit
    */
   assert(is_allowed_image_unit_format(info->internalformat));
   if (info->internalformat != image_unit->internalformat)
      goto end;

   enum glxx_tex_target target = calc_image_unit->use_face_layer ? GL_TEXTURE_2D : texture->target;
   if (info->sampler.texture_type != target)
      goto end;

   GFX_LFMT_T api_fmt = gfx_api_fmt_from_sized_internalformat(image_unit->internalformat);
   unsigned       num_planes;
   GFX_LFMT_T     fmts[GFX_BUFFER_MAX_PLANES];
   glxx_hw_fmts_from_api_fmt(&num_planes, fmts, api_fmt);
   assert(num_planes == 1);
   assert(api_fmt == fmts[0]);
   calc_image_unit->fmt = fmts[0];

   assert(gfx_lfmt_num_slots_from_type(calc_image_unit->fmt) == 1);
   GFX_LFMT_TYPE_T unit_lfmt_type;
   unit_lfmt_type = calc_image_unit->fmt & GFX_LFMT_TYPE_MASK;
   if (unit_lfmt_type == GFX_LFMT_TYPE_UNORM ||
       unit_lfmt_type == GFX_LFMT_TYPE_SNORM ||
       unit_lfmt_type == GFX_LFMT_TYPE_UFLOAT )
      unit_lfmt_type = GFX_LFMT_TYPE_FLOAT;
   if (info->lfmt_type != unit_lfmt_type)
      goto end;

    /* TODO: get info from the shader if this image is used in an imageStore */
   calc_image_unit->write = (image_unit->access != GL_READ_ONLY);
   acc = GLXX_ACC_OK;
end:
   return acc;
}

#if V3D_VER_AT_LEAST(3,3,0,0)

static bool is_allowed_access(GLenum access)
{
   switch(access)
   {
   case GL_READ_ONLY:
   case GL_WRITE_ONLY:
   case GL_READ_WRITE:
      return true;
   default:
      return false;
   }
}

GL_APICALL void GL_APIENTRY glBindImageTexture (GLuint unit, GLuint tex_id,
      GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format)
{
   GLXX_SERVER_STATE_T *state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   GLenum error = GL_NO_ERROR;
   if (unit >= GLXX_CONFIG_MAX_IMAGE_UNITS)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   glxx_image_unit *image_unit = &state->image_unit[unit];

   if (tex_id == 0)
   {
      glxx_image_unit_deinit(image_unit);
      goto end;
   }

   if (level < 0 || layer < 0)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!is_allowed_image_unit_format(format))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!is_allowed_access(access))
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   GLXX_TEXTURE_T *texture = glxx_shared_get_texture(state->shared, tex_id);
   if (!texture)
   {
      error = GL_INVALID_VALUE;
      goto end;
   }

   if (!texture->immutable_format && texture->target != GL_TEXTURE_BUFFER)
   {
      error = GL_INVALID_OPERATION;
      goto end;
   }

   KHRN_MEM_ASSIGN(image_unit->texture, texture);
   image_unit->level = level;
   image_unit->layered = layered;
   image_unit->layer = layer;
   image_unit->access = access;
   image_unit->internalformat = format;

end:
   if (error != GL_NO_ERROR)
      glxx_server_state_set_error(state, error);

   glxx_unlock_server_state();
}

#endif
