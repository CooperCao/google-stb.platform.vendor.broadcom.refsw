/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_common.h"
#include "v3d_tmu.h"
#include "v3d_align.h"

#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

v3d_tmu_blend_type_t v3d_maybe_get_tmu_blend_type(
   v3d_tmu_type_t type, bool srgb, bool shadow, bool output_32)
{
   assert(!srgb || v3d_tmu_type_supports_srgb(type));
   assert(!shadow || v3d_tmu_type_supports_shadow(type));

   if (srgb || shadow)
      return V3D_TMU_BLEND_TYPE_FLOAT16;

   switch (type)
   {
   case V3D_TMU_TYPE_R8:
   case V3D_TMU_TYPE_RG8:
   case V3D_TMU_TYPE_RGBA8:
#if V3D_VER_AT_LEAST(3,3,0,0)
      return output_32 ? V3D_TMU_BLEND_TYPE_UNORM16 : V3D_TMU_BLEND_TYPE_FLOAT16;
#else
      return V3D_TMU_BLEND_TYPE_FLOAT16;
#endif
   case V3D_TMU_TYPE_R8_SNORM:
   case V3D_TMU_TYPE_RG8_SNORM:
   case V3D_TMU_TYPE_RGBA8_SNORM:
#if V3D_VER_AT_LEAST(3,3,0,0)
      return output_32 ? V3D_TMU_BLEND_TYPE_SNORM15 : V3D_TMU_BLEND_TYPE_FLOAT16;
#else
      return V3D_TMU_BLEND_TYPE_FLOAT16;
#endif
   case V3D_TMU_TYPE_RGB565:
   case V3D_TMU_TYPE_RGBA4:
   case V3D_TMU_TYPE_RGB5_A1:
#if V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_TMU_TYPE_RGB5_A1_REV:
#endif
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
      return V3D_TMU_BLEND_TYPE_FLOAT16;
   case V3D_TMU_TYPE_R16:
   case V3D_TMU_TYPE_RG16:
   case V3D_TMU_TYPE_RGBA16:
      return output_32 ? V3D_TMU_BLEND_TYPE_UNORM16 : V3D_TMU_BLEND_TYPE_FLOAT16;
   case V3D_TMU_TYPE_R16_SNORM:
   case V3D_TMU_TYPE_RG16_SNORM:
   case V3D_TMU_TYPE_RGBA16_SNORM:
      return output_32 ? V3D_TMU_BLEND_TYPE_SNORM16 : V3D_TMU_BLEND_TYPE_FLOAT16;
   case V3D_TMU_TYPE_R32F:
   case V3D_TMU_TYPE_RG32F:
   case V3D_TMU_TYPE_RGBA32F:
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
#if !V3D_HAS_TMU_R32F_R16_SHAD
   case V3D_TMU_TYPE_DEPTH_COMP16:
   case V3D_TMU_TYPE_DEPTH_COMP32F:
#endif
   case V3D_TMU_TYPE_DEPTH_COMP24:
   case V3D_TMU_TYPE_DEPTH24_X8:
      return V3D_TMU_BLEND_TYPE_INVALID;
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
   v3d_tmu_type_t type, bool srgb, bool shadow, bool output_32)
{
   v3d_tmu_blend_type_t blend_type = v3d_maybe_get_tmu_blend_type(
      type, srgb, shadow, output_32);
   if (blend_type != V3D_TMU_BLEND_TYPE_INVALID)
      return v3d_tmu_blend_fmt_from_type(blend_type);

   assert(!srgb); /* All sRGB types are blendable */
   assert(!shadow); /* Depth compare result always blendable */

   switch (type)
   {
   case V3D_TMU_TYPE_S8:      /* Pad to 4 channels for consistency. The   */
   case V3D_TMU_TYPE_S16:     /* correct channels are returned to the QPU */
      return GFX_LFMT_R32_G32_B32_A32_UINT;

#if V3D_VER_AT_LEAST(3,3,0,0)
   case V3D_TMU_TYPE_R8I:
   case V3D_TMU_TYPE_RG8I:
   case V3D_TMU_TYPE_RGBA8I:
   case V3D_TMU_TYPE_R16I:
   case V3D_TMU_TYPE_RG16I:
   case V3D_TMU_TYPE_RGBA16I:
   case V3D_TMU_TYPE_R32I:
   case V3D_TMU_TYPE_RG32I:
   case V3D_TMU_TYPE_RGBA32I:
      return GFX_LFMT_R32_G32_B32_A32_INT;

   case V3D_TMU_TYPE_R8UI:
   case V3D_TMU_TYPE_RG8UI:
   case V3D_TMU_TYPE_RGBA8UI:
   case V3D_TMU_TYPE_R16UI:
   case V3D_TMU_TYPE_RG16UI:
   case V3D_TMU_TYPE_RGBA16UI:
   case V3D_TMU_TYPE_RGB10_A2UI:
   case V3D_TMU_TYPE_R32UI:
   case V3D_TMU_TYPE_RG32UI:
   case V3D_TMU_TYPE_RGBA32UI:
      return GFX_LFMT_R32_G32_B32_A32_UINT;
#endif

   case V3D_TMU_TYPE_R32F:
   case V3D_TMU_TYPE_RG32F:
   case V3D_TMU_TYPE_RGBA32F:
   // Depth types when shadow compare is disabled
#if !V3D_HAS_TMU_R32F_R16_SHAD
   case V3D_TMU_TYPE_DEPTH_COMP16:
   case V3D_TMU_TYPE_DEPTH_COMP32F:
#endif
   case V3D_TMU_TYPE_DEPTH_COMP24:
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
   v3d_tmu_type_t type, bool output_32)
{
   uint32_t half_words_per_channel;
   if ((type == V3D_TMU_TYPE_S8) || (type == V3D_TMU_TYPE_S16))
      /* Output type ignored for S8/S16... */
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
#if !V3D_HAS_TMU_R32F_R16_SHAD
   case V3D_TMU_TYPE_DEPTH_COMP16:
   case V3D_TMU_TYPE_DEPTH_COMP32F:
#endif
   case V3D_TMU_TYPE_DEPTH_COMP24:
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
#if V3D_VER_AT_LEAST(4,1,34,0)
   case V3D_TMU_TYPE_RGB5_A1_REV:
#endif
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

#if !V3D_VER_AT_LEAST(4,0,2,0)

bool v3d_tmu_auto_output_32(v3d_tmu_type_t type, bool shadow)
{
   if (shadow)
   {
      assert(v3d_tmu_type_supports_shadow(type));
      return false;
   }

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
   case V3D_TMU_TYPE_DEPTH_COMP16:
   case V3D_TMU_TYPE_DEPTH_COMP24:
   case V3D_TMU_TYPE_DEPTH_COMP32F:
   case V3D_TMU_TYPE_DEPTH24_X8:
   case V3D_TMU_TYPE_R32I:
   case V3D_TMU_TYPE_RG32I:
   case V3D_TMU_TYPE_RGBA32I:
   case V3D_TMU_TYPE_R32UI:
   case V3D_TMU_TYPE_RG32UI:
   case V3D_TMU_TYPE_RGBA32UI:
      return true;
   case V3D_TMU_TYPE_S8:
   case V3D_TMU_TYPE_S16:
      /* Output type should have no effect on S8/S16 */
      return false;
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
      type, !misccfg_ovrtmuout && v3d_tmu_auto_output_32(type, /*shadow=*/false));
}

#endif

uint32_t v3d_tmu_get_word_read_max(v3d_tmu_op_class_t op_class, bool is_write,
   v3d_tmu_type_t type, bool gather, bool output_32)
{
   /* Cache ops and regular writes do not return any output */
   if(op_class == V3D_TMU_OP_CLASS_CACHE || (is_write && op_class == V3D_TMU_OP_CLASS_REGULAR))
   {
      return 0;
   }

   /* Full 4 channels available in all cases except for regular S8/S16 lookup... */
   uint32_t num_channels;
   if (!gather &&
      ((type == V3D_TMU_TYPE_S8) || (type == V3D_TMU_TYPE_S16)))
      num_channels = 1;
   else
      num_channels = 4;

   return get_num_words(num_channels, type, output_32);
}

bool v3d_tmu_type_supports_srgb(v3d_tmu_type_t type)
{
   switch (type)
   {
#if V3D_VER_AT_LEAST(3,3,0,0)
   case V3D_TMU_TYPE_R8:
   case V3D_TMU_TYPE_RG8:
#endif
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

#if !V3D_VER_AT_LEAST(4,1,34,0)
static void set_filters(struct v3d_tmu_cfg *cfg, v3d_tmu_filters_t filters)
{
   cfg->aniso_level = 0;
   switch (filters)
   {
   case V3D_TMU_FILTERS_MIN_LIN_MAG_LIN:
      cfg->magfilt = V3D_TMU_FILTER_LINEAR;
      cfg->minfilt = V3D_TMU_FILTER_LINEAR;
      cfg->mipfilt = V3D_TMU_MIPFILT_BASE;
      break;
   case V3D_TMU_FILTERS_MIN_LIN_MAG_NEAR:
      cfg->magfilt = V3D_TMU_FILTER_NEAREST;
      cfg->minfilt = V3D_TMU_FILTER_LINEAR;
      cfg->mipfilt = V3D_TMU_MIPFILT_BASE;
      break;
   case V3D_TMU_FILTERS_MIN_NEAR_MAG_LIN:
      cfg->magfilt = V3D_TMU_FILTER_LINEAR;
      cfg->minfilt = V3D_TMU_FILTER_NEAREST;
      cfg->mipfilt = V3D_TMU_MIPFILT_BASE;
      break;
   case V3D_TMU_FILTERS_MIN_NEAR_MAG_NEAR:
      cfg->magfilt = V3D_TMU_FILTER_NEAREST;
      cfg->minfilt = V3D_TMU_FILTER_NEAREST;
      cfg->mipfilt = V3D_TMU_MIPFILT_BASE;
      break;
   case V3D_TMU_FILTERS_MIN_NEAR_MIP_NEAR_MAG_LIN:
      cfg->magfilt = V3D_TMU_FILTER_LINEAR;
      cfg->minfilt = V3D_TMU_FILTER_NEAREST;
      cfg->mipfilt = V3D_TMU_MIPFILT_NEAREST;
      break;
   case V3D_TMU_FILTERS_MIN_NEAR_MIP_NEAR_MAG_NEAR:
      cfg->magfilt = V3D_TMU_FILTER_NEAREST;
      cfg->minfilt = V3D_TMU_FILTER_NEAREST;
      cfg->mipfilt = V3D_TMU_MIPFILT_NEAREST;
      break;
   case V3D_TMU_FILTERS_MIN_NEAR_MIP_LIN_MAG_LIN:
      cfg->magfilt = V3D_TMU_FILTER_LINEAR;
      cfg->minfilt = V3D_TMU_FILTER_NEAREST;
      cfg->mipfilt = V3D_TMU_MIPFILT_LINEAR;
      break;
   case V3D_TMU_FILTERS_MIN_NEAR_MIP_LIN_MAG_NEAR:
      cfg->magfilt = V3D_TMU_FILTER_NEAREST;
      cfg->minfilt = V3D_TMU_FILTER_NEAREST;
      cfg->mipfilt = V3D_TMU_MIPFILT_LINEAR;
      break;
   case V3D_TMU_FILTERS_MIN_LIN_MIP_NEAR_MAG_LIN:
      cfg->magfilt = V3D_TMU_FILTER_LINEAR;
      cfg->minfilt = V3D_TMU_FILTER_LINEAR;
      cfg->mipfilt = V3D_TMU_MIPFILT_NEAREST;
      break;
   case V3D_TMU_FILTERS_MIN_LIN_MIP_NEAR_MAG_NEAR:
      cfg->magfilt = V3D_TMU_FILTER_NEAREST;
      cfg->minfilt = V3D_TMU_FILTER_LINEAR;
      cfg->mipfilt = V3D_TMU_MIPFILT_NEAREST;
      break;
   case V3D_TMU_FILTERS_MIN_LIN_MIP_LIN_MAG_LIN:
      cfg->magfilt = V3D_TMU_FILTER_LINEAR;
      cfg->minfilt = V3D_TMU_FILTER_LINEAR;
      cfg->mipfilt = V3D_TMU_MIPFILT_LINEAR;
      break;
   case V3D_TMU_FILTERS_MIN_LIN_MIP_LIN_MAG_NEAR:
      cfg->magfilt = V3D_TMU_FILTER_NEAREST;
      cfg->minfilt = V3D_TMU_FILTER_LINEAR;
      cfg->mipfilt = V3D_TMU_MIPFILT_LINEAR;
      break;
   case V3D_TMU_FILTERS_ANISOTROPIC2:
   case V3D_TMU_FILTERS_ANISOTROPIC4:
   case V3D_TMU_FILTERS_ANISOTROPIC8:
   case V3D_TMU_FILTERS_ANISOTROPIC16:
      cfg->magfilt = V3D_TMU_FILTER_LINEAR;
      cfg->minfilt = V3D_TMU_FILTER_LINEAR;
      cfg->mipfilt = V3D_TMU_MIPFILT_LINEAR;
      switch (filters)
      {
      case V3D_TMU_FILTERS_ANISOTROPIC2:  cfg->aniso_level = 1; break;
      case V3D_TMU_FILTERS_ANISOTROPIC4:  cfg->aniso_level = 2; break;
      case V3D_TMU_FILTERS_ANISOTROPIC8:  cfg->aniso_level = 3; break;
      case V3D_TMU_FILTERS_ANISOTROPIC16: cfg->aniso_level = 4; break;
      default:                            unreachable();
      }
      break;
   default:
      unreachable();
   }
}
#endif

static void set_wraps(struct v3d_tmu_cfg *cfg,
   v3d_tmu_wrap_t wrap_s, v3d_tmu_wrap_t wrap_t, v3d_tmu_wrap_t wrap_r)
{
#if !V3D_VER_AT_LEAST(3,3,0,0)
   if (cfg->fetch)
   {
      /* Fetch ignored wrap modes and just always clamped coords on 3.2 */
      cfg->wrap_s = V3D_TMU_WRAP_CLAMP;
      cfg->wrap_t = V3D_TMU_WRAP_CLAMP;
      cfg->wrap_r = V3D_TMU_WRAP_CLAMP;
      return;
   }
#endif

   cfg->wrap_s = wrap_s;
#if V3D_VER_AT_LEAST(4,0,2,0)
   cfg->wrap_t = (cfg->dims > 1) ? wrap_t : V3D_TMU_WRAP_CLAMP;
#else
   cfg->wrap_t = wrap_t;
#endif
   cfg->wrap_r = wrap_r;
}

/* cfg->dims, cfg->array, cfg->fetch and cfg->cube must be set! */
static void set_whd_elems(struct v3d_tmu_cfg *cfg,
   uint32_t raw_width, uint32_t raw_height, uint32_t raw_depth)
{
   if (cfg->dims == 1)
   {
#if V3D_VER_AT_LEAST(4,1,34,0)
      uint32_t low = gfx_pack_uint_0_is_max(raw_width, V3D_MAX_TEXTURE_DIM_BITS);
      uint32_t high = gfx_pack_uint_0_is_max(raw_height, V3D_MAX_TEXTURE_DIM_BITS);

      if (low == 0 && high == 0)
         cfg->width = 1 << (2 * V3D_MAX_TEXTURE_DIM_BITS);
      else
         cfg->width =  low | (high << V3D_MAX_TEXTURE_DIM_BITS);

      assert((!cfg->fetch && cfg->width <= (1 << V3D_MAX_TEXTURE_DIM_BITS)) ||
             (cfg->fetch && cfg->width <= (1 << (2 * V3D_MAX_TEXTURE_DIM_BITS))));
#else
      assert(raw_height == 1); /* HW requires height to be set to 1 for 1D images */
      cfg->width = raw_width;
#endif
      cfg->height = 1;
   }
   else
   {
      cfg->width = raw_width;
      cfg->height = raw_height;
   }
   cfg->depth = (cfg->dims > 2) ? raw_depth : 1;

   cfg->num_array_elems = cfg->array ? raw_depth : 1;
   if (cfg->cube)
      cfg->num_array_elems *= 6;
}

static void raw_bcolour(struct v3d_tmu_cfg *cfg,
#if V3D_VER_AT_LEAST(4,0,2,0)
   const uint32_t bcolour[4]
#else
   bool border_rrra, const uint32_t bcolour[2]
#endif
   )
{
   if (v3d_tmu_is_depth_type(cfg->type))
   {
#if V3D_VER_AT_LEAST(4,0,2,0)
      cfg->bcolour.fmt = GFX_LFMT_R32_FLOAT;
      cfg->bcolour.u.ui32[0] = bcolour[0];
#else
      cfg->bcolour.fmt = gfx_lfmt_translate_from_tmu_type(cfg->type, cfg->srgb);
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
#endif
   }
   else
   {
      cfg->bcolour.fmt = cfg->blend_fmt;
      switch (cfg->bcolour.fmt & ~GFX_LFMT_TYPE_MASK)
      {
      case GFX_LFMT_R16_G16_B16_A16:
      case GFX_LFMT_R15X1_G15X1_B15X1_A15X1:
#if V3D_VER_AT_LEAST(4,0,2,0)
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
#if !V3D_VER_AT_LEAST(4,2,13,0)
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
#endif
         break;
      case GFX_LFMT_R32_G32_B32_A32:
#if V3D_VER_AT_LEAST(4,0,2,0)
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
    * might access. */

   assert(cfg->min_lod <= cfg->max_lod);

   if (cfg->fetch)
   {
      /* In fetch mode, the TMU clamps the requested (integral) mip level using
       * min_lod/max_lod... */
      cfg->minlvl = cfg->min_lod >> V3D_TMU_F_BITS;
      cfg->miplvls = (cfg->max_lod >> V3D_TMU_F_BITS) + 1;
   }
   else
   {
      bool min_possible = cfg->max_lod > (cfg->base_level << V3D_TMU_F_BITS);
      bool mag_possible = cfg->min_lod <= (cfg->base_level << V3D_TMU_F_BITS);
      assert(min_possible || mag_possible);

      // Minification
      if (min_possible)
      {
         uint32_t min_lod = gfx_umax(cfg->min_lod, (cfg->base_level << V3D_TMU_F_BITS) + 1);
         switch (cfg->mipfilt)
         {
#if !V3D_VER_AT_LEAST(4,1,34,0)
         case V3D_TMU_MIPFILT_BASE:
            cfg->minlvl = cfg->base_level;
            cfg->miplvls = cfg->base_level + 1;
            break;
#endif
         case V3D_TMU_MIPFILT_NEAREST:
            cfg->minlvl = v3d_tmu_nearest_mip_level(min_lod);
            cfg->miplvls = v3d_tmu_nearest_mip_level(cfg->max_lod) + 1;
            break;
         case V3D_TMU_MIPFILT_LINEAR:
            cfg->minlvl = min_lod >> V3D_TMU_F_BITS;
            cfg->miplvls = gfx_udiv_round_up(cfg->max_lod, V3D_TMU_F_ONE) + 1;
            break;
         default:
            unreachable();
         }
      }

      // Magnification
      if (mag_possible)
      {
         // Always use base level
         if (!min_possible || (cfg->minlvl > cfg->base_level))
            cfg->minlvl = cfg->base_level;
         if (!min_possible || (cfg->miplvls <= cfg->base_level))
            cfg->miplvls = cfg->base_level + 1;
      }
   }

   assert(cfg->minlvl < cfg->miplvls);
}

void v3d_tmu_calc_mip_levels(GFX_BUFFER_DESC_T *mip_levels,
   uint32_t dims, bool srgb, v3d_tmu_type_t type,
   const struct gfx_buffer_uif_cfg *uif_cfg, uint32_t arr_str,
   uint32_t width, uint32_t height, uint32_t depth,
   uint32_t num_mip_levels)
{
   GFX_LFMT_T lfmt = gfx_lfmt_translate_from_tmu_type(type, srgb);
   gfx_lfmt_set_dims(&lfmt, gfx_lfmt_dims_to_enum(dims));

   struct gfx_buffer_ml_cfg ml0_cfg;
   ml0_cfg.uif = *uif_cfg;
   ml0_cfg.force_slice_pitch = dims == 3;
   ml0_cfg.slice_pitch = arr_str;

   size_t size, align;
   gfx_buffer_desc_gen_with_ml0_cfg(mip_levels,
      &size, &align,
      GFX_BUFFER_USAGE_V3D_TEXTURE,
      width, height, depth,
      num_mip_levels, 1, &lfmt, &ml0_cfg);
}

static v3d_tmu_op_class_t classify_op(v3d_tmu_op_t op, bool is_write)
{
   switch (op)
   {
   case V3D_TMU_OP_WR_ADD_RD_PREFETCH:
   case V3D_TMU_OP_WR_SUB_RD_CLEAR:
   case V3D_TMU_OP_WR_XCHG_RD_FLUSH:
   case V3D_TMU_OP_WR_CMPXCHG_RD_CLEAN:
   case V3D_TMU_OP_WR_UMIN_RD_FULL_L1_CLEAR:
      return is_write ? V3D_TMU_OP_CLASS_ATOMIC : V3D_TMU_OP_CLASS_CACHE;
   case V3D_TMU_OP_WR_UMAX:
   case V3D_TMU_OP_WR_SMIN:
   case V3D_TMU_OP_WR_SMAX:
      assert(is_write);
      /* Fall through... */
   case V3D_TMU_OP_WR_AND_RD_INC:
   case V3D_TMU_OP_WR_OR_RD_DEC:
   case V3D_TMU_OP_WR_XOR_RD_NOT:
      return V3D_TMU_OP_CLASS_ATOMIC;
   case V3D_TMU_OP_REGULAR:
      return V3D_TMU_OP_CLASS_REGULAR;
   default:
      unreachable();
   }
}

static void calc_derived_texture(struct v3d_tmu_cfg *cfg)
{
   calc_miplvls_and_minlvl(cfg);

   assert(cfg->miplvls <= V3D_MAX_MIP_COUNT);
   v3d_tmu_calc_mip_levels(cfg->mip_levels,
         cfg->dims, cfg->srgb, cfg->type,
         &cfg->uif_cfg, cfg->arr_str,
         cfg->width, cfg->height, cfg->depth,
         cfg->miplvls);

   cfg->base_addr = cfg->l0_addr - cfg->mip_levels[0].planes[0].offset;
   cfg->blend_fmt = v3d_get_tmu_blend_fmt(cfg->type, cfg->srgb, cfg->shadow, cfg->output_32);

   cfg->op_class = classify_op(cfg->op, cfg->is_write);
   if ((cfg->op == V3D_TMU_OP_REGULAR) && cfg->is_write)
   {
      uint32_t bytes = gfx_lfmt_bytes_per_block(cfg->mip_levels[0].planes[0].lfmt);
      if (bytes < 4)
         cfg->bytes_per_data_word = bytes;
      else
         cfg->bytes_per_data_word = 4;
      assert((bytes % cfg->bytes_per_data_word) == 0);
      cfg->num_data_words = bytes / cfg->bytes_per_data_word;
   }
}

static void check_config_texture(const struct v3d_tmu_cfg *cfg, bool off_written)
{
   assert(cfg->texture);

#if V3D_VER_AT_LEAST(4,0,2,0)
   assert(cfg->dims >= 1);
#else
   assert(cfg->dims >= 2); // 1D textures were quite broken on 3.2/3.3 (see eg GFXH-1608)
#endif
   assert(cfg->dims <= 3);
   if (cfg->array)
   {
      assert(cfg->dims < 3); // 3D arrays not supported
#if !V3D_VER_AT_LEAST(4,0,2,0)
      assert(!cfg->cube); // Cubemap arrays not supported
#endif
   }
   if (cfg->cube)
      assert(cfg->dims == 2);
#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (cfg->child_image)
      assert(cfg->dims == 2);
#endif

   assert(v3d_addr_aligned(cfg->l0_addr, V3D_TMU_ML_ALIGN));

   uint32_t out_words = v3d_tmu_get_word_read_max(cfg->op_class, cfg->is_write,
      cfg->type, cfg->gather, cfg->output_32);
   if (out_words != 0)
      /* Must output at least one word */
      assert(cfg->word_en[0] || cfg->word_en[1] || cfg->word_en[2] || cfg->word_en[3]);
   for (uint32_t i = out_words; i != 4; ++i)
      assert(!cfg->word_en[i]);

#if !V3D_VER_AT_LEAST(4,0,2,0)
   assert((cfg->cxoff + cfg->cwidth) <= cfg->width);
   assert((cfg->cyoff + cfg->cheight) <= cfg->height);
#endif

#if V3D_VER_AT_LEAST(4,0,2,0)
   if (cfg->unnorm)
#else
   if (cfg->child_image || cfg->unnorm)
#endif
   {
      /* Aniso/mipmapping not supported... */
      assert(cfg->aniso_level == 0);
#if V3D_VER_AT_LEAST(4,1,34,0)
      assert(cfg->mipfilt == V3D_TMU_MIPFILT_NEAREST);
#else
      assert((cfg->mipfilt == V3D_TMU_MIPFILT_BASE) || (cfg->mipfilt == V3D_TMU_MIPFILT_NEAREST));
#endif
      assert(cfg->minlvl == cfg->base_level);
      assert(cfg->miplvls == (cfg->base_level + 1));
   }

   if (cfg->gather)
   {
      /* Only bilinear filtering allowed for gather */
      assert(cfg->aniso_level == 0);
      assert(cfg->magfilt == V3D_TMU_FILTER_LINEAR);
      assert(cfg->minfilt == V3D_TMU_FILTER_LINEAR);
#if V3D_VER_AT_LEAST(4,1,34,0)
      assert(cfg->mipfilt == V3D_TMU_MIPFILT_NEAREST);
#else
      assert((cfg->mipfilt == V3D_TMU_MIPFILT_BASE) || (cfg->mipfilt == V3D_TMU_MIPFILT_NEAREST));
#endif
   }

#if !V3D_VER_AT_LEAST(3,3,0,0)
   assert(!cfg->unnorm); /* Not supported! */
#endif

#if !V3D_VER_AT_LEAST(3,3,1,0)
   // GFXH-1371 : Should not use ASTC cube maps if not nearest filtered.
   // GFXH-1371 : Should not use pix_mask with any ASTC textures.
   if (cfg->type >= V3D_TMU_TYPE_C_ASTC_4X4 && cfg->type <= V3D_TMU_TYPE_C_ASTC_12X12)
   {
      assert(!cfg->pix_mask);

      if (cfg->cube)
      {
         assert(cfg->magfilt == V3D_TMU_FILTER_NEAREST);
         assert(cfg->minfilt == V3D_TMU_FILTER_NEAREST);
      }
   }
#endif

   /* Must not specify both; this will confuse HW!
    * fetch implies unnormalised (integer) coords. */
   assert(!cfg->fetch || !cfg->unnorm);

   if (cfg->fetch || cfg->unnorm)
      assert(!cfg->cube);

   if (cfg->fetch)
   {
      assert(!cfg->bslod); /* fetch essentially implies bslod. Don't set them together. */
      assert(!cfg->shadow); /* Shadow compare not supported with fetch */
      assert(!cfg->gather);
   }

   if (cfg->aniso_level != 0)
   {
      assert(cfg->dims < 3); /* Aniso not supported with 3D */
#if !V3D_VER_AT_LEAST(4,1,34,0)
      assert(!cfg->bslod);
#endif
   }

   if (cfg->cube)
   {
      assert(cfg->width == cfg->height);
      assert(!cfg->flipx);
      assert(!cfg->flipy);
   }

#if V3D_VER_AT_LEAST(4,0,2,0)
   if (cfg->array && cfg->cube)
      /* wrap_i=border does not work with cubemap arrays! */
      assert(cfg->wrap_i != V3D_TMU_WRAP_I_BORDER);
#endif

   if (cfg->tmuoff_4x)
   {
      assert(!cfg->fetch);
      assert(!cfg->cube);
      assert(cfg->dims < 3);
   }

   if((cfg->op == V3D_TMU_OP_REGULAR && cfg->is_write) || cfg->op_class == V3D_TMU_OP_CLASS_ATOMIC)
   {
      /* Offset slot in the bus is reused for write data in hw */
      assert(!off_written);
   }

   if ((cfg->op != V3D_TMU_OP_REGULAR) || cfg->is_write)
   {
      /* Only support atomic/cache/write ops with fetch config */
      assert(cfg->fetch);
#if V3D_VER_AT_LEAST(4,2,13,0)
      assert(!cfg->lod_query);
#endif
   }

#if !V3D_VER_AT_LEAST(4,2,13,0)
   if (cfg->op == V3D_TMU_OP_REGULAR && cfg->is_write)
      assert(cfg->bytes_per_data_word == 4);
#endif

   if (cfg->op_class == V3D_TMU_OP_CLASS_ATOMIC)
      assert(gfx_lfmt_bytes_per_block(cfg->mip_levels[0].planes[0].lfmt) == 4);
}

#if V3D_VER_AT_LEAST(4,0,2,0)

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

void v3d_tmu_cfg_collect_texture(struct v3d_tmu_cfg *cfg,
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

   cfg->texture = true;

   cfg->is_write = written->d;
   if (written->scm)
   {
      cfg->dims = 2;
      cfg->cube = true;
   }
   else if (written->r)
      cfg->dims = 3;
   else if (written->t)
      cfg->dims = 2;
   else
      cfg->dims = 1;
   cfg->array = written->i;
   cfg->shadow = written->dref;
   cfg->fetch = written->sfetch;

   memcpy(cfg->word_en, p0->word_en, sizeof(cfg->word_en));
   assert(p0->tex_state_addr);

   if (p1)
   {
      cfg->output_32 = p1->output_32;
      cfg->unnorm = !cfg->cube && !cfg->fetch && p1->unnorm;
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
      cfg->tmuoff_4x = written->off && p2->tmuoff_4x;
      cfg->bslod = p2->bslod;
      cfg->sample_num = p2->sample_num;
      cfg->gather = p2->gather;
      cfg->tex_off_s = p2->offsets[0];
      cfg->tex_off_t = p2->offsets[1];
      cfg->tex_off_r = p2->offsets[2];
      cfg->op = p2->op;
#if V3D_VER_AT_LEAST(4,2,13,0)
      cfg->lod_query = p2->lod_query;
#endif
   }
   else
      cfg->op = V3D_TMU_OP_REGULAR;
   if (written->slod)
      cfg->bslod = true;

   cfg->flipx = tex_state->flipx;
   cfg->flipy = tex_state->flipy;
   cfg->srgb = tex_state->srgb && (!sampler || !sampler->srgb_override);
   cfg->ahdr = tex_state->ahdr;
   cfg->l0_addr = tex_state->l0_addr;
   cfg->arr_str = tex_state->arr_str;
   set_whd_elems(cfg, tex_state->width, tex_state->height, tex_state->depth);
   cfg->type = tex_state->type;
   memcpy(cfg->swizzles, tex_state->swizzles, sizeof(cfg->swizzles));
   cfg->base_level = tex_state->base_level;
   assert(!tex_state->extended == !tex_extension);
   if (tex_extension)
   {
      cfg->uif_cfg.ub_pads[0] = tex_extension->ub_pad;
      cfg->uif_cfg.ub_xor = tex_extension->ub_xor;
      cfg->uif_cfg.force = tex_extension->uif_top;
      cfg->uif_cfg.xor_dis = tex_extension->xor_dis;
   }

   cfg->min_lod = cfg->max_lod = (tex_state->base_level << V3D_TMU_F_BITS);
   if (sampler)
   {
#if V3D_VER_AT_LEAST(4,1,34,0)
      if (sampler->aniso_en)
      {
         switch (sampler->max_aniso)
         {
         case V3D_MAX_ANISO_2:   cfg->aniso_level = 1; break;
         case V3D_MAX_ANISO_4:   cfg->aniso_level = 2; break;
         case V3D_MAX_ANISO_8:   cfg->aniso_level = 3; break;
         case V3D_MAX_ANISO_16:  cfg->aniso_level = 4; break;
         default:                unreachable();
         }
      }
      else
         cfg->aniso_level = 0;
      cfg->magfilt = sampler->magfilt;
      cfg->minfilt = sampler->minfilt;
      cfg->mipfilt = sampler->mipfilt;
#else
      set_filters(cfg, sampler->filters);
#endif
      cfg->compare_func = sampler->compare_func;
#if V3D_HAS_SAMPLER_LOD_DIS
      if (p2 && p2->sampler_lod_dis)
      {
         cfg->max_lod += 0xfff;
         cfg->fixed_bias = 0;
      }
      else
#endif
      {
         cfg->min_lod += sampler->min_lod;
         cfg->max_lod += sampler->max_lod;
         cfg->fixed_bias = sampler->fixed_bias;
      }
      set_wraps(cfg, sampler->wrap_s, sampler->wrap_t, sampler->wrap_r);
      cfg->wrap_i = sampler->wrap_i;
   }
   else
   {
      cfg->aniso_level = 0;
      cfg->magfilt = V3D_TMU_FILTER_LINEAR;
      cfg->minfilt = V3D_TMU_FILTER_LINEAR;
#if V3D_VER_AT_LEAST(4,1,34,0)
      cfg->mipfilt = V3D_TMU_MIPFILT_NEAREST;
#else
      cfg->mipfilt = V3D_TMU_MIPFILT_BASE;
#endif
      cfg->compare_func = V3D_COMPARE_FUNC_LEQUAL;
      cfg->max_lod += 0xfff;
      cfg->fixed_bias = 0;
      set_wraps(cfg, V3D_TMU_WRAP_BORDER, V3D_TMU_WRAP_BORDER, V3D_TMU_WRAP_BORDER);
      cfg->wrap_i = V3D_TMU_WRAP_I_BORDER;
   }

   /* Anisotropic filtering not supported with 3D textures. Just disable... */
   /* On old hardware it was also incompatible with bslod */
   if ((cfg->dims == 3) || (!V3D_VER_AT_LEAST(4,1,34,0) && cfg->bslod))
      cfg->aniso_level = 0;

   if (cfg->gather)
   {
      assert(p2->gather_comp < 4);
      cfg->swizzles[0] = cfg->swizzles[p2->gather_comp];
#if V3D_HAS_GATHER_NO_ANISO /* GFXH-1669 */
      cfg->aniso_level = 0;
#endif
      cfg->magfilt = V3D_TMU_FILTER_LINEAR;
      cfg->minfilt = V3D_TMU_FILTER_LINEAR;
      cfg->mipfilt = V3D_TMU_MIPFILT_NEAREST;
   }

   assert(tex_state->max_level >= tex_state->base_level);
   if (!cfg->fetch && (tex_state->base_level == tex_state->max_level))
   {
      /* See GFXS-732 */
#if V3D_VER_AT_LEAST(4,1,34,0)
      cfg->mipfilt = V3D_TMU_MIPFILT_NEAREST;
      cfg->min_lod = gfx_umin(cfg->min_lod, (tex_state->base_level << V3D_TMU_F_BITS) + 1);
      cfg->max_lod = gfx_umin(cfg->max_lod, (tex_state->base_level << V3D_TMU_F_BITS) + 1);
#else
      cfg->aniso_level = 0;
      cfg->mipfilt = V3D_TMU_MIPFILT_BASE;
#endif
   }
   else
   {
      cfg->min_lod = gfx_umin(cfg->min_lod, tex_state->max_level << V3D_TMU_F_BITS);
      cfg->max_lod = gfx_umin(cfg->max_lod, tex_state->max_level << V3D_TMU_F_BITS);
   }

   calc_derived_texture(cfg);

#if !V3D_HAS_TMU_UNMASKED_WR
   if((cfg->op == V3D_TMU_OP_REGULAR && cfg->is_write) || cfg->op_class == V3D_TMU_OP_CLASS_ATOMIC)
   {
      /* GFXH-1671 */
      cfg->pix_mask = true;
   }
#endif

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

   check_config_texture(cfg, written->off);
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
      return V3D_TMU_WRAP_CLAMP;
   default:
      unreachable();
      return V3D_TMU_WRAP_INVALID;
   }
}

static void set_ltype(struct v3d_tmu_cfg *cfg, v3d_tmu_ltype_t ltype)
{
   cfg->array = false;
   cfg->cube = false;
   cfg->child_image = false;
   switch (ltype)
   {
   case V3D_TMU_LTYPE_2D:           cfg->dims = 2; break;
   case V3D_TMU_LTYPE_2D_ARRAY:     cfg->dims = 2; cfg->array = true; break;
   case V3D_TMU_LTYPE_3D:           cfg->dims = 3; break;
   case V3D_TMU_LTYPE_CUBE_MAP:     cfg->dims = 2; cfg->cube = true; break;
   case V3D_TMU_LTYPE_1D:           cfg->dims = 1; break;
   case V3D_TMU_LTYPE_1D_ARRAY:     cfg->dims = 1; cfg->array = true; break;
   case V3D_TMU_LTYPE_CHILD_IMAGE:  cfg->dims = 2; cfg->child_image = true; break;
   default:                         unreachable();
   }
}

static uint32_t get_miplvls_from_dims(const struct v3d_tmu_cfg *cfg)
{
   return gfx_msb(gfx_umax3(cfg->width, cfg->height, cfg->depth)) + 1;
}

void v3d_tmu_cfg_collect_texture(struct v3d_tmu_cfg *cfg,
   const V3D_MISCCFG_T *misccfg,
   const V3D_TMU_PARAM0_T *p0,
   const V3D_TMU_PARAM1_CFG0_T *p1_cfg0, const V3D_TMU_PARAM1_CFG1_T *p1_cfg1,
   const V3D_TMU_INDIRECT_T *ind)
{
   memset(cfg, 0, sizeof(*cfg));

   cfg->texture = true;
   cfg->op = V3D_TMU_OP_REGULAR;

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

      v3d_tmu_ltype_t ltype = V3D_TMU_LTYPE_2D;
      set_wraps(cfg,
         wrap_from_cfg0(NULL, p1_cfg0->wrap_s),
         wrap_from_cfg0(&ltype, p1_cfg0->wrap_t),
         V3D_TMU_WRAP_REPEAT);
      set_ltype(cfg, ltype);
      set_filters(cfg, p1_cfg0->filters);
      cfg->bslod = p1_cfg0->bslod;
      cfg->l0_addr = p1_cfg0->base;

      /* Must be done after set_ltype */
      set_whd_elems(cfg, p0->u.cfg0.width, p0->u.cfg0.height, 1);
      /* Can't do child image with cfg0... */
      cfg->cwidth = cfg->width;
      cfg->cheight = cfg->height;

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

      cfg->max_lod = (get_miplvls_from_dims(cfg) - 1) << V3D_TMU_F_BITS;

      output_type = misccfg->ovrtmuout ? V3D_TMU_OUTPUT_TYPE_16 : V3D_TMU_OUTPUT_TYPE_AUTO;

      break;
   case 1:
      assert(!p1_cfg0);

      set_ltype(cfg, p0->u.cfg1.ltype);
      cfg->fetch = p0->u.cfg1.fetch;
      cfg->gather = p0->u.cfg1.gather;
      cfg->bias = p0->u.cfg1.bias;
      cfg->bslod = p0->u.cfg1.bslod;
      cfg->shadow = p0->u.cfg1.shadow;
      set_wraps(cfg, p0->u.cfg1.wrap_s, p0->u.cfg1.wrap_t, p0->u.cfg1.wrap_r); /* Must be after cfg->dims & cfg->fetch set */
      cfg->tex_off_s = p0->u.cfg1.tex_off_s;
      cfg->tex_off_t = p0->u.cfg1.tex_off_t;
      cfg->tex_off_r = p0->u.cfg1.tex_off_r;
      cfg->pix_mask = p0->u.cfg1.pix_mask;

      cfg->word_en[0] = p1_cfg1->word0_en;
      cfg->word_en[1] = p1_cfg1->word1_en;
      cfg->word_en[2] = p1_cfg1->word2_en;
      cfg->word_en[3] = p1_cfg1->word3_en;
      cfg->unnorm = p1_cfg1->unnorm;

      set_filters(cfg, ind->filters);
      border_rrra = ind->border_rrra;
      cfg->l0_addr = ind->base;
      cfg->arr_str = ind->arr_str;
      set_whd_elems(cfg, ind->width, ind->height, ind->depth);
      cfg->type = ind->ttype;
      cfg->srgb = ind->srgb;
      cfg->ahdr = ind->ahdr;
      cfg->compare_func = ind->compare_func;
      memcpy(cfg->swizzles, ind->swizzles, sizeof(cfg->swizzles));
      cfg->flipx = ind->flipx;
      cfg->flipy = ind->flipy;
      bcolour[0] = (uint32_t)ind->bcolour;
      bcolour[1] = (uint32_t)(ind->bcolour >> 32);
      if (cfg->child_image)
      {
         /* max_lod not provided. Mipmapping is not supported in child image
          * mode, but lambda is still used for min/mag determination. Hardware
          * defaults max_lod to V3D_TMU_F_ONE (rather than 0) to avoid always
          * choosing magnification. */
         cfg->max_lod = V3D_TMU_F_ONE;
         cfg->cwidth = ind->u.child_image.cwidth;
         cfg->cheight = ind->u.child_image.cheight;
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
         cfg->sample_num = ind->u.not_child_image.samp_num;
         cfg->cwidth = cfg->width;
         cfg->cheight = cfg->height;
         output_type = misccfg->ovrtmuout ? ind->u.not_child_image.output_type : V3D_TMU_OUTPUT_TYPE_AUTO;
      }
      cfg->uif_cfg.ub_pads[0] = ind->ub_pad;
      cfg->uif_cfg.ub_xor = ind->ub_xor;
      cfg->uif_cfg.force = ind->uif_top;
      cfg->uif_cfg.xor_dis = ind->xor_dis;

      break;
   default:
      unreachable();
   }

   cfg->output_32 = v3d_tmu_output_type_32(output_type, cfg->type, cfg->shadow);

   calc_derived_texture(cfg);

   /* Need to know blend format for raw_bcolour, so do this last... */
   raw_bcolour(cfg, border_rrra, bcolour);

   check_config_texture(cfg, false);
}

#endif

void v3d_tmu_cfg_collect_general(struct v3d_tmu_cfg *cfg,
   const struct v3d_tmu_reg_flags *written,
   const V3D_TMU_GENERAL_CONFIG_T *general_cfg)
{
   memset(cfg, 0, sizeof(*cfg));
   cfg->is_write = written->d;
   cfg->pix_mask = general_cfg->per_pixel_enable;
   cfg->op = general_cfg->op;
   cfg->op_class = classify_op(cfg->op, cfg->is_write);
   switch (cfg->op_class)
   {
   case V3D_TMU_OP_CLASS_REGULAR:
      switch (general_cfg->type)
      {
      case V3D_TMU_GENERAL_TYPE_S8:    cfg->data_signed = true;  cfg->num_data_words = 1; cfg->bytes_per_data_word = 1; break;
      case V3D_TMU_GENERAL_TYPE_S16:   cfg->data_signed = true;  cfg->num_data_words = 1; cfg->bytes_per_data_word = 2; break;
      case V3D_TMU_GENERAL_TYPE_VEC2:  cfg->data_signed = false; cfg->num_data_words = 2; cfg->bytes_per_data_word = 4; break;
      case V3D_TMU_GENERAL_TYPE_VEC3:  cfg->data_signed = false; cfg->num_data_words = 3; cfg->bytes_per_data_word = 4; break;
      case V3D_TMU_GENERAL_TYPE_VEC4:  cfg->data_signed = false; cfg->num_data_words = 4; cfg->bytes_per_data_word = 4; break;
      case V3D_TMU_GENERAL_TYPE_8:     cfg->data_signed = false; cfg->num_data_words = 1; cfg->bytes_per_data_word = 1; break;
      case V3D_TMU_GENERAL_TYPE_16:    cfg->data_signed = false; cfg->num_data_words = 1; cfg->bytes_per_data_word = 2; break;
      case V3D_TMU_GENERAL_TYPE_32:    cfg->data_signed = false; cfg->num_data_words = 1; cfg->bytes_per_data_word = 4; break;
      default:                         unreachable();
      }
      if (!cfg->is_write)
         for (unsigned i = 0; i != cfg->num_data_words; ++i)
            cfg->word_en[i] = true;
      break;
   case V3D_TMU_OP_CLASS_ATOMIC:
      cfg->word_en[0] = true;
      break;
   case V3D_TMU_OP_CLASS_CACHE:
      break;
   default:
      unreachable();
   }
}

#if V3D_VER_AT_LEAST(4,1,34,0)
void v3d_tmu_get_wh_for_1d_tex_state(uint32_t *w, uint32_t *h,
      uint32_t width_in)
{
   //split width_in in two 14 bit fields
   uint32_t max_width = (1u << ( 2 * V3D_MAX_TEXTURE_DIM_BITS));
   uint32_t mask = (1u << V3D_MAX_TEXTURE_DIM_BITS) - 1;

   assert(width_in <= max_width);

   *w = width_in & mask;
   *h = (width_in >> V3D_MAX_TEXTURE_DIM_BITS) & mask;
   *w = gfx_unpack_uint_0_is_max(*w, V3D_MAX_TEXTURE_DIM_BITS);
   *h = gfx_unpack_uint_0_is_max(*h, V3D_MAX_TEXTURE_DIM_BITS);
}
#endif
