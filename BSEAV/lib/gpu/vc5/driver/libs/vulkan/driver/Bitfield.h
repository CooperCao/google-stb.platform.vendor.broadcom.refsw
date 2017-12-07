/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

namespace bvk {

// Wrapper on a user-defined length bitfield
template <typename T, uint32_t NumBits, typename Storage>
class Bitfield
{
public:
   Bitfield() : m_bits(0) { static_assert(sizeof(Storage) * 8 >= NumBits,
                            "Bitfield storage not large enough"); }
   Bitfield(Storage bits) : m_bits(bits) { static_assert(sizeof(Storage) * 8 >= NumBits,
                                           "Bitfield storage not large enough"); }
   operator Storage() const { return m_bits; }

   void ClearAll()            { m_bits = 0; }
   void SetAll()              { m_bits = (static_cast<Storage>(1) << NumBits) - 1; }
   void Set(T state)          { m_bits |= (static_cast<Storage>(1) << static_cast<uint32_t>(state)); }
   void Clear(T state)        { m_bits &= ~(static_cast<Storage>(1) << static_cast<uint32_t>(state)); }
   bool IsSet(T state) const  { return (m_bits & (static_cast<Storage>(1) << static_cast<uint32_t>(state))) != 0; }
   bool AnySet() const        { return m_bits != 0; }

   void Set(std::initializer_list<T> bits)
   {
      for (auto b : bits)
         m_bits |= b;
   }

   void Clear(std::initializer_list<T> bits)
   {
      for (auto b : bits)
         m_bits &= ~b;
   }

private:
   Storage m_bits;
};

} // namespace bvk
