/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "helpers/gfx/gfx_lfmt_translate_v3d.h"
#include "helpers/v3d/v3d_tmu.h"

v3d_index_type_t gfx_lfmt_translate_index_type(GFX_LFMT_T lfmt)
{
   assert(gfx_lfmt_is_1d(lfmt));
   switch (lfmt & GFX_LFMT_FORMAT_MASK)
   {
   case GFX_LFMT_R16_UINT: return V3D_INDEX_TYPE_16BIT;
   case GFX_LFMT_R32_UINT: return V3D_INDEX_TYPE_32BIT;
   default:                not_impl(); return V3D_INDEX_TYPE_INVALID;
   }
}

/** TLB/TFU memory format */

GFX_LFMT_T gfx_lfmt_translate_from_memory_format(
   v3d_memory_format_t memory_format)
{
   switch (memory_format)
   {
   case V3D_MEMORY_FORMAT_RASTER:      return GFX_LFMT_2D_RSO;
   case V3D_MEMORY_FORMAT_LINEARTILE:  return GFX_LFMT_2D_LT;
   case V3D_MEMORY_FORMAT_UBLINEAR_1:
   case V3D_MEMORY_FORMAT_UBLINEAR_2:  return GFX_LFMT_2D_UBLINEAR;
   case V3D_MEMORY_FORMAT_UIF_NO_XOR:  return GFX_LFMT_2D_UIF;
   case V3D_MEMORY_FORMAT_UIF_XOR:     return GFX_LFMT_2D_UIF_XOR;
   default:                            unreachable(); return GFX_LFMT_NONE;
   }
}

GFX_LFMT_T gfx_lfmt_translate_from_tfu_iformat(
   v3d_tfu_iformat_t tfu_iformat)
{
   switch (tfu_iformat)
   {
   case V3D_TFU_IFORMAT_RASTER:     return GFX_LFMT_2D_RSO;
   case V3D_TFU_IFORMAT_SAND_128:   return GFX_LFMT_2D_SAND_128;
   case V3D_TFU_IFORMAT_SAND_256:   return GFX_LFMT_2D_SAND_256;
   case V3D_TFU_IFORMAT_LINEARTILE: return GFX_LFMT_2D_LT;
   case V3D_TFU_IFORMAT_UBLINEAR_1:
   case V3D_TFU_IFORMAT_UBLINEAR_2: return GFX_LFMT_2D_UBLINEAR;
   case V3D_TFU_IFORMAT_UIF_NO_XOR: return GFX_LFMT_2D_UIF;
   case V3D_TFU_IFORMAT_UIF_XOR:    return GFX_LFMT_2D_UIF_XOR;
   default:                         unreachable(); return GFX_LFMT_NONE;
   }
}

GFX_LFMT_T gfx_lfmt_translate_from_tfu_oformat(
   v3d_tfu_oformat_t tfu_oformat)
{
   switch (tfu_oformat)
   {
   case V3D_TFU_OFORMAT_LINEARTILE: return GFX_LFMT_2D_LT;
   case V3D_TFU_OFORMAT_UBLINEAR_1:
   case V3D_TFU_OFORMAT_UBLINEAR_2: return GFX_LFMT_2D_UBLINEAR;
   case V3D_TFU_OFORMAT_UIF_NO_XOR: return GFX_LFMT_2D_UIF;
   case V3D_TFU_OFORMAT_UIF_XOR:    return GFX_LFMT_2D_UIF_XOR;
   default:                         unreachable(); return GFX_LFMT_NONE;
   }
}

/** TLB pixel format (for color buffers) */

v3d_pixel_format_t gfx_lfmt_maybe_translate_pixel_format(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_FORMAT_MASK & ~GFX_LFMT_PRE_MASK)
   {
   case GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM: return V3D_PIXEL_FORMAT_SRGB8_ALPHA8;
   case GFX_LFMT_R8_G8_B8_SRGB:                    return V3D_PIXEL_FORMAT_SRGB8;
   case GFX_LFMT_R10G10B10A2_UINT:                 return V3D_PIXEL_FORMAT_RGB10_A2UI;
   case GFX_LFMT_R10G10B10A2_UNORM:                return V3D_PIXEL_FORMAT_RGB10_A2;
   case GFX_LFMT_A1B5G5R5_UNORM:                   return V3D_PIXEL_FORMAT_A1_BGR5; /* TODO a1_bgr5_am? */
   case GFX_LFMT_A4B4G4R4_UNORM:                   return V3D_PIXEL_FORMAT_ABGR4;
   case GFX_LFMT_B5G6R5_UNORM:                     return V3D_PIXEL_FORMAT_BGR565;
   case GFX_LFMT_R11G11B10_UFLOAT:                 return V3D_PIXEL_FORMAT_R11F_G11F_B10F;
   case GFX_LFMT_R32_G32_B32_A32_FLOAT:            return V3D_PIXEL_FORMAT_RGBA32F;
   case GFX_LFMT_R32_G32_FLOAT:                    return V3D_PIXEL_FORMAT_RG32F;
   case GFX_LFMT_R32_FLOAT:                        return V3D_PIXEL_FORMAT_R32F;
   case GFX_LFMT_R32_G32_B32_A32_INT:              return V3D_PIXEL_FORMAT_RGBA32I;
   case GFX_LFMT_R32_G32_INT:                      return V3D_PIXEL_FORMAT_RG32I;
   case GFX_LFMT_R32_INT:                          return V3D_PIXEL_FORMAT_R32I;
   case GFX_LFMT_R32_G32_B32_A32_UINT:             return V3D_PIXEL_FORMAT_RGBA32UI;
   case GFX_LFMT_R32_G32_UINT:                     return V3D_PIXEL_FORMAT_RG32UI;
   case GFX_LFMT_R32_UINT:                         return V3D_PIXEL_FORMAT_R32UI;
   case GFX_LFMT_R16_G16_B16_A16_FLOAT:            return V3D_PIXEL_FORMAT_RGBA16F;
   case GFX_LFMT_R16_G16_FLOAT:                    return V3D_PIXEL_FORMAT_RG16F;
   case GFX_LFMT_R16_FLOAT:                        return V3D_PIXEL_FORMAT_R16F;
   case GFX_LFMT_R16_G16_B16_A16_INT:              return V3D_PIXEL_FORMAT_RGBA16I;
   case GFX_LFMT_R16_G16_INT:                      return V3D_PIXEL_FORMAT_RG16I;
   case GFX_LFMT_R16_INT:                          return V3D_PIXEL_FORMAT_R16I;
   case GFX_LFMT_R16_G16_B16_A16_UINT:             return V3D_PIXEL_FORMAT_RGBA16UI;
   case GFX_LFMT_R16_G16_UINT:                     return V3D_PIXEL_FORMAT_RG16UI;
   case GFX_LFMT_R16_UINT:                         return V3D_PIXEL_FORMAT_R16UI;
   case GFX_LFMT_R8_G8_B8_A8_UNORM:                return V3D_PIXEL_FORMAT_RGBA8;
   case GFX_LFMT_R8_G8_B8_UNORM:                   return V3D_PIXEL_FORMAT_RGB8;
   case GFX_LFMT_R8_G8_UNORM:                      return V3D_PIXEL_FORMAT_RG8;
   case GFX_LFMT_R8_UNORM:                         return V3D_PIXEL_FORMAT_R8;
   case GFX_LFMT_R8_G8_B8_A8_INT:                  return V3D_PIXEL_FORMAT_RGBA8I;
   case GFX_LFMT_R8_G8_INT:                        return V3D_PIXEL_FORMAT_RG8I;
   case GFX_LFMT_R8_INT:                           return V3D_PIXEL_FORMAT_R8I;
   case GFX_LFMT_R8_G8_B8_A8_UINT:                 return V3D_PIXEL_FORMAT_RGBA8UI;
   case GFX_LFMT_R8_G8_UINT:                       return V3D_PIXEL_FORMAT_RG8UI;
   case GFX_LFMT_R8_UINT:                          return V3D_PIXEL_FORMAT_R8UI;
   case GFX_LFMT_R8_G8_B8_X8_SRGB:                 return V3D_PIXEL_FORMAT_SRGBX8;
   case GFX_LFMT_R8_G8_B8_X8_UNORM:                return V3D_PIXEL_FORMAT_RGBX8;
   case GFX_LFMT_BSTC_RGBA_UNORM:
   case GFX_LFMT_BSTCYFLIP_RGBA_UNORM:             return V3D_PIXEL_FORMAT_BSTC;
   default:                                        return V3D_PIXEL_FORMAT_INVALID;
   }
}

v3d_pixel_format_t gfx_lfmt_translate_pixel_format(GFX_LFMT_T lfmt)
{
   v3d_pixel_format_t pixel_format = gfx_lfmt_maybe_translate_pixel_format(lfmt);
   assert(pixel_format != V3D_PIXEL_FORMAT_INVALID);
   return pixel_format;
}

GFX_LFMT_T gfx_lfmt_translate_from_pixel_format(v3d_pixel_format_t pixel_format)
{
   switch (pixel_format)
   {
   case V3D_PIXEL_FORMAT_SRGB8_ALPHA8:    return GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM;
   case V3D_PIXEL_FORMAT_SRGB8:           return GFX_LFMT_R8_G8_B8_SRGB;
   case V3D_PIXEL_FORMAT_RGB10_A2UI:      return GFX_LFMT_R10G10B10A2_UINT;
   case V3D_PIXEL_FORMAT_RGB10_A2:        return GFX_LFMT_R10G10B10A2_UNORM;
   case V3D_PIXEL_FORMAT_A1_BGR5:
   case V3D_PIXEL_FORMAT_A1_BGR5_AM:      return GFX_LFMT_A1B5G5R5_UNORM;
   case V3D_PIXEL_FORMAT_ABGR4:           return GFX_LFMT_A4B4G4R4_UNORM;
   case V3D_PIXEL_FORMAT_BGR565:          return GFX_LFMT_B5G6R5_UNORM;
   case V3D_PIXEL_FORMAT_R11F_G11F_B10F:  return GFX_LFMT_R11G11B10_UFLOAT;
   case V3D_PIXEL_FORMAT_RGBA32F:         return GFX_LFMT_R32_G32_B32_A32_FLOAT;
   case V3D_PIXEL_FORMAT_RG32F:           return GFX_LFMT_R32_G32_FLOAT;
   case V3D_PIXEL_FORMAT_R32F:            return GFX_LFMT_R32_FLOAT;
   case V3D_PIXEL_FORMAT_RGBA32I:         return GFX_LFMT_R32_G32_B32_A32_INT;
   case V3D_PIXEL_FORMAT_RG32I:           return GFX_LFMT_R32_G32_INT;
   case V3D_PIXEL_FORMAT_R32I:            return GFX_LFMT_R32_INT;
   case V3D_PIXEL_FORMAT_RGBA32UI:        return GFX_LFMT_R32_G32_B32_A32_UINT;
   case V3D_PIXEL_FORMAT_RG32UI:          return GFX_LFMT_R32_G32_UINT;
   case V3D_PIXEL_FORMAT_R32UI:           return GFX_LFMT_R32_UINT;
   case V3D_PIXEL_FORMAT_RGBA16F:         return GFX_LFMT_R16_G16_B16_A16_FLOAT;
   case V3D_PIXEL_FORMAT_RG16F:           return GFX_LFMT_R16_G16_FLOAT;
   case V3D_PIXEL_FORMAT_R16F:            return GFX_LFMT_R16_FLOAT;
   case V3D_PIXEL_FORMAT_RGBA16I:         return GFX_LFMT_R16_G16_B16_A16_INT;
   case V3D_PIXEL_FORMAT_RG16I:           return GFX_LFMT_R16_G16_INT;
   case V3D_PIXEL_FORMAT_R16I:            return GFX_LFMT_R16_INT;
   case V3D_PIXEL_FORMAT_RGBA16UI:        return GFX_LFMT_R16_G16_B16_A16_UINT;
   case V3D_PIXEL_FORMAT_RG16UI:          return GFX_LFMT_R16_G16_UINT;
   case V3D_PIXEL_FORMAT_R16UI:           return GFX_LFMT_R16_UINT;
   case V3D_PIXEL_FORMAT_RGBA8:           return GFX_LFMT_R8_G8_B8_A8_UNORM;
   case V3D_PIXEL_FORMAT_RGB8:            return GFX_LFMT_R8_G8_B8_UNORM;
   case V3D_PIXEL_FORMAT_RG8:             return GFX_LFMT_R8_G8_UNORM;
   case V3D_PIXEL_FORMAT_R8:              return GFX_LFMT_R8_UNORM;
   case V3D_PIXEL_FORMAT_RGBA8I:          return GFX_LFMT_R8_G8_B8_A8_INT;
   case V3D_PIXEL_FORMAT_RG8I:            return GFX_LFMT_R8_G8_INT;
   case V3D_PIXEL_FORMAT_R8I:             return GFX_LFMT_R8_INT;
   case V3D_PIXEL_FORMAT_RGBA8UI:         return GFX_LFMT_R8_G8_B8_A8_UINT;
   case V3D_PIXEL_FORMAT_RG8UI:           return GFX_LFMT_R8_G8_UINT;
   case V3D_PIXEL_FORMAT_R8UI:            return GFX_LFMT_R8_UINT;
   case V3D_PIXEL_FORMAT_SRGBX8:          return GFX_LFMT_R8_G8_B8_X8_SRGB;
   case V3D_PIXEL_FORMAT_RGBX8:           return GFX_LFMT_R8_G8_B8_X8_UNORM;
   case V3D_PIXEL_FORMAT_BSTC:            return GFX_LFMT_BSTC_RGBA_UNORM;
   default:                               unreachable(); return GFX_LFMT_NONE;
   }
}

GFX_LFMT_T gfx_lfmt_translate_internal_raw_mode(GFX_LFMT_T output_lfmt)
{
   v3d_pixel_format_t   v3d_format    = gfx_lfmt_translate_pixel_format(output_lfmt);
   v3d_pixel_format_t   v3d_raw       = v3d_pixel_format_raw_mode(v3d_format);
   GFX_LFMT_T           fmt           = gfx_lfmt_translate_from_pixel_format(v3d_raw);
   GFX_LFMT_T           fmt_with_dims = gfx_lfmt_to_2d(fmt);
   return fmt_with_dims;
}

/** TLB depth format */

v3d_depth_format_t gfx_lfmt_maybe_translate_depth_format(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_FORMAT_MASK)
   {
   case GFX_LFMT_D32_FLOAT:         return V3D_DEPTH_FORMAT_32F;
   case GFX_LFMT_D24X8_UNORM:       return V3D_DEPTH_FORMAT_24;
   case GFX_LFMT_D16_UNORM:         return V3D_DEPTH_FORMAT_16;
   case GFX_LFMT_S8D24_UINT_UNORM:  return V3D_DEPTH_FORMAT_24_STENCIL8;
   default:                         return V3D_DEPTH_FORMAT_INVALID;
   }
}

v3d_depth_format_t gfx_lfmt_translate_depth_format(GFX_LFMT_T lfmt)
{
   v3d_depth_format_t depth_format = gfx_lfmt_maybe_translate_depth_format(lfmt);
   assert(depth_format != V3D_DEPTH_FORMAT_INVALID);
   return depth_format;
}

v3d_depth_type_t gfx_lfmt_translate_depth_type(GFX_LFMT_T depth_lfmt)
{
   GFX_LFMT_T fmt = gfx_lfmt_fmt(depth_lfmt);

   switch (fmt)
   {
   case GFX_LFMT_D16_UNORM:            return V3D_DEPTH_TYPE_16;
   case GFX_LFMT_D24X8_UNORM:          return V3D_DEPTH_TYPE_24;
   case GFX_LFMT_S8D24_UINT_UNORM:     return V3D_DEPTH_TYPE_24;
   case GFX_LFMT_D32_FLOAT:            return V3D_DEPTH_TYPE_32F;
   case GFX_LFMT_D32_S8X24_FLOAT_UINT: return V3D_DEPTH_TYPE_32F;
   default:                            unreachable(); return V3D_DEPTH_TYPE_INVALID;
   }
}

GFX_LFMT_T gfx_lfmt_translate_from_depth_format(v3d_depth_format_t depth_format)
{
   switch (depth_format)
   {
   case V3D_DEPTH_FORMAT_32F:          return GFX_LFMT_D32_FLOAT;
   case V3D_DEPTH_FORMAT_24:           return GFX_LFMT_D24X8_UNORM;
   case V3D_DEPTH_FORMAT_16:           return GFX_LFMT_D16_UNORM;
   case V3D_DEPTH_FORMAT_24_STENCIL8:  return GFX_LFMT_S8D24_UINT_UNORM;
   default:                            unreachable(); return GFX_LFMT_NONE;
   }
}

/** TMU */

static v3d_tmu_type_t try_get_tmu_type_and_out_fmt(
   GFX_LFMT_T *tmu_out_fmt, /* Just channels & type */
   GFX_LFMT_T lfmt)
{
   *tmu_out_fmt = lfmt & (GFX_LFMT_CHANNELS_MASK | GFX_LFMT_TYPE_MASK);
   switch (gfx_lfmt_srgb_to_unorm(lfmt) & (GFX_LFMT_BASE_MASK | GFX_LFMT_TYPE_MASK))
   {
   case GFX_LFMT_C1_UNORM:                return V3D_TMU_TYPE_R1;
   case GFX_LFMT_C4_UNORM:                return V3D_TMU_TYPE_R4;

   case GFX_LFMT_C8_UNORM:                return V3D_TMU_TYPE_R8;
   case GFX_LFMT_C8_SNORM:                return V3D_TMU_TYPE_R8_SNORM;
   case GFX_LFMT_C8_UINT:                 return V3D_TMU_TYPE_R8UI;
   case GFX_LFMT_C8_INT:                  return V3D_TMU_TYPE_R8I;

   case GFX_LFMT_C8_C8_UNORM:             return V3D_TMU_TYPE_RG8;
   case GFX_LFMT_C8_C8_SNORM:             return V3D_TMU_TYPE_RG8_SNORM;
   case GFX_LFMT_C8_C8_UINT:              return V3D_TMU_TYPE_RG8UI;
   case GFX_LFMT_C8_C8_INT:               return V3D_TMU_TYPE_RG8I;

   case GFX_LFMT_C8_C8_C8_C8_UNORM:       return V3D_TMU_TYPE_RGBA8;
   case GFX_LFMT_C8_C8_C8_C8_SNORM:       return V3D_TMU_TYPE_RGBA8_SNORM;
   case GFX_LFMT_C8_C8_C8_C8_UINT:        return V3D_TMU_TYPE_RGBA8UI;
   case GFX_LFMT_C8_C8_C8_C8_INT:         return V3D_TMU_TYPE_RGBA8I;

   case GFX_LFMT_C16_UNORM:               return V3D_TMU_TYPE_R16;
   case GFX_LFMT_C16_SNORM:               return V3D_TMU_TYPE_R16_SNORM;
   case GFX_LFMT_C16_UINT:                return V3D_TMU_TYPE_R16UI;
   case GFX_LFMT_C16_INT:                 return V3D_TMU_TYPE_R16I;
   case GFX_LFMT_C16_FLOAT:               return V3D_TMU_TYPE_R16F;

   case GFX_LFMT_C16_C16_UNORM:           return V3D_TMU_TYPE_RG16;
   case GFX_LFMT_C16_C16_SNORM:           return V3D_TMU_TYPE_RG16_SNORM;
   case GFX_LFMT_C16_C16_UINT:            return V3D_TMU_TYPE_RG16UI;
   case GFX_LFMT_C16_C16_INT:             return V3D_TMU_TYPE_RG16I;
   case GFX_LFMT_C16_C16_FLOAT:           return V3D_TMU_TYPE_RG16F;

   case GFX_LFMT_C16_C16_C16_C16_UNORM:   return V3D_TMU_TYPE_RGBA16;
   case GFX_LFMT_C16_C16_C16_C16_SNORM:   return V3D_TMU_TYPE_RGBA16_SNORM;
   case GFX_LFMT_C16_C16_C16_C16_UINT:    return V3D_TMU_TYPE_RGBA16UI;
   case GFX_LFMT_C16_C16_C16_C16_INT:     return V3D_TMU_TYPE_RGBA16I;
   case GFX_LFMT_C16_C16_C16_C16_FLOAT:   return V3D_TMU_TYPE_RGBA16F;

   case GFX_LFMT_C32_UINT:                return V3D_TMU_TYPE_R32UI;
   case GFX_LFMT_C32_INT:                 return V3D_TMU_TYPE_R32I;
   case GFX_LFMT_C32_FLOAT:               return V3D_TMU_TYPE_R32F;

   case GFX_LFMT_C32_C32_UINT:            return V3D_TMU_TYPE_RG32UI;
   case GFX_LFMT_C32_C32_INT:             return V3D_TMU_TYPE_RG32I;
   case GFX_LFMT_C32_C32_FLOAT:           return V3D_TMU_TYPE_RG32F;

   case GFX_LFMT_C32_C32_C32_C32_UINT:    return V3D_TMU_TYPE_RGBA32UI;
   case GFX_LFMT_C32_C32_C32_C32_INT:     return V3D_TMU_TYPE_RGBA32I;
   case GFX_LFMT_C32_C32_C32_C32_FLOAT:   return V3D_TMU_TYPE_RGBA32F;

   case GFX_LFMT_C5C6C5_UNORM:
      *tmu_out_fmt = gfx_lfmt_reverse_channels(gfx_lfmt_reverse_type(*tmu_out_fmt));
      return V3D_TMU_TYPE_RGB565;
   case GFX_LFMT_C4C4C4C4_UNORM:
      *tmu_out_fmt = gfx_lfmt_reverse_channels(gfx_lfmt_reverse_type(*tmu_out_fmt));
      return V3D_TMU_TYPE_RGBA4;
   case GFX_LFMT_C1C5C5C5_UNORM:
      *tmu_out_fmt = gfx_lfmt_reverse_channels(gfx_lfmt_reverse_type(*tmu_out_fmt));
      return V3D_TMU_TYPE_RGB5_A1;

   case GFX_LFMT_C10C10C10C2_UNORM:       return V3D_TMU_TYPE_RGB10_A2;
   case GFX_LFMT_C10C10C10C2_UINT:        return V3D_TMU_TYPE_RGB10_A2UI;

   case GFX_LFMT_C11C11C10_UFLOAT:        return V3D_TMU_TYPE_R11F_G11F_B10F;
   case GFX_LFMT_C9C9C9SHAREDEXP5_UFLOAT: return V3D_TMU_TYPE_RGB9_E5;

   case GFX_LFMT_C24C8_UNORM:
   {
      GFX_LFMT_FMT_DETAIL_T fd;
      gfx_lfmt_fmt_detail(&fd, lfmt);
      assert(fd.num_slots == 2);
      if (fd.slts[1].channel != GFX_LFMT_CHANNELS_X)
         return V3D_TMU_TYPE_INVALID;
      *tmu_out_fmt = fd.slts[0].channel | fd.slts[0].type;
      return V3D_TMU_TYPE_DEPTH_COMP24;
   }
   case GFX_LFMT_C8C24_UNORM:
   {
      GFX_LFMT_FMT_DETAIL_T fd;
      gfx_lfmt_fmt_detail(&fd, lfmt);
      assert(fd.num_slots == 2);
      if (fd.slts[0].channel != GFX_LFMT_CHANNELS_X)
         return V3D_TMU_TYPE_INVALID;
      *tmu_out_fmt = fd.slts[1].channel | fd.slts[1].type;
      return V3D_TMU_TYPE_DEPTH24_X8;
   }
   case GFX_LFMT_C8C24_UINT:
   {
      if (gfx_lfmt_get_channels(&lfmt) != GFX_LFMT_CHANNELS_RX)
         return V3D_TMU_TYPE_INVALID;
      *tmu_out_fmt = GFX_LFMT_CHANNELS_RXXX | GFX_LFMT_TYPE_UINT;
      return V3D_TMU_TYPE_RGBA8UI;
   }
   case GFX_LFMT_ETC1_UNORM:              return V3D_TMU_TYPE_C_RGB8_ETC2; /* ETC1 textures are valid ETC2 textures */
   case GFX_LFMT_ETC2_UNORM:              return V3D_TMU_TYPE_C_RGB8_ETC2;
   case GFX_LFMT_PUNCHTHROUGH_ETC2_UNORM: return V3D_TMU_TYPE_C_RGB8_PUNCHTHROUGH_ALPHA1_ETC2;
   case GFX_LFMT_EAC_UNORM:               return V3D_TMU_TYPE_C_R11_EAC;
   case GFX_LFMT_EAC_SNORM:               return V3D_TMU_TYPE_C_SIGNED_R11_EAC;
   case GFX_LFMT_EAC_EAC_UNORM:           return V3D_TMU_TYPE_C_RG11_EAC;
   case GFX_LFMT_EAC_EAC_SNORM:           return V3D_TMU_TYPE_C_SIGNED_RG11_EAC;
   case GFX_LFMT_ETC2_EAC_UNORM:          return V3D_TMU_TYPE_C_RGBA8_ETC2_EAC;
   case GFX_LFMT_BC1_UNORM:               return V3D_TMU_TYPE_C_BC1;
   case GFX_LFMT_BC2_UNORM:               return V3D_TMU_TYPE_C_BC2;
   case GFX_LFMT_BC3_UNORM:               return V3D_TMU_TYPE_C_BC3;
   case GFX_LFMT_ASTC4X4_UNORM:           return V3D_TMU_TYPE_C_ASTC_4X4;
   case GFX_LFMT_ASTC5X4_UNORM:           return V3D_TMU_TYPE_C_ASTC_5X4;
   case GFX_LFMT_ASTC5X5_UNORM:           return V3D_TMU_TYPE_C_ASTC_5X5;
   case GFX_LFMT_ASTC6X5_UNORM:           return V3D_TMU_TYPE_C_ASTC_6X5;
   case GFX_LFMT_ASTC6X6_UNORM:           return V3D_TMU_TYPE_C_ASTC_6X6;
   case GFX_LFMT_ASTC8X5_UNORM:           return V3D_TMU_TYPE_C_ASTC_8X5;
   case GFX_LFMT_ASTC8X6_UNORM:           return V3D_TMU_TYPE_C_ASTC_8X6;
   case GFX_LFMT_ASTC8X8_UNORM:           return V3D_TMU_TYPE_C_ASTC_8X8;
   case GFX_LFMT_ASTC10X5_UNORM:          return V3D_TMU_TYPE_C_ASTC_10X5;
   case GFX_LFMT_ASTC10X6_UNORM:          return V3D_TMU_TYPE_C_ASTC_10X6;
   case GFX_LFMT_ASTC10X8_UNORM:          return V3D_TMU_TYPE_C_ASTC_10X8;
   case GFX_LFMT_ASTC10X10_UNORM:         return V3D_TMU_TYPE_C_ASTC_10X10;
   case GFX_LFMT_ASTC12X10_UNORM:         return V3D_TMU_TYPE_C_ASTC_12X10;
   case GFX_LFMT_ASTC12X12_UNORM:         return V3D_TMU_TYPE_C_ASTC_12X12;

   default:                               return V3D_TMU_TYPE_INVALID;
   }
}

static bool try_get_swizzles(v3d_tmu_swizzle_t swizzles[4], GFX_LFMT_T tmu_out_fmt)
{
   #define S(R,G,B,A)                        \
      do                                     \
      {                                      \
         swizzles[0] = V3D_TMU_SWIZZLE_##R;  \
         swizzles[1] = V3D_TMU_SWIZZLE_##G;  \
         swizzles[2] = V3D_TMU_SWIZZLE_##B;  \
         swizzles[3] = V3D_TMU_SWIZZLE_##A;  \
      } while (0)

   switch (gfx_lfmt_get_channels(&tmu_out_fmt))
   {
   case GFX_LFMT_CHANNELS_R:     S(R,G,B,A); return true;
   case GFX_LFMT_CHANNELS_A:     S(0,0,0,R); return true;
   case GFX_LFMT_CHANNELS_L:     S(R,R,R,1); return true;

   case GFX_LFMT_CHANNELS_RG:    S(R,G,B,A); return true;
   case GFX_LFMT_CHANNELS_RX:    S(R,0,0,1); return true;
   case GFX_LFMT_CHANNELS_LA:    S(R,R,R,G); return true;

   case GFX_LFMT_CHANNELS_RGB:   S(R,G,B,A); return true;
   case GFX_LFMT_CHANNELS_BGR:   S(B,G,R,1); return true;

   case GFX_LFMT_CHANNELS_RGBA:  S(R,G,B,A); return true;
   case GFX_LFMT_CHANNELS_BGRA:  S(B,G,R,A); return true;
   case GFX_LFMT_CHANNELS_ARGB:  S(G,B,A,R); return true;
   case GFX_LFMT_CHANNELS_ABGR:  S(A,B,G,R); return true;
   case GFX_LFMT_CHANNELS_RGBX:  S(R,G,B,1); return true;
   case GFX_LFMT_CHANNELS_BGRX:  S(B,G,R,1); return true;
   case GFX_LFMT_CHANNELS_XRGB:  S(G,B,A,1); return true;
   case GFX_LFMT_CHANNELS_XBGR:  S(A,B,G,1); return true;
   case GFX_LFMT_CHANNELS_RXXX:  S(R,0,0,1); return true;

   default:                      return false;
   }

   #undef S
}

static bool srgb_ok(v3d_tmu_type_t tmu_type, GFX_LFMT_T tmu_out_fmt,
   v3d_tfu_rgbord_t rgbord, int v3d_version)
{
   if (!v3d_tmu_type_supports_srgb(tmu_type, v3d_version))
      return false;

   uint32_t chan_mask = gfx_lfmt_valid_chan_mask(tmu_out_fmt);
   GFX_LFMT_TYPE_T exp_type;
   switch (rgbord)
   {
   case V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV:
   case V3D_TFU_RGBORD_BGRA_OR_VUYY:
      exp_type = (chan_mask & 0x8) ? GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM : GFX_LFMT_TYPE_SRGB;
      break;
   case V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU:
   case V3D_TFU_RGBORD_ARGB_OR_YYUV:
      exp_type = ((v3d_tmu_get_num_channels(tmu_type) == 4) && (chan_mask & 0x1)) ?
         GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB : GFX_LFMT_TYPE_SRGB;
      break;
   default:
      unreachable();
   }
   return gfx_lfmt_get_type(&tmu_out_fmt) == exp_type;
}

static void replace_in_swizzles(v3d_tmu_swizzle_t swizzles[4],
   v3d_tmu_swizzle_t to_replace, v3d_tmu_swizzle_t replace_with)
{
   for (unsigned i = 0; i != 4; ++i)
      if (swizzles[i] == to_replace)
         swizzles[i] = replace_with;
}

static void avoid_integer_types(GFX_LFMT_TMU_TRANSLATION_T *t)
{
   assert(t->ret == GFX_LFMT_TMU_RET_AUTO);

   switch (t->type)
   {
   case V3D_TMU_TYPE_R8I:
   case V3D_TMU_TYPE_R8UI:
   case V3D_TMU_TYPE_RG8I:
   case V3D_TMU_TYPE_RG8UI:
   case V3D_TMU_TYPE_RGBA8I:
   case V3D_TMU_TYPE_RGBA8UI:
      t->ret = GFX_LFMT_TMU_RET_8;
      break;
   case V3D_TMU_TYPE_R16I:
   case V3D_TMU_TYPE_R16UI:
   case V3D_TMU_TYPE_RG16I:
   case V3D_TMU_TYPE_RG16UI:
   case V3D_TMU_TYPE_RGBA16I:
   case V3D_TMU_TYPE_RGBA16UI:
      t->ret = GFX_LFMT_TMU_RET_16;
      break;
   case V3D_TMU_TYPE_R32I:
   case V3D_TMU_TYPE_R32UI:
   case V3D_TMU_TYPE_RG32I:
   case V3D_TMU_TYPE_RG32UI:
   case V3D_TMU_TYPE_RGBA32I:
   case V3D_TMU_TYPE_RGBA32UI:
      t->ret = GFX_LFMT_TMU_RET_32;
      break;
   case V3D_TMU_TYPE_RGB10_A2UI:
      t->ret = GFX_LFMT_TMU_RET_1010102;
      break;
   default:
      break;
   }

   if (t->ret != GFX_LFMT_TMU_RET_AUTO)
   {
      uint32_t num_channels = v3d_tmu_get_num_channels(t->type);

      uint32_t avg_bits_per_channel;
      switch (t->ret)
      {
      case GFX_LFMT_TMU_RET_8:         avg_bits_per_channel = 8; break;
      case GFX_LFMT_TMU_RET_16:        avg_bits_per_channel = 16; break;
      case GFX_LFMT_TMU_RET_32:        avg_bits_per_channel = 32; break;
      case GFX_LFMT_TMU_RET_1010102:   avg_bits_per_channel = 8; break;
      default:                         unreachable();
      }

      switch (num_channels * avg_bits_per_channel)
      {
      case 8:     t->type = V3D_TMU_TYPE_S8; break;
      case 16:    t->type = V3D_TMU_TYPE_S16; break;
      case 32:    t->type = V3D_TMU_TYPE_R32F; break;
      case 64:    t->type = V3D_TMU_TYPE_RG32F; break;
      case 128:   t->type = V3D_TMU_TYPE_RGBA32F; break;
      default:    unreachable();
      }

      /* S8/S16 cannot be swizzled by the TMU. *32F *can* be swizzled by the
       * TMU but you will get floating point 1 rather than integer 1. So
       * just always have the shader do swizzling. */
      t->shader_swizzle = true;

      /* Explicitly avoid absent channels in swizzles to stop shader trying
       * to read them -- with normal texture types sensible data (0 for RGB,
       * 1 for A) is returned for absent channels, but
       * a) S8/S16 are *not* normal texture types
       * b) the texture type isn't set correctly */
      switch (num_channels)
      {
      case 1:
         replace_in_swizzles(t->swizzles, V3D_TMU_SWIZZLE_G, V3D_TMU_SWIZZLE_0);
         /* Fall through... */
      case 2:
         replace_in_swizzles(t->swizzles, V3D_TMU_SWIZZLE_B, V3D_TMU_SWIZZLE_0);
         replace_in_swizzles(t->swizzles, V3D_TMU_SWIZZLE_A, V3D_TMU_SWIZZLE_1);
         /* Fall through... */
      case 4:
         break;
      default:
         unreachable();
      }
   }
}

bool gfx_lfmt_maybe_translate_tmu(GFX_LFMT_TMU_TRANSLATION_T *t,
   GFX_LFMT_T lfmt, gfx_lfmt_tmu_depth_pref_t depth_pref, int v3d_version)
{
   GFX_LFMT_T tmu_out_fmt;
   t->type = try_get_tmu_type_and_out_fmt(&tmu_out_fmt, lfmt);
   if (t->type == V3D_TMU_TYPE_INVALID)
      return false;
   gfx_lfmt_check_num_slots_eq(tmu_out_fmt, v3d_tmu_get_num_channels(t->type));

   switch (depth_pref)
   {
   case GFX_LFMT_TMU_DEPTH_ALWAYS:
      /* Caller requires v3d_tmu_is_depth_type(t->type) */
      switch (t->type)
      {
      case V3D_TMU_TYPE_R16:  t->type = V3D_TMU_TYPE_DEPTH_COMP16; break;
      case V3D_TMU_TYPE_R32F: t->type = V3D_TMU_TYPE_DEPTH_COMP32F; break;
      default:
         if (!v3d_tmu_is_depth_type(t->type))
            return false;
      }
      break;
   case GFX_LFMT_TMU_DEPTH_NEVER:
      /* Caller requires !v3d_tmu_is_depth_type(t->type) */
      if (v3d_tmu_is_depth_type(t->type))
         return false;
      break;
   case GFX_LFMT_TMU_DEPTH_DONT_CARE:
      break;
   default:
      unreachable();
   }

   t->srgb = gfx_lfmt_contains_srgb(tmu_out_fmt);
   if (t->srgb && !srgb_ok(t->type, tmu_out_fmt, V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV, v3d_version))
      return false;

   if (!try_get_swizzles(t->swizzles, tmu_out_fmt))
      return false;

   t->ret = GFX_LFMT_TMU_RET_AUTO;
   t->shader_swizzle = false;
   if (v3d_version < V3D_MAKE_VER(3,3))
      /* Integer types not supported... */
      avoid_integer_types(t);

   return true;
}

void gfx_lfmt_translate_tmu(GFX_LFMT_TMU_TRANSLATION_T *t,
   GFX_LFMT_T lfmt, gfx_lfmt_tmu_depth_pref_t depth_pref, int v3d_version)
{
   bool success = gfx_lfmt_maybe_translate_tmu(t, lfmt, depth_pref, v3d_version);
   assert(success);
   vcos_unused_in_release(success);
}

GFX_LFMT_T gfx_lfmt_translate_from_tmu_type(v3d_tmu_type_t tmu_type, bool srgb, int v3d_version)
{
   GFX_LFMT_T lfmt;
   switch (tmu_type)
   {
   case V3D_TMU_TYPE_R8:                              lfmt = GFX_LFMT_R8_UNORM; break;
   case V3D_TMU_TYPE_R8_SNORM:                        lfmt = GFX_LFMT_R8_SNORM; break;
   case V3D_TMU_TYPE_R8I:                             lfmt = GFX_LFMT_R8_INT; break;
   case V3D_TMU_TYPE_R8UI:                            lfmt = GFX_LFMT_R8_UINT; break;
   case V3D_TMU_TYPE_RG8:                             lfmt = GFX_LFMT_R8_G8_UNORM; break;
   case V3D_TMU_TYPE_RG8_SNORM:                       lfmt = GFX_LFMT_R8_G8_SNORM; break;
   case V3D_TMU_TYPE_RG8I:                            lfmt = GFX_LFMT_R8_G8_INT; break;
   case V3D_TMU_TYPE_RG8UI:                           lfmt = GFX_LFMT_R8_G8_UINT; break;
   case V3D_TMU_TYPE_RGBA8:                           lfmt = GFX_LFMT_R8_G8_B8_A8_UNORM; break;
   case V3D_TMU_TYPE_RGBA8_SNORM:                     lfmt = GFX_LFMT_R8_G8_B8_A8_SNORM; break;
   case V3D_TMU_TYPE_RGBA8I:                          lfmt = GFX_LFMT_R8_G8_B8_A8_INT; break;
   case V3D_TMU_TYPE_RGBA8UI:                         lfmt = GFX_LFMT_R8_G8_B8_A8_UINT; break;
   case V3D_TMU_TYPE_RGB565:                          lfmt = GFX_LFMT_B5G6R5_UNORM; break;
   case V3D_TMU_TYPE_RGBA4:                           lfmt = GFX_LFMT_A4B4G4R4_UNORM; break;
   case V3D_TMU_TYPE_RGB5_A1:                         lfmt = GFX_LFMT_A1B5G5R5_UNORM; break;
   case V3D_TMU_TYPE_RGB10_A2:                        lfmt = GFX_LFMT_R10G10B10A2_UNORM; break;
   case V3D_TMU_TYPE_RGB10_A2UI:                      lfmt = GFX_LFMT_R10G10B10A2_UINT; break;
   case V3D_TMU_TYPE_R16:                             lfmt = GFX_LFMT_R16_UNORM; break;
   case V3D_TMU_TYPE_R16_SNORM:                       lfmt = GFX_LFMT_R16_SNORM; break;
   case V3D_TMU_TYPE_R16I:                            lfmt = GFX_LFMT_R16_INT; break;
   case V3D_TMU_TYPE_R16UI:                           lfmt = GFX_LFMT_R16_UINT; break;
   case V3D_TMU_TYPE_RG16:                            lfmt = GFX_LFMT_R16_G16_UNORM; break;
   case V3D_TMU_TYPE_RG16_SNORM:                      lfmt = GFX_LFMT_R16_G16_SNORM; break;
   case V3D_TMU_TYPE_RG16I:                           lfmt = GFX_LFMT_R16_G16_INT; break;
   case V3D_TMU_TYPE_RG16UI:                          lfmt = GFX_LFMT_R16_G16_UINT; break;
   case V3D_TMU_TYPE_RGBA16:                          lfmt = GFX_LFMT_R16_G16_B16_A16_UNORM; break;
   case V3D_TMU_TYPE_RGBA16_SNORM:                    lfmt = GFX_LFMT_R16_G16_B16_A16_SNORM; break;
   case V3D_TMU_TYPE_RGBA16I:                         lfmt = GFX_LFMT_R16_G16_B16_A16_INT; break;
   case V3D_TMU_TYPE_RGBA16UI:                        lfmt = GFX_LFMT_R16_G16_B16_A16_UINT; break;
   case V3D_TMU_TYPE_R16F:                            lfmt = GFX_LFMT_R16_FLOAT; break;
   case V3D_TMU_TYPE_RG16F:                           lfmt = GFX_LFMT_R16_G16_FLOAT; break;
   case V3D_TMU_TYPE_RGBA16F:                         lfmt = GFX_LFMT_R16_G16_B16_A16_FLOAT; break;
   case V3D_TMU_TYPE_R11F_G11F_B10F:                  lfmt = GFX_LFMT_R11G11B10_UFLOAT; break;
   case V3D_TMU_TYPE_RGB9_E5:                         lfmt = GFX_LFMT_R9G9B9SHAREDEXP5_UFLOAT; break;
   case V3D_TMU_TYPE_DEPTH_COMP16:                    lfmt = GFX_LFMT_R16_UNORM; break;
   case V3D_TMU_TYPE_DEPTH_COMP24:                    lfmt = GFX_LFMT_R24X8_UNORM; break;
   case V3D_TMU_TYPE_DEPTH_COMP32F:                   lfmt = GFX_LFMT_R32_FLOAT; break;
   case V3D_TMU_TYPE_DEPTH24_X8:                      lfmt = GFX_LFMT_X8R24_UNORM; break;
   case V3D_TMU_TYPE_R4:                              lfmt = GFX_LFMT_R4_UNORM; break;
   case V3D_TMU_TYPE_R1:                              lfmt = GFX_LFMT_R1_UNORM; break;
   case V3D_TMU_TYPE_C_RGB8_ETC2:                     lfmt = GFX_LFMT_ETC2_RGB_UNORM; break;
   case V3D_TMU_TYPE_C_RGB8_PUNCHTHROUGH_ALPHA1_ETC2: lfmt = GFX_LFMT_PUNCHTHROUGH_ETC2_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_R11_EAC:                       lfmt = GFX_LFMT_EAC_R_UNORM; break;
   case V3D_TMU_TYPE_C_SIGNED_R11_EAC:                lfmt = GFX_LFMT_EAC_R_SNORM; break;
   case V3D_TMU_TYPE_C_RG11_EAC:                      lfmt = GFX_LFMT_EAC_EAC_RG_UNORM; break;
   case V3D_TMU_TYPE_C_SIGNED_RG11_EAC:               lfmt = GFX_LFMT_EAC_EAC_RG_SNORM; break;
   case V3D_TMU_TYPE_C_RGBA8_ETC2_EAC:                lfmt = GFX_LFMT_ETC2_EAC_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_BC1:                           lfmt = GFX_LFMT_BC1_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_BC2:                           lfmt = GFX_LFMT_BC2_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_BC3:                           lfmt = GFX_LFMT_BC3_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_4X4:                      lfmt = GFX_LFMT_ASTC4X4_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_5X4:                      lfmt = GFX_LFMT_ASTC5X4_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_5X5:                      lfmt = GFX_LFMT_ASTC5X5_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_6X5:                      lfmt = GFX_LFMT_ASTC6X5_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_6X6:                      lfmt = GFX_LFMT_ASTC6X6_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_8X5:                      lfmt = GFX_LFMT_ASTC8X5_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_8X6:                      lfmt = GFX_LFMT_ASTC8X6_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_8X8:                      lfmt = GFX_LFMT_ASTC8X8_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_10X5:                     lfmt = GFX_LFMT_ASTC10X5_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_10X6:                     lfmt = GFX_LFMT_ASTC10X6_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_10X8:                     lfmt = GFX_LFMT_ASTC10X8_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_10X10:                    lfmt = GFX_LFMT_ASTC10X10_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_12X10:                    lfmt = GFX_LFMT_ASTC12X10_RGBA_UNORM; break;
   case V3D_TMU_TYPE_C_ASTC_12X12:                    lfmt = GFX_LFMT_ASTC12X12_RGBA_UNORM; break;
   case V3D_TMU_TYPE_YCBCR_LUMA:                      not_impl(); break; /* TODO */
   case V3D_TMU_TYPE_YCBCR_420_CHROMA:                not_impl(); break; /* TODO */
   case V3D_TMU_TYPE_S8:                              lfmt = GFX_LFMT_R8_UINT; break;
   case V3D_TMU_TYPE_S16:                             lfmt = GFX_LFMT_R16_UINT; break;
   case V3D_TMU_TYPE_R32F:                            lfmt = GFX_LFMT_R32_FLOAT; break;
   case V3D_TMU_TYPE_R32I:                            lfmt = GFX_LFMT_R32_INT; break;
   case V3D_TMU_TYPE_R32UI:                           lfmt = GFX_LFMT_R32_UINT; break;
   case V3D_TMU_TYPE_RG32F:                           lfmt = GFX_LFMT_R32_G32_FLOAT; break;
   case V3D_TMU_TYPE_RG32I:                           lfmt = GFX_LFMT_R32_G32_INT; break;
   case V3D_TMU_TYPE_RG32UI:                          lfmt = GFX_LFMT_R32_G32_UINT; break;
   case V3D_TMU_TYPE_RGBA32F:                         lfmt = GFX_LFMT_R32_G32_B32_A32_FLOAT; break;
   case V3D_TMU_TYPE_RGBA32I:                         lfmt = GFX_LFMT_R32_G32_B32_A32_INT; break;
   case V3D_TMU_TYPE_RGBA32UI:                        lfmt = GFX_LFMT_R32_G32_B32_A32_UINT; break;
   default:                                           unreachable();
   }

   if (srgb)
   {
      assert(v3d_tmu_type_supports_srgb(tmu_type, v3d_version));
      assert(gfx_lfmt_get_type(&lfmt) == GFX_LFMT_TYPE_UNORM);
      if (v3d_tmu_get_num_channels(tmu_type) == 4)
         gfx_lfmt_set_type(&lfmt, GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM);
      else
         gfx_lfmt_set_type(&lfmt, GFX_LFMT_TYPE_SRGB);
   }

   return lfmt;
}

GFX_LFMT_DIMS_T gfx_lfmt_translate_from_tmu_ltype(v3d_tmu_ltype_t ltype)
{
   return gfx_lfmt_dims_to_enum(v3d_tmu_ltype_num_dims(ltype));
}

/** TFU */

static void reorder_tfu_fmts(
   uint32_t num_planes, GFX_LFMT_T fmts[],
   v3d_tfu_type_t tfu_type, v3d_tfu_rgbord_t rgbord, bool srgb)
{
   GFX_LFMT_CHANNELS_T chs = gfx_lfmt_get_channels(&fmts[0]);
   if (v3d_is_tfu_ttype_yuv(tfu_type))
   {
      switch (num_planes)
      {
      case 1:
         assert(chs == GFX_LFMT_CHANNELS_YUYV);
         switch (rgbord)
         {
         case V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV: break;
         case V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU: chs = GFX_LFMT_CHANNELS_VYUY; break;
         case V3D_TFU_RGBORD_ARGB_OR_YYUV:          chs = GFX_LFMT_CHANNELS_YYUV; break;
         case V3D_TFU_RGBORD_BGRA_OR_VUYY:          chs = GFX_LFMT_CHANNELS_VUYY; break;
         default: unreachable();
         }
         break;
      case 2:
      {
         GFX_LFMT_CHANNELS_T chs1 = gfx_lfmt_get_channels(&fmts[1]);
         assert(chs1 == GFX_LFMT_CHANNELS_UV);
         switch (rgbord)
         {
         case V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV: break;
         case V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU: chs1 = GFX_LFMT_CHANNELS_VU; break;
         default: unreachable();
         }
         gfx_lfmt_set_channels(&fmts[1], chs1);
         break;
      }
      case 3:
         /* Channel reordering not supported */
         assert(rgbord == V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV);
         break;
      default: unreachable();
      }
   }
   else
   {
      assert(num_planes == 1);
      switch (chs)
      {
      case GFX_LFMT_CHANNELS_RG:
         switch (rgbord)
         {
         case V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV: break;
         case V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU: chs = GFX_LFMT_CHANNELS_GR; break;
         default: unreachable();
         }
         break;
      case GFX_LFMT_CHANNELS_RGBA:
         switch (rgbord)
         {
         case V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV: break;
         case V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU: chs = GFX_LFMT_CHANNELS_ABGR; break;
         case V3D_TFU_RGBORD_ARGB_OR_YYUV:          chs = GFX_LFMT_CHANNELS_ARGB; break;
         case V3D_TFU_RGBORD_BGRA_OR_VUYY:          chs = GFX_LFMT_CHANNELS_BGRA; break;
         default: unreachable();
         }
         break;
      case GFX_LFMT_CHANNELS_ABGR:
         switch (rgbord)
         {
         case V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV: break;
         case V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU: chs = GFX_LFMT_CHANNELS_RGBA; break;
         case V3D_TFU_RGBORD_ARGB_OR_YYUV:          chs = GFX_LFMT_CHANNELS_BGRA; break;
         case V3D_TFU_RGBORD_BGRA_OR_VUYY:          chs = GFX_LFMT_CHANNELS_ARGB; break;
         default: unreachable();
         }
         break;
      default:
         /* formats like r8 and bgr565 disregard rgbord */
         break;
      }

      if (srgb)
      {
         GFX_LFMT_TYPE_T type = gfx_lfmt_get_type(&fmts[0]);
         switch (type)
         {
         case GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM:
            switch (rgbord)
            {
            case V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV:
            case V3D_TFU_RGBORD_BGRA_OR_VUYY:
               break;
            case V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU:
            case V3D_TFU_RGBORD_ARGB_OR_YYUV:
               type = GFX_LFMT_TYPE_UNORM_SRGB_SRGB_SRGB;
               break;
            default: unreachable();
            }
            break;
         case GFX_LFMT_TYPE_SRGB: break;
         default: unreachable();
         }
         gfx_lfmt_set_type(&fmts[0], type);
      }
   }
   gfx_lfmt_set_channels(&fmts[0], chs);
}

void gfx_lfmt_translate_from_tfu_type(
   GFX_LFMT_T *fmts, uint32_t *num_planes, v3d_tfu_yuv_col_space_t *yuv_col_space,
   v3d_tfu_type_t tfu_type, v3d_tfu_rgbord_t rgbord, bool srgb, int v3d_version,
   bool is_sand)
{
   gfx_buffer_lfmts_none(fmts);
   *num_planes = 1;

   switch (tfu_type)
   {
   case V3D_TFU_TYPE_YUV_420_3PLANE_REC709:
   case V3D_TFU_TYPE_YUV_420_3PLANE_REC601:
   case V3D_TFU_TYPE_YUV_420_3PLANE_JPEG:
      fmts[0] = GFX_LFMT_Y8_UNORM;
      fmts[1] = GFX_LFMT_V8_2X2_UNORM;
      fmts[2] = GFX_LFMT_U8_2X2_UNORM;
      *num_planes = 3;
      break;
   case V3D_TFU_TYPE_YUV_420_2PLANE_REC709:
   case V3D_TFU_TYPE_YUV_420_2PLANE_REC601:
   case V3D_TFU_TYPE_YUV_420_2PLANE_JPEG:
      fmts[0] = GFX_LFMT_Y8_UNORM;
      fmts[1] = GFX_LFMT_U8_V8_2X2_UNORM;
      *num_planes = 2;
      break;
   case V3D_TFU_TYPE_YUV_422_2PLANE_REC709:
   case V3D_TFU_TYPE_YUV_422_2PLANE_REC601:
   case V3D_TFU_TYPE_YUV_422_2PLANE_JPEG:
      fmts[0] = GFX_LFMT_Y8_UNORM;
      fmts[1] = GFX_LFMT_U8_V8_2X1_UNORM;
      *num_planes = 2;
      break;
   case V3D_TFU_TYPE_YUYV_422_1PLANE_REC709:
   case V3D_TFU_TYPE_YUYV_422_1PLANE_REC601:
   case V3D_TFU_TYPE_YUYV_422_1PLANE_JPEG:
      fmts[0] = GFX_LFMT_Y8_U8_Y8_V8_2X1_UNORM;
      break;
   default:
      assert(!is_sand);
      /* the tfu ttype should be a valid tmu ttype */
      fmts[0] = gfx_lfmt_translate_from_tmu_type((v3d_tmu_type_t)tfu_type, srgb, v3d_version);
   }

   if (is_sand)
      /* See GFXH-1344 */
      rgbord = v3d_tfu_reverse_rgbord(rgbord);
   reorder_tfu_fmts(*num_planes, fmts, tfu_type, rgbord, srgb);

   *yuv_col_space = v3d_yuv_col_space_from_tfu_type(tfu_type);
}

static bool swizzles_eq_or_inv(v3d_tmu_swizzle_t a, v3d_tmu_swizzle_t b)
{
   return (a == V3D_TMU_SWIZZLE_INVALID) || (b == V3D_TMU_SWIZZLE_INVALID) || (a == b);
}

static v3d_tfu_rgbord_t maybe_get_rgbord(uint32_t num_channels,
   const v3d_tmu_swizzle_t src_swizzles[4],
   const v3d_tmu_swizzle_t dst_swizzles[4])
{
   /* cswizzle = combined swizzle
    *
    * We want TFU to do:
    * for i in 0..num_channels-1:
    *    if cswizzle[i] != V3D_TMU_SWIZZLE_INVALID:
    *       dst[i] = apply_swizzle(src, cswizzle[i])
    *    else:
    *       dst[i] = whatever */

   v3d_tmu_swizzle_t cswizzle[4] = {
      V3D_TMU_SWIZZLE_INVALID,
      V3D_TMU_SWIZZLE_INVALID,
      V3D_TMU_SWIZZLE_INVALID,
      V3D_TMU_SWIZZLE_INVALID};
   for (unsigned i = 0; i != 4; ++i)
   {
      v3d_tmu_swizzle_t ss = src_swizzles[i];
      v3d_tmu_swizzle_t ds = dst_swizzles[i];
      if ((ds >= V3D_TMU_SWIZZLE_R) && (ds <= V3D_TMU_SWIZZLE_A))
      {
         unsigned j = ds - V3D_TMU_SWIZZLE_R;
         if (j < num_channels)
            cswizzle[j] = ss;
      }
   }

   #define CSWIZZLE_MATCHES(R, G, B, A) (                      \
      swizzles_eq_or_inv(cswizzle[0], V3D_TMU_SWIZZLE_##R) &&  \
      swizzles_eq_or_inv(cswizzle[1], V3D_TMU_SWIZZLE_##G) &&  \
      swizzles_eq_or_inv(cswizzle[2], V3D_TMU_SWIZZLE_##B) &&  \
      swizzles_eq_or_inv(cswizzle[3], V3D_TMU_SWIZZLE_##A))

   /* Return the rgbord that has the same effect as cswizzle, if there is one */
   if (CSWIZZLE_MATCHES(R, G, B, A))
      return V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV;
   if (((num_channels == 4) && CSWIZZLE_MATCHES(A, B, G, R)) ||
      ((num_channels == 2) && CSWIZZLE_MATCHES(G, R, INVALID, INVALID)))
      return V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU;
   if ((num_channels == 4) && CSWIZZLE_MATCHES(G, B, A, R))
      return V3D_TFU_RGBORD_ARGB_OR_YYUV;
   if ((num_channels == 4) && CSWIZZLE_MATCHES(B, G, R, A))
      return V3D_TFU_RGBORD_BGRA_OR_VUYY;

   #undef CSWIZZLE_MATCHES

   return V3D_TFU_RGBORD_INVALID;
}

/* The TFU does not support the integer types but the equivalent normalised
 * types will do considering the only operations the TFU can do are copying and
 * subsampling. */
static GFX_LFMT_T fudge_lfmt_for_tfu(GFX_LFMT_T lfmt)
{
   switch (gfx_lfmt_get_base(&lfmt))
   {
   case GFX_LFMT_BASE_C32:
   case GFX_LFMT_BASE_C32_C32:
   case GFX_LFMT_BASE_C32_C32_C32_C32:
      /* 32-bit types cannot be subsampled, only copied. So float will do. */
      gfx_lfmt_set_type(&lfmt, GFX_LFMT_TYPE_FLOAT);
      return lfmt;
   default:
      return gfx_lfmt_int_to_norm(lfmt);
   }
}

bool gfx_lfmt_maybe_translate_to_tfu_type(
   v3d_tfu_type_t *tfu_type, bool *srgb, v3d_tfu_rgbord_t *rgbord,
   const GFX_LFMT_T *src_lfmts, uint32_t src_num_planes,
   v3d_tfu_yuv_col_space_t src_yuv_col_space,
   GFX_LFMT_T dst_lfmt, int v3d_version)
{
   GFX_LFMT_TMU_TRANSLATION_T dst_t;
   if (!gfx_lfmt_maybe_translate_tmu(&dst_t, fudge_lfmt_for_tfu(dst_lfmt),
         GFX_LFMT_TMU_DEPTH_DONT_CARE, v3d_version))
      return false;
   assert(dst_t.ret == GFX_LFMT_TMU_RET_AUTO);

   /* deal with the src YUV cases */
   if (gfx_lfmt_has_y(src_lfmts[0]))
   {
      /* Only support yuv -> rgba8 */
      if (dst_t.type != V3D_TMU_TYPE_RGBA8 ||
          dst_t.swizzles[0] != V3D_TMU_SWIZZLE_R ||
          dst_t.swizzles[1] != V3D_TMU_SWIZZLE_G ||
          dst_t.swizzles[2] != V3D_TMU_SWIZZLE_B ||
          dst_t.swizzles[3] != V3D_TMU_SWIZZLE_A)
         return false;

      *rgbord = V3D_TFU_RGBORD_RGBA_OR_RG_YUYV_OR_UV;
      switch (gfx_lfmt_fmt(src_lfmts[0]))
      {
      case GFX_LFMT_Y8_UNORM:
         switch (src_num_planes)
         {
         case 2:
            switch (gfx_lfmt_fmt(src_lfmts[1]))
            {
            case GFX_LFMT_V8_U8_2X2_UNORM:
               *rgbord = V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU;
               /* Fall through... */
            case GFX_LFMT_U8_V8_2X2_UNORM:
               *tfu_type = v3d_tfu_type_yuv_420_2plane(src_yuv_col_space);
               break;
            case GFX_LFMT_V8_U8_2X1_UNORM:
               *rgbord = V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU;
               /* Fall through... */
            case GFX_LFMT_U8_V8_2X1_UNORM:
               *tfu_type = v3d_tfu_type_yuv_422_2plane(src_yuv_col_space);
               break;
            default:
               return false;
            }
            break;
         case 3:
            if ((v3d_version < V3D_VER_TFU_YV12) ||
               (gfx_lfmt_fmt(src_lfmts[1]) != GFX_LFMT_V8_2X2_UNORM) ||
               (gfx_lfmt_fmt(src_lfmts[2]) != GFX_LFMT_U8_2X2_UNORM))
               return false;
            *tfu_type = v3d_tfu_type_yuv_420_3plane(src_yuv_col_space);
            break;
         default:
            return false;
         }
         break;
      case GFX_LFMT_V8_Y8_U8_Y8_2X1_UNORM:
         *rgbord = V3D_TFU_RGBORD_ABGR_OR_GR_VYUY_OR_VU;
         /* Fall through... */
      case GFX_LFMT_Y8_U8_Y8_V8_2X1_UNORM:
         if (src_num_planes != 1)
            return false;
         *tfu_type = v3d_tfu_type_yuyv_422_1plane(src_yuv_col_space);
         break;
      default:
         return false;
      }

      if (gfx_lfmt_is_sand_family(src_lfmts[0]))
         /* See GFXH-1344 */
         *rgbord = v3d_tfu_reverse_rgbord(*rgbord);

      *srgb = dst_t.srgb;

      return true;
   }

   if ((src_num_planes > 1) || gfx_lfmt_is_sand_family(src_lfmts[0]))
      return false;

   GFX_LFMT_T src_out_fmt;
   v3d_tmu_type_t src_tmu_type = try_get_tmu_type_and_out_fmt(
      &src_out_fmt, fudge_lfmt_for_tfu(src_lfmts[0]));
   if (src_tmu_type != dst_t.type)
      return false;
   *tfu_type = (v3d_tfu_type_t)src_tmu_type;

   v3d_tmu_swizzle_t src_swizzles[4];
   if (!try_get_swizzles(src_swizzles, src_out_fmt))
      return false;

   uint32_t num_channels = v3d_tmu_get_num_channels(src_tmu_type);
   gfx_lfmt_check_num_slots_eq(src_out_fmt, num_channels);

   *rgbord = maybe_get_rgbord(num_channels, src_swizzles, dst_t.swizzles);
   if (*rgbord == V3D_TFU_RGBORD_INVALID)
      return false;

   *srgb = gfx_lfmt_contains_srgb(src_out_fmt);
   return (*srgb == dst_t.srgb) &&
      (!*srgb || srgb_ok(src_tmu_type, src_out_fmt, *rgbord, v3d_version));
}

extern void gfx_lfmt_translate_to_tfu_type(
   v3d_tfu_type_t *tfu_type, bool *srgb, v3d_tfu_rgbord_t *rgbord,
   const GFX_LFMT_T *src_lfmts, uint32_t src_num_planes,
   v3d_tfu_yuv_col_space_t src_yuv_col_space,
   GFX_LFMT_T dst_lfmt, int v3d_version)
{
   bool ok = gfx_lfmt_maybe_translate_to_tfu_type(tfu_type, srgb, rgbord, src_lfmts,
      src_num_planes, src_yuv_col_space, dst_lfmt, v3d_version);
   assert(ok);
}
