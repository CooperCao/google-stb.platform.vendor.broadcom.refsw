/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "DescriptorMap.h"

namespace bvk {

uint32_t DescriptorMap::Add(const DescriptorInfo &descInfo)
{
   assert(m_map.find(descInfo) == m_map.end());

   uint32_t index = m_table->size();
   m_table->push_back(descInfo);

   m_map[descInfo] = index;

   return index;
}

uint32_t DescriptorMap::Find(const DescriptorInfo &descInfo) const
{
   auto iter = m_map.find(descInfo);
   assert(iter != m_map.end());

   return iter->second;
}

DescriptorMaps::DescriptorMaps(const SpvAllocator &allocator, DescriptorTables *tables) :
   m_ubo(allocator, &tables->GetUBO()),
   m_ssbo(allocator, &tables->GetSSBO()),
   m_sampler(allocator, &tables->GetSampler()),
   m_image(allocator, &tables->GetImage())
{
}

} // namespace bvk
