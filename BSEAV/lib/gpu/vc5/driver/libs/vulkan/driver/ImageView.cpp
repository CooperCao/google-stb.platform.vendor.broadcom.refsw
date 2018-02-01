/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "ControlListBuilder.h"

#include "libs/core/gfx_buffer/gfx_buffer_translate_v3d.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_tmu.h"

namespace bvk {

static uint32_t BasicDims(VkImageViewType type)
{
   switch (type)
   {
   case VK_IMAGE_VIEW_TYPE_1D         :
   case VK_IMAGE_VIEW_TYPE_1D_ARRAY   : return 1;
   case VK_IMAGE_VIEW_TYPE_2D         :
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY   :
   case VK_IMAGE_VIEW_TYPE_CUBE       : // For query purposes, cube is only 2d
   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY : return 2;
   case VK_IMAGE_VIEW_TYPE_3D         : return 3;
   default                            : unreachable();
   }
}

static bool IsArrayed(VkImageViewType type)
{
   switch (type)
   {
   case VK_IMAGE_VIEW_TYPE_1D_ARRAY    :
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY    :
   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY  : return true;
   default                             : return false;
   }
}

ImageView::ImageView(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkImageViewCreateInfo   *pCreateInfo) :
      Allocating(pCallbacks)
{
   m_image = fromHandle<Image>(pCreateInfo->image);
   m_isCubeArray = pCreateInfo->viewType == VK_IMAGE_VIEW_TYPE_CUBE_ARRAY;

   m_subresourceRange = pCreateInfo->subresourceRange;

   if (m_subresourceRange.layerCount == VK_REMAINING_ARRAY_LAYERS)
      m_subresourceRange.layerCount = m_image->ArrayLayers() - m_subresourceRange.baseArrayLayer;

   if (m_subresourceRange.levelCount == VK_REMAINING_MIP_LEVELS)
      m_subresourceRange.levelCount = m_image->MipLevels() - m_subresourceRange.baseMipLevel;

   GFX_BUFFER_DESC_T desc = m_image->GetDescriptor(m_subresourceRange.baseMipLevel);

   // Cache the dimensions
   m_baseExtent.width  = desc.width;
   m_baseExtent.height = desc.height;
   m_baseExtent.depth  = desc.depth;

   uint32_t dims = BasicDims(pCreateInfo->viewType);
   uint32_t slice = 0;

   if (dims == 2 && gfx_lfmt_is_3d(m_image->LFMT()))
   {
      // We want a 2D slice of the underlying 3D image
      m_baseExtent.depth = 1;

      // The required slice is unhelpfully passed in by Vulkan as the baseArrayLayer
      slice = m_subresourceRange.baseArrayLayer;
      m_subresourceRange.baseArrayLayer = 0;
   }

   if (IsArrayed(pCreateInfo->viewType))
   {
      assert(dims == 1 || dims == 2);

      if (dims == 1)
         m_baseExtent.height = m_subresourceRange.layerCount;
      else
      {
         if (m_isCubeArray)
            // Cubemap arrays report the number of cubes in the last element
            m_baseExtent.depth = m_subresourceRange.layerCount / 6;
         else
            m_baseExtent.depth = m_subresourceRange.layerCount;
      }
   }

   const VkImageUsageFlags requiresTLB = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT |
                                         VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
   const VkImageUsageFlags requiresTMU = VK_IMAGE_USAGE_SAMPLED_BIT | VK_IMAGE_USAGE_STORAGE_BIT |
                                         VK_IMAGE_USAGE_INPUT_ATTACHMENT_BIT;

   if (m_image->Usage() & requiresTMU)
   {
      // Create lazy texture memory now - it can't stay empty if we're going to texture from it
      m_image->AllocateLazyMemory();

      CreateTSR(m_image, pCreateInfo->format, pCreateInfo->viewType, pCreateInfo->components,
                m_subresourceRange);
   }

   if (m_image->Usage() & requiresTLB)
   {
      bool ms = m_image->Samples() != VK_SAMPLE_COUNT_1_BIT;

      // Replace the native lfmt in the descriptor with the HW lfmt of the
      // image view format (which may be different from the image).
      GFX_LFMT_T tlbLfmt = Formats::GetLFMT(pCreateInfo->format);
      Image::AdjustDescForTLBLfmt(&desc, tlbLfmt, ms);

      m_layerStride = m_image->LayerOffset(1);

      // Get a base physical address for the image memory block.
      // This is the base address of the entire memory block (which may be shared), not
      // the exact address of this particular image.
      uint32_t bOffset = m_image->LayerOffset(m_subresourceRange.baseArrayLayer);
      v3d_addr_t basePhys = bOffset;
      if (!m_image->HasUnallocatedLazyMemory())
         basePhys += m_image->PhysAddr();

      // Calculate the tlb params
      bool hasColor = gfx_lfmt_has_color(desc.planes[0].lfmt);
      gfx_buffer_translate_tlb_ldst(&m_tlbParams, basePhys, &desc, /*plane_i=*/0, slice,
                                    hasColor, ms, /*ext_ms=*/ms, V3D_DITHER_OFF);
   }
}

ImageView::~ImageView() noexcept
{
}

void ImageView::AllocateLazyMemory()
{
   if (m_image->HasUnallocatedLazyMemory())
   {
      m_image->AllocateLazyMemory();
      m_tlbParams.addr += m_image->PhysAddr();
   }
}

static v3d_tmu_swizzle_t CalcSwizzle(uint32_t channel, VkComponentSwizzle comp, const v3d_tmu_swizzle_t src[4])
{
   switch (comp)
   {
   case VK_COMPONENT_SWIZZLE_IDENTITY: return src[channel];
   case VK_COMPONENT_SWIZZLE_ZERO:     return V3D_TMU_SWIZZLE_0;
   case VK_COMPONENT_SWIZZLE_ONE:      return V3D_TMU_SWIZZLE_1;
   case VK_COMPONENT_SWIZZLE_R:        return src[0];
   case VK_COMPONENT_SWIZZLE_G:        return src[1];
   case VK_COMPONENT_SWIZZLE_B:        return src[2];
   case VK_COMPONENT_SWIZZLE_A:        return src[3];
   default:                            unreachable();
   }
}

static void SetSwizzles(v3d_tmu_swizzle_t dst[4], VkComponentMapping mapping, const v3d_tmu_swizzle_t src[4])
{
   dst[0] = CalcSwizzle(0, mapping.r, src);
   dst[1] = CalcSwizzle(1, mapping.g, src);
   dst[2] = CalcSwizzle(2, mapping.b, src);
   dst[3] = CalcSwizzle(3, mapping.a, src);
}

static uint32_t CalcArrayStride(VkImageViewType viewType, const GFX_BUFFER_DESC_T &imgDesc,
                                uint32_t imgArrayStride)
{
   switch (viewType)
   {
   case VK_IMAGE_VIEW_TYPE_1D:
   case VK_IMAGE_VIEW_TYPE_2D:
      return 0;

   case VK_IMAGE_VIEW_TYPE_3D:
      return imgDesc.planes[0].slice_pitch;

   case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
   case VK_IMAGE_VIEW_TYPE_CUBE:
   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
      return imgArrayStride;

   default:
      unreachable();
   }
}

static uint32_t CalcDepth(VkImageViewType viewType, const GFX_BUFFER_DESC_T &imgDesc,
                          uint32_t layerCount)
{
   switch (viewType)
   {
   case VK_IMAGE_VIEW_TYPE_1D:
   case VK_IMAGE_VIEW_TYPE_2D:
   case VK_IMAGE_VIEW_TYPE_3D:
      return imgDesc.depth;

   case VK_IMAGE_VIEW_TYPE_1D_ARRAY:
   case VK_IMAGE_VIEW_TYPE_2D_ARRAY:
   case VK_IMAGE_VIEW_TYPE_CUBE:
      return layerCount;

   case VK_IMAGE_VIEW_TYPE_CUBE_ARRAY:
      return layerCount / 6;

   default:
      unreachable();
   }
}

static void GetTMUTranslation(GFX_LFMT_TMU_TRANSLATION_T *t,
                              GFX_LFMT_T fmt, VkImageAspectFlags aspects)
{
   GFX_LFMT_T fmtAspect = fmt;
   if (gfx_lfmt_has_depth(fmt) && gfx_lfmt_has_stencil(fmt))
   {
      // Format has both depth & stencil - not a valid TMU format
      if ((aspects & VK_IMAGE_ASPECT_DEPTH_BIT) && (aspects & VK_IMAGE_ASPECT_STENCIL_BIT))
      {
         // Both depth & stencil are wanted. The CTS makes images like this but won't actually
         // texture directly from them. It will make separate depth or stencil views from it.
         // To avoid crashing, we will simply fake a same sized R32 image.
         fmtAspect = GFX_LFMT_R32_UINT;
      }
      else if (aspects & VK_IMAGE_ASPECT_STENCIL_BIT)      // Only stencil is wanted
         fmtAspect = gfx_lfmt_depth_to_x(fmt);
      else if (aspects & VK_IMAGE_ASPECT_DEPTH_BIT)   // Only depth is wanted
         fmtAspect = gfx_lfmt_stencil_to_x(fmt);
   }

   gfx_lfmt_translate_tmu(t, gfx_lfmt_ds_to_red(fmtAspect)
#if !V3D_HAS_TMU_R32F_R16_SHAD
      , gfx_lfmt_has_depth(fmtAspect)
#endif
      );
}

void ImageView::CreateTSR(const Image *image, VkFormat format, VkImageViewType viewType,
                          VkComponentMapping components, const VkImageSubresourceRange &subresourceRange)
{
   const auto &imgDesc = image->GetDescriptor(/*baseMip=*/0);

   V3D_TMU_TEX_STATE_T tsr = {};

   tsr.l0_addr = image->PhysAddr() + imgDesc.planes[0].offset +
                 image->LayerOffset(subresourceRange.baseArrayLayer);

   if (BasicDims(viewType) == 1)
      v3d_tmu_get_wh_for_1d_tex_state(&tsr.width, &tsr.height, imgDesc.width);
   else
   {
      tsr.width  = imgDesc.width;
      tsr.height = imgDesc.height;
   }

   tsr.depth      = CalcDepth(viewType, imgDesc, subresourceRange.layerCount);
   tsr.base_level = subresourceRange.baseMipLevel;
   tsr.max_level  = subresourceRange.baseMipLevel + subresourceRange.levelCount - 1;
   tsr.arr_str    = CalcArrayStride(viewType, imgDesc, image->LayerSize());

   GFX_LFMT_T fmt = Formats::GetLFMT(format);

   GFX_LFMT_TMU_TRANSLATION_T t;
   GetTMUTranslation(&t, fmt, subresourceRange.aspectMask);

   tsr.type = t.type;
   tsr.srgb = t.srgb;

   SetSwizzles(tsr.swizzles, components, t.swizzles);

   gfx_buffer_uif_cfg uifCfg;
   gfx_buffer_get_tmu_uif_cfg(&uifCfg, &imgDesc, 0);

   tsr.extended = uifCfg.force;

   v3d_pack_tmu_tex_state(m_texState, &tsr);
   m_texStateSize = V3D_TMU_TEX_STATE_PACKED_SIZE;

   if (tsr.extended)
   {
      assert(!uifCfg.ub_noutile);

      V3D_TMU_TEX_EXTENSION_T e;
      e.ub_pad  = uifCfg.ub_pads[0];
      e.ub_xor  = uifCfg.ub_xor;
      e.uif_top = uifCfg.force;
      e.xor_dis = uifCfg.xor_dis;
      v3d_pack_tmu_tex_extension(m_texState + m_texStateSize, &e);
      m_texStateSize += V3D_TMU_TEX_EXTENSION_PACKED_SIZE;
   }
}

void ImageView::WriteTextureStateRecord(uint8_t *ptr, VkDescriptorType type) const
{
   assert(m_texStateSize > 0);

   // Copy the TSR from system memory
   memcpy(ptr, m_texState, m_texStateSize);

   if (type == VK_DESCRIPTOR_TYPE_STORAGE_IMAGE && m_isCubeArray)
   {
      // Fix-up the depth for cube arrays now we know for sure that it will be fetched, not sampled.
      // The only safe way to do this is to unpack-modify-and-repack the structure. Yuck
      V3D_TMU_TEX_STATE_T tsr;
      v3d_unpack_tmu_tex_state(&tsr, m_texState);
      tsr.depth *= 6;
      v3d_pack_tmu_tex_state(ptr, &tsr);
   }
}

} // namespace bvk
