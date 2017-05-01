/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_QUATERNION_H__
#define __BSG_QUATERNION_H__

#include "bsg_common.h"
#include "bsg_vector.h"
#include "bsg_matrix.h"

namespace bsg
{

//! @addtogroup math
//! @{

//! Quaternions are used to represent rotations around an arbitrary axis.
//! They are very useful in 3D graphics, since they interpolate nicely, unlike matrices.
//! BSG uses quaternions to encode the rotations in a SceneNode Transform.
class Quaternion
{
public:
   Quaternion();
   //! Construct using an axis and angle (in degrees)
   Quaternion(float degrees, float x, float y, float z);
   //! Construct using an axis and angle (in degrees)
   Quaternion(float degrees, const Vec3 &axis);
   //! Construct the quaternion values directly
   Quaternion(const Vec4 &v);

   //! Normalize the Quaternion
   void Normalize();

   //! Spherically interpolate between two quaternions (0 <= t <= 1)
   Quaternion SLerp(float t, const Quaternion &lerpTo) const;

   //! Return the axis and angle
   void GetAxisAngle(float *degrees, Vec3 *axis) const;
   //! Set the axis and angle
   void SetAxisAngle(float degrees, const Vec3 &axis);

   //! Convert to matrix and return.
   //! We deliberately do not provide the opposite conversion!
   void AsMatrix(Mat4 *mx) const;

   //! Combine two quaternions
   Quaternion operator*(const Quaternion &rhs) const;

   //! Compare two quaternions for equality -- this is a literal comparison of the elements rather than reflecting equivalence between quaternions
   bool operator==(const Quaternion &rhs) const
   {
      return m_q == rhs.m_q;
   }

   //! Compare two quaternions for inequality -- this is a literal comparison of the elements rather than reflecting equivalence between quaternions
   bool operator!=(const Quaternion &rhs) const
   {
      return m_q != rhs.m_q;
   }

   float X() const { return m_q.X(); }
   float Y() const { return m_q.Y(); }
   float Z() const { return m_q.Z(); }
   float W() const { return m_q.W(); }

   float &X() { return m_q.X(); }
   float &Y() { return m_q.Y(); }
   float &Z() { return m_q.Z(); }
   float &W() { return m_q.W(); }

private:
   Vec4 m_q;
};

}

//! @}
#endif /* __BSG_QUATERNION_H__ */
