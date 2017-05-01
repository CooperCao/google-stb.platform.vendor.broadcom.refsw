/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
