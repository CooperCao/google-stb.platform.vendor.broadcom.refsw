/******************************************************************************
 *   (c)2011-2012 Broadcom Corporation
 *
 * This program is the proprietary software of Broadcom Corporation and/or its
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

