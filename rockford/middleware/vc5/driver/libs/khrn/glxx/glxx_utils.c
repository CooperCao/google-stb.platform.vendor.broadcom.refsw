/*=============================================================================
  Broadcom Proprietary and Confidential. (c)2015 Broadcom.
  All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Miscellaneous glxx functions
=============================================================================*/
#include "glxx_utils.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"

unsigned int glxx_get_type_size(GLenum type, int count)
{
   switch (type) {
   case GL_BYTE:
   case GL_UNSIGNED_BYTE:
      return 1 * count;
   case GL_SHORT:
   case GL_UNSIGNED_SHORT:
   case GL_HALF_FLOAT_OES:
   case GL_HALF_FLOAT:
      return 2 * count;
   case GL_FIXED:
   case GL_FLOAT:
   case GL_INT:
   case GL_UNSIGNED_INT:
      return 4 * count;
   case GL_INT_2_10_10_10_REV:
   case GL_UNSIGNED_INT_2_10_10_10_REV:
      return 4;
   default:
      UNREACHABLE();
      return 0;
   }
}

bool glxx_is_color_renderable_internalformat(GLenum internalformat)
{
   //  An internal format is color-renderable
   //  if it is one of the formats from table 3.12
   //  noted as color-renderable or if it is unsized
   //  format RGBA or RGB. No other formats, including
   // compressed internal formats, are color-renderable

   switch (internalformat)
   {
   // ES 2: Table 4.5: Renderbuffer image formats
   // ES 3: internalformat must be a sized internal format that is
   //       color-renderable, depth-renderable, or stencilrenderable (as defined in section 4.4.4)
   // Table 3.12 in ES 3
   case GL_R8:
   case GL_RG8:
   case GL_RGB8:
   case GL_RGB565:         // ES 2 Table 4.5
   case GL_RGBA4:          // ES 2 Table 4.5
   case GL_RGB5_A1:        // ES 2 Table 4.5
   case GL_RGBA8:
   case GL_RGB10_A2:
   case GL_RGB10_A2UI:
   case GL_SRGB8_ALPHA8:
   case GL_R8I:
   case GL_R8UI:
   case GL_R16I:
   case GL_R16UI:
   case GL_R32I:
   case GL_R32UI:
   case GL_RG8I:
   case GL_RG8UI:
   case GL_RG16I:
   case GL_RG16UI:
   case GL_RG32I:
   case GL_RG32UI:
   case GL_RGBA8I:
   case GL_RGBA8UI:
   case GL_RGBA16I:
   case GL_RGBA16UI:
   case GL_RGBA32I:
   case GL_RGBA32UI:
#if GL_EXT_color_buffer_float
   case GL_RGBA32F:
   case GL_RGBA16F:
   case GL_R11F_G11F_B10F:
   case GL_R32F:
   case GL_RG32F:
   case GL_R16F:
   case GL_RG16F:
#endif
      return true;
   default:
      return false;
   }
}

bool glxx_is_depth_renderable_internalformat(GLenum internalformat)
{
   // An internal format is depth-renderable if it is one of the formats
   // from table 3.13. No other formats are depth-renderable.
   switch (internalformat)
   {
   case GL_DEPTH_COMPONENT16:    // ES 2 Table 4.5
   case GL_DEPTH_COMPONENT24:
   case GL_DEPTH_COMPONENT32F:
   case GL_DEPTH24_STENCIL8:
   case GL_DEPTH32F_STENCIL8:
      return true;
   default:
      return false;
   }
}
bool glxx_is_stencil_renderable_internalformat(GLenum internalformat)
{
   //  An internal format is stencil-renderable if it is STENCIL_INDEX8 or one of
   //  the formats from table 3.13 whose base internal format is DEPTH_STENCIL.
   //  No other formats are stencil-renderable.
   switch (internalformat)
   {
   case GL_STENCIL_INDEX8:    // ES 2 Table 4.5
   case GL_DEPTH24_STENCIL8:
   case GL_DEPTH32F_STENCIL8:
      return true;
   default:
      return false;
   }
}

bool glxx_has_integer_internalformat(GLenum internalformat)
{
   switch (internalformat)
   {
   case GL_RGB10_A2UI:
   case GL_R8I:
   case GL_R8UI:
   case GL_R16I:
   case GL_R16UI:
   case GL_R32I:
   case GL_R32UI:
   case GL_RG8I:
   case GL_RG8UI:
   case GL_RG16I:
   case GL_RG16UI:
   case GL_RG32I:
   case GL_RG32UI:
   case GL_RGBA8I:
   case GL_RGBA8UI:
   case GL_RGBA16I:
   case GL_RGBA16UI:
   case GL_RGBA32I:
   case GL_RGBA32UI:
      return true;

   default:
      return false;
   }
}

glxx_ms_mode glxx_max_ms_mode_for_internalformat(GLenum internalformat)
{
   if (glxx_has_integer_internalformat(internalformat))
      return glxx_samples_to_ms_mode(GLXX_CONFIG_MAX_INTEGER_SAMPLES);

   switch (internalformat)
   {
   case GL_RGBA32F:
   case GL_RGB32F:
   case GL_RG32F:
   case GL_R32F:
      return GLXX_NO_MS;
   default:
      break;
   }
   return glxx_samples_to_ms_mode(GLXX_CONFIG_MAX_SAMPLES);
}

glxx_ms_mode glxx_samples_to_ms_mode(unsigned samples)
{
   glxx_ms_mode ms_mode = GLXX_NO_MS;
   assert(samples <= GLXX_CONFIG_MAX_SAMPLES);

   if (samples == 0)
      ms_mode = GLXX_NO_MS;
   else if (samples <= GLXX_4X_MS)
      ms_mode = GLXX_4X_MS;
   else
      UNREACHABLE();
   return ms_mode;
}

unsigned glxx_ms_mode_get_scale(glxx_ms_mode ms_mode)
{
   unsigned scale = 0;
   switch (ms_mode)
   {
      case GLXX_NO_MS:
         scale = 1;
         break;
      case GLXX_4X_MS:
         scale = 2;
         break;
      default:
         UNREACHABLE();
   }
   return scale;
}

bool glxx_is_color_renderable_from_api_fmt(GFX_LFMT_T fmt)
{
   bool res = false;

   GFX_LFMT_T fmt_pre_masked_out = fmt & GFX_LFMT_FORMAT_MASK & ~GFX_LFMT_PRE_MASK;

   switch (fmt_pre_masked_out)
   {
   case GFX_LFMT_R8_G8_B8_A8_SRGB_SRGB_SRGB_UNORM:
   case GFX_LFMT_R10G10B10A2_UINT:
   case GFX_LFMT_R10G10B10A2_UNORM:
   case GFX_LFMT_A1B5G5R5_UNORM:
   case GFX_LFMT_A4B4G4R4_UNORM:
   case GFX_LFMT_B5G6R5_UNORM:
   case GFX_LFMT_R32_G32_B32_A32_INT:
   case GFX_LFMT_R32_G32_INT:
   case GFX_LFMT_R32_INT:
   case GFX_LFMT_R32_G32_B32_A32_UINT:
   case GFX_LFMT_R32_G32_UINT:
   case GFX_LFMT_R32_UINT:
   case GFX_LFMT_R16_G16_B16_A16_INT:
   case GFX_LFMT_R16_G16_INT:
   case GFX_LFMT_R16_INT:
   case GFX_LFMT_R16_G16_B16_A16_UINT:
   case GFX_LFMT_R16_G16_UINT:
   case GFX_LFMT_R16_UINT:
   case GFX_LFMT_R8_G8_B8_A8_UNORM:
   case GFX_LFMT_R8_G8_B8_UNORM:
   case GFX_LFMT_R8_G8_UNORM:
   case GFX_LFMT_R8_UNORM:
   case GFX_LFMT_R8_G8_B8_A8_INT:
   case GFX_LFMT_R8_G8_INT:
   case GFX_LFMT_R8_INT:
   case GFX_LFMT_R8_G8_B8_A8_UINT:
   case GFX_LFMT_R8_G8_UINT:
   case GFX_LFMT_R8_UINT:
   case GFX_LFMT_R8_G8_B8_X8_UNORM:
#if GL_EXT_color_buffer_float
   case GFX_LFMT_R11G11B10_UFLOAT:
   case GFX_LFMT_R32_G32_B32_A32_FLOAT:
   case GFX_LFMT_R32_G32_FLOAT:
   case GFX_LFMT_R32_FLOAT:
   case GFX_LFMT_R16_G16_B16_A16_FLOAT:
   case GFX_LFMT_R16_G16_FLOAT:
   case GFX_LFMT_R16_FLOAT:
#endif
      res = true;
      break;
   default:
      res = false;
   }

   return res;
}

bool glxx_is_depth_renderable_from_api_fmt(GFX_LFMT_T fmt)
{
   bool res = false;

   switch (fmt & GFX_LFMT_FORMAT_MASK & ~GFX_LFMT_PRE_MASK)
   {
   case GFX_LFMT_D32_FLOAT:
   case GFX_LFMT_D24X8_UNORM:
   case GFX_LFMT_D16_UNORM:
   case GFX_LFMT_S8D24_UINT_UNORM:
   case GFX_LFMT_D32_S8X24_FLOAT_UINT:
      res = true;
      break;
   default:
      res = false;
   }
   return res;
}

bool glxx_is_stencil_renderable_from_api_fmt(GFX_LFMT_T fmt)
{
   switch (fmt & GFX_LFMT_FORMAT_MASK & ~GFX_LFMT_PRE_MASK)
   {
    case GFX_LFMT_S8_UINT:
    case GFX_LFMT_S8D24_UINT_UNORM:
    case GFX_LFMT_D32_S8X24_FLOAT_UINT:
       return true;
       break;
    default:
       return false;
   }
}
