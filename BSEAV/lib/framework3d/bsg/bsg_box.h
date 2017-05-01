/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
