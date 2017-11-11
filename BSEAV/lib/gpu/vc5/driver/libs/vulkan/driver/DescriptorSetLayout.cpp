/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

namespace bvk {

DescriptorSetLayoutState::DescriptorSetLayoutState(
   const VkAllocationCallbacks            *pCallbacks,
   bvk::Device                            *pDevice,
   const VkDescriptorSetLayoutCreateInfo  *pCreateInfo) :
      Allocating(pCallbacks),
      m_bindings(GetObjScopeAllocator<PerBindingData>())
{
   // The bindings given to us may be sparse, but we want to allow for fast
   // lookup of data based on binding number, so expand out.
   // The spec states ".. allows the descriptor bindings to be specified sparsely
   // such that not all binding numbers between 0 and the maximum binding number
   // need to be specified in the pBindings array. ... However, all binding numbers
   // between 0 and the maximum binding number in the VkDescriptorSetLayoutCreateInfo::
   // pBindings array may consume memory in the descriptor set layout."

   // First find the maximum binding number
   uint32_t maxBinding = 0;
   for (uint32_t i = 0; i < pCreateInfo->bindingCount; i++)
      maxBinding = std::max(maxBinding, pCreateInfo->pBindings[i].binding);

   m_numBindings = maxBinding + 1;

   // Create non-sparse storage
   m_bindings.resize(m_numBindings);

   // Clear every entry
   for (uint32_t i = 0; i < m_numBindings; i++)
      m_bindings[i] = {};

   size_t offset    = 0;
   size_t devOffset = 0;

   // Copy data and calculate offsets to start of each binding
   for (uint32_t i = 0; i < pCreateInfo->bindingCount; i++)
   {
      auto b         = pCreateInfo->pBindings[i];
      auto &bindData = m_bindings[b.binding];

      bindData.descriptorType     = b.descriptorType;
      bindData.bytesPerElement    = DescriptorPool::CalcDescriptorTypeBytes(b.descriptorType);
      bindData.devBytesPerElement = DescriptorPool::CalcDescriptorTypeDevMemBytes(b.descriptorType);
      bindData.offset             = offset;
      bindData.devOffset          = devOffset;
      bindData.dynamicOffset      = 0;
      bindData.isDynamic          = (b.descriptorType == VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC ||
                                     b.descriptorType == VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC);

      if (bindData.isDynamic)
      {
         bindData.dynamicOffset = m_numDynamicOffsetsNeeded;
         m_numDynamicOffsetsNeeded += b.descriptorCount;
      }

      if (b.pImmutableSamplers != nullptr)
      {
         for (uint32_t d = 0; d < b.descriptorCount; d++)
            bindData.immutableSamplers.push_back(fromHandle<Sampler>(b.pImmutableSamplers[d]));
      }

      offset    += bindData.bytesPerElement    * b.descriptorCount;
      devOffset += bindData.devBytesPerElement * b.descriptorCount;
   }

   m_bytesRequired    = offset;
   m_devBytesRequired = devOffset;
}

// Write sampler state records for immutable samplers
// Returns true if the memory block should be flushed (was written to)
bool DescriptorSetLayoutState::WriteImmutableSamplerRecords(uint8_t *sysPtr, size_t devOffset, gmem_handle_t devHandle) const
{
   bool flush = false;

   for (uint32_t b = 0; b < m_numBindings; b++)
   {
      VkDescriptorType descType = m_bindings[b].descriptorType;

      for (uint32_t e = 0; e < static_cast<uint32_t>(m_bindings[b].immutableSamplers.size()); e++)
      {
         Sampler *samp = GetImmutableSampler(b, e);
         assert(samp != nullptr);

         assert(descType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER ||
                descType == VK_DESCRIPTOR_TYPE_SAMPLER);

         uint8_t *devPtr = static_cast<uint8_t*>(gmem_get_ptr(devHandle));
         size_t physOffset = devOffset + DeviceOffsetFor(b, e);

         // Combined image/sampler has the texture-state-record first followed by the sampler-state.
         // Skip over the TSR.
         if (descType == VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER)
            physOffset += DescriptorPool::GetDevMemRecordSize();

         samp->WriteSamplerRecord(devPtr + physOffset);

         V3D_TMU_PARAM1_T p1 = {};
         p1.unnorm       = samp->UnnormalizedCoordinates();
         p1.sampler_addr = gmem_get_addr(devHandle) + physOffset;

         auto data = reinterpret_cast<DescriptorPool::ImageInfo*>(sysPtr + OffsetFor(b,e));
         data->samplerParam = v3d_pack_tmu_param1(&p1);

         flush = true;
      }
   }

   return flush;
}

} // namespace bvk
