/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

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

