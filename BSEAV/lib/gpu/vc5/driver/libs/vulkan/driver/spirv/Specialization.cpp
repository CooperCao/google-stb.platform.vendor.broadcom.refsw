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

template <typename V> bool Specialization::Get(V *value, uint32_t id) const
{
   auto  iter = m_map.find(id);

   if (iter == m_map.end())
      return false;

   const Specialization::Entry &entry = (*iter).second;

   assert(entry.GetSize() == sizeof(V));

   auto ptr = reinterpret_cast<const V *>(&m_data[entry.GetOffset()]);

   *value = *ptr;

   return true;
}

bool Specialization::GetBool(bool *value, uint32_t id) const
{
   VkBool32 res;
   bool     ok = Get<VkBool32>(&res, id);

   if (ok)
      *value = (res != 0);

   return ok;
}

bool Specialization::GetValue(uint32_t *value, uint32_t id) const
{
   return Get<uint32_t>(value, id);
}

}
