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

