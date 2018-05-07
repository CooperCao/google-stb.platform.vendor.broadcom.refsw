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

   uint32_t Add(const DescriptorInfo &descInfo);
   uint32_t Find(const DescriptorInfo &descInfo) const;

private:
   DescriptorTable                     *m_table; // Table of unique (set, binding, element) tuples
   spv::map<DescriptorInfo, uint32_t>   m_map;   // Map to optimize finding an entry in m_table
};

class DescriptorMaps
{
public:
   DescriptorMaps(spv::ModuleAllocator<uint32_t> &allocator, DescriptorTables *tables);

   uint32_t AddSampler(const DescriptorInfo &descInfo)
   {
      return m_sampler.Add(descInfo);
   }

   uint32_t FindSampler(const DescriptorInfo &descInfo) const
   {
      return m_sampler.Find(descInfo);
   }

   uint32_t AddImage(const DescriptorInfo &descInfo)
   {
      return m_image.Add(descInfo);
   }

   uint32_t FindImage(const DescriptorInfo &descInfo) const
   {
      return m_image.Find(descInfo);
   }

   uint32_t AddBuffer(bool ssbo, const DescriptorInfo &descInfo)
   {
      return (ssbo ? m_ssbo : m_ubo).Add(descInfo);
   }

   uint32_t FindBuffer(bool ssbo, const DescriptorInfo &descInfo) const
   {
      return (ssbo ? m_ssbo : m_ubo).Find(descInfo);
   }

private:
   // Tables of unique (set, binding, element) descriptor entries:
   DescriptorMap m_ubo;      // UBOs
   DescriptorMap m_ssbo;     // SSBOs
   DescriptorMap m_sampler;  // Samplers
   DescriptorMap m_image;    // Images or Sampled Images
};

} // namespace bvk
