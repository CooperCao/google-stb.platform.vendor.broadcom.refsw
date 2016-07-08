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

#ifndef __BSG_CIRCLE_H__
#define __BSG_CIRCLE_H__

#include "bsg_common.h"
#include "bsg_vector.h"
#include "bsg_axis.h"

namespace bsg
{

//! @ingroup math animation
//! @{

static const float   PI = 3.141592654f;

//! Parametric description of a circle perpendicular.
//! @code
//! x(t) = r * cos(t)
//! y(t) = r * sin(t)
//! @endcode
//!
//! Winding is as viewed from the positive axis towards the origin.  Starting point when
//! t = 0 is: , eX_AXIS (0, 1, 0), eY_AXIS (0, 0, 1), eZ_AXIS (1, 0, 0)
//!
//! Circle is used in the animation module to create a bsg::CircleEvaluator.
class Circle
{
public:
   //! Default constructor
   Circle() :
      m_radius(0.0f),
      m_phase(0.0f),
      m_axis(eY_AXIS),
      m_winding(eCCW_WINDING)
   {}

   //! Create a circles with specified radius, perpendicular to the axis and with a phase offset.
   //! The winding controls the direction of positive "t"
   Circle(float radius, float phase = 0.0f, eAxis axis = Y_AXIS, eWinding winding = eCCW_WINDING) :
      m_radius(radius),
      m_phase(phase * 2.0f * PI / 360.0f),
      m_axis(axis),
      m_winding(winding)
   {}

   //! Calculate position in 3D on the circle for a given "t".
   Vec3 Evaluate(float t) const
   {
      float theta = t * (2.0f * PI) + m_phase;

      theta = theta * (m_winding == eCW_WINDING ? -1.0f : 1.0f);

      float x = m_radius * cosf(theta);
      float y = m_radius * sinf(theta);

      Vec3 result;

      switch (m_axis)
      {
      case eX_AXIS:
         result = Vec3(0.0f, x, y);
         break;

      case eY_AXIS:
         result = Vec3(y, 0.0f, x);
         break;

      case eZ_AXIS:
         result = Vec3(x, y, 0.0f);
         break;
      }

      return result;
   }

private:
   float    m_radius;
   float    m_phase;
   eAxis    m_axis;
   eWinding m_winding;
};

//! @}
}

#endif /* __BSG_CIRCLE_H__ */

