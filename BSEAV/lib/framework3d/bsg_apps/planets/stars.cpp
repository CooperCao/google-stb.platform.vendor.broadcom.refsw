/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_surface.h"
#include "bsg_material.h"
#include "bsg_effect_semantics.h"
#include "bsg_gl_buffer.h"
#include "bsg_scene_node_callback.h"
#include "stars.h"
#include "planets.h"

using namespace bsg;

struct StarVertex
{
   Vec3  m_position;
   Vec3  m_color;
   float m_size;
};

class NoTranslate : public CallbackOnRenderData
{
public:
   virtual bool OnRenderData(SemanticData &semData)
   {
      Mat4  mv = semData.GetModelViewMatrix();

      mv(0, 3) = 0.0f;
      mv(1, 3) = 0.0f;
      mv(2, 3) = 0.0f;

      semData.SetModelViewMatrix(mv);

      return true;
   }
};

Vec3 Stars::ColorOf(const std::string &starClass) const
{
   Vec3  res(1.0f);

   switch (starClass[0])
   {
   case 'O':
      res = m_colors[0];
      break;
   case 'B':
      res = m_colors[1];
      break;
   case 'A':
      res = m_colors[2];
      break;
   case 'F':
      res = m_colors[3];
      break;
   case 'G':
      res = m_colors[4];
      break;
   case 'K':
      res = m_colors[5];
      break;
   case 'M':
      res = m_colors[6];
      break;
   }

   return res;
}

SceneNodeHandle Stars::CreateGeometry(const PlanetEffects &effects) const
{
   const float    RADIUS = 1000.0f;
   const uint32_t STRIDE = sizeof(StarVertex);

   std::vector<StarVertex>   vb;

   vb.reserve(m_stars.size());

   for (uint32_t s = 0; s < m_stars.size(); ++s)
   {
      const StarInfo  &star = m_stars[s];

      StarVertex  v;

      float theta = star.GetRA();
      float phi   = star.GetDec() + (float)M_PI / 2.0f;

      float x     = -RADIUS * cosf(theta) * sinf(phi);
      float y     = -RADIUS * cosf(phi);
      float z     =  RADIUS * sinf(theta) * sinf(phi);

      v.m_position = Vec3(x, y, z);

      v.m_color    = ColorOf(star.GetClass());
      v.m_size     = (m_magOffset - star.GetMag()) * m_magScale;

      vb.push_back(v);
   }

   SurfaceHandle surf(New);

   surf->SetDraw(GL_POINTS, vb.size(), vb.size() * sizeof(StarVertex), &vb[0]);
   surf->SetPointer(EffectSemantics::eVATTR_POSITION, GLVertexPointer(3, GL_FLOAT, STRIDE, 0 * sizeof(float)));
   surf->SetPointer(EffectSemantics::eVATTR_COLOR,    GLVertexPointer(3, GL_FLOAT, STRIDE, 3 * sizeof(float)));
   surf->SetPointer(EffectSemantics::eVATTR_USER1,    GLVertexPointer(1, GL_FLOAT, STRIDE, 6 * sizeof(float)));
   surf->SetCull(false);
   surf->SetViewFrustumCull(false);

   MaterialHandle material(New, "Stars");

   material->SetEffect(effects.GetStarEffect());

   GeometryHandle geom(New);

   geom->AppendSurface(surf, material);

   SceneNodeHandle node(New);

   node->AppendGeometry(geom);

   return node;
}
