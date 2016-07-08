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

