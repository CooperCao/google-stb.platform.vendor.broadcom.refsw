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

#include "bsg_catmullrom.h"

namespace bsg
{

CatmullRomSpline::CatmullRomSpline(uint32_t estimationIters) : 
   m_totalLength(0.0f),
   m_endReplicated(false),
   m_startControl(0, 0, 0),
   m_endControl(0, 0, 0),
   m_startSet(false),
   m_endSet(false),
   m_estimationIters(estimationIters)
{
}

void CatmullRomSpline::AddPoint(const Vec3 &pt)
{
   uint32_t numPtsBeforeAdd = m_points.size();
   m_points.push_back(pt);

   if (numPtsBeforeAdd == 0)
   {
      // Setup for start gradient
      m_points.push_back(pt);
      if (m_startSet)
         m_points[0] = m_startControl;
      m_lengths.push_back(0.0f);
      numPtsBeforeAdd = 1;
   }

   if (m_endReplicated)
   {
      // Remove the replicated end point if we're going to add more points
      m_points.resize(numPtsBeforeAdd - 1);
      m_endReplicated = false;

      if (m_lengths.size() > 1)
         m_lengths.resize(m_lengths.size() - 1);
   }

   if (numPtsBeforeAdd >= 3)
   {
      // We have at least 4 points, so start working out segment lengths
      float segmentLen = EstimateArcLength(numPtsBeforeAdd - 2);
      m_lengths.push_back(segmentLen);
      m_totalLength += segmentLen;
   }
}

Vec3 CatmullRomSpline::EvaluateLocal(float t, uint32_t p1)
{
   uint32_t p0 = p1 - 1;
   uint32_t p2 = p1 + 1;
   uint32_t p3 = p1 + 2;

   float t2 = t * t;
   float t3 = t2 * t;

   // Now evaluate the curve locally
   Vec3 result = (m_points[p1] * 2.0f) + 
      (m_points[p2] - m_points[p0]) * t + 
      (m_points[p0] * 2.0f - m_points[p1] * 5.0f + m_points[p2] * 4.0f - m_points[p3]) * t2 + 
      (m_points[p3] - m_points[p0] + m_points[p1] * 3.0f - m_points[p2] * 3.0f) * t3;
   
   result = result * 0.5f;

   return result;
}

Vec3 CatmullRomSpline::Evaluate(float t)
{
   Vec3 result;

   if (!m_endReplicated)
   {
      // Replicate the final point if needed and adjust for end gradient
      if (m_endSet)
         AddPoint(m_endControl);
      else
         AddPoint(m_points[m_points.size() - 1]);

      m_endReplicated = true;
   }

   uint32_t numPts = m_points.size();
   uint32_t numLens = m_lengths.size();

   if (numPts < 4)
   {
      if (numPts > 0)
         result = m_points[0];
   }
   else
   {
      // Which points?
      float distALong = t * m_totalLength;
      float distToP1 = 0.0f;
      uint32_t p1;
      
      for (p1 = 0; p1 < numLens; p1++)
      {
         distToP1 += m_lengths[p1];
         if (distToP1 >= distALong)
            break;
      }
      if (p1 == numLens)
         p1 = numLens - 1;

      // Index1 is the index of p1 in the spline
      float distToP0 = distToP1 - m_lengths[p1];
      float localT = 0.0f;
      if (distToP1 - distToP0 > 0.0f)
         localT = (distALong - distToP0) / (distToP1 - distToP0);

      if (p1 == 0)
         p1 = 1;

      // Now evaluate the curve locally
      result = EvaluateLocal(localT, p1);
   }

   return result;
}

float CatmullRomSpline::EstimateArcLength(uint32_t from)
{
   float len = 0.0f;

   Vec3 p1 = EvaluateLocal(0.0f, from);

   for (uint32_t i = 1; i < m_estimationIters; i++)
   {
      float t = (float)i / (float)(m_estimationIters - 1);
      Vec3 p2 = EvaluateLocal(t, from);

      len += Distance(p1, p2);

      p1 = p2;
   }

   return len;
}

void CatmullRomSpline::SetStartControlPoint(const Vec3 &startPt)
{
   m_startControl = startPt;
   m_startSet = true;
   if (m_points.size() > 1)
   {
      m_points[0] = m_startControl;

      // Recalculate segment length of first segment
      m_lengths[1] = EstimateArcLength(1);
   }
}

void CatmullRomSpline::SetEndControlPoint(const Vec3 &endPt)
{
   m_endControl = endPt;
   m_endSet = true;
   if (m_endReplicated)
   {
      AddPoint(m_endControl);
      m_endReplicated = true;
   }
}


}
