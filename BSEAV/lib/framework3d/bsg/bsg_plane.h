/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_PLANE_H__
#define __BSG_PLANE_H__

#include "bsg_common.h"
#include "bsg_vector.h"
#include "bsg_matrix.h"

#undef max
#undef min

#include <algorithm>    // For min(a, b)
#include <limits>       // For float max()

namespace bsg
{

//! @addtogroup math
//! @{

//! Represents a plane
//!
//! Ax + By + Cz + D = 0
//!
class Plane
{
public:
   Plane() :
      m_coef(0.0, 1.0f, 0.0f, 0.0f)
   {}

   Plane(float A, float B, float C, float D, bool normalized = false) :
      m_coef(A, B, C, D)
   {
      if (!normalized)
         Normalize();
   }

   //! Construct a plane from three (not colinear) points.
   //! If v1 -> v2 is clockwise normal points down
   Plane(const Vec3 &pCommon, const Vec3 &p1, const Vec3 &p2)
   {
      Vec3  v1 = p1 - pCommon;
      Vec3  v2 = p2 - pCommon;

      Vec3  norm = bsg::Normalize(Cross(v1, v2));

      m_coef = Vec4(norm.X(), norm.Y(), norm.Z(), -norm.Dot(pCommon));
   }

   Plane(const Vec4 &rhs) :
      m_coef(rhs)
   {
      Normalize();
   }

   Plane(const Plane &plane) :
      m_coef(plane.m_coef)
   {
   }

   void Flip() { m_coef = -m_coef; }

   //! Access the coefficients with familiar names
   float A() const { return m_coef.X(); }
   float B() const { return m_coef.Y(); }
   float C() const { return m_coef.Z(); }
   float D() const { return m_coef.W(); }

   //! Unitize normal
   void Normalize()
   {
      // Magnitude of normal (A,B,C)
      float mag = Length(m_coef.Drop());

      if (mag != 0.0f)
         m_coef = m_coef / mag;
   }

   //! Returns the distance of v from the plane.
   //! Assumes that the plane is normalized (which it should be
   //! by construction)
   float Distance(const Vec3 &v) const
   {
      return m_coef.Drop().Dot(v) + D();
   }

private:
   Vec4  m_coef;
};

//! A collection of six planes which can be used to represent a closed volume.
//! Used internally for view volume clipping.
class ViewVolume
{
public:
   //! Identify the planes by index.
   enum ePlane
   {
      eLEFT = 0,
      eRIGHT,
      eBOTTOM,
      eTOP,
      eFARTHEST,
      eNEAREST,

      NUM_PLANES
   };

   //! Access planes (non const)
   Plane &      operator[](uint32_t i)        { return m_planes[i]; }
   //! Access planes (const)
   const Plane &operator[](uint32_t i) const  { return m_planes[i]; }

   // Returns the minimum distance (most negative) from point to frustum.
   float MinDistance(const Vec3 &p) const
   {
      float dist = std::numeric_limits<float>::max();

      for (uint32_t i = 0; i < NUM_PLANES; ++i)
      {
         float d = m_planes[i].Distance(p);

         dist = std::min(dist, d);
      }

      return dist;
   }

private:
   Plane m_planes[NUM_PLANES];
};

//! @}
}

#endif /* __BSG_PLANE_H__ */
