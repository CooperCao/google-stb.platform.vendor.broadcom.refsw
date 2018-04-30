/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "vulkan.h"
#include "Allocating.h"

#include <map>
#include <vector>

namespace bvk
{

class Specialization
{
public:
   Specialization(const VkSpecializationInfo *info);

   bool GetBool(bool *value, uint32_t id) const;
   bool GetValue(uint32_t *value, uint32_t id) const;

private:
   class Entry
   {
   public:
      Entry() = default;
      Entry(uint32_t offset, uint32_t size) :
         m_offset(offset),
         m_size(size)
      {}

      uint32_t GetOffset() const { return m_offset; }
      uint32_t GetSize()   const { return m_size;   }

   private:
      uint32_t m_offset;
      uint32_t m_size;
   };

   template <typename V> bool Get(V *value, uint32_t id) const;

private:
   bvk::map<uint32_t, Entry>  m_map;
   bvk::vector<uint8_t>       m_data;
};

}
