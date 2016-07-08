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

