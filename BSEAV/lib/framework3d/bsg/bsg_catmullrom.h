/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BSG_CATMULLROM_H__
#define __BSG_CATMULLROM_H__

#include "bsg_common.h"
#include "bsg_vector.h"
#include <vector>

namespace bsg
{

/** @addtogroup animation
@{
*/

//! Encapsulates the definition of a Catmull-Rom spline of arbitrary length.
//!
//! Optional start and end control points can be added to control the tangent
//! at either end of the spline.
class CatmullRomSpline
{
public:
   //! Construct a spline.
   //! When estimating the length of a spline segment, the segment will be subdivided into
   //! estimationIters pieces. A large value here will be very accurate but also slow.
   //!
   //! The length calculation will be done only once, unless the spline is extended.
   CatmullRomSpline(uint32_t estimationIters = 15);

   //! Appends a new point to the spline. The spline will pass through the point.
   void AddPoint(const Vec3 &pt);

   //! Parametrically evaluate a point on the spline (0 <= t <= 1)
   Vec3 Evaluate(float t);

   //! Sets the start control point to determine the tangent at the start of the spline
   void SetStartControlPoint(const Vec3 &startCtrl);

   //! Sets the end control point to determine the tangent at the end of the spline
   void SetEndControlPoint(const Vec3 &endCtrl);

private:
   float EstimateArcLength(uint32_t from);
   Vec3 EvaluateLocal(float t, uint32_t p1);

private:
   std::vector<Vec3>    m_points;
   std::vector<float>   m_lengths;
   float                m_totalLength;
   bool                 m_endReplicated;
   Vec3                 m_startControl;
   Vec3                 m_endControl;
   bool                 m_startSet;
   bool                 m_endSet;
   uint32_t             m_estimationIters;
};

/** @} */
}

#endif /* __BSG_CATMULLROM_H__ */
