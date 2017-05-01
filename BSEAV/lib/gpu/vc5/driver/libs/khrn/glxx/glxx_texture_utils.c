/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_texture_utils.h"
#include "glxx_texture.h"
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#include "glxx_server_internal.h"
#include "../common/khrn_process.h"

/******************************************************************************
 * Helper functions
 *****************************************************************************/

void glxx_lfmt_add_dim(GFX_LFMT_T *lfmts, uint32_t num_planes, uint32_t dim)
{
   uint32_t p;
   for (p = 0; p != num_planes; ++p)
      gfx_lfmt_set_dims(&lfmts[p], gfx_lfmt_dims_to_enum(dim));
}

bool glxx_is_texture_filterable_api_fmt(GFX_LFMT_T api_fmt)
{
   switch (api_fmt)
   {
      case GFX_LFMT_R8_UNORM:
      case GFX_LFMT_R8_SNORM:
      case GFX_LFMT_R8_G8_UNORM:
      case GFX_LFMT_R8_G8_SNORM:
      case GFX_LFMT_R8_G8_B8_UNORM:
      case GFX_LFMT_R8_G8_B8_SNORM:
      case GFX_LFMT_B5G6R5_UNORM:
      case GFX_LFMT_A4B4G4R4_UNORM:
      case GFX_LFMT_A1B5G5R5_UNORM:
      case GFX_LFMT_R8_G8_B8_A8_UNORM:
      case GFX_LFMT_R8_G8_B8_A8_SNORM:
      case GFX_LFMT_R10G10B10A2_UNORM:
      case GFX_LFMT_R8_SRGB:
      case GFX_LFMT_R8_G8_SRGB:
      case GFX_LFMT_R8_G8_B8_SRGB:
      case GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM:
      case GFX_LFMT_R16_FLOAT:
      case GFX_LFMT_R16_G16_FLOAT:
      case GFX_LFMT_R16_G16_B16_FLOAT:
      case GFX_LFMT_R16_G16_B16_A16_FLOAT:
      case GFX_LFMT_R11G11B10_UFLOAT:
      case GFX_LFMT_R9G9B9SHAREDEXP5_UFLOAT:
      case GFX_LFMT_R16_UNORM:
      case GFX_LFMT_R16_G16_UNORM:
      case GFX_LFMT_R16_G16_B16_A16_UNORM:
      case GFX_LFMT_R16_SNORM:
      case GFX_LFMT_R16_G16_SNORM:
      case GFX_LFMT_R16_G16_B16_A16_SNORM:
      return true;
   default:
      return false;
   }
}

/*
<hw_fmts> is what the image must be converted to be supported by the TMU and
TLB, as appropriate.
*/
void glxx_hw_fmts_from_api_fmt(
   /* outputs: */
   uint32_t *num_planes,
   /* postcondition: format only - layout bits empty */
   GFX_LFMT_T hw_fmts[GFX_BUFFER_MAX_PLANES],
   GFX_LFMT_T api_fmt)
{
   gfx_buffer_lfmts_none(hw_fmts);

   /* for some formats that are not supported by HW natively, like
    * GFX_LFMT_R32_G32_B32_INT, pad up to 128-bits by adding _X32.
    * BGRA is internally represented as RGBA and swizzled on upload.
    */
   switch (api_fmt)
   {
   case GFX_LFMT_R8_G8_B8_UNORM:    hw_fmts[0] = GFX_LFMT_R8_G8_B8_X8_UNORM; break;
   case GFX_LFMT_R8_G8_B8_SNORM:    hw_fmts[0] = GFX_LFMT_R8_G8_B8_X8_SNORM; break;
   case GFX_LFMT_R8_G8_B8_SRGB:     hw_fmts[0] = GFX_LFMT_R8_G8_B8_X8_SRGB; break;
   case GFX_LFMT_R16_G16_B16_FLOAT: hw_fmts[0] = GFX_LFMT_R16_G16_B16_X16_FLOAT; break;
   case GFX_LFMT_R32_G32_B32_FLOAT: hw_fmts[0] = GFX_LFMT_R32_G32_B32_X32_FLOAT; break;
   case GFX_LFMT_R8_G8_B8_INT:      hw_fmts[0] = GFX_LFMT_R8_G8_B8_X8_INT; break;
   case GFX_LFMT_R8_G8_B8_UINT:     hw_fmts[0] = GFX_LFMT_R8_G8_B8_X8_UINT; break;
   case GFX_LFMT_R16_G16_B16_INT:   hw_fmts[0] = GFX_LFMT_R16_G16_B16_X16_INT; break;
   case GFX_LFMT_R16_G16_B16_UINT:  hw_fmts[0] = GFX_LFMT_R16_G16_B16_X16_UINT; break;
   case GFX_LFMT_R32_G32_B32_INT:   hw_fmts[0] = GFX_LFMT_R32_G32_B32_X32_INT; break;
   case GFX_LFMT_R32_G32_B32_UINT:  hw_fmts[0] = GFX_LFMT_R32_G32_B32_X32_UINT; break;
   case GFX_LFMT_B8_G8_R8_A8_UNORM: hw_fmts[0] = GFX_LFMT_R8_G8_B8_A8_UNORM; break;
   case GFX_LFMT_D32_S8X24_FLOAT_UINT:
      hw_fmts[0] = GFX_LFMT_D32_FLOAT;
      hw_fmts[1] = GFX_LFMT_S8_UINT;
      break;
   default:
      hw_fmts[0] = api_fmt;
   }

   *num_planes = gfx_buffer_lfmts_num_planes(hw_fmts);
}

bool glxx_texture_are_legal_dimensions(GLenum target, int w, int h, int d)
{
   if (w < 0 || h < 0 || d < 0)
      return false;

   bool ok = false;
   switch (target)
   {
      case GL_TEXTURE_1D_BRCM:
         ok = w <= MAX_TEXTURE_SIZE;
         break;
      case GL_TEXTURE_1D_ARRAY_BRCM:
         ok = w <= MAX_TEXTURE_SIZE && d <= MAX_ARRAY_TEXTURE_LAYERS;
         break;
      case GL_TEXTURE_2D:
      case GL_TEXTURE_EXTERNAL_OES:
      case GL_TEXTURE_CUBE_MAP:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      case GL_TEXTURE_2D_MULTISAMPLE:
         ok = w <= MAX_TEXTURE_SIZE && h <= MAX_TEXTURE_SIZE;
         break;
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      case GL_TEXTURE_CUBE_MAP_ARRAY:
         ok = w <= MAX_TEXTURE_SIZE && h <= MAX_TEXTURE_SIZE &&
            d <= MAX_ARRAY_TEXTURE_LAYERS;
         break;
      case GL_TEXTURE_3D:
         ok = w <= MAX_3D_TEXTURE_SIZE && h <= MAX_3D_TEXTURE_SIZE &&
            d <= MAX_3D_TEXTURE_SIZE;
         break;
      case GL_TEXTURE_BUFFER:
         /* texture buffer width gets calculated from buffer size and fmt and
          * gets clamped to GLXX_CONFIG_MAX_TEXTURE_BUFFER_SIZE, so there is no
          * legal width supplied as input */
      default:
         unreachable();
   }

   return ok;
}

bool glxx_texture_is_legal_level(GLenum target, int level)
{
   bool ok = false;
   uint32_t max_legal_level;
   if (level < 0)
      return false;

   switch (target)
   {
      case GL_TEXTURE_1D_BRCM:
      case GL_TEXTURE_1D_ARRAY_BRCM:
      case GL_TEXTURE_2D:
      case GL_TEXTURE_EXTERNAL_OES:
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
      case GL_TEXTURE_CUBE_MAP:
      case GL_TEXTURE_CUBE_MAP_ARRAY:
         max_legal_level = LOG2_MAX_TEXTURE_SIZE;
         break;
      case GL_TEXTURE_3D:
         max_legal_level = LOG2_MAX_3D_TEXTURE_SIZE;
         break;
      case GL_TEXTURE_2D_MULTISAMPLE:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      case GL_TEXTURE_BUFFER:
         max_legal_level = 0;
         break;
      default:
         unreachable();
   }

   ok = (uint32_t)level <= max_legal_level;
   return ok;
}

bool glxx_texture_is_legal_layer(GLenum target, int layer)
{
   int max_legal_layer;

   if (layer < 0)
      return false;

   switch(target)
   {
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      case GL_TEXTURE_CUBE_MAP_ARRAY:
         max_legal_layer = MAX_ARRAY_TEXTURE_LAYERS - 1;
         break;
      case GL_TEXTURE_3D:
         max_legal_layer = MAX_3D_TEXTURE_SIZE -1;
         break;
      default:
         unreachable();
         max_legal_layer = -1;
         break;
   }
   return (layer <= max_legal_layer);
}

uint32_t glxx_texture_get_face(GLenum target)
{
   uint32_t face;
   switch (target)
   {
      case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
      case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
      case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
         face = target - GL_TEXTURE_CUBE_MAP_POSITIVE_X;
         break;
      case GL_TEXTURE_1D_ARRAY_BRCM:
      case GL_TEXTURE_1D_BRCM:
      case GL_TEXTURE_2D:
      case GL_TEXTURE_EXTERNAL_OES:
      case GL_TEXTURE_3D:
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_2D_MULTISAMPLE:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
      case GL_TEXTURE_CUBE_MAP_ARRAY:
      case GL_TEXTURE_BUFFER:
         face = 0;
         break;
      default:
         unreachable();
   }
   return face;
}

bool glxx_tex_target_valid_in_es1(GLenum target)
{
   switch (target)
   {
      case GL_TEXTURE_2D:
      case GL_TEXTURE_EXTERNAL_OES:
         return true;
      default:
         return false;
   }
}

bool glxx_tex_target_is_multisample(GLenum target) {
   return target == GL_TEXTURE_2D_MULTISAMPLE ||
          target == GL_TEXTURE_2D_MULTISAMPLE_ARRAY;
}

bool glxx_tex_target_has_layers(enum glxx_tex_target target)
{
   switch(target)
   {
   case GL_TEXTURE_3D:
   case GL_TEXTURE_2D_ARRAY:
   case GL_TEXTURE_CUBE_MAP_ARRAY:
   case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
   case GL_TEXTURE_1D_ARRAY_BRCM:
      return true;

   case GL_TEXTURE_1D_BRCM:
   case GL_TEXTURE_2D:
   case GL_TEXTURE_EXTERNAL_OES:
   case GL_TEXTURE_CUBE_MAP:
   case GL_TEXTURE_2D_MULTISAMPLE:
   case GL_TEXTURE_BUFFER:
      return false;
   default:
      unreachable();
   }
}

bool glxx_tex_target_has_layers_or_faces(enum glxx_tex_target target)
{
   if (target == GL_TEXTURE_CUBE_MAP)
      return true;

   return glxx_tex_target_has_layers(target);
}

/* unsized internalformats give the driver some flexibility as to what
 * texture format we choose;
 *
 * src_lfmt is the framebuffer format;
 */
void glxx_choose_copy_format_from_unsized_internalformat(GLenum internalformat,
      GFX_LFMT_T src_lfmt, GFX_LFMT_T dst_fmts[GFX_BUFFER_MAX_PLANES],
      uint32_t *dst_num_planes)
{
   GFX_LFMT_T src_fmt;
   assert(gfx_gl_is_unsized_internalformat(internalformat));

   src_fmt = gfx_lfmt_fmt(src_lfmt);
   gfx_buffer_lfmts_none(dst_fmts);

   /* the function "(src_lfmt, internalformat) -> dst_format" is not trivial:
    * sometimes you want to just swizzle (example: (rgba5551, rgb) ->
    * rgbx5551), other times you want to change the base too (example:
    * (rgba8888, alpha) -> a8). Fow now, just detect some special cases, and
    * fall back to 8-bit unorm otherwise */

   /* TODO are we expecting the unsized internalformats, which are deprecated
    * in ES3, to work with (format,type) pairs new to ES3? */
   if (src_fmt == GFX_LFMT_A4B4G4R4_UNORM && internalformat == GL_RGBA)
      dst_fmts[0] = GFX_LFMT_A4B4G4R4_UNORM;
   else if (src_fmt == GFX_LFMT_A1B5G5R5_UNORM && internalformat == GL_RGBA)
      dst_fmts[0] = GFX_LFMT_A1B5G5R5_UNORM;
   else if (src_fmt == GFX_LFMT_B5G6R5_UNORM && internalformat == GL_RGB)
      dst_fmts[0] = GFX_LFMT_B5G6R5_UNORM;
   else
   {
      switch (internalformat)
      {
      case GL_RGBA:              dst_fmts[0] = GFX_LFMT_R8_G8_B8_A8_UNORM; break;
      case GL_RGB:               dst_fmts[0] = GFX_LFMT_R8_G8_B8_X8_UNORM; break;
      case GL_LUMINANCE_ALPHA:   dst_fmts[0] = GFX_LFMT_L8_A8_UNORM; break;
      case GL_LUMINANCE:         dst_fmts[0] = GFX_LFMT_L8_UNORM; break;
      case GL_ALPHA:             dst_fmts[0] = GFX_LFMT_A8_UNORM; break;
      default:                   unreachable(); *dst_num_planes = 0;
      }
   }
   *dst_num_planes = gfx_buffer_lfmts_num_planes(dst_fmts);
}

/* do not add cube faces to this list of targets;
 * this must be the same as enum glxx_tex_target
 */
bool glxx_texture_is_tex_target(const GLXX_SERVER_STATE_T *state, GLenum target)
{
   if (IS_GL_11(state))
      return glxx_tex_target_valid_in_es1(target);

   bool ok;
   switch(target)
   {
      case GL_TEXTURE_1D_BRCM:
      case GL_TEXTURE_1D_ARRAY_BRCM:
      case GL_TEXTURE_2D:
      case GL_TEXTURE_EXTERNAL_OES:
      case GL_TEXTURE_3D:
      case GL_TEXTURE_2D_ARRAY:
      case GL_TEXTURE_CUBE_MAP:
         ok = true;
         break;
      case GL_TEXTURE_2D_MULTISAMPLE:
      case GL_TEXTURE_2D_MULTISAMPLE_ARRAY:
#if V3D_VER_AT_LEAST(4,0,2,0)
      case GL_TEXTURE_CUBE_MAP_ARRAY:
#endif
      case GL_TEXTURE_BUFFER:
         ok = KHRN_GLES31_DRIVER ? true : false;
         break;
      default:
         ok = false;
   }
   return ok;
}

bool glxx_texture_is_cube_face(GLenum target)
{
   return (target >= GL_TEXTURE_CUBE_MAP_POSITIVE_X) &&
      (target <= GL_TEXTURE_CUBE_MAP_NEGATIVE_Z);
}

void glxx_clamp_rect(int src_width, int src_height, int *x, int *y,
      int *width, int * height, int *dst_x, int *dst_y)
{
   assert((*width >= 0) && (*height >= 0) && (src_width >= 0) && (src_height >= 0));

   if (*x < 0)
   {
      *dst_x -= *x;
      *width += *x;
      *x = 0;
   }

   if (*y < 0)
   {
      *dst_y -= *y;
      *height += *y;
      *y = 0;
   }

   if (*x + *width > src_width)
      *width = src_width - *x;

   if (*y + *height > src_height)
      *height = src_height - *y;

}

void glxx_get_pack_unpack_info(const GLXX_PIXEL_STORE_STATE_T *pack_unpack,
      bool for_pack, unsigned width, unsigned height, GLenum fmt, GLenum type,
      struct glxx_pixels_info *info)
{
   const GLXX_PIXEL_PACK_STATE_T *pack;
   unsigned elements_per_group;
   unsigned size_per_element;
   unsigned no_groups, no_rows;

   pack = for_pack ? &pack_unpack->pack : &pack_unpack->unpack;

   if (pack->row_length)
      no_groups = pack->row_length;
   else
      no_groups = width;

   if (!for_pack && pack_unpack->unpack_image_height)
      no_rows = pack_unpack->unpack_image_height;
   else
      no_rows = height;

   glxx_elements_per_group(fmt, type, &elements_per_group, &size_per_element);

   /* the input width in in elements, the info->width is in bytes */
   info->width = width * elements_per_group * size_per_element;

   info->stride = glxx_pixel_rectangle_get_stride(no_groups, fmt, type,
                  pack->alignment);
   info->slice_pitch = info->stride * no_rows;

   info->offset  = pack->skip_rows * info->stride;
   info->offset += pack->skip_pixels * elements_per_group * size_per_element;

   if (!for_pack)
      info->offset += pack_unpack->unpack_skip_images * info->slice_pitch;
}

bool glxx_check_buffer_valid_access(const GLXX_BUFFER_T *pixel_buffer,
      const GFX_BUFFER_DESC_T *desc, size_t offset_1, size_t offset_2,
      unsigned num_array_elems, size_t array_pitch)
{
   size_t buf_copy_size, buf_offset, buf_size;
   GFX_LFMT_BASE_DETAIL_T bd;
   unsigned int num_blocks_x, num_blocks_y;

   buf_offset = offset_1 + offset_2;
   if (buf_offset < offset_1)
      return false;

   /* calculate how much we copy from the buffer */
   gfx_lfmt_base_detail(&bd, desc->planes[0].lfmt);

   num_blocks_x = gfx_udiv_round_up(desc->width, bd.block_w);
   num_blocks_y = gfx_udiv_round_up(desc->height, bd.block_h);

   buf_copy_size = (num_array_elems - 1) * array_pitch +
      desc->planes[0].pitch * (num_blocks_y - 1) +
      num_blocks_x * bd.bytes_per_block;

   /* make sure we do not wrap and we are not going beyond the size of the
    * pixel buffer object */
   buf_size = glxx_buffer_get_size(pixel_buffer);
   if (buf_offset + buf_copy_size < buf_offset ||
         buf_offset + buf_copy_size > buf_size)
   {
      return false;
   }

   return true;
}
