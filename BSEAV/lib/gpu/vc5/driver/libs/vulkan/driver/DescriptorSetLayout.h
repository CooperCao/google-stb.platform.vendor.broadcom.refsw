/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

// The internal state of the DescriptorSetLayout
class DescriptorSetLayoutState : public NonCopyable, public Allocating
{
public:
   DescriptorSetLayoutState(
      const VkAllocationCallbacks            *pCallbacks,
      Device                                 *pDevice,
      const VkDescriptorSetLayoutCreateInfo  *pCreateInfo);

   // Implementation specific from this point on
   size_t BytesRequired()       const { return m_bytesRequired;    }
   size_t DeviceBytesRequired() const { return m_devBytesRequired; }

   size_t OffsetFor(uint32_t binding, uint32_t arrayElement) const
   {
      assert(binding < m_bindings.size());
      auto &b = m_bindings[binding];
      return b.offset + b.bytesPerElement * arrayElement;
   }

   size_t DeviceOffsetFor(uint32_t binding, uint32_t arrayElement) const
   {
      assert(binding < m_bindings.size());
      auto &b = m_bindings[binding];
      return b.devOffset + b.devBytesPerElement * arrayElement;
   }

   bool HasDynamicOffset(uint32_t binding) const
   {
      assert(binding < m_bindings.size());
      return m_bindings[binding].isDynamic;
   }

   uint32_t DynamicOffsetIndexFor(uint32_t binding) const
   {
      assert(binding < m_bindings.size());
      return m_bindings[binding].dynamicOffset;
   }

   VkDescriptorType BindingType(uint32_t binding) const
   {
      assert(binding < m_bindings.size());
      return m_bindings[binding].descriptorType;
   }

   uint32_t GetNumDynamicOffsetsNeeded() const { return m_numDynamicOffsetsNeeded; }

   bool HasImmutableSamplers(uint32_t binding) const
   {
      assert(binding < m_bindings.size());
      return m_bindings[binding].immutableSamplers.size() != 0;
   }

   bool WriteImmutableSamplerRecords(uint8_t *sysPtr, size_t devOffset, gmem_handle_t devHandle) const;

private:
   struct PerBindingData
   {
      VkDescriptorType      descriptorType;
      size_t                offset;
      size_t                devOffset;
      size_t                bytesPerElement;
      size_t                devBytesPerElement;
      bool                  isDynamic;
      uint32_t              dynamicOffset;
      std::vector<Sampler*> immutableSamplers;
   };

   bvk::vector<PerBindingData>      m_bindings; // Keyed off binding id
   size_t                           m_bytesRequired = 0;
   size_t                           m_devBytesRequired = 0;
   uint32_t                         m_numDynamicOffsetsNeeded = 0;
};

// The API level object just wraps an internal state object which uses a shared_ptr.
// The internal state is shared with all DescriptorSets that use it.
class DescriptorSetLayout : public NonCopyable, public Allocating
{
public:
   DescriptorSetLayout(
      const VkAllocationCallbacks            *pCallbacks,
      Device                                 *pDevice,
      const VkDescriptorSetLayoutCreateInfo  *pCreateInfo) :
      Allocating(pCallbacks)
   {
      m_state = std::make_shared<DescriptorSetLayoutState>(pCallbacks, pDevice, pCreateInfo);
   }

   const std::shared_ptr<DescriptorSetLayoutState> &State() const { return m_state; }

private:
   // Implementation specific from this point on

   // We share the internal state with the descriptor set so it won't go out of scope.
   // From the spec:
   // "VkDescriptorSetLayout objects may be accessed by commands that operate on descriptor
   // sets allocated using that layout, and those descriptor sets must not be updated with
   // vkUpdateDescriptorSets after the descriptor set layout has been destroyed. Otherwise,
   // descriptor set layouts can be destroyed any time they are not in use by an API command."
   std::shared_ptr<DescriptorSetLayoutState> m_state;
};


} // namespace bvk
