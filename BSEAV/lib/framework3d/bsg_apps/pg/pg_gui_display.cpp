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

#include "pg_gui_display.h"
#include "bsg_animator.h"

using namespace pg;
using namespace bsg;

const float ROTATION_ANGLE       =  10.0f;
const Vec3  ROTATION_VEC         = Vec3(1.0f, 1.0f, 0.0f);
const Vec3  ONE_TO_ONE_SCALE     = Vec3(1.0f, 1.0f, 1.0f);
const Vec3  REDUCED_SCALE        = Vec3(0.8f, 0.8f, 0.8f);

GUIDisplay::GUIDisplay(void) :
   m_guiNode(New),
   m_isRotated(false)
{

}

GUIDisplay::~GUIDisplay(void)
{
}

void GUIDisplay::Init(const bsg::SceneNodeHandle &guiNode)
{
   m_guiNode = guiNode;
}

VectorOfAnim GUIDisplay::Rotate(Time now, AnimationDoneNotifier *notifier)
{
   VectorOfAnim animationVec;

   if (m_isRotated)
   {
      AnimBindingHermiteQuaternionAngle *anim = new AnimBindingHermiteQuaternionAngle(&m_guiNode->GetRotation());
      anim->Evaluator()->Init(ROTATION_VEC, -ROTATION_ANGLE, 0.0f);
      anim->Interpolator()->Init(now, now + Time(1, Time::eSECONDS), BaseInterpolator::eLIMIT);

      animationVec.push_back(anim);

      AnimBindingHermiteVec3 *anim2 = new AnimBindingHermiteVec3(&m_guiNode->GetScale());
      anim2->Init(REDUCED_SCALE, now, ONE_TO_ONE_SCALE, now + Time(1, Time::eSECONDS),
                  BaseInterpolator::eLIMIT, notifier);

      animationVec.push_back(anim2);

      m_isRotated = false;
   }
   else
   {
      AnimBindingHermiteQuaternionAngle *anim = new AnimBindingHermiteQuaternionAngle(&m_guiNode->GetRotation());
      anim->Evaluator()->Init(ROTATION_VEC, 0.0f, - ROTATION_ANGLE);
      anim->Interpolator()->Init(now, now + Time(1, Time::eSECONDS), BaseInterpolator::eLIMIT);

      animationVec.push_back(anim);

      AnimBindingHermiteVec3 *anim2 = new AnimBindingHermiteVec3(&m_guiNode->GetScale());
      anim2->Init(ONE_TO_ONE_SCALE, now, REDUCED_SCALE, now + Time(1, Time::eSECONDS), 
                  BaseInterpolator::eLIMIT, notifier);

      animationVec.push_back(anim2);

      m_isRotated = true;
   }

   return animationVec;
}

void GUIDisplay::ResetRotation()
{
   if (m_isRotated)
   {
      m_guiNode->SetRotation(0.0, ROTATION_VEC);
      m_guiNode->SetScale(Vec3(1.0,1.0,1.0));

      m_isRotated = !m_isRotated;
   }
}

void GUIDisplay::Rotate()
{
   if (!m_isRotated)
   {
      m_guiNode->SetRotation(-ROTATION_ANGLE, ROTATION_VEC);
      m_guiNode->SetScale(REDUCED_SCALE);

      m_isRotated = !m_isRotated;
   }
}
