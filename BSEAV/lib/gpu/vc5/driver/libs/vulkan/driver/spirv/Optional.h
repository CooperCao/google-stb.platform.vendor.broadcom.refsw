/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <cassert>

namespace bvk {

///////////////////////////////////////////////////////////////////////////////
// Optional
//
// Utility class that holds an arbitrary optional value.
// IsValid() tells you if the value is present or not.
///////////////////////////////////////////////////////////////////////////////

template <typename T>
class Optional
{
public:
   Optional() :
      m_valid(false)
   {}

   Optional(const T value) :
      m_valid(true),
      m_value(value)
   {}

   void operator=(const T value)
   {
      m_valid = true;
      m_value = value;
   }

   void Set(T value)
   {
      m_value = value;
      m_valid = true;
   }

   void Clear()
   {
      m_valid = false;
   }

   bool IsValid() const
   {
      return m_valid;
   }

   const T &Get() const
   {
      assert(m_valid);
      return m_value;
   }

private:
   bool  m_valid;
   T     m_value;
};

} // namespace bvk
