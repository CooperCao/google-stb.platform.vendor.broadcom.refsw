/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Common.h"
#include "CommandBuffer.h"
#include "Command.h"
#include "ColorAspectCommandBuilder.h"

#include "libs/util/gfx_util/gfx_util_conv.h"
#include "libs/util/log/log.h"

#include "libs/core/lfmt/lfmt_translate_v3d.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::CommandBuffer");

// Helper to handle formats not natively supported by the TLB.
static void FixupLFMTAndClearColorsForTLB(
      Image                    *img,
      const VkClearColorValue *pColor,
      uint32_t                *hwColors,
      GFX_LFMT_T              *tlbLFMT)
{
   // TODO: This function has lots of special cases. A more general method might work better.
   const auto fmtMask = GFX_LFMT_BASE_MASK | GFX_LFMT_TYPE_MASK | GFX_LFMT_CHANNELS_MASK;

   v3d_pixel_format_t pf;
   bool rev, swap;
   bool hasTLBSupport = gfx_lfmt_maybe_translate_pixel_format(img->LFMT(), &pf, &rev, &swap);

   if (!hasTLBSupport)
   {
      // Formats supported by the TMU but not the TLB which we have to be
      // able to clear. If the spec also mandates color attachment support
      // then we would be in the case above.
      switch (img->LFMT() & fmtMask)
      {
         case GFX_LFMT_R9G9B9SHAREDEXP5_UFLOAT:
            *tlbLFMT = GFX_LFMT_R32_UINT;
            hwColors[0] = gfx_floats_to_rgb9e5(pColor->float32);
            break;
         case GFX_LFMT_R8_SNORM:
         case GFX_LFMT_R8_G8_SNORM:
         case GFX_LFMT_R8_G8_B8_A8_SNORM:
         case GFX_LFMT_B8_G8_R8_A8_SNORM:
         case GFX_LFMT_R16_SNORM:
         case GFX_LFMT_R16_G16_SNORM:
         case GFX_LFMT_R16_G16_B16_A16_SNORM:
            *tlbLFMT = img->LFMT();
            gfx_lfmt_set_type(tlbLFMT, GFX_LFMT_TYPE_UINT);
            // All channels have the same number of bits, so use red, which we know exists
            for (int i = 0; i < 4; i++)
               hwColors[i] = gfx_float_to_snorm(pColor->float32[i], gfx_lfmt_red_bits(*tlbLFMT));
            break;
         case GFX_LFMT_R16_UNORM:
         case GFX_LFMT_R16_G16_UNORM:
         case GFX_LFMT_R16_G16_B16_A16_UNORM:
            *tlbLFMT = img->LFMT();
            gfx_lfmt_set_type(tlbLFMT, GFX_LFMT_TYPE_UINT);
            // All channels have the same number of bits, so use red, which we know exists
            for (int i = 0; i < 4; i++)
               hwColors[i] = gfx_float_to_unorm(pColor->float32[i], gfx_lfmt_red_bits(*tlbLFMT));
            break;
         case GFX_LFMT_R8_SRGB:
         case GFX_LFMT_R8_G8_SRGB:
            *tlbLFMT = img->LFMT();
            gfx_lfmt_set_type(tlbLFMT, GFX_LFMT_TYPE_UNORM);
            for (int i = 0; i < 4; i++)
               hwColors[i] = gfx_float_to_bits(gfx_lin_to_srgb_float(pColor->float32[i]));
            break;
         default:
            unreachable();
            return;
      }
   }
}

void CommandBuffer::CmdClearColorImage(
   bvk::Image                    *image,
   VkImageLayout                  imageLayout,
   const VkClearColorValue       *pColor,
   uint32_t                       rangeCount,
   const VkImageSubresourceRange *pRanges) noexcept
{
   CMD_BEGIN
   assert(!InRenderPass());
   assert(gfx_lfmt_has_color(image->LFMT()));
   assert(!gfx_lfmt_is_compressed(image->LFMT()));

   log_trace("CmdClearColorImage: image = %p", image);

   GFX_LFMT_T tlbLFMT = GFX_LFMT_NONE;
   uint32_t hwColors[4] = {
         pColor->uint32[0], pColor->uint32[1], pColor->uint32[2], pColor->uint32[3]
   };

   if (image->Extent().width  > V3D_MAX_CLIP_WIDTH || image->Extent().height > V3D_MAX_CLIP_HEIGHT)
   {
      NOT_IMPLEMENTED_YET;
      log_warn("\tClearing of images larger than will fit in the TLB is not supported yet");
   }

   FixupLFMTAndClearColorsForTLB(image, pColor, hwColors, &tlbLFMT);

   for (uint32_t i = 0; i < rangeCount; i++)
   {
      uint32_t mipLevelCount = (pRanges[i].levelCount == VK_REMAINING_MIP_LEVELS) ?
            image->MipLevels() - pRanges[i].baseMipLevel : pRanges[i].levelCount;

      for (uint32_t m = 0; m < mipLevelCount; m++)
      {
         uint32_t mipLevel = pRanges[i].baseMipLevel + m;
         constexpr uint32_t concurrentTargets = V3D_MAX_RENDER_TARGETS;

         // If we are clearing multiple array layers or a 3D image then use as
         // many rendertargets as we can to minimize the number of jobs
         // submitted.
         uint32_t layerCount, baseArrayLayer;

         if (gfx_lfmt_is_3d(image->LFMT()))
         {
            layerCount = std::max(1u, static_cast<uint32_t>(image->Extent().depth >> mipLevel));
            baseArrayLayer = 0;
         }
         else
         {
            layerCount = (pRanges[i].layerCount == VK_REMAINING_ARRAY_LAYERS) ?
                  image->ArrayLayers() - pRanges[i].baseArrayLayer : pRanges[i].layerCount;
            baseArrayLayer = pRanges[i].baseArrayLayer;
         }

         for (uint32_t l = 0; l < layerCount; l += concurrentTargets)
         {
            ColorAspectCommandBuilder cb {GetCallbacks(), this};

            uint32_t numTargets = std::min(layerCount - l, concurrentTargets);
            uint32_t startLayer = baseArrayLayer + l;

            cb.SetImageSubresources(image, tlbLFMT, mipLevel, startLayer, numTargets);
            cb.SetClearParams(hwColors);

            auto cmd = NewObject<CmdBinRenderJobObj>(m_device->GetPhysicalDevice());

            log_trace("\tadding job for mipLevel = %u startLayer = %d numTargets = %u",
                  mipLevel, startLayer, numTargets);

            // Default control list builder sync flags are OK for TLB clears
            cb.CreateMasterControlLists(cmd);

            // We need to add our own execution barriers between regions as each
            // job may be writing to portions of the same memory. On the multi-core
            // simulator this will cause rendering errors as tile writes fight
            // against each other.
            if (i != 0)
               InsertExecutionBarrier();

            m_commandList.push_back(cmd);
         }
      }
   }
   CMD_END
}

} // namespace bvk
