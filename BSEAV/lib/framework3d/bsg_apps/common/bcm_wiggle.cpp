/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bcm_wiggle.h"

#include "bsg_application_options.h"
#include "bsg_animator.h"
#include "bsg_exception.h"
#include "bsg_shape.h"

using namespace bsg;

QuadStripFactory::QuadStripFactory(uint32_t steps, const Vec2 &origin, float width, float height, eAxis axis) :
   StdFactory(GL_TRIANGLE_STRIP, steps * 2 + 2, false)
{
   float    x = origin.X();
   float    y = origin.Y();
   float    u = 0.0f;
   float    ustep = 1.0f / steps;

   for (uint32_t i = 0; i <= steps; ++i)
   {
      AddVertex(x, y,          0.0f, 0.0f, 1.0f, 0.0f, u, 0.0f, axis);
      AddVertex(x, y + height, 0.0f, 0.0f, 1.0f, 0.0f, u, 1.0f, axis);

      x += width;
      u += ustep;

      if (u > 1.0f)
         u = 1.0f;
   }
}

static const Vec3 s_lightBlue(0.1f, 0.58f, 0.72f);
static const Vec3 s_darkBlue(0.02f, 0.33f, 0.42f);
static const float s_pi = 3.14159265f;

/////////////////////////////////////////////////////////////////////

static AnimBindingLerpFloat *LerpFloat(AnimTarget<float> &target, const Time &now, float duration, float start, float finish)
{
   AnimBindingLerpFloat *anim = new AnimBindingLerpFloat(&target);

   anim->Interpolator()->Init(now, now + duration, BaseInterpolator::eEXTRAPOLATE);
   anim->Evaluator()->Init(start, finish);

   return anim;
}

BCMWiggle::BCMWiggle(AnimationList &animList, const Time &now,  float scale) :
   m_root      (New),
   m_effect    (New),
   m_wiggle1(m_effect, 50, 4.0f * scale, 4.0f * scale),
   m_wiggle2(m_effect, 50, 3.0f * scale, 6.0f * scale),
   m_wiggle3(m_effect, 50, 3.0f * scale, 6.0f * scale)
{
   m_effect->Load("bcm_wiggle.bfx");

   // WIGGLE 1
   m_wiggle1.SetWave (5.0f * scale,               1.0f, 0.0f);
   m_wiggle1.SetBulge(0.3f * scale, 0.3f * scale, 1.0f, 0.0f);
   m_wiggle1.SetAlpha(0.75f);
   m_wiggle1.SetColor(s_lightBlue);

   animList.Append(LerpFloat(m_wiggle1.GetPhase(),      now, 10.0f, 0.0f, s_pi       ));
   animList.Append(LerpFloat(m_wiggle1.GetBulgePhase(), now, 2.0f,  0.0f, s_pi / 4.0f));

   // WIGGLE 2
   m_wiggle2.SetWave (6.0f * scale,               0.8f, s_pi);
   m_wiggle2.SetBulge(0.3f * scale, 0.6f * scale, 1.0f, 0.0f);
   m_wiggle2.SetAlpha(0.75f);
   m_wiggle2.SetColor(s_lightBlue);

   animList.Append(LerpFloat(m_wiggle2.GetPhase(),      now, 10.0f, s_pi, s_pi + s_pi / 1.5f));
   animList.Append(LerpFloat(m_wiggle2.GetBulgePhase(), now, 2.0f,  0.0f, s_pi / 3.0f));

   // WIGGLE 3
   m_wiggle3.SetWave (4.0f * scale,               0.7f, s_pi);
   m_wiggle3.SetBulge(0.2f * scale, 0.3f * scale, 2.0f, 0.0f);
   m_wiggle3.SetAlpha(0.75f);
   m_wiggle3.SetColor(s_lightBlue);

   animList.Append(LerpFloat(m_wiggle3.GetPhase(),      now, 10.0f, s_pi, s_pi + s_pi / 2.5f));
   animList.Append(LerpFloat(m_wiggle3.GetBulgePhase(), now, 2.0f,  0.0f, s_pi / 2.0f));

   SceneNodeHandle   node(New);
   node->AppendGeometry(m_wiggle3.GetGeometry());
   node->SetPosition(Vec3(0.0f, -10.0f * scale, 0.0f));
   node->SetRotation(3.0f, Vec3(0.0f, 0.0f, 1.0f));

   m_root->AppendGeometry(m_wiggle1.GetGeometry());
   m_root->AppendGeometry(m_wiggle2.GetGeometry());
   m_root->AppendChild(node);
}
