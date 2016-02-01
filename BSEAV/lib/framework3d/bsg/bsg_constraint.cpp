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
#include "bsg_constraint.h"
#include "bsg_scene_node.h"

using namespace bsg;

////////////////////////////////
// ConstraintFollowPosition
////////////////////////////////

//! Destructor
ConstraintFollowPosition::~ConstraintFollowPosition()
{
}

//! ConstraintFixedPosition installation
void ConstraintFollowPosition::Install(ConstraintFollowPositionHandle thisHandle, SceneNodeHandle &owner, SceneNodeHandle &follow)
{
   m_follow = follow.Ptr();

   m_memoFollow.Clear();

   m_follow->AppendChild(owner);

   owner->AddConstraint(thisHandle);
}

void ConstraintFollowPosition::Remove(const SceneNode &owner)
{
   m_follow->RemoveChild(owner);
}

bool ConstraintFollowPosition::Visit(const SceneNode *who, const Mat4 &xform) const
{
   if (who == m_follow)
      m_memoFollow.Set(GetTranslation(xform));

   return true;
}

////////////////////////////////
// ConstraintLerpPosition
////////////////////////////////

//! Destructor
ConstraintLerpPosition::~ConstraintLerpPosition()
{
}

//! ConstraintLerpPosition installation set the initial parameters of the constraint, adds it to its owner node and connect the parent nodes i.e. those which
//! constrain us.
void ConstraintLerpPosition::Install(ConstraintLerpPositionHandle thisHandle, SceneNodeHandle &owner, SceneNodeHandle &from, SceneNodeHandle &to, float alpha)
{
   m_from  = from.Ptr();
   m_to    = to.Ptr();

   m_alpha.Set(alpha);

   m_memoFrom.Clear();
   m_memoTo.Clear();

   m_from->AppendChild(owner);
   m_to->AppendChild(owner);

   owner->AddConstraint(thisHandle);
}

void ConstraintLerpPosition::Remove(const SceneNode &owner)
{
   m_from->RemoveChild(owner);
   m_to->RemoveChild(owner);
}

bool ConstraintLerpPosition::Visit(const SceneNode *who, const Mat4 &xform) const
{
   if (who == m_from)
      m_memoFrom.Set(GetTranslation(xform));

   if (who == m_to)
      m_memoTo.Set(GetTranslation(xform));

   return Ready();
}

////////////////////////////////
// ConstraintLookAt
////////////////////////////////

//! Destructor
ConstraintLookAt::~ConstraintLookAt()
{
}

//! ConstraintLoookAt installation set the initial parameters of the constraint, adds it to its owner node and connect the parent nodes i.e. those which
//! constrain us.
void ConstraintLookAt::Install(ConstraintLookAtHandle thisHandle, SceneNodeHandle &owner, SceneNodeHandle &location, SceneNodeHandle &lookAt, const Vec3 &up)
{
   m_location = location.Ptr();
   m_lookAt   = lookAt.Ptr();

   m_up       = up;

   m_memoLocation.Clear();
   m_memoLookAt.Clear();

   m_location->AppendChild(owner);
   m_lookAt->AppendChild(owner);

   owner->AddConstraint(thisHandle);
}

void ConstraintLookAt::Remove(const SceneNode &owner)
{
   m_location->RemoveChild(owner);
   m_lookAt->RemoveChild(owner);
}

//! Remember the value from a parent node.  Return true if all values have been acquired.
bool ConstraintLookAt::Visit(const SceneNode *who, const Mat4 &xform) const
{
   if (who == m_location)
      m_memoLocation.Set(GetTranslation(xform));

   if (who == m_lookAt)
      m_memoLookAt.Set(GetTranslation(xform));

   return Ready();
}

