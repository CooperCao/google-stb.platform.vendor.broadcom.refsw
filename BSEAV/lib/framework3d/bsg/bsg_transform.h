/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_TRANSFORM_H__
#define __BSG_TRANSFORM_H__

#include "bsg_common.h"
#include "bsg_matrix.h"
#include "bsg_vector.h"
#include "bsg_quaternion.h"
#include "bsg_animatable.h"

namespace bsg
{

//! @ingroup scenegraph math
//! A transform composes a position, a rotation (Quaternion) and a scale.
//! The order of application is scale, then rotation, then position.
//!
//! The transform is used primarily by the SceneNode class to encode a node transform.
//!
//! Transforms cannot be combined in the same way that matrices might. The reason for this is that
//! we want transforms to be sensibly interpolated by animators; matrices cannot do this. There are
//! some facilities for rotating, scaling and translating Transforms however.
//!
//! If you need to combine Transforms, you can always use the GetMatrix() method, and combine matrices.
class Transform
{
public:
   Transform();

   //! Retrieve the position as an Animatable reference
   const AnimatableVec3 &GetPosition() const                { return m_position;                      }
   //! Retrieve the position as an Animatable reference
   AnimatableVec3 &GetPosition()                            { m_referenced = true; return m_position; }
   //! Overwrite the current position
   void SetPosition(const Vec3 &val)                        { m_dirty = true; m_position = val;       }

   //! Retrieve the rotation as an Animatable Quaternion reference
   const AnimatableQuaternion &GetRotation() const          { return m_rotation;                      }
   //! Retrieve the rotation as an Animatable Quaternion reference
   AnimatableQuaternion &GetRotation()                      { m_referenced = true; return m_rotation; }
   //! Overwrite the current rotation
   void SetRotation(const Quaternion &val)                  { m_dirty = true; m_rotation = val;       }
   //! Overwrite the current rotation
   void SetRotation(float degrees, const Vec3 &axis)        { m_dirty = true; m_rotation = Quaternion(degrees, axis);  }

   //! Retrieve the scale as an Animatable Vec3 reference
   const AnimatableVec3 &GetScale() const                   { return m_scale;                         }
   //! Retrieve the scale as an Animatable Vec3 reference
   AnimatableVec3 &GetScale()                               { m_referenced = true; return m_scale;    }
   //! Overwrite the current scale
   void SetScale(const Vec3 &val)                           { m_dirty = true;  m_scale = val;         }

   //! Retrieve a Mat4 matrix which reflects this Transform
   const Mat4 &GetMatrix() const;

   //! Pre-rotate this transform using the given Quaternion
   void PreRotate(const Quaternion &val);
   //! Pre-rotate this transform using the given axis and angle
   void PreRotate(float degrees, const Vec3 &axis);
   //! Pre-scale this transform using the given Vec3 scale factors (x,y,z)
   void PreScale(const Vec3 &vec);
   //! Post-translate this transform using the given vector
   void PostTranslate(const Vec3 &vec);

   //! Check if the transform is identity (assumed to be id if unchanged)
   bool IsIdentity() const
   {
      return m_position == Vec3() && m_scale == Vec3(1.0f) && m_rotation == Quaternion();
   }

private:
   AnimatableVec3         m_position;
   AnimatableQuaternion   m_rotation;
   AnimatableVec3         m_scale;

   mutable bool           m_dirty;        // The matrix is out of date

   bool                   m_referenced;   // Has the transform exposed itself (sticky)
   mutable Mat4           m_matrix;
};

/** @} */

typedef Animatable<Transform> AnimatableTransform;

}

#endif /* __BSG_TRANSFORM_H__ */
