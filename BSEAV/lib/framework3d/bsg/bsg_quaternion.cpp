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
#include "bsg_quaternion.h"
#include "bsg_exception.h"

#include <math.h>
#include <algorithm>

namespace bsg
{

static const float DEG2RAD = (float)M_PI / 180.0f;
static const float EPSILON = 0.00001f;

Quaternion::Quaternion() :
   m_q(0.0f, 0.0f, 0.0f, 1.0f)
{
}

Quaternion::Quaternion(float degrees, float x, float y, float z)
{
   SetAxisAngle(degrees, Vec3(x, y, z));
}

Quaternion::Quaternion(float degrees, const Vec3 &axis)
{
   SetAxisAngle(degrees, axis);
}

Quaternion::Quaternion(const Vec4 &v) :
   m_q(v.X(), v.Y(), v.Z(), v.W())
{
}

void Quaternion::SetAxisAngle(float degrees, const Vec3 &axis)
{
   if (degrees == 0.0f)
      degrees = 0.0000001f;

   float s = sinf(degrees * DEG2RAD * 0.5f);
   float c = cosf(degrees * DEG2RAD * 0.5f);

   m_q[0] = axis[0] * s;
   m_q[1] = axis[1] * s;
   m_q[2] = axis[2] * s;
   m_q[3] = c;

   Normalize();
}


void Quaternion::Normalize()
{
   float mag = sqrtf(m_q[0] * m_q[0] + m_q[1] * m_q[1] + m_q[2] * m_q[2] + m_q[3] * m_q[3]);

   if (mag != 0.0f)
   {
      mag = 1.0f / mag;
      m_q[0] *= mag;
      m_q[1] *= mag;
      m_q[2] *= mag;
      m_q[3] *= mag;
   }
}

Quaternion Quaternion::SLerp(float t, const Quaternion &rhs) const
{
/*
   if (t < 0.0f || t > 1.0f)
      BSG_THROW("Parameter out of range");
*/

   float cosine = m_q[3] * rhs.m_q[3] + m_q[0] * rhs.m_q[0] + m_q[1] * rhs.m_q[1] + m_q[2] * rhs.m_q[2];

   if (cosine < 0.0f)
   {
      Quaternion invq;
      invq.m_q[0] = -rhs.m_q[0];
      invq.m_q[1] = -rhs.m_q[1];
      invq.m_q[2] = -rhs.m_q[2];
      invq.m_q[3] = -rhs.m_q[3];

      return SLerp(t, invq);
   }

   cosine = std::min(cosine, 1.0f);
   float ang = acosf(cosine);

   if (ang == 0.0f)
      return *this;

   float rsinang = 1.0f / sinf(ang);

   float t1 = sinf((1.0f - t) * ang) * rsinang;
   float t2 = sinf(t * ang) * rsinang;

   Quaternion  res;

   res.m_q[0] = t1 * m_q[0] + t2 * rhs.m_q[0];
   res.m_q[1] = t1 * m_q[1] + t2 * rhs.m_q[1];
   res.m_q[2] = t1 * m_q[2] + t2 * rhs.m_q[2];
   res.m_q[3] = t1 * m_q[3] + t2 * rhs.m_q[3];

   res.Normalize();
   return res;
}

void Quaternion::GetAxisAngle(float *degrees, Vec3 *axis) const
{
   float tmp  = 1.0f - (m_q[3] * m_q[3]);
   float rSin = sqrtf(tmp);

   *degrees = acosf(m_q[3]) * 2.0f;

   if (fabsf(rSin) < EPSILON)
      rSin = 1.0f;

   rSin = 1.0f / rSin;

   (*axis)[0] = m_q[0] * rSin;
   (*axis)[1] = m_q[1] * rSin;
   (*axis)[2] = m_q[2] * rSin;
}

void Quaternion::AsMatrix(Mat4 *mx) const
{
   (*mx)(0, 0) = 1.0f - 2.0f * m_q[1] * m_q[1] - 2.0f * m_q[2] * m_q[2];
   (*mx)(0, 1) = 2.0f * m_q[0] * m_q[1] - 2.0f * m_q[2] * m_q[3];
   (*mx)(0, 2) = 2.0f * m_q[0] * m_q[2] + 2.0f * m_q[1] * m_q[3];
   (*mx)(0, 3) = 0.0f;

   (*mx)(1, 0) = 2.0f * m_q[0] * m_q[1] + 2.0f * m_q[2] * m_q[3];
   (*mx)(1, 1) = 1.0f - 2.0f * m_q[0] * m_q[0] - 2.0f * m_q[2] * m_q[2];
   (*mx)(1, 2) = 2.0f * m_q[1] * m_q[2] - 2.0f * m_q[0] * m_q[3];
   (*mx)(1, 3) = 0.0f;

   (*mx)(2, 0) = 2.0f * m_q[0] * m_q[2] - 2 * m_q[1] * m_q[3];
   (*mx)(2, 1) = 2.0f * m_q[1] * m_q[2] + 2.0f * m_q[0] * m_q[3];
   (*mx)(2, 2) = 1.0f - 2.0f * m_q[0] * m_q[0] - 2 * m_q[1] * m_q[1];
   (*mx)(2, 3) = 0.0f;

   (*mx)(3, 0) = 0.0f;
   (*mx)(3, 1) = 0.0f;
   (*mx)(3, 2) = 0.0f;
   (*mx)(3, 3) = 1.0f;
}

Quaternion Quaternion::operator*(const Quaternion &rhs) const
{
   Quaternion  result;
   Vec3        cross;

   result.m_q[3] = (m_q[3] * rhs.m_q[3]) - (m_q[0] * rhs.m_q[0] + m_q[1] * rhs.m_q[1] + m_q[2] * rhs.m_q[2]);
   result.m_q[3] = (m_q[3] * rhs.m_q[3]) - (m_q[0] * rhs.m_q[0] + m_q[1] * rhs.m_q[1] + m_q[2] * rhs.m_q[2]);

   cross[0] = m_q[1] * rhs.m_q[2] - m_q[2] * rhs.m_q[1];
   cross[1] = m_q[2] * rhs.m_q[0] - m_q[0] * rhs.m_q[2];
   cross[2] = m_q[0] * rhs.m_q[1] - m_q[1] * rhs.m_q[0];

   result.m_q[0] = cross[0] + (m_q[3] * rhs.m_q[0]) + (rhs.m_q[3] * m_q[0]);
   result.m_q[1] = cross[1] + (m_q[3] * rhs.m_q[1]) + (rhs.m_q[3] * m_q[1]);
   result.m_q[2] = cross[2] + (m_q[3] * rhs.m_q[2]) + (rhs.m_q[3] * m_q[2]);

   result.Normalize();

   return result;
}
}
