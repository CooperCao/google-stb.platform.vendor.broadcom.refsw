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
   Formats();

   static GFX_LFMT_T GetLFMT (VkFormat fmt) { return m_fmts[fmt].lfmt;     }
   static bool       IsScaled(VkFormat fmt) { return m_fmts[fmt].isScaled; }

   static bool HasTMUSupport(GFX_LFMT_T lfmt);
   static bool HasTLBSupport(GFX_LFMT_T lfmt);

   static VkImageAspectFlags GetAspects(GFX_LFMT_T lfmt)
   {
      VkImageAspectFlags flags = 0;
      if (gfx_lfmt_has_depth(lfmt))
         flags |= VK_IMAGE_ASPECT_DEPTH_BIT;
      if (gfx_lfmt_has_stencil(lfmt))
         flags |= VK_IMAGE_ASPECT_STENCIL_BIT;
      if (gfx_lfmt_has_color(lfmt))
         flags |= VK_IMAGE_ASPECT_COLOR_BIT;
      return flags;
   }

   static bool NeedsAttributeRBSwap(GFX_LFMT_T lfmt)
   {
      switch (gfx_lfmt_get_channels(&lfmt))
      {
      case GFX_LFMT_CHANNELS_BGR:
      case GFX_LFMT_CHANNELS_BGRA:
      case GFX_LFMT_CHANNELS_BGRX: return true;
      default:                     return false;
      }
   }

   static v3d_attr_type_t GetAttributeType(GFX_LFMT_T fmt);

private:
   struct Format
   {
      VkFormat       fmtEnum;
      bool           isScaled;
      GFX_LFMT_T     lfmt;
   };

   static Format m_fmts[];
};

} // namespace bvk
