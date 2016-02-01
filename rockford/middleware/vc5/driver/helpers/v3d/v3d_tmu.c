/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "v3d_common.h"
#include "v3d_tmu.h"
#include "v3d_align.h"

#include "helpers/gfx/gfx_lfmt_translate_v3d.h"

v3d_tmu_blend_type_t v3d_maybe_get_tmu_blend_type(
   v3d_tmu_type_t type, bool srgb, bool shadow, bool output_32,
   int v3d_version)
{
   switch (type)
   {
   case V3D_TMU_TYPE_R8:
   case V3D_TMU_TYPE_RG8:
   case V3D_TMU_TYPE_RGBA8:
      assert(!shadow);
      return ((v3d_version < V3D_MAKE_VER(3, 3)) || srgb || !output_32) ?
         V3D_TMU_BLEND_TYPE_FLOAT16 : V3D_TMU_BLEND_TYPE_UNORM16;
   case V3D_TMU_TYPE_R8_SNORM:
   case V3D_TMU_TYPE_RG8_SNORM:
   case V3D_TMU_TYPE_RGBA8_SNORM:
      assert(!srgb && !shadow);
      return ((v3d_version < V3D_MAKE_VER(3, 3)) || !output_32) ?
         V3D_TMU_BLEND_TYPE_FLOAT16 : V3D_TMU_BLEND_TYPE_SNORM15;
   case V3D_TMU_TYPE_RGB565:
   case V3D_TMU_TYPE_RGBA4:
   case V3D_TMU_TYPE_RGB5_A1:
   case V3D_TMU_TYPE_RGB10_A2:
   case V3D_TMU_TYPE_R16F:
   case V3D_TMU_TYPE_RG16F:
   case V3D_TMU_TYPE_RGBA16F:
   case V3D_TMU_TYPE_R11F_G11F_B10F:
   case V3D_TMU_TYPE_RGB9_E5:
   case V3D_TMU_TYPE_R4:
   case V3D_TMU_TYPE_R1:
   case V3D_TMU_TYPE_C_R11_EAC:
   case V3D_TMU_TYPE_C_SIGNED_R11_EAC:
   case V3D_TMU_TYPE_C_RG11_EAC:
   case V3D_TMU_TYPE_C_SIGNED_RG11_EAC:
      assert(!srgb);
      /* Fall through... */
   case V3D_TMU_TYPE_C_RGB8_ETC2:
   case V3D_TMU_TYPE_C_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
   case V3D_TMU_TYPE_C_RGBA8_ETC2_EAC:
   case V3D_TMU_TYPE_C_BC1:
   case V3D_TMU_TYPE_C_BC2:
   case V3D_TMU_TYPE_C_BC3:
   case V3D_TMU_TYPE_C_ASTC_4X4:
   case V3D_TMU_TYPE_C_ASTC_5X4:
   case V3D_TMU_TYPE_C_ASTC_5X5:
   case V3D_TMU_TYPE_C_ASTC_6X5:
   case V3D_TMU_TYPE_C_ASTC_6X6:
   case V3D_TMU_TYPE_C_ASTC_8X5:
   case V3D_TMU_TYPE_C_ASTC_8X6:
   case V3D_TMU_TYPE_C_ASTC_8X8:
   case V3D_TMU_TYPE_C_ASTC_10X5:
   case V3D_TMU_TYPE_C_ASTC_10X6:
   case V3D_TMU_TYPE_C_ASTC_10X8:
   case V3D_TMU_TYPE_C_ASTC_10X10:
   case V3D_TMU_TYPE_C_ASTC_12X10:
   case V3D_TMU_TYPE_C_ASTC_12X12:
      assert(!shadow);
      return V3D_TMU_BLEND_TYPE_FLOAT16;
   case V3D_TMU_TYPE_R16:
   case V3D_TMU_TYPE_RG16:
   case V3D_TMU_TYPE_RGBA16:
      assert(!srgb && !shadow);
      return output_32 ? V3D_TMU_BLEND_TYPE_UNORM16 : V3D_TMU_BLEND_TYPE_FLOAT16;
   case V3D_TMU_TYPE_R16_SNORM:
   case V3D_TMU_TYPE_RG16_SNORM:
   case V3D_TMU_TYPE_RGBA16_SNORM:
      assert(!srgb && !shadow);
      return output_32 ? V3D_TMU_BLEND_TYPE_SNORM16 : V3D_TMU_BLEND_TYPE_FLOAT16;
   case V3D_TMU_TYPE_R32F:
   case V3D_TMU_TYPE_RG32F:
   case V3D_TMU_TYPE_RGBA32F:
      assert(!srgb && !shadow);
      return V3D_TMU_BLEND_TYPE_INVALID;
   case V3D_TMU_TYPE_R8I:
   case V3D_TMU_TYPE_R8UI:
   case V3D_TMU_TYPE_RG8I:
   case V3D_TMU_TYPE_RG8UI:
   case V3D_TMU_TYPE_RGBA8I:
   case V3D_TMU_TYPE_RGBA8UI:
   case V3D_TMU_TYPE_R16I:
   case V3D_TMU_TYPE_R16UI:
   case V3D_TMU_TYPE_RG16I:
   case V3D_TMU_TYPE_RG16UI:
   case V3D_TMU_TYPE_RGBA16I:
   case V3D_TMU_TYPE_RGBA16UI:
   case V3D_TMU_TYPE_R32I:
   case V3D_TMU_TYPE_R32UI:
   case V3D_TMU_TYPE_RG32I:
   case V3D_TMU_TYPE_RG32UI:
   case V3D_TMU_TYPE_RGBA32I:
   case V3D_TMU_TYPE_RGBA32UI:
   case V3D_TMU_TYPE_RGB10_A2UI:
   case V3D_TMU_TYPE_S8:
   case V3D_TMU_TYPE_S16:
   case V3D_TMU_TYPE_YCBCR_LUMA:
   case V3D_TMU_TYPE_YCBCR_420_CHROMA:
      assert(!srgb && !shadow);
      return V3D_TMU_BLEND_TYPE_INVALID;
   case V3D_TMU_TYPE_DEPTH_COMP16:
   case V3D_TMU_TYPE_DEPTH_COMP24:
   case V3D_TMU_TYPE_DEPTH_COMP32F:
   case V3D_TMU_TYPE_DEPTH24_X8:
      assert(!srgb);
      return shadow ? V3D_TMU_BLEND_TYPE_FLOAT16 : V3D_TMU_BLEND_TYPE_INVALID;
   default:
      unreachable();
      return V3D_TMU_BLEND_TYPE_INVALID;
   }
}

GFX_LFMT_T v3d_tmu_blend_fmt_from_type(v3d_tmu_blend_type_t blend_type)
{
   switch (blend_type)
   {
   case V3D_TMU_BLEND_TYPE_FLOAT16: return GFX_LFMT_R16_G16_B16_A16_FLOAT;
   case V3D_TMU_BLEND_TYPE_UNORM16: return GFX_LFMT_R16_G16_B16_A16_UNORM;
   case V3D_TMU_BLEND_TYPE_SNORM16: return GFX_LFMT_R16_G16_B16_A16_SNORM;
   case V3D_TMU_BLEND_TYPE_SNORM15: return GFX_LFMT_R15X1_G15X1_B15X1_A15X1_SNORM;
   default:                         unreachable(); return GFX_LFMT_NONE;
   }
}

GFX_LFMT_T v3d_get_tmu_blend_fmt(
   v3d_tmu_type_t type, bool srgb, bool shadow, bool output_32,
   int v3d_version)
{
   v3d_tmu_blend_type_t blend_type = v3d_maybe_get_tmu_blend_type(
      type, srgb, shadow, output_32, v3d_version);
   if (blend_type != V3D_TMU_BLEND_TYPE_INVALID)
      return v3d_tmu_blend_fmt_from_type(blend_type);

   assert(!srgb); /* All sRGB types are blendable */
   assert(!shadow); /* Depth compare result always blendable */

   switch (type)
   {
   case V3D_TMU_TYPE_R8I:
   case V3D_TMU_TYPE_RG8I:
   case V3D_TMU_TYPE_RGBA8I:
   case V3D_TMU_TYPE_R16I:
   case V3D_TMU_TYPE_RG16I:
   case V3D_TMU_TYPE_RGBA16I:
      assert(v3d_version >= V3D_MAKE_VER(3,3));
      return GFX_LFMT_R16_G16_B16_A16_INT;

   case V3D_TMU_TYPE_R32I:
   case V3D_TMU_TYPE_RG32I:
   case V3D_TMU_TYPE_RGBA32I:
      assert(v3d_version >= V3D_MAKE_VER(3,3));
      return output_32 ? GFX_LFMT_R32_G32_B32_A32_INT : GFX_LFMT_R16_G16_B16_A16_INT;

   case V3D_TMU_TYPE_S8:      /* Pad to 4 channels for consistency. The   */
   case V3D_TMU_TYPE_S16:     /* correct channels are returned to the QPU */
      return GFX_LFMT_R32_G32_B32_A32_UINT;

   case V3D_TMU_TYPE_R8UI:
   case V3D_TMU_TYPE_RG8UI:
   case V3D_TMU_TYPE_RGBA8UI:
   case V3D_TMU_TYPE_R16UI:
   case V3D_TMU_TYPE_RG16UI:
   case V3D_TMU_TYPE_RGBA16UI:
   case V3D_TMU_TYPE_RGB10_A2UI:
      assert(v3d_version >= V3D_MAKE_VER(3,3));
      return GFX_LFMT_R16_G16_B16_A16_UINT;

   case V3D_TMU_TYPE_R32UI:
   case V3D_TMU_TYPE_RG32UI:
   case V3D_TMU_TYPE_RGBA32UI:
      assert(v3d_version >= V3D_MAKE_VER(3,3));
      return output_32 ? GFX_LFMT_R32_G32_B32_A32_UINT : GFX_LFMT_R16_G16_B16_A16_UINT;

   case V3D_TMU_TYPE_R32F:
   case V3D_TMU_TYPE_RG32F:
   case V3D_TMU_TYPE_RGBA32F:
   // Depth types when shadow compare is disabled
   case V3D_TMU_TYPE_DEPTH_COMP16:
   case V3D_TMU_TYPE_DEPTH_COMP24:
   case V3D_TMU_TYPE_DEPTH_COMP32F:
   case V3D_TMU_TYPE_DEPTH24_X8:
      /* Note that HW does not supporting blending of these types, even though
       * when !output_32 we convert to float16 before the blend pipeline stage */
      return output_32 ? GFX_LFMT_R32_G32_B32_A32_FLOAT : GFX_LFMT_R16_G16_B16_A16_FLOAT;

   case V3D_TMU_TYPE_YCBCR_LUMA:
   case V3D_TMU_TYPE_YCBCR_420_CHROMA:
      // TODO
      not_impl();
      return GFX_LFMT_NONE;

   default:
      unreachable();
      return GFX_LFMT_NONE;
   }
}

/* Get the number of words required to return num_channels channels of data
 * from the TMU with the specified configuration */
static uint32_t get_num_words(uint32_t num_channels,
   v3d_tmu_type_t type, bool coefficient, bool output_32)
{
   uint32_t half_words_per_channel;
   if (!coefficient && ((type == V3D_TMU_TYPE_S8) || (type == V3D_TMU_TYPE_S16)))
      /* Output type ignored for S8/S16 (unless in coefficient-fetch mode)... */
      half_words_per_channel = 2;
   else
      half_words_per_channel = output_32 ? 2 : 1;

   uint32_t half_words = num_channels * half_words_per_channel;
   return (half_words + 1) / 2;
}

uint32_t v3d_tmu_get_num_channels(v3d_tmu_type_t type)
{
   switch (type)
   {
   case V3D_TMU_TYPE_R8:
   case V3D_TMU_TYPE_R8_SNORM:
   case V3D_TMU_TYPE_R8I:
   case V3D_TMU_TYPE_R8UI:
   case V3D_TMU_TYPE_R16:
   case V3D_TMU_TYPE_R16_SNORM:
   case V3D_TMU_TYPE_R16F:
   case V3D_TMU_TYPE_R16I:
   case V3D_TMU_TYPE_R16UI:
   case V3D_TMU_TYPE_DEPTH_COMP16:
   case V3D_TMU_TYPE_DEPTH_COMP24:
   case V3D_TMU_TYPE_DEPTH_COMP32F:
   case V3D_TMU_TYPE_DEPTH24_X8:
   case V3D_TMU_TYPE_R4:
   case V3D_TMU_TYPE_R1:
   case V3D_TMU_TYPE_S8:
   case V3D_TMU_TYPE_S16:
   case V3D_TMU_TYPE_R32F:
   case V3D_TMU_TYPE_R32I:
   case V3D_TMU_TYPE_R32UI:
   case V3D_TMU_TYPE_C_R11_EAC:
   case V3D_TMU_TYPE_C_SIGNED_R11_EAC:
      return 1;
   case V3D_TMU_TYPE_RG8:
   case V3D_TMU_TYPE_RG8_SNORM:
   case V3D_TMU_TYPE_RG8I:
   case V3D_TMU_TYPE_RG8UI:
   case V3D_TMU_TYPE_RG16:
   case V3D_TMU_TYPE_RG16_SNORM:
   case V3D_TMU_TYPE_RG16F:
   case V3D_TMU_TYPE_RG16I:
   case V3D_TMU_TYPE_RG16UI:
   case V3D_TMU_TYPE_RG32F:
   case V3D_TMU_TYPE_RG32I:
   case V3D_TMU_TYPE_RG32UI:
   case V3D_TMU_TYPE_C_RG11_EAC:
   case V3D_TMU_TYPE_C_SIGNED_RG11_EAC:
      return 2;
   case V3D_TMU_TYPE_RGBA8:
   case V3D_TMU_TYPE_RGBA8_SNORM:
   case V3D_TMU_TYPE_RGBA8I:
   case V3D_TMU_TYPE_RGBA8UI:
   case V3D_TMU_TYPE_RGBA4:
   case V3D_TMU_TYPE_RGB5_A1:
   case V3D_TMU_TYPE_RGB10_A2:
   case V3D_TMU_TYPE_RGBA16:
   case V3D_TMU_TYPE_RGBA16_SNORM:
   case V3D_TMU_TYPE_RGBA16I:
   case V3D_TMU_TYPE_RGBA16UI:
   case V3D_TMU_TYPE_RGBA16F:
   case V3D_TMU_TYPE_RGBA32F:
   case V3D_TMU_TYPE_RGBA32I:
   case V3D_TMU_TYPE_RGBA32UI:
   case V3D_TMU_TYPE_RGB10_A2UI:
   case V3D_TMU_TYPE_C_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
   case V3D_TMU_TYPE_C_RGBA8_ETC2_EAC:
   case V3D_TMU_TYPE_C_BC1:
   case V3D_TMU_TYPE_C_BC2:
   case V3D_TMU_TYPE_C_BC3:
   case V3D_TMU_TYPE_C_ASTC_4X4:
   case V3D_TMU_TYPE_C_ASTC_5X4:
   case V3D_TMU_TYPE_C_ASTC_5X5:
   case V3D_TMU_TYPE_C_ASTC_6X5:
   case V3D_TMU_TYPE_C_ASTC_6X6:
   case V3D_TMU_TYPE_C_ASTC_8X5:
   case V3D_TMU_TYPE_C_ASTC_8X6:
   case V3D_TMU_TYPE_C_ASTC_8X8:
   case V3D_TMU_TYPE_C_ASTC_10X5:
   case V3D_TMU_TYPE_C_ASTC_10X6:
   case V3D_TMU_TYPE_C_ASTC_10X8:
   case V3D_TMU_TYPE_C_ASTC_10X10:
   case V3D_TMU_TYPE_C_ASTC_12X10:
   case V3D_TMU_TYPE_C_ASTC_12X12:
      return 4;
   case V3D_TMU_TYPE_RGB565:
   case V3D_TMU_TYPE_R11F_G11F_B10F:
   case V3D_TMU_TYPE_RGB9_E5:
   case V3D_TMU_TYPE_C_RGB8_ETC2:
      return 3;
   default:
      unreachable();
      return 0;
   }
}

#if !V3D_HAS_NEW_TMU_CFG

bool v3d_tmu_auto_output_32(
   v3d_tmu_type_t type, bool shadow, bool coefficient)
{
   /* type/shadow ignored when in coefficient-fetch mode -- the default output
    * type is always F16 */
   if (coefficient)
      return false;

   switch (type)
   {
   case V3D_TMU_TYPE_R8:
   case V3D_TMU_TYPE_R8_SNORM:
   case V3D_TMU_TYPE_RG8:
   case V3D_TMU_TYPE_RG8_SNORM:
   case V3D_TMU_TYPE_RGBA8:
   case V3D_TMU_TYPE_RGBA8_SNORM:
   case V3D_TMU_TYPE_RGB565:
   case V3D_TMU_TYPE_RGBA4:
   case V3D_TMU_TYPE_RGB5_A1:
   case V3D_TMU_TYPE_RGB10_A2:
   case V3D_TMU_TYPE_R16F:
   case V3D_TMU_TYPE_RG16F:
   case V3D_TMU_TYPE_RGBA16F:
   case V3D_TMU_TYPE_R11F_G11F_B10F:
   case V3D_TMU_TYPE_RGB9_E5:
   case V3D_TMU_TYPE_R4:
   case V3D_TMU_TYPE_R1:
   case V3D_TMU_TYPE_C_RGB8_ETC2:
   case V3D_TMU_TYPE_C_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
   case V3D_TMU_TYPE_C_R11_EAC:
   case V3D_TMU_TYPE_C_SIGNED_R11_EAC:
   case V3D_TMU_TYPE_C_RG11_EAC:
   case V3D_TMU_TYPE_C_SIGNED_RG11_EAC:
   case V3D_TMU_TYPE_C_RGBA8_ETC2_EAC:
   case V3D_TMU_TYPE_YCBCR_LUMA:
   case V3D_TMU_TYPE_YCBCR_420_CHROMA:
   case V3D_TMU_TYPE_C_BC1:
   case V3D_TMU_TYPE_C_BC2:
   case V3D_TMU_TYPE_C_BC3:
   case V3D_TMU_TYPE_C_ASTC_4X4:
   case V3D_TMU_TYPE_C_ASTC_5X4:
   case V3D_TMU_TYPE_C_ASTC_5X5:
   case V3D_TMU_TYPE_C_ASTC_6X5:
   case V3D_TMU_TYPE_C_ASTC_6X6:
   case V3D_TMU_TYPE_C_ASTC_8X5:
   case V3D_TMU_TYPE_C_ASTC_8X6:
   case V3D_TMU_TYPE_C_ASTC_8X8:
   case V3D_TMU_TYPE_C_ASTC_10X5:
   case V3D_TMU_TYPE_C_ASTC_10X6:
   case V3D_TMU_TYPE_C_ASTC_10X8:
   case V3D_TMU_TYPE_C_ASTC_10X10:
   case V3D_TMU_TYPE_C_ASTC_12X10:
   case V3D_TMU_TYPE_C_ASTC_12X12:
      assert(!shadow);
      return false;
   case V3D_TMU_TYPE_R16:
   case V3D_TMU_TYPE_R16_SNORM:
   case V3D_TMU_TYPE_RG16:
   case V3D_TMU_TYPE_RG16_SNORM:
   case V3D_TMU_TYPE_RGBA16:
   case V3D_TMU_TYPE_RGBA16_SNORM:
   case V3D_TMU_TYPE_R32F:
   case V3D_TMU_TYPE_RG32F:
   case V3D_TMU_TYPE_RGBA32F:
      assert(!shadow);
      return true;
   case V3D_TMU_TYPE_DEPTH_COMP16:
   case V3D_TMU_TYPE_DEPTH_COMP24:
   case V3D_TMU_TYPE_DEPTH_COMP32F:
   case V3D_TMU_TYPE_DEPTH24_X8:
      return !shadow;
   case V3D_TMU_TYPE_S8:
   case V3D_TMU_TYPE_S16:
      /* Output type should have no effect on S8/S16 */
      return false;
   case V3D_TMU_TYPE_R8I:
   case V3D_TMU_TYPE_RG8I:
   case V3D_TMU_TYPE_RGBA8I:
   case V3D_TMU_TYPE_R8UI:
   case V3D_TMU_TYPE_RG8UI:
   case V3D_TMU_TYPE_RGBA8UI:
   case V3D_TMU_TYPE_R16I:
   case V3D_TMU_TYPE_RG16I:
   case V3D_TMU_TYPE_RGBA16I:
   case V3D_TMU_TYPE_R16UI:
   case V3D_TMU_TYPE_RG16UI:
   case V3D_TMU_TYPE_RGBA16UI:
   case V3D_TMU_TYPE_RGB10_A2UI:
      assert(!shadow);
      return false;
   case V3D_TMU_TYPE_R32I:
   case V3D_TMU_TYPE_RG32I:
   case V3D_TMU_TYPE_RGBA32I:
   case V3D_TMU_TYPE_R32UI:
   case V3D_TMU_TYPE_RG32UI:
   case V3D_TMU_TYPE_RGBA32UI:
      assert(!shadow);
      return true;
   default:
      unreachable();
      return false;
   }
}

uint32_t v3d_tmu_get_word_read_default(
   v3d_tmu_type_t type, bool misccfg_ovrtmuout)
{
   /* In cfg=0 mode, TMU returns as many words as are needed to cover all
    * channels present in texture data... */
   return get_num_words(v3d_tmu_get_num_channels(type),
      type, /*coefficient=*/false,
      !misccfg_ovrtmuout &&
         v3d_tmu_auto_output_32(type, /*shadow=*/false, /*coefficient=*/false));
}

#endif

uint32_t v3d_tmu_get_word_read_max(
   v3d_tmu_type_t type, bool coefficient, bool gather,
   bool output_32)
{
   /* Enabling both gather and coefficient-fetch doesn't make sense... */
   assert(!coefficient || !gather);

   /* Full 4 channels available in all cases except for regular S8/S16 lookup... */
   uint32_t num_channels;
   if (!coefficient && !gather &&
      ((type == V3D_TMU_TYPE_S8) || (type == V3D_TMU_TYPE_S16)))
      num_channels = 1;
   else
      num_channels = 4;

   return get_num_words(num_channels, type, coefficient, output_32);
}

bool v3d_tmu_type_supports_srgb(v3d_tmu_type_t type, int v3d_version)
{
   switch (type)
   {
   case V3D_TMU_TYPE_R8:
   case V3D_TMU_TYPE_RG8:
      return v3d_version >= V3D_MAKE_VER(3, 3);
   case V3D_TMU_TYPE_RGBA8:
   case V3D_TMU_TYPE_C_RGB8_ETC2:
   case V3D_TMU_TYPE_C_RGB8_PUNCHTHROUGH_ALPHA1_ETC2:
   case V3D_TMU_TYPE_C_RGBA8_ETC2_EAC:
   case V3D_TMU_TYPE_C_BC1:
   case V3D_TMU_TYPE_C_BC2:
   case V3D_TMU_TYPE_C_BC3:
   case V3D_TMU_TYPE_C_ASTC_4X4:
   case V3D_TMU_TYPE_C_ASTC_5X4:
   case V3D_TMU_TYPE_C_ASTC_5X5:
   case V3D_TMU_TYPE_C_ASTC_6X5:
   case V3D_TMU_TYPE_C_ASTC_6X6:
   case V3D_TMU_TYPE_C_ASTC_8X5:
   case V3D_TMU_TYPE_C_ASTC_8X6:
   case V3D_TMU_TYPE_C_ASTC_8X8:
   case V3D_TMU_TYPE_C_ASTC_10X5:
   case V3D_TMU_TYPE_C_ASTC_10X6:
   case V3D_TMU_TYPE_C_ASTC_10X8:
   case V3D_TMU_TYPE_C_ASTC_10X10:
   case V3D_TMU_TYPE_C_ASTC_12X10:
   case V3D_TMU_TYPE_C_ASTC_12X12:
      return true;
   default:
      return false;
   }
}

uint32_t v3d_tmu_ltype_num_dims(v3d_tmu_ltype_t ltype)
{
   switch (ltype)
   {
   case V3D_TMU_LTYPE_1D:
   case V3D_TMU_LTYPE_1D_ARRAY:
      return 1;
   case V3D_TMU_LTYPE_2D:
   case V3D_TMU_LTYPE_2D_ARRAY:
   case V3D_TMU_LTYPE_CUBE_MAP:
   case V3D_TMU_LTYPE_CHILD_IMAGE:
      return 2;
   case V3D_TMU_LTYPE_3D:
      return 3;
   default:
      unreachable();
      return 0;
   }
}

bool v3d_tmu_ltype_is_array(v3d_tmu_ltype_t ltype)
{
   switch (ltype)
   {
   case V3D_TMU_LTYPE_1D:
   case V3D_TMU_LTYPE_2D:
   case V3D_TMU_LTYPE_3D:
   case V3D_TMU_LTYPE_CUBE_MAP:
   case V3D_TMU_LTYPE_CHILD_IMAGE:
      return false;
   case V3D_TMU_LTYPE_1D_ARRAY:
   case V3D_TMU_LTYPE_2D_ARRAY:
      return true;
   default:
      unreachable();
      return false;
   }
}

static void split_filter(struct v3d_tmu_cfg *cfg, v3d_tmu_filter_t filter)
{
   cfg->aniso_level = 0;
   switch (filter)
   {
   case V3D_TMU_FILTER_MIN_LIN_MAG_LIN:
      cfg->minfilt = V3D_TMU_MIN_FILT_LINEAR;
      cfg->magfilt = V3D_TMU_MAG_FILT_LINEAR;
      break;
   case V3D_TMU_FILTER_MIN_LIN_MAG_NEAR:
      cfg->minfilt = V3D_TMU_MIN_FILT_LINEAR;
      cfg->magfilt = V3D_TMU_MAG_FILT_NEAREST;
      break;
   case V3D_TMU_FILTER_MIN_NEAR_MAG_LIN:
      cfg->minfilt = V3D_TMU_MIN_FILT_NEAREST;
      cfg->magfilt = V3D_TMU_MAG_FILT_LINEAR;
      break;
   case V3D_TMU_FILTER_MIN_NEAR_MAG_NEAR:
      cfg->minfilt = V3D_TMU_MIN_FILT_NEAREST;
      cfg->magfilt = V3D_TMU_MAG_FILT_NEAREST;
      break;
   case V3D_TMU_FILTER_MIN_NEAR_MIP_NEAR_MAG_LIN:
      cfg->minfilt = V3D_TMU_MIN_FILT_NEAR_MIP_NEAR;
      cfg->magfilt = V3D_TMU_MAG_FILT_LINEAR;
      break;
   case V3D_TMU_FILTER_MIN_NEAR_MIP_NEAR_MAG_NEAR:
      cfg->minfilt = V3D_TMU_MIN_FILT_NEAR_MIP_NEAR;
      cfg->magfilt = V3D_TMU_MAG_FILT_NEAREST;
      break;
   case V3D_TMU_FILTER_MIN_NEAR_MIP_LIN_MAG_LIN:
      cfg->minfilt = V3D_TMU_MIN_FILT_NEAR_MIP_LIN;
      cfg->magfilt = V3D_TMU_MAG_FILT_LINEAR;
      break;
   case V3D_TMU_FILTER_MIN_NEAR_MIP_LIN_MAG_NEAR:
      cfg->minfilt = V3D_TMU_MIN_FILT_NEAR_MIP_LIN;
      cfg->magfilt = V3D_TMU_MAG_FILT_NEAREST;
      break;
   case V3D_TMU_FILTER_MIN_LIN_MIP_NEAR_MAG_LIN:
      cfg->minfilt = V3D_TMU_MIN_FILT_LIN_MIP_NEAR;
      cfg->magfilt = V3D_TMU_MAG_FILT_LINEAR;
      break;
   case V3D_TMU_FILTER_MIN_LIN_MIP_NEAR_MAG_NEAR:
      cfg->minfilt = V3D_TMU_MIN_FILT_LIN_MIP_NEAR;
      cfg->magfilt = V3D_TMU_MAG_FILT_NEAREST;
      break;
   case V3D_TMU_FILTER_MIN_LIN_MIP_LIN_MAG_LIN:
      cfg->minfilt = V3D_TMU_MIN_FILT_LIN_MIP_LIN;
      cfg->magfilt = V3D_TMU_MAG_FILT_LINEAR;
      break;
   case V3D_TMU_FILTER_MIN_LIN_MIP_LIN_MAG_NEAR:
      cfg->minfilt = V3D_TMU_MIN_FILT_LIN_MIP_LIN;
      cfg->magfilt = V3D_TMU_MAG_FILT_NEAREST;
      break;
   case V3D_TMU_FILTER_ANISOTROPIC2:
   case V3D_TMU_FILTER_ANISOTROPIC4:
   case V3D_TMU_FILTER_ANISOTROPIC8:
   case V3D_TMU_FILTER_ANISOTROPIC16:
      cfg->minfilt = V3D_TMU_MIN_FILT_LIN_MIP_LIN;
      cfg->magfilt = V3D_TMU_MAG_FILT_LINEAR;
      switch (filter)
      {
      case V3D_TMU_FILTER_ANISOTROPIC2:   cfg->aniso_level = 1; break;
      case V3D_TMU_FILTER_ANISOTROPIC4:   cfg->aniso_level = 2; break;
      case V3D_TMU_FILTER_ANISOTROPIC8:   cfg->aniso_level = 3; break;
      case V3D_TMU_FILTER_ANISOTROPIC16:  cfg->aniso_level = 4; break;
      default:                            unreachable();
      }
      break;
   default:
      unreachable();
   }
}

/* cfg->ltype & cfg->swapst must be set! */
static void calc_logical_dims(struct v3d_tmu_cfg *cfg,
   uint32_t raw_width, uint32_t raw_height, uint32_t raw_depth)
{
   uint32_t dims = v3d_tmu_ltype_num_dims(cfg->ltype);
   cfg->logical_width = cfg->swapst ? raw_height : raw_width;
   cfg->logical_height = (dims > 1) ? (cfg->swapst ? raw_width : raw_height) : 1;
   cfg->logical_depth = (dims > 2) ? raw_depth : 1;

   if (v3d_tmu_ltype_is_array(cfg->ltype))
      cfg->num_array_elems = raw_depth;
   else if (cfg->ltype == V3D_TMU_LTYPE_CUBE_MAP)
      cfg->num_array_elems = 6;
   else
      cfg->num_array_elems = 1;
}

static void raw_bcolour(struct v3d_tmu_cfg *cfg,
#if V3D_HAS_NEW_TMU_CFG
   const uint32_t bcolour[4]
#else
   bool border_rrra, const uint32_t bcolour[2]
#endif
   )
{
   if (v3d_tmu_is_depth_type(cfg->type))
   {
      cfg->bcolour.fmt = gfx_lfmt_translate_from_tmu_type(cfg->type, cfg->srgb, V3D_VER);
      switch (cfg->bcolour.fmt)
      {
      case GFX_LFMT_R16_UNORM:
         cfg->bcolour.u.ui16[0] = (uint16_t)bcolour[0];
         break;
      case GFX_LFMT_R24X8_UNORM:
      case GFX_LFMT_R32_FLOAT:
      case GFX_LFMT_X8R24_UNORM:
         cfg->bcolour.u.ui32[0] = bcolour[0];
         break;
      default:
         unreachable();
      }
   }
   else
   {
      cfg->bcolour.fmt = cfg->blend_fmt;
      switch (cfg->bcolour.fmt & ~GFX_LFMT_TYPE_MASK)
      {
      case GFX_LFMT_R16_G16_B16_A16:
      case GFX_LFMT_R15X1_G15X1_B15X1_A15X1:
#if V3D_HAS_NEW_TMU_CFG
         cfg->bcolour.u.ui16[0] = (uint16_t)bcolour[0];
         cfg->bcolour.u.ui16[1] = (uint16_t)bcolour[1];
         cfg->bcolour.u.ui16[2] = (uint16_t)bcolour[2];
         cfg->bcolour.u.ui16[3] = (uint16_t)bcolour[3];
#else
         cfg->bcolour.u.ui16[0] = (uint16_t)bcolour[0];
         cfg->bcolour.u.ui16[1] = (uint16_t)(bcolour[0] >> 16);
         cfg->bcolour.u.ui16[2] = (uint16_t)bcolour[1];
         cfg->bcolour.u.ui16[3] = (uint16_t)(bcolour[1] >> 16);
#endif
         if ((cfg->bcolour.fmt & ~GFX_LFMT_TYPE_MASK) == GFX_LFMT_R15X1_G15X1_B15X1_A15X1)
         {
            /* Top 2 bits of each channel *must* match as unlike penrose,
             * hardware will not replicate bit 14 to bit 15 before passing to
             * blender. We could make the penrose behaviour match hardware but
             * as the HW behaviour is not particularly useful and specifying a
             * border color outside of the snorm15 range is almost certainly a
             * mistake, I think it's best to just assert here. */
            for (unsigned i = 0; i != 4; ++i)
               assert((((cfg->bcolour.u.ui16[i] >> 14) ^ (cfg->bcolour.u.ui16[i] >> 15)) & 1) == 0);
         }
         break;
      case GFX_LFMT_R32_G32_B32_A32:
#if V3D_HAS_NEW_TMU_CFG
         for (unsigned i = 0; i != 4; ++i)
            cfg->bcolour.u.ui32[i] = bcolour[i];
#else
         if (border_rrra)
         {
            for (unsigned i = 0; i != 3; ++i)
               cfg->bcolour.u.ui32[i] = bcolour[0];
            cfg->bcolour.u.ui32[3] = bcolour[1];
         }
         else
         {
            for (unsigned i = 0; i != 2; ++i)
            {
               cfg->bcolour.u.ui32[i] = bcolour[i];
               cfg->bcolour.u.ui32[2 + i] = bcolour[i];
            }
         }
#endif
         break;
      default:
         unreachable();
      }
   }
}

static void calc_miplvls_and_minlvl(struct v3d_tmu_cfg *cfg)
{
   /* minlvl and miplvls indicate the range of mip levels that the hardware
    * might access. Note that in coefficient mode, the hardware will never
    * actually access any memory, but we still set minlvl/miplvls as if it
    * would. */

   assert(cfg->min_lod <= cfg->max_lod);

   if (cfg->fetch)
   {
      /* In fetch mode, the TMU clamps the requested (integral) mip level using
       * min_lod/max_lod... */
      cfg->minlvl = cfg->min_lod / 256;
      cfg->miplvls = (cfg->max_lod / 256) + 1;
   }
   else
   {
      /* When minifying... */
      if (!v3d_tmu_min_filt_does_mipmapping(cfg->minfilt))
      {
         /* ...and there is no mipmapping...
          *
          * From the GLES 3 spec:
          *
          *    When the value of TEXTURE_MIN_FILTER is LINEAR, a 2 x 2 x 2 cube
          *    of texels in the image array of level level_base is selected.
          *
          *    When the value of TEXTURE_MIN_FILTER is NEAREST, the texel in
          *    the image array of level level_base that is nearest...
          *
          * ie We always do the actual lookup in the base_level mipmap.
          *
          * (The hardware only uses lambda, and hence min_lod and max_lod, to
          * decide whether to minify or magnify)
          */

         assert(cfg->aniso_level == 0); /* Aniso filtering implies mipmapping */

         cfg->minlvl = cfg->base_level;
         cfg->miplvls = cfg->base_level + 1;
      }
      else
      {
         /* ...and there is mipmapping...
          *
          * The hardware should not access mip levels outside of the range
          * covered by min_lod/max_lod.
          */

         cfg->minlvl = cfg->min_lod / 256; /* min_lod is .8 fixed-point */
         cfg->miplvls = gfx_udiv_round_up(cfg->max_lod, 256) + 1; /* max_lod is .8 fixed-point */
      }

      /* When magnifying...
       *
       * From the GLES 3 spec:
       *
       *    The level-of-detail level_base texel array is always used for
       *    magnification.
       *
       * So ensure that base_level is covered by minlvl and miplvls.
       */
      if (cfg->minlvl > cfg->base_level)
         cfg->minlvl = cfg->base_level;

      if (cfg->miplvls <= cfg->base_level)
         cfg->miplvls = cfg->base_level + 1;
   }

   assert(cfg->minlvl < cfg->miplvls);
}

static void calc_mip_levels(struct v3d_tmu_cfg *cfg)
{
   GFX_LFMT_T lfmt = gfx_lfmt_translate_from_tmu_type_and_ltype(
      cfg->type, cfg->srgb, cfg->ltype, V3D_VER);

   struct gfx_buffer_ml_cfg ml0_cfg;
   gfx_buffer_default_ml_cfg(&ml0_cfg);
   if (cfg->uif_top)
   {
      ml0_cfg.uif.force = true;
      ml0_cfg.uif.ub_xor = cfg->ub_xor;
      ml0_cfg.uif.ub_noutile = false;
      ml0_cfg.uif.ub_pads[0] = cfg->ub_pad;
   }
   if (cfg->ltype == V3D_TMU_LTYPE_3D)
   {
      ml0_cfg.force_slice_pitch = true;
      ml0_cfg.slice_pitch = cfg->arr_str;
   }

   assert(cfg->miplvls <= V3D_MAX_MIP_COUNT);
   size_t size_unused, align_unused;
   gfx_buffer_desc_gen_with_ml0_cfg(cfg->mip_levels,
      &size_unused, &align_unused,
      GFX_BUFFER_USAGE_V3D_TEXTURE,
      cfg->logical_width, cfg->logical_height, cfg->logical_depth,
      cfg->miplvls, 1, &lfmt, &ml0_cfg);

   cfg->base_addr = cfg->l0_addr - cfg->mip_levels[0].planes[0].offset;
}

static void calc_derived(struct v3d_tmu_cfg *cfg)
{
   calc_miplvls_and_minlvl(cfg);
   calc_mip_levels(cfg);
   cfg->blend_fmt = v3d_get_tmu_blend_fmt(cfg->type, cfg->srgb, cfg->shadow, cfg->output_32, V3D_VER);
}

static void check_config(const struct v3d_tmu_cfg *cfg)
{
   assert(v3d_addr_aligned(cfg->l0_addr, V3D_TMU_ML_ALIGN));

   /* Must output at least one word */
   assert(cfg->word_en[0] || cfg->word_en[1] || cfg->word_en[2] || cfg->word_en[3]);

   switch (v3d_tmu_get_word_read_max(
      cfg->type, cfg->coefficient, cfg->gather, cfg->output_32))
   {
   case 1:  assert(!cfg->word_en[1]); /* Fall through... */
   case 2:  assert(!cfg->word_en[2]); /* Fall through... */
   case 3:  assert(!cfg->word_en[3]); /* Fall through... */
   case 4:  break;
   default: unreachable();
   }

#if !V3D_HAS_NEW_TMU_CFG
   if (cfg->ltype == V3D_TMU_LTYPE_CHILD_IMAGE)
   {
      assert((cfg->cxoff + cfg->logical_cwidth) <= cfg->logical_width);
      assert((cfg->cyoff + cfg->logical_cheight) <= cfg->logical_height);
      /* Ansio/mipmapping & child images not supported... */
      assert(cfg->aniso_level == 0);
      assert(!v3d_tmu_min_filt_does_mipmapping(cfg->minfilt));
   }
#endif

   if (cfg->swapst)
   {
      /* swapst not supported with cubemaps or 1D lookups... */
      assert(
         (cfg->ltype != V3D_TMU_LTYPE_CUBE_MAP) &&
         (v3d_tmu_ltype_num_dims(cfg->ltype) != 1));
   }

   if (cfg->gather)
   {
      /* Only bilinear filtering allowed for gather */
      assert(cfg->aniso_level == 0);
      assert(cfg->minfilt == V3D_TMU_MIN_FILT_LINEAR || cfg->minfilt == V3D_TMU_MIN_FILT_LIN_MIP_NEAR);
      assert(cfg->magfilt == V3D_TMU_MAG_FILT_LINEAR);
   }

#if !V3D_VER_AT_LEAST(3, 3)
   assert(!cfg->unnorm); /* Not supported! */
#endif

   /* Must not specify both; this will confuse HW!
    * fetch implies unnormalised (integer) coords. */
   assert(!cfg->fetch || !cfg->unnorm);

   if (cfg->fetch || cfg->unnorm)
      assert(cfg->ltype != V3D_TMU_LTYPE_CUBE_MAP);

   if (cfg->unnorm)
   {
      /* Aniso/mipmapping must not be enabled */
      assert(cfg->aniso_level == 0);
      assert(!v3d_tmu_min_filt_does_mipmapping(cfg->minfilt));
   }

   /* fetch essentially implies bslod. Don't set them together. */
   assert(!cfg->fetch || !cfg->bslod);
}

#if V3D_HAS_NEW_TMU_CFG

static void gen_bcolour(struct v3d_tmu_cfg *cfg, const bool one[4])
{
   if (v3d_tmu_is_depth_type(cfg->type))
   {
      /* Format doesn't really matter -- TMU just converts to 32-bit float
       * after border is handled anyway */
      cfg->bcolour.fmt = GFX_LFMT_R32_FLOAT;
      cfg->bcolour.u.f[0] = one[0] ? 1.0f : 0.0f;
   }
   else
   {
      cfg->bcolour.fmt = cfg->blend_fmt;
      memset(&cfg->bcolour.u, 0, sizeof(cfg->bcolour.u));
      switch (cfg->bcolour.fmt & ~GFX_LFMT_TYPE_MASK)
      {
      case GFX_LFMT_R16_G16_B16_A16:
      case GFX_LFMT_R15X1_G15X1_B15X1_A15X1:
      {
         uint16_t packed_one;
         switch (cfg->bcolour.fmt)
         {
         case GFX_LFMT_R16_G16_B16_A16_FLOAT:
            packed_one = gfx_float_to_float16(1.0f);
            break;
         case GFX_LFMT_R16_G16_B16_A16_INT:
         case GFX_LFMT_R16_G16_B16_A16_UINT:
            packed_one = 1;
            break;
         case GFX_LFMT_R16_G16_B16_A16_UNORM:
            packed_one = 0xffff;
            break;
         case GFX_LFMT_R16_G16_B16_A16_SNORM:
            packed_one = 0x7fff;
            break;
         case GFX_LFMT_R15X1_G15X1_B15X1_A15X1_SNORM:
            packed_one = 0x3fff;
            break;
         default:
            unreachable();
         }
         for (unsigned i = 0; i != 4; ++i)
            if (one[i])
               cfg->bcolour.u.ui16[i] = packed_one;
         break;
      }
      case GFX_LFMT_R32_G32_B32_A32:
      {
         uint32_t packed_one;
         switch (cfg->bcolour.fmt)
         {
         case GFX_LFMT_R32_G32_B32_A32_FLOAT:
            packed_one = gfx_float_to_bits(1.0f);
            break;
         case GFX_LFMT_R32_G32_B32_A32_INT:
         case GFX_LFMT_R32_G32_B32_A32_UINT:
            packed_one = 1;
            break;
         default:
            unreachable();
         }
         for (unsigned i = 0; i != 4; ++i)
            if (one[i])
               cfg->bcolour.u.ui32[i] = packed_one;
         break;
      }
      default:
         unreachable();
      }
   }
}

void v3d_tmu_cfg_collect(struct v3d_tmu_cfg *cfg,
   const struct v3d_tmu_reg_flags *written,
   const V3D_TMU_PARAM0_T *p0,
   const V3D_TMU_PARAM1_T *p1, /* May be NULL */
   const V3D_TMU_PARAM2_T *p2, /* May be NULL */
   const V3D_TMU_TEX_STATE_T *tex_state,
   const V3D_TMU_TEX_EXTENSION_T *tex_extension, /* May be NULL */
   const V3D_TMU_SAMPLER_T *sampler, /* May be NULL */
   const uint32_t bcolour[4]) /* May be NULL */
{
   assert(p0 && tex_state);

   memset(cfg, 0, sizeof(*cfg));

   if (written->i)
   {
      assert(!written->scm && !written->r); /* 3D/cubemap arrays not supported */
      if (written->t)
         cfg->ltype = V3D_TMU_LTYPE_2D_ARRAY;
      else
         cfg->ltype = V3D_TMU_LTYPE_1D_ARRAY;
   }
   else if (written->scm)
      cfg->ltype = V3D_TMU_LTYPE_CUBE_MAP;
   else if (written->r)
      cfg->ltype = V3D_TMU_LTYPE_3D;
   else if (written->t)
      cfg->ltype = V3D_TMU_LTYPE_2D;
   else
      cfg->ltype = V3D_TMU_LTYPE_1D;
   cfg->shadow = written->dref;
   cfg->fetch = written->sfetch;

   memcpy(cfg->word_en, p0->word_en, sizeof(cfg->word_en));
   assert(p0->tex_state_addr);

   if (p1)
   {
      cfg->output_32 = p1->output_32;
      cfg->unnorm = (cfg->ltype != V3D_TMU_LTYPE_CUBE_MAP) && !cfg->fetch && p1->unnorm;
      cfg->pix_mask = p1->pix_mask;
      assert(!p1->sampler_addr == !sampler);
   }
   else
   {
      cfg->pix_mask = true;
      assert(!sampler);
   }

   if (p2)
   {
      cfg->tmuoff_per_sample = p2->tmuoff_per_sample;
      cfg->bslod = p2->bslod;
      cfg->coefficient = p2->coefficient;
      cfg->coeff_sample = p2->coeff_sample;
      cfg->gather = p2->gather;
      cfg->logical_tex_off_s = tex_state->swapst ? p2->offsets[1] : p2->offsets[0];
      cfg->logical_tex_off_t = tex_state->swapst ? p2->offsets[0] : p2->offsets[1];
      cfg->tex_off_r = p2->offsets[2];
   }
   if (written->slod)
      cfg->bslod = true;

   cfg->logical_flipx = tex_state->swapst ? tex_state->flipy : tex_state->flipx;
   cfg->logical_flipy = tex_state->swapst ? tex_state->flipx : tex_state->flipy;
   cfg->swapst = tex_state->swapst;
   cfg->srgb = tex_state->srgb && (!sampler || !sampler->srgb_override);
   cfg->ahdr = tex_state->ahdr;
   cfg->l0_addr = tex_state->l0_addr;
   cfg->arr_str = tex_state->arr_str;
   calc_logical_dims(cfg, tex_state->width, tex_state->height, tex_state->depth);
   cfg->type = tex_state->type;
   memcpy(cfg->swizzles, tex_state->swizzles, sizeof(cfg->swizzles));
   cfg->base_level = tex_state->base_level;
   assert(!tex_state->extended == !tex_extension);
   if (tex_extension)
   {
      cfg->ub_pad = tex_extension->ub_pad;
      cfg->ub_xor = tex_extension->ub_xor;
      cfg->uif_top = tex_extension->uif_top;
      cfg->xor_dis = tex_extension->xor_dis;
   }

   if (sampler)
   {
      split_filter(cfg, sampler->filter);
      cfg->compare_func = sampler->compare_func;
      cfg->min_lod = (tex_state->base_level << 8) + sampler->min_lod;
      cfg->max_lod = (tex_state->base_level << 8) + sampler->max_lod;
      cfg->fixed_bias = sampler->fixed_bias;
      cfg->logical_wrap_s = tex_state->swapst ? sampler->wrap_t : sampler->wrap_s;
      cfg->logical_wrap_t = tex_state->swapst ? sampler->wrap_s : sampler->wrap_t;
      cfg->wrap_r = sampler->wrap_r;
   }
   else
   {
      cfg->aniso_level = 0;
      cfg->minfilt = V3D_TMU_MIN_FILT_LINEAR;
      cfg->magfilt = V3D_TMU_MAG_FILT_LINEAR;
      cfg->compare_func = V3D_COMPARE_FUNC_LEQUAL;
      cfg->min_lod = 0;
      cfg->max_lod = 0xfff;
      cfg->fixed_bias = 0;
      cfg->logical_wrap_s = V3D_TMU_WRAP_REPEAT;
      cfg->logical_wrap_t = V3D_TMU_WRAP_REPEAT;
      cfg->wrap_r = V3D_TMU_WRAP_REPEAT;
   }
   if (!cfg->fetch && (tex_state->base_level == tex_state->max_level))
   {
      /* See GFXS-732 */
      cfg->aniso_level = 0;
      cfg->minfilt = v3d_tmu_min_filt_disable_mipmapping(cfg->minfilt);
   }
   else
   {
      cfg->min_lod = gfx_umin(cfg->min_lod, tex_state->max_level << 8);
      cfg->max_lod = gfx_umin(cfg->max_lod, tex_state->max_level << 8);
   }

   if (cfg->gather)
   {
      assert(p2->gather_comp < 4);
      cfg->swizzles[0] = cfg->swizzles[p2->gather_comp];
      cfg->aniso_level = 0;
      cfg->minfilt = V3D_TMU_MIN_FILT_LINEAR;
      cfg->magfilt = V3D_TMU_MIN_FILT_LINEAR;
   }

   calc_derived(cfg);

   /* Need to know blend format for raw_bcolour/gen_bcolour, so do this last... */
   assert((sampler && (sampler->std_bcol == V3D_TMU_STD_BCOL_NON_STD)) == !!bcolour);
   if (bcolour)
      raw_bcolour(cfg, bcolour);
   else
   {
      bool one[4];
      switch (sampler ? sampler->std_bcol : V3D_TMU_STD_BCOL_0000)
      {
      case V3D_TMU_STD_BCOL_0000:
         one[0] = false;
         one[1] = false;
         one[2] = false;
         one[3] = false;
         break;
      case V3D_TMU_STD_BCOL_0001:
         one[0] = tex_state->reverse_std_bcol;
         one[1] = false;
         one[2] = false;
         one[3] = !tex_state->reverse_std_bcol;
         break;
      case V3D_TMU_STD_BCOL_1111:
         one[0] = true;
         one[1] = true;
         one[2] = true;
         one[3] = true;
         break;
      default:
         unreachable();
      }
      gen_bcolour(cfg, one);
   }

   check_config(cfg);
}

#else

static v3d_tmu_wrap_t wrap_from_cfg0(
   v3d_tmu_ltype_t *ltype,
   v3d_tmu_wrap_cfg0_t wrap)
{
   switch (wrap)
   {
   case V3D_TMU_WRAP_CFG0_REPEAT: return V3D_TMU_WRAP_REPEAT;
   case V3D_TMU_WRAP_CFG0_CLAMP:  return V3D_TMU_WRAP_CLAMP;
   case V3D_TMU_WRAP_CFG0_MIRROR: return V3D_TMU_WRAP_MIRROR;
   case V3D_TMU_WRAP_CFG0_1D:
      assert(ltype);
      *ltype = V3D_TMU_LTYPE_1D;
      return V3D_TMU_WRAP_CLAMP; /* Simpenrose insists on a valid value here */
   default:
      unreachable();
      return V3D_TMU_WRAP_INVALID;
   }
}

static uint32_t get_miplvls_from_dims(const struct v3d_tmu_cfg *cfg)
{
   return gfx_msb(gfx_umax3(
      cfg->logical_width,
      cfg->logical_height,
      cfg->logical_depth)) + 1;
}

void v3d_tmu_cfg_collect(struct v3d_tmu_cfg *cfg,
   const V3D_MISCCFG_T *misccfg,
   const V3D_TMU_PARAM0_T *p0,
   const V3D_TMU_PARAM1_CFG0_T *p1_cfg0, const V3D_TMU_PARAM1_CFG1_T *p1_cfg1,
   const V3D_TMU_INDIRECT_T *ind)
{
   memset(cfg, 0, sizeof(*cfg));

   v3d_tmu_output_type_t output_type;
   bool border_rrra = false;
   uint32_t bcolour[2] = {0};
   switch (p0->cfg)
   {
   case 0:
      assert(!p1_cfg1 && !ind);

      cfg->type = p0->u.cfg0.type;
      cfg->srgb = p0->u.cfg0.srgb;
      cfg->pix_mask = p0->u.cfg0.pix_mask;

      cfg->logical_wrap_s = wrap_from_cfg0(NULL, p1_cfg0->wrap_s);
      cfg->logical_wrap_t = wrap_from_cfg0(&cfg->ltype, p1_cfg0->wrap_t);
      split_filter(cfg, p1_cfg0->filter);
      cfg->bslod = p1_cfg0->bslod;
      cfg->l0_addr = p1_cfg0->base;

      /* Must be done after cfg->ltype is set */
      calc_logical_dims(cfg, p0->u.cfg0.width, p0->u.cfg0.height, 1);

      /* Word enables not provided; calculate defaults based on the texture type */
      switch (v3d_tmu_get_word_read_default(cfg->type, misccfg->ovrtmuout))
      {
      case 4:  cfg->word_en[3] = true; /* Fall through... */
      case 3:  cfg->word_en[2] = true; /* Fall through... */
      case 2:  cfg->word_en[1] = true; /* Fall through... */
      case 1:  cfg->word_en[0] = true; break;
      default: unreachable();
      }

      cfg->swizzles[0] = V3D_TMU_SWIZZLE_R;
      cfg->swizzles[1] = V3D_TMU_SWIZZLE_G;
      cfg->swizzles[2] = V3D_TMU_SWIZZLE_B;
      cfg->swizzles[3] = V3D_TMU_SWIZZLE_A;

      cfg->max_lod = (get_miplvls_from_dims(cfg) - 1) << 8;

      output_type = misccfg->ovrtmuout ? V3D_TMU_OUTPUT_TYPE_16 : V3D_TMU_OUTPUT_TYPE_AUTO;

      break;
   case 1:
      assert(!p1_cfg0);

      cfg->ltype = p0->u.cfg1.ltype;
      cfg->fetch = p0->u.cfg1.fetch;
      cfg->gather = p0->u.cfg1.gather;
      cfg->bias = p0->u.cfg1.bias;
      cfg->bslod = p0->u.cfg1.bslod;
      cfg->coefficient = p0->u.cfg1.coefficient;
      cfg->shadow = p0->u.cfg1.shadow;
      cfg->logical_wrap_s = ind->swapst ? p0->u.cfg1.wrap_t : p0->u.cfg1.wrap_s;
      cfg->logical_wrap_t = ind->swapst ? p0->u.cfg1.wrap_s : p0->u.cfg1.wrap_t;
      cfg->wrap_r = p0->u.cfg1.wrap_r;
      cfg->logical_tex_off_s = ind->swapst ? p0->u.cfg1.tex_off_t : p0->u.cfg1.tex_off_s;
      cfg->logical_tex_off_t = ind->swapst ? p0->u.cfg1.tex_off_s : p0->u.cfg1.tex_off_t;
      cfg->tex_off_r = p0->u.cfg1.tex_off_r;
      cfg->pix_mask = p0->u.cfg1.pix_mask;

      cfg->word_en[0] = p1_cfg1->word0_en;
      cfg->word_en[1] = p1_cfg1->word1_en;
      cfg->word_en[2] = p1_cfg1->word2_en;
      cfg->word_en[3] = p1_cfg1->word3_en;
      cfg->unnorm = p1_cfg1->unnorm;

      split_filter(cfg, ind->filter);
      border_rrra = ind->border_rrra;
      cfg->l0_addr = ind->base;
      cfg->arr_str = ind->arr_str;
      cfg->swapst = ind->swapst; /* Must be set before calling calc_logical_dims */
      calc_logical_dims(cfg, ind->width, ind->height, ind->depth);
      cfg->type = ind->ttype;
      cfg->srgb = ind->srgb;
      cfg->ahdr = ind->ahdr;
      cfg->compare_func = ind->compare_func;
      memcpy(cfg->swizzles, ind->swizzles, sizeof(cfg->swizzles));
      cfg->logical_flipx = ind->swapst ? ind->flipy : ind->flipx;
      cfg->logical_flipy = ind->swapst ? ind->flipx : ind->flipy;
      assert(ind->etcflip); /* We do not support etcflip=0 */
      bcolour[0] = (uint32_t)ind->bcolour;
      bcolour[1] = (uint32_t)(ind->bcolour >> 32);
      if (cfg->ltype == V3D_TMU_LTYPE_CHILD_IMAGE)
      {
         /* max_lod not provided. Mipmapping is not supported in child image
          * mode, but lambda is still used for min/mag determination. Hardware
          * defaults max_lod to 0x100 (rather than 0) to avoid always choosing
          * magnification. */
         cfg->max_lod = 0x100;
         cfg->logical_cwidth = ind->swapst ? ind->u.child_image.cheight : ind->u.child_image.cwidth;
         cfg->logical_cheight = ind->swapst ? ind->u.child_image.cwidth : ind->u.child_image.cheight;
         cfg->cxoff = ind->u.child_image.cxoff;
         cfg->cyoff = ind->u.child_image.cyoff;
         output_type = V3D_TMU_OUTPUT_TYPE_AUTO;
      }
      else
      {
         /* It's unclear why min_lod/max_lod were signed in the old cfg struct
          * -- negative values don't make much sense and Tim M isn't sure what
          * the hardware would do. Just assert they're positive here. The
          * fields in cfg are unsigned. */
         assert(ind->u.not_child_image.min_lod >= 0);
         assert(ind->u.not_child_image.max_lod >= 0);
         cfg->min_lod = ind->u.not_child_image.min_lod;
         cfg->max_lod = ind->u.not_child_image.max_lod;
         cfg->fixed_bias = ind->u.not_child_image.fixed_bias;
         cfg->base_level = ind->u.not_child_image.base_level;
         cfg->coeff_sample = ind->u.not_child_image.samp_num;
         output_type = misccfg->ovrtmuout ? ind->u.not_child_image.output_type : V3D_TMU_OUTPUT_TYPE_AUTO;
      }
      cfg->ub_pad = ind->ub_pad;
      cfg->ub_xor = ind->ub_xor;
      cfg->uif_top = ind->uif_top;
      cfg->xor_dis = ind->xor_dis;

      break;
   default:
      unreachable();
   }

   cfg->output_32 = v3d_tmu_output_type_32(output_type,
      cfg->type, cfg->shadow, cfg->coefficient);

   calc_derived(cfg);

   /* Need to know blend format for raw_bcolour, so do this last... */
   raw_bcolour(cfg, border_rrra, bcolour);

   check_config(cfg);
}

#endif
