/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "ModuleAllocator.h"
#include "DescriptorInfo.h"

namespace bvk {

// Helper class to wrap a vector of DescriptorInfo with faster lookup
// TODO assess whether having the map is a real benefit
class DescriptorMap
{
public:
   DescriptorMap(spv::ModuleAllocator<uint32_t> &allocator, DescriptorTable *table) :
      m_table(table),
      m_map(std::less<DescriptorInfo>(), spv::ModuleAllocator<
            std::pair<const DescriptorInfo, uint32_t>>(allocator))
   {
      assert(table->size() == 0);
   }

   uint32_t FindEntry(const DescriptorInfo &descInfo);

private:
   DescriptorTable                     *m_table; // Table of unique (set, binding, element) tuples
   spv::map<DescriptorInfo, uint32_t>   m_map;   // Map to optimize finding an entry in m_table
};

class DescriptorMaps
{
public:
   DescriptorMaps(spv::ModuleAllocator<uint32_t> &allocator, DescriptorTables *tables);

   uint32_t FindUBOEntry(const DescriptorInfo &descInfo)
   {
      return m_ubo.FindEntry(descInfo);
   }

   uint32_t FindSSBOEntry(const DescriptorInfo &descInfo)
   {
      return m_ssbo.FindEntry(descInfo);
   }

   uint32_t FindSamplerEntry(const DescriptorInfo &descInfo)
   {
      return m_sampler.FindEntry(descInfo);
   }

   uint32_t FindImageEntry(const DescriptorInfo &descInfo)
   {
      return m_image.FindEntry(descInfo);
   }

   uint32_t FindBufferEntry(bool ssbo, const DescriptorInfo &descInfo)
   {
      return ssbo ? FindSSBOEntry(descInfo) : FindUBOEntry(descInfo);
   }

private:
   // Tables of unique (set, binding, element) descriptor entries:
   DescriptorMap m_ubo;      // UBOs
   DescriptorMap m_ssbo;     // SSBOs
   DescriptorMap m_sampler;  // Samplers
   DescriptorMap m_image;    // Images or Sampled Images
};

} // namespace bvk
