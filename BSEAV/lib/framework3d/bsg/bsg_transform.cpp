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

#include "bsg_transform.h"

namespace bsg
{

Transform::Transform() :
   m_dirty(false),
   m_referenced(false)
{
   m_scale    = Vec3(1.0f, 1.0f, 1.0f);
   m_position = Vec3(0.0f, 0.0f, 0.0f);
}

const Mat4 &Transform::GetMatrix() const
{
   // If it has been changed (dirty) or has exposed itself (referenced) then we need to recalculate
   if (m_dirty || m_referenced)
   {
      m_matrix = Translate(m_position.X(), m_position.Y(), m_position.Z());

      if (m_rotation != Quaternion())
      {
         Mat4 rot;
         m_rotation.AsMatrix(&rot);
         m_matrix = m_matrix * rot;
      }

      if (m_scale != Vec3(1.0f))
         m_matrix = m_matrix * Scale(m_scale.X(), m_scale.Y(), m_scale.Z());

      m_dirty    = false;
   }

   return m_matrix;
}

void Transform::PreRotate(const bsg::Quaternion &val)
{
   m_dirty = true;
   m_rotation = m_rotation * val;
}

void Transform::PreRotate(float degrees, const Vec3 &axis)
{
   m_dirty = true;
   m_rotation = m_rotation * Quaternion(degrees, axis);
}

void Transform::PreScale(const Vec3 &vec)
{
   m_dirty = true;
   m_scale[0] *= vec[0];
   m_scale[1] *= vec[1];
   m_scale[2] *= vec[2];
}

void Transform::PostTranslate(const Vec3 &vec)
{
   m_dirty = true;
   m_position[0] += vec[0];
   m_position[1] += vec[1];
   m_position[2] += vec[2];
}

}

