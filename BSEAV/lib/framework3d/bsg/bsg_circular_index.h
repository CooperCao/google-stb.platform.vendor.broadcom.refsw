/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_CIRCULAR_INDEX_H__
#define __BSG_CIRCULAR_INDEX_H__

#include "bsg_common.h"

#include <stdint.h>

namespace bsg
{

//! A circular index can be thought of as a uint32_t which is limited to a certain range.
//! When the index is incremented or decremented beyond its range, it will wrap to still
//! contain an in range value.
//!
//! e.g. Incrementing a CircularIndex with range 0-9 and value 9 will yield value 0.
//! Decrementing a CircularIndex with range 0-9 and value 0 will yield value 9.
class CircularIndex
{
public:
   CircularIndex() : m_current(0), m_max(0) {}
   CircularIndex(uint32_t start, uint32_t max) : m_current(start), m_max(max) {}

   void Setup(uint32_t start, uint32_t max) { m_current = start; m_max = max; }

   void Increment() { m_current++; if (m_current > m_max) m_current = 0; }
   void Decrement() { m_current--; if (m_current > m_max) m_current = m_max; }
   void Set(uint32_t cur) { m_current = cur; }
   uint32_t Current() const { return m_current; }
   operator uint32_t() const { return m_current; }

   CircularIndex& operator++() { Increment(); return *this; }
   CircularIndex operator++(int) { CircularIndex old(*this); ++(*this); return old; }
   CircularIndex& operator--() { Decrement(); return *this; }
   CircularIndex operator--(int) { CircularIndex old(*this); --(*this); return old; }

   CircularIndex Plus1() const { CircularIndex ret(*this); ret.Increment(); return ret; }
   CircularIndex Minus1() const { CircularIndex ret(*this); ret.Decrement(); return ret; }

   CircularIndex PlusN(uint32_t n)  const { return CircularIndex((m_current + n) % (m_max + 1), m_max); }

   CircularIndex MinusN(uint32_t n) const
   {
      uint32_t mod = m_max + 1;
      uint32_t c   = n / mod + 1;
      return CircularIndex((mod * c + m_current - n) % mod, m_max);
   }

   uint32_t GetMax() const { return  m_max; };

private:
   uint32_t m_current;
   uint32_t m_max;
};

}

#endif /* __BSG_CIRCULAR_INDEX_H__ */
