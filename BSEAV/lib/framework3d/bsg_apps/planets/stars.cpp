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
