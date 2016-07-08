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

