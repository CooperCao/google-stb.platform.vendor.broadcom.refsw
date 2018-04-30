/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "libs/core/gfx_buffer/gfx_buffer.h"

namespace bvk {

class ImageView: public NonCopyable, public Allocating
{
public:
   ImageView(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkImageViewCreateInfo   *pCreateInfo);

   ~ImageView() noexcept;

   // Implementation specific from this point on
   void GetTLBParams(v3d_tlb_ldst_params *p) const { *p = m_tlbParams; }

   const VkExtent3D &GetBaseExtent() const  { return m_baseExtent;   }
   uint32_t          GetNumLevels()  const  { return m_subresourceRange.levelCount; }
   uint32_t          GetNumLayers()  const  { return m_subresourceRange.layerCount; }
   uint32_t          GetLayerStride() const { return m_layerStride;  }

   VkSampleCountFlagBits GetSamples() const { return m_image->Samples(); }
   const VkImageSubresourceRange &GetSubresourceRange() const { return m_subresourceRange; }

   void AllocateLazyMemory();
   bool HasUnallocatedLazyMemory() const { return m_image->HasUnallocatedLazyMemory(); }

   void WriteTextureStateRecord(uint8_t *ptr, VkDescriptorType type) const;

private:
   void CreateTSR(const Image *image, VkFormat format, VkImageViewType viewType,
                  VkComponentMapping m_components, const VkImageSubresourceRange &subresourceRange,
                  uint32_t widthDivide, uint32_t heightDivide);

private:
   VkExtent3D                 m_baseExtent;  // Sizes for texture size
   VkImageSubresourceRange    m_subresourceRange;
   bool                       m_isCubeArray;
   Image                     *m_image = nullptr;
   v3d_tlb_ldst_params        m_tlbParams;
   uint32_t                   m_layerStride = 0;
   uint8_t                    m_texState[V3D_TMU_TEX_STATE_PACKED_SIZE + V3D_TMU_TEX_EXTENSION_PACKED_SIZE];
   size_t                     m_texStateSize = 0;
};

} // namespace bvk
