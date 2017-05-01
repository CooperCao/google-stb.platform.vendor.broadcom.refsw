/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
