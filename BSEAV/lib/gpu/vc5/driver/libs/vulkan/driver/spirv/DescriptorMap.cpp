/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "DescriptorMap.h"

namespace bvk {

uint32_t DescriptorMap::FindEntry(const DescriptorInfo &descInfo)
{
   uint32_t tableIndex;

   // Find the index for descInfo if it exists, otherwise add a new entry
   const auto &iter = m_map.find(descInfo);
   if (iter == m_map.end())
   {
      m_table->push_back(descInfo);
      tableIndex = m_table->size() - 1;
      m_map[descInfo] = tableIndex;
   }
   else
      tableIndex = iter->second;

   return tableIndex;
}

DescriptorMaps::DescriptorMaps(spv::ModuleAllocator<uint32_t> &allocator, DescriptorTables *tables) :
   m_ubo(allocator, &tables->GetUBO()),
   m_ssbo(allocator, &tables->GetSSBO()),
   m_sampler(allocator, &tables->GetSampler()),
   m_image(allocator, &tables->GetImage())
{
}

} // namespace bvk
