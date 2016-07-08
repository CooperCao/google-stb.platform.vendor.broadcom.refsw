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
#ifndef __BSG_BOX_H__
#define __BSG_BOX_H__

#undef min
#undef max

#include <limits>
#include <algorithm>

#include "bsg_common.h"
#include "bsg_vector.h"
#include "bsg_matrix.h"

namespace bsg
{

//! @addtogroup math
//! @{

static const float inf = std::numeric_limits<float>::infinity();

//! An axis aligned bounding box
class Box
{
public:
   Box() :
      m_min( inf,  inf,  inf),
      m_max(-inf, -inf, -inf)
   {}

   Box(const Vec3 p1, const Vec3 p2) :
      m_min(std::min(p1.X(), p2.X()), std::min(p1.Y(), p2.Y()), std::min(p1.Z(), p2.Z())),
      m_max(std::max(p1.X(), p2.X()), std::max(p1.Y(), p2.Y()), std::max(p1.Z(), p2.Z()))
   {}

   const Vec3  &Max()   { return m_max; }
   const Vec3  &Min()   { return m_min; }

   Box &operator+=(const Vec3 &vec)
   {
      m_max.X() = std::max(m_max.X(), vec.X());
      m_max.Y() = std::max(m_max.Y(), vec.Y());
      m_max.Z() = std::max(m_max.Z(), vec.Z());

      m_min.X() = std::min(m_min.X(), vec.X());
      m_min.Y() = std::min(m_min.Y(), vec.Y());
      m_min.Z() = std::min(m_min.Z(), vec.Z());

      return *this;
   }

   Box &operator+=(const Box &rhs)
   {
      m_max.X() = std::max(m_max.X(), rhs.m_max.X());
      m_max.Y() = std::max(m_max.Y(), rhs.m_max.Y());
      m_max.Z() = std::max(m_max.Z(), rhs.m_max.Z());

      m_min.X() = std::min(m_min.X(), rhs.m_min.X());
      m_min.Y() = std::min(m_min.Y(), rhs.m_min.Y());
      m_min.Z() = std::min(m_min.Z(), rhs.m_min.Z());

      return *this;
   }

   Vec3 GetCenter() const
   {
      return (m_min + m_max) / 2.0f;
   }

   float GetRadius() const
   {
      return Length(m_min - GetCenter());
   }

private:
   Vec3  m_min;
   Vec3  m_max;
};

//! @}
}

#endif /* __BSG_BOX_H__ */

