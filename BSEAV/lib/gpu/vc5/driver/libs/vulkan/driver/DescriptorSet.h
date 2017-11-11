/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "DescriptorSetLayout.h"
#include "DescriptorPool.h"

namespace bvk {

class DescriptorSet: public NonCopyable
{
public:
   DescriptorSet(const DescriptorSetLayout *layout, void *data, size_t dataSize,
                 gmem_handle_t devMem, size_t devMemOffset, size_t devDataSize);
   ~DescriptorSet() noexcept;

   void Write(const VkWriteDescriptorSet *writeInfo);
   void Copy(const VkCopyDescriptorSet *copyInfo);

   uint32_t   GetImageParam(uint32_t binding, uint32_t element) const;
   uint32_t   GetSamplerParam(uint32_t binding, uint32_t element) const;
   v3d_addr_t GetBufferAddress(uint32_t binding, uint32_t element) const;
   uint32_t   GetBufferSize(uint32_t binding, uint32_t element) const;
   uint32_t   GetBufferRange(uint32_t binding, uint32_t element) const;
   uint32_t   GetNumLevels(uint32_t binding, uint32_t element) const;
   VkExtent3D GetTextureSize(uint32_t binding, uint32_t element) const;

   bool HasDynamicOffset(uint32_t binding, uint32_t arrayElement) const
   {
      return m_layout->HasDynamicOffset(binding, arrayElement);
   }

   uint32_t DynamicOffsetIndexFor(uint32_t binding, uint32_t arrayElement) const
   {
      return m_layout->DynamicOffsetIndexFor(binding, arrayElement);
   }

   uint32_t GetNumDynamicOffsetsNeeded() const
   {
      return m_layout->GetNumDynamicOffsetsNeeded();
   }

   bool GetDevMemDetails(size_t *offset, size_t *size) const
   {
      *offset = m_devMemOffset;
      *size   = m_devDataSize;
      return m_devDataSize > 0;
   }

private:
   void WriteImage(const VkWriteDescriptorSet *writeInfo, size_t sysOffset, size_t devOffset);
   void WriteTexelBuffer(const VkWriteDescriptorSet *writeInfo, size_t sysOffset, size_t devOffset);
   void WriteBuffer(const VkWriteDescriptorSet *writeInfo, size_t sysOffset, size_t devOffset);
   const ImageView  *GetImageView (uint32_t binding, uint32_t element) const;

private:
   // The layout state is shared with the DescriptorSetLayout because the lifetime of the
   // layout isn't guaranteed to exceed the lifetime of the set.
   std::shared_ptr<DescriptorSetLayoutState> m_layout;

   uint8_t       *m_data;
   size_t         m_dataSize;
   gmem_handle_t  m_devMemHandle;
   size_t         m_devMemOffset;
   size_t         m_devDataSize;
};

} // namespace bvk
