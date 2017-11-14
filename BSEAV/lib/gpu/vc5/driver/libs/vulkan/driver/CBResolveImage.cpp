/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Common.h"
#include "CommandBuffer.h"
#include "Command.h"
#include "ColorAspectCommandBuilder.h"

#include "libs/core/lfmt/lfmt.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/util/log/log.h"


namespace bvk {

LOG_DEFAULT_CAT("bvk::CommandBuffer");

void CommandBuffer::ResolveImageRegionTLB(
   bvk::Image *srcImage,
   bvk::Image *dstImage,
   const VkImageResolve &region)
{
   const bool dstIs3D         = gfx_lfmt_is_3d(dstImage->NaturalLFMT());
   const uint32_t layerCount  = region.dstSubresource.layerCount;
   const uint32_t dstMipLevel = region.dstSubresource.mipLevel;

   constexpr uint32_t concurrentLayers = V3D_MAX_RENDER_TARGETS;

   for (uint32_t l = 0; l < layerCount; l += concurrentLayers)
   {
      ColorAspectCommandBuilder cb {GetCallbacks(), this};

      uint32_t numLayers = std::min(layerCount - l, concurrentLayers);
      // For 3D destination images we know layerCount must be 1 and we can
      // only be resolving into a single slice at the destination offset z.
      uint32_t srcBaseLayer =
            dstIs3D ? 0 : region.srcSubresource.baseArrayLayer + l;
      uint32_t dstBaseLayer =
            dstIs3D ? region.dstOffset.z : region.dstSubresource.baseArrayLayer + l;

      cb.SetLoadOffset(region.srcOffset.x, region.srcOffset.y);
      cb.SetStoreOffset(region.dstOffset.x, region.dstOffset.y);

      cb.SetImageSubresources(
            srcImage, dstImage, GFX_LFMT_NONE,
            /* srcMipLevel =*/ 0, dstMipLevel,
            srcBaseLayer, dstBaseLayer, numLayers,
            region.extent.width, region.extent.height);

      auto cmd = NewObject<CmdBinRenderJobObj>(m_device->GetPhysicalDevice());

      log_trace("\tTLB LD/ST job for dstMipLevel = %u srcBaseLayer = %d dstBaseLayer = %d numLayers = %u",
            dstMipLevel, srcBaseLayer, dstBaseLayer, numLayers);

      // Default control list builder sync flags are OK for TLB only resolve
      cb.CreateMasterControlLists(cmd);

      m_commandList.push_back(cmd);
   }
}

void CommandBuffer::ResolveImageRegionTMU(
   bvk::Image *srcImage,
   bvk::Image *dstImage,
   const VkImageResolve &region)
{
   VkImageBlit blit;
   blit.srcSubresource = region.srcSubresource;
   blit.dstSubresource = region.dstSubresource;

   blit.srcOffsets[0].x = region.srcOffset.x * 2;
   blit.srcOffsets[0].y = region.srcOffset.y * 2;
   blit.srcOffsets[0].z = 0;
   blit.srcOffsets[1].x = (region.srcOffset.x + region.extent.width) * 2;
   blit.srcOffsets[1].y = (region.srcOffset.y + region.extent.height) * 2;
   blit.srcOffsets[1].z = 1;

   blit.dstOffsets[0] = region.dstOffset;
   blit.dstOffsets[1].x = region.dstOffset.x + region.extent.width;
   blit.dstOffsets[1].y = region.dstOffset.y + region.extent.height;
   blit.dstOffsets[1].z = region.dstOffset.z + region.extent.depth;

   GFX_LFMT_T tmuLFMT = srcImage->HardwareLFMT();
   VkFilter filter = gfx_lfmt_contains_int(tmuLFMT) ? VK_FILTER_NEAREST : VK_FILTER_LINEAR;

   if (srcImage->Extent().width  > V3D_MAX_CLIP_WIDTH  || dstImage->Extent().width  > V3D_MAX_CLIP_WIDTH ||
       dstImage->Extent().height > V3D_MAX_CLIP_HEIGHT || dstImage->Extent().height > V3D_MAX_CLIP_HEIGHT )
   {
      NOT_IMPLEMENTED_YET;
      log_warn("\tResolving of images larger than will fit in the TLB is not supported yet");
   }

   if (gfx_lfmt_contains_float(tmuLFMT) && gfx_lfmt_red_bits(tmuLFMT) == 32)
   {
      // TODO: The TMU cannot linear filter float32 formats, we would need to
      //       write a special shader that loaded the samples and did the maths
      //       itself.
      NOT_IMPLEMENTED_YET;
      log_warn("\tSelecting sample 0 for float32 format");
      filter = VK_FILTER_NEAREST;
   }

   BlitImageRegion(srcImage, dstImage, blit, filter);
}

void CommandBuffer::CmdResolveImage(
   bvk::Image           *srcImage,
   VkImageLayout         srcImageLayout,
   bvk::Image           *dstImage,
   VkImageLayout         dstImageLayout,
   uint32_t              regionCount,
   const VkImageResolve *pRegions) noexcept
{
   CMD_BEGIN
   assert(m_mode == eRECORDING);
   assert(!InRenderPass());

   GFX_LFMT_T srcLFMT = srcImage->NaturalLFMT();
   GFX_LFMT_T dstLFMT = dstImage->NaturalLFMT();

   // Only 2D multi-sampled images are allowed by the spec, see:
   // v1.0.26: 31.4.1 Supported Sample Counts
   assert(gfx_lfmt_is_2d(srcLFMT));
   assert(srcImage->Samples() == VK_SAMPLE_COUNT_4_BIT);

   // This call can only resolve uncompressed color images, it is specifically
   // stated that resolve of depth/stencil is not supported.
   assert(gfx_lfmt_has_color(srcLFMT) && !gfx_lfmt_is_compressed(srcLFMT));

   assert(dstImage->Samples() == VK_SAMPLE_COUNT_1_BIT);
   assert(gfx_lfmt_has_color(dstLFMT) && !gfx_lfmt_is_compressed(dstLFMT));

   log_trace("CmdResolveImage: srcImage = %p dstImage = %p", srcImage, dstImage);

   const bool dstIs3D = gfx_lfmt_is_3d(dstLFMT);

   for (uint32_t i = 0; i < regionCount; i++)
   {
      // Note: The source must be a format supported as a color attachment
      //       in tiled optimal features
      //
      //       The destination must a format supported as a color attachment
      //       in tiled linear features if it is a linear tiled image

      assert(pRegions[i].srcSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT);
      assert(pRegions[i].dstSubresource.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT);
      assert(pRegions[i].srcSubresource.layerCount == pRegions[i].dstSubresource.layerCount);
      if (dstIs3D)
         assert(pRegions[i].dstSubresource.layerCount == 1);

      // multisample images cannot have more than one miplevel
      assert(pRegions[i].srcSubresource.mipLevel == 0);

      const VkOffset3D &srcOffset = pRegions[i].srcOffset;
      const VkOffset3D &dstOffset = pRegions[i].dstOffset;

#ifndef NDEBUG
      {
         const GFX_BUFFER_DESC_T &dDesc = dstImage->GetDescriptor(pRegions[i].dstSubresource.mipLevel);
         assert (dDesc.width  >= dstOffset.x + pRegions[i].extent.width);
         assert (dDesc.height >= dstOffset.y + pRegions[i].extent.height);

         const GFX_BUFFER_DESC_T &sDesc = srcImage->GetDescriptor(0);
         assert (sDesc.width  >= srcOffset.x + pRegions[i].extent.width);
         assert (sDesc.height >= srcOffset.y + pRegions[i].extent.height);
      }
#endif

      log_trace("\t%ux%u@(%d,%d) [Mip%u,Arry%u+%u] -> %ux%u@(%d,%d) [Mip%u,Arry%u+%u]",
         pRegions[i].extent.width, pRegions[i].extent.height,
         srcOffset.x, srcOffset.y,
         pRegions[i].srcSubresource.mipLevel,
         pRegions[i].srcSubresource.baseArrayLayer,
         pRegions[i].srcSubresource.layerCount,
         pRegions[i].extent.width, pRegions[i].extent.height,
         dstOffset.x, dstOffset.y,
         pRegions[i].dstSubresource.mipLevel,
         pRegions[i].dstSubresource.baseArrayLayer,
         pRegions[i].dstSubresource.layerCount
      );

      // We need to add our own execution barriers between regions as each
      // job may be writing to portions of the same memory. On the multi-core
      // simulator this will cause rendering errors as tile writes fight
      // against each other.
      if (i != 0)
         InsertExecutionBarrier();

      bool canUseTLB = true;
      // We can use the TLB directly for some partial resolves, but this will
      // not trigger very often, we are mostly allowing it to test the
      // underlying code that offsets the image start address and sets up the
      // TLB.
      //
      // Resolving the whole source image into a bigger raster destination
      // is probably the only realistic usage. The limitations of UIF images,
      // particularly when XOR addressing is in use, means unless you happen to
      // know how the organisation works you are unlikely to trigger the
      // code path.
      VkOffset2D srcOffsetAlignments = ColorAspectCommandBuilder::GetImageOffsetAlignments(srcImage, 0);
      VkOffset2D dstOffsetAlignments = ColorAspectCommandBuilder::GetImageOffsetAlignments(dstImage,
                                                               pRegions[i].dstSubresource.mipLevel);

      log_trace("\tsrcOffsetAlignments = (%u,%u) dstOffsetAlignments = (%u,%u)",
            srcOffsetAlignments.x, srcOffsetAlignments.y,
            dstOffsetAlignments.x, dstOffsetAlignments.y);

      if (srcOffsetAlignments.x == 0 || srcOffsetAlignments.y == 0)
         canUseTLB = false;
      else if ((srcOffset.x % srcOffsetAlignments.x) != 0 || (srcOffset.y % srcOffsetAlignments.y) != 0)
         canUseTLB = false;

      if (dstOffsetAlignments.x == 0 || dstOffsetAlignments.y == 0)
         canUseTLB = false;
      else if ((dstOffset.x % dstOffsetAlignments.x) != 0 || (dstOffset.y % dstOffsetAlignments.y) != 0)
         canUseTLB = false;

#if !V3D_VER_AT_LEAST(4,1,34,0)
      // The BRGA (SRGB) format cases where we have to use RGBA internally. If
      // we are resolving to a linear BGRA image we will have to RB swap the
      // output using a shader.
      if (dstImage->KeepVulkanComponentOrder())
         canUseTLB = false;
#endif

      // The TLB cannot resolve 32bit floating point formats directly, it
      // needs a shader to read the samples and write back the resolved value.
      // So we might as well defer this problem to the TMU path as we have to
      // solve it there for the general case where the source and destination
      // offsets mean we cannot use the TLB anyway.
      if (gfx_lfmt_contains_float(srcLFMT) && gfx_lfmt_red_bits(srcLFMT) == 32)
         canUseTLB = false;

      if (canUseTLB)
         ResolveImageRegionTLB(srcImage, dstImage, pRegions[i]);
      else
         ResolveImageRegionTMU(srcImage, dstImage, pRegions[i]);
   }
   CMD_END
}

} // namespace bvk
