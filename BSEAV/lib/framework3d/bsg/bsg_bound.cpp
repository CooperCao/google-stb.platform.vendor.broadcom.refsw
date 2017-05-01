/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_bound.h"

#include <algorithm>
#include <limits>

#undef max

namespace bsg
{

static Vec3 ToVec3(float *ptr, uint32_t numCoords)
{
   Vec3  res;

   if (numCoords == 2)
      res = Vec2(ptr).Lift(0.0);

   if (numCoords == 3)
      res = Vec3(ptr);

   if (numCoords == 4)
      res = Vec4(ptr).Proj();

   return res;
}

Bound::Bound(uint32_t numVerts, float *geom, uint32_t floatStride, uint32_t numCoords)
{
   float *ptr = geom;
   Vec3  mx(-std::numeric_limits<float>::max());
   Vec3  mn(std::numeric_limits<float>::max());

   for (uint32_t i = 0; i < numVerts; ++i, ptr += floatStride)
   {
      Vec3  p(ToVec3(ptr, numCoords));

      mx.X() = std::max(mx.X(), p.X());
      mx.Y() = std::max(mx.Y(), p.Y());
      mx.Z() = std::max(mx.Z(), p.Z());

      mn.X() = std::min(mn.X(), p.X());
      mn.Y() = std::min(mn.Y(), p.Y());
      mn.Z() = std::min(mn.Z(), p.Z());
   }

   m_center = (mn + mx) / 2.0f;
   m_radius = Length(m_center - mn);
}

}
