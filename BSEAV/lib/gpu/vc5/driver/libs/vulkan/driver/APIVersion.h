/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <stdint.h>

namespace bvk {

class APIVersion
{
public:
   // The major version number is a 10 - bit integer packed into bits 31 - 22.
   // The minor version number is a 10 - bit integer packed into bits 21 - 12.
   // The patch version number is a 12 - bit integer packed into bits 11 - 0.

   APIVersion(uint16_t maj, uint16_t min, uint16_t patch) :
      m_maj(maj),
      m_min(min),
      m_patch(patch)
   {
   }

   APIVersion(uint32_t codedVersion)
   {
      m_maj = (codedVersion >> 22) & 0x3FF;
      m_min = (codedVersion >> 12) & 0x3FF;
      m_patch = codedVersion & 0xFFF;
   }

   bool operator==(const APIVersion &) = delete;

   operator uint32_t() const
   {
      return ((m_maj & 0x3FF) << 22) | ((m_min & 0x3FF) << 12) | (m_patch & 0xFFF);
   }

   bool IsExactlyEqualTo(const APIVersion &rhs) const
   {
      return m_maj == rhs.m_maj && m_min == rhs.m_min && m_patch == rhs.m_patch;
   }

   bool IsFullyCompatibleWith(const APIVersion &rhs) const
   {
      return m_maj == rhs.m_maj && m_min == rhs.m_min;
   }

   bool CanSupport(const APIVersion &rhs) const
   {
      return m_maj == rhs.m_maj && m_min >= rhs.m_min;
   }

   uint16_t Major() const { return m_maj;   }
   uint16_t Minor() const { return m_min;   }
   uint16_t Patch() const { return m_patch; }

   static const APIVersion &CurAPIVersion();

private:
   uint16_t m_maj;
   uint16_t m_min;
   uint16_t m_patch;
};

} // namespace bvk
