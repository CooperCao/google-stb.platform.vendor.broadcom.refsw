/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
