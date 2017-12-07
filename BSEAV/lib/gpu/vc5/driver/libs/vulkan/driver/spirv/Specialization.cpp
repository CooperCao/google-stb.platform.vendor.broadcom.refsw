/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Specialization.h"
#include <assert.h>
#include <string.h>

namespace bvk
{

Specialization::Specialization(const VkSpecializationInfo *info)
{
   if (info == nullptr)
      return;

   for (uint32_t i = 0; i < info->mapEntryCount; ++i)
   {
      const VkSpecializationMapEntry &entry = info->pMapEntries[i];

      m_map[entry.constantID] = Entry(entry.offset, entry.size);
   }

   m_data.assign( (const uint8_t*)info->pData,
                  (const uint8_t*)info->pData + info->dataSize);
}

bool Specialization::GetBool(bool *value, uint32_t id) const
{
   auto  iter = m_map.find(id);

   if (iter == m_map.end())
      return false;

   const Specialization::Entry &entry = (*iter).second;

   assert(entry.GetSize() == sizeof(VkBool32));

   auto ptr = reinterpret_cast<const VkBool32 *>(&m_data[entry.GetOffset()]);

   *value = (*ptr != 0);

   return true;
}

bool Specialization::GetValue(uint32_t *value, uint32_t id) const
{
   auto  iter = m_map.find(id);

   if (iter == m_map.end())
      return false;

   const Specialization::Entry &entry = (*iter).second;

   assert(entry.GetSize() == sizeof(uint32_t));

   auto ptr = reinterpret_cast<const uint32_t *>(&m_data[entry.GetOffset()]);

   *value = *ptr;

   return true;
}

}
