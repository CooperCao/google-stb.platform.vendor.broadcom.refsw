/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_BOUND_H__
#define __BSG_BOUND_H__

#include <algorithm> // For min/max

#include "bsg_common.h"
#include "bsg_vector.h"
#include "bsg_matrix.h"

namespace bsg
{

//! @addtogroup math
//! @{

//! A bounding sphere.
class Bound
{
public:
   //! Create a unit bounding sphere at the origin
   Bound() :
      m_radius(1.0f)
      // m_centre is (0,0,0)
   {}

   //! Create a bounding sphere at center with radius
   Bound(float radius, const Vec3 &center) :
      m_radius(radius),
      m_center(center)
   {}

   Bound(const Vec3 &mn, const Vec3 &mx)
   {
      m_center = (mn + mx) / 2.0f;
      m_radius = Length(m_center - mn);
   }

   //! Creates a bounding sphere for a set of vertices stored in a float-array geom
   //! @arg numVerts is the number of vertices in the array
   //! @arg geom is the data
   //! @arg floatStride is the distance in floats between successive vertex positions
   //! @arg numCoords is the number of elements in each vertex position (2 = 2D points (implied z = 0), 3 = 3D points (x, y, z), 4 = homogeneous points (x/w, y/w, z/w))
   Bound(uint32_t numVerts, float *geom, uint32_t floatStride, uint32_t numCoords = 3);

   //! Transform a bounding sphere.
   //! Only works for rigid body transformations.
   Bound operator*(const Mat4 &xform) const
   {
      Vec3  rad    = xform.Drop() * Vec3(m_radius);
      Vec3  center = (xform * m_center.Lift(1.0f)).Proj();

      float radius = Length(rad);

      return Bound(radius, center);
   }

   //! Find the minimum point
   Vec3 Min() const { return m_center - m_radius; }
   //! Find the maximium point
   Vec3 Max() const { return m_center + m_radius; }

   const Vec3  &GetCenter() const
   {
      return m_center;
   }

   float       GetRadius()  const
   {
      return m_radius;
   }

private:
   float    m_radius;
   Vec3     m_center;
};

//! @}

}

#endif /*__BSG_BOUND_H__ */
