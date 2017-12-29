/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "vulkan.h"
#include "libs/core/lfmt/lfmt.h"
#include "libs/core/v3d/v3d_gen.h"

namespace bvk
{

class Formats
{
public:
   enum FmtNumeric
   {
      NO_NUMFMT = 0,
      UNORM,
      SNORM,
      USCALED,
      SSCALED,
      UINT,
      SINT,
      UFLOAT,
      SFLOAT,
      SRGB,
      UNORM_S8_UINT,
      SFLOAT_S8_UINT,
   };

   enum FmtType
   {
      NO_FMTTYPE = 0,
      BLOCK = 1,
      _BLOCK = 1,
      PACK8 = 2,
      _PACK8 = 2,
      PACK16 = 3,
      _PACK16 = 3,
      PACK32 = 4,
      _PACK32 = 4
   };

   enum FmtComponent
   {
      UNDEFINED = 0,
      R4G4,
      R4G4B4A4,
      B4G4R4A4,
      R5G6B5,
      B5G6R5,
      R5G5B5A1,
      B5G5R5A1,
      A1R5G5B5,
      R8,
      R8G8,
      R8G8B8,
      B8G8R8,
      R8G8B8A8,
      B8G8R8A8,
      A8B8G8R8,
      A2R10G10B10,
      A2B10G10R10,
      R16,
      R16G16,
      R16G16B16,
      R16G16B16A16,
      R32,
      R32G32,
      R32G32B32,
      R32G32B32A32,
      R64,
      R64G64,
      R64G64B64,
      R64G64B64A64,
      B10G11R11,
      E5B9G9R9,
      X8_D24,
      S8,
      D16,
      D24,
      D32,
      BC1,
      BC2,
      BC3,
      BC4,
      BC5,
      BC6H,
      BC7,
      BC1_RGB,
      BC1_RGBA,
      ETC2_R8G8B8,
      ETC2_R8G8B8A1,
      ETC2_R8G8B8A8,
      EAC_R11,
      EAC_R11G11,
      ASTC_4x4,
      ASTC_5x4,
      ASTC_5x5,
      ASTC_6x5,
      ASTC_6x6,
      ASTC_8x5,
      ASTC_8x6,
      ASTC_8x8,
      ASTC_10x5,
      ASTC_10x6,
      ASTC_10x8,
      ASTC_10x10,
      ASTC_12x10,
      ASTC_12x12
   };

   enum FmtClass
   {
      CLASS_NONE = 0,
      CLASS_8,
      CLASS_16,
      CLASS_24,
      CLASS_32,
      CLASS_48,
      CLASS_64,
      CLASS_96,
      CLASS_128,
      CLASS_192,
      CLASS_256,
      CLASS_BC1_RGB,
      CLASS_BC1_RGBA,
      CLASS_BC2,
      CLASS_BC3,
      CLASS_BC4,
      CLASS_BC5,
      CLASS_BC6H,
      CLASS_BC7,
      CLASS_ETC2_RGB,
      CLASS_ETC2_RGBA,
      CLASS_ETC2_EAC_RGBA,
      CLASS_EAC_R,
      CLASS_EAC_RG,
      CLASS_ASTC_4x4,
      CLASS_ASTC_5x4,
      CLASS_ASTC_5x5,
      CLASS_ASTC_6x5,
      CLASS_ASTC_6x6,
      CLASS_ASTC_8x5,
      CLASS_ASTC_8x6,
      CLASS_ASTC_8x8,
      CLASS_ASTC_10x5,
      CLASS_ASTC_10x6,
      CLASS_ASTC_10x8,
      CLASS_ASTC_10x10,
      CLASS_ASTC_12x10,
      CLASS_ASTC_12x12,
      CLASS_D16,
      CLASS_D24,
      CLASS_D32,
      CLASS_S8,
      CLASS_D16S8,
      CLASS_D24S8,
      CLASS_D32S8
   };

   enum FormatUsage
   {
      NATURAL,
      HW,
   };

public:
   Formats();

   static bool AreCompatible(VkFormat fmt1, VkFormat fmt2)
   {
      return m_fmts[fmt1].fmtClass == m_fmts[fmt2].fmtClass;
   }

   static bool IsClass(VkFormat fmt, FmtClass fmtClass)
   {
      return m_fmts[fmt].fmtClass == fmtClass;
   }

   static bool IsCompressed(VkFormat fmt)
   {
      return m_fmts[fmt].fmtType == BLOCK;
   }

   static FmtClass CompressedBlockCompatibleClass(VkFormat fmt)
   {
      assert(IsCompressed(fmt));
      switch (m_fmts[fmt].fmtClass)
      {
      case CLASS_BC1_RGB:
      case CLASS_BC1_RGBA:
      case CLASS_BC4:
      case CLASS_ETC2_RGB:
      case CLASS_ETC2_RGBA:
      case CLASS_EAC_R: return CLASS_64;

      case CLASS_BC2:
      case CLASS_BC3:
      case CLASS_BC5:
      case CLASS_EAC_RG:
      case CLASS_ETC2_EAC_RGBA:
      case CLASS_ASTC_4x4:
      case CLASS_ASTC_5x4:
      case CLASS_ASTC_5x5:
      case CLASS_ASTC_6x5:
      case CLASS_ASTC_6x6:
      case CLASS_ASTC_8x5:
      case CLASS_ASTC_8x6:
      case CLASS_ASTC_8x8:
      case CLASS_ASTC_10x5:
      case CLASS_ASTC_10x6:
      case CLASS_ASTC_10x8:
      case CLASS_ASTC_10x10:
      case CLASS_ASTC_12x10:
      case CLASS_ASTC_12x12: return CLASS_128;

      default: return CLASS_NONE; // Block format unsupported by this driver
      }
   }

   static FmtComponent Component(VkFormat fmt)
   {
      return m_fmts[fmt].component;
   }

   static GFX_LFMT_T GetLFMT(VkFormat fmt, FormatUsage usage)
   {
      switch (usage)
      {
      default      :
      case NATURAL : return m_fmts[fmt].naturalFmt;
      case HW      : return m_fmts[fmt].hwFmt;
      }
   }

   static bool HasHardwareSupport(VkFormat fmt)
   {
      return m_fmts[fmt].naturalFmt != GFX_LFMT_NONE;
   }

   static bool HasNativeSupport(VkFormat fmt)
   {
         return (m_fmts[fmt].naturalFmt == m_fmts[fmt].hwFmt);
   }

   static bool HasTMUSupport(VkFormat fmt);
   static bool HasTLBSupport(VkFormat fmt);

   static bool HasDepth(VkFormat fmt)
   {
      switch (fmt)
      {
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_X8_D24_UNORM_PACK32:
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_D16_UNORM_S8_UINT:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
         return true;
      default:
         return false;
      }
   }

   static uint32_t DepthBits(VkFormat fmt)
   {
      switch (fmt)
      {
      case VK_FORMAT_D16_UNORM:
      case VK_FORMAT_D16_UNORM_S8_UINT:   return 16;
      case VK_FORMAT_X8_D24_UNORM_PACK32:
      case VK_FORMAT_D24_UNORM_S8_UINT:   return 24;
      case VK_FORMAT_D32_SFLOAT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT:  return 32;
      default:                            return 0;
      }
   }

   static bool HasStencil(VkFormat fmt)
   {
      switch (fmt)
      {
      case VK_FORMAT_S8_UINT:
      case VK_FORMAT_D16_UNORM_S8_UINT:
      case VK_FORMAT_D24_UNORM_S8_UINT:
      case VK_FORMAT_D32_SFLOAT_S8_UINT:
         return true;
      default:
         return false;
      }
   }

   static bool HasColor(VkFormat fmt)
   {
      return !(HasDepth(fmt) || HasStencil(fmt));
   }

   static VkImageAspectFlags GetAspects(VkFormat fmt)
   {
      VkImageAspectFlags flags = 0;
      if (HasDepth(fmt))
         flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
      if (HasStencil(fmt))
         flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
      if (HasColor(fmt))
         flags |= VK_IMAGE_ASPECT_COLOR_BIT;
      return flags;
   }

   static bool IsInteger(VkFormat fmt)
   {
      switch (m_fmts[fmt].numericFmt)
      {
      case UINT:
      case SINT: return true;
      default  : return false;
      }
   }

   static bool IsScaled(VkFormat fmt)
   {
      switch (m_fmts[fmt].numericFmt)
      {
      case USCALED:
      case SSCALED: return true;
      default     : return false;
      }
   }

   static bool IsSigned(VkFormat fmt)
   {
      switch (m_fmts[fmt].numericFmt)
      {
      case SNORM           :
      case SSCALED         :
      case SINT            :
      case SFLOAT          :
      case SFLOAT_S8_UINT  : return true;
      default              : return false;
      }
   }

   static bool IsNormalized(VkFormat fmt)
   {
      switch (m_fmts[fmt].numericFmt)
      {
      case UNORM           :
      case SNORM           :
      case UNORM_S8_UINT   : return true;
      default              : return false;
      }
   }

   static bool NeedsAttributeRBSwap(VkFormat fmt)
   {
      switch (m_fmts[fmt].component)
      {
      case B8G8R8    :
      case B8G8R8A8  : return true;
      default        : return false;
      }
   }

   static size_t NumBytes(VkFormat fmt)
   {
      return gfx_lfmt_bytes_per_block(m_fmts[fmt].hwFmt);
   }

   static v3d_attr_type_t GetAttributeType(VkFormat fmt);

private:
   struct Format
   {
      VkFormat       fmtEnum;
      FmtComponent   component;
      FmtNumeric     numericFmt;
      FmtClass       fmtClass;
      FmtType        fmtType;
      GFX_LFMT_T     naturalFmt; // Internal format that most naturally matches VkFormat
      GFX_LFMT_T     hwFmt;      // The actual HW format we will use internally
   };

   static Format m_fmts[];
};

} // namespace bvk
