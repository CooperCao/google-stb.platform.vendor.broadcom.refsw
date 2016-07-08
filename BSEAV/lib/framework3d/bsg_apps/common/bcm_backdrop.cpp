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

#include "bcm_backdrop.h"

#include "bsg_application_options.h"
#include "bsg_scene_node.h"
#include "bsg_shape.h"
#include "bsg_material.h"
#include "bsg_effect.h"
#include "bsg_gl_texture.h"
#include "bsg_image_pkm.h"
#include "bsg_exception.h"

#include <iostream>
#include <istream>
#include <fstream>
#include <vector>

using namespace bsg;

/////////////////////////////////////////////////////////////////////

BCMBackdrop::BCMBackdrop(uint32_t numSquaresY, float yBottom, float yTop, uint32_t yBlankBottom, uint32_t yBlankTop,
                         bool cycleColors /*= true*/, float zPos /* = 0.99 */, int32_t sortPriority/* = INT_MIN*/) :
   m_root(New),
   m_gradientEffect(New),
   m_centerMat(New),
   m_currentColors(4),
   m_zPos(zPos),
   m_coloredCorner(eBCM_BR),
   m_colorToUse(eBCM_TR),
   m_cycleColors(cycleColors),
   m_sortPriority(sortPriority)
{
   Application   *app = Application::Instance();

   // Building the gradiant material
   m_gradientEffect->Load("bcm_backdropGrad.bfx");

   m_gradientMat = MaterialHandle(New);
   m_gradientMat->SetEffect(m_gradientEffect);

   m_blues[eBCM_BL] = Vec3(Color((uint8_t)5, 83, 106).VecF());
   m_blues[eBCM_BR] = Vec3(Color((uint8_t)0, 30, 42).VecF());
   m_blues[eBCM_TL] = Vec3(Color((uint8_t)15, 134, 169).VecF());
   m_blues[eBCM_TR] = Vec3(Color((uint8_t)5, 83, 106).VecF());

   m_greens[eBCM_BL] = Vec3(Color((uint8_t)90, 117, 3).VecF());
   m_greens[eBCM_BR] = Vec3(Color((uint8_t)68, 90, 1).VecF());
   m_greens[eBCM_TL] = Vec3(Color((uint8_t)142, 174, 40).VecF());
   m_greens[eBCM_TR] = Vec3(Color((uint8_t)90, 117, 3).VecF());

   m_purples[eBCM_BL] = Vec3(Color((uint8_t)91, 12, 64).VecF());
   m_purples[eBCM_BR] = Vec3(Color((uint8_t)64, 3, 43).VecF());
   m_purples[eBCM_TL] = Vec3(Color((uint8_t)151, 56, 117).VecF());
   m_purples[eBCM_TR] = Vec3(Color((uint8_t)91, 12, 64).VecF());

   for (uint32_t c = 0; c < 4; c++)
      m_currentColors[c] = m_blues[c];

   m_gradientMat->SetUniformValue("u_colors", m_currentColors);

   // Create the material for square animation
   // but it might not be used if there is no square
   m_backdropMat = MaterialHandle(New);

   // If there is some square to animate
   if (0 != numSquaresY)
   {
      // Load the effect files for the backdrop and create materials
      EffectHandle  effect(New);
      effect->Load("bcm_backdrop.bfx");

      //m_backdropMat = MaterialHandle(New);
      m_backdropMat->SetEffect(effect);

      TextureHandle texture(New);
      texture->SetAutoMipmap(true);
      texture->TexImage2D(Image("bcm_backdropCheck.png", Image::eL8));
      m_backdropMat->SetTexture("u_tex", texture);

      uint32_t numSquaresX = (uint32_t)ceilf((float)numSquaresY * (float)app->GetWindowWidth() / (float)app->GetWindowHeight());
      m_windowSizeDiv = Vec2((float)numSquaresX, (float)numSquaresY * (yTop - yBottom));
      Vec2 windowClip((float)yBlankBottom, (float)numSquaresY - (float)yBlankTop);

      float yMin = yBottom * 2.0f - 1.0f;
      float yMax = yTop * 2.0f - 1.0f;

      m_backdropMat->SetUniformValue("u_colors", m_currentColors);

      GeometryHandle geom = CreateMyQuad(Vec2(-1.0f, yMin), Vec2(1.0f, yMax), m_zPos, m_windowSizeDiv, m_backdropMat);
      geom->SetSortPriority(m_sortPriority);
      m_root->AppendGeometry(geom);

      // Setup material animation
      float startP = 0.0f;
      float endP   = 1.0f * m_windowSizeDiv.X();

      m_backdropMat->SetUniform("u_windowSizeDiv", m_windowSizeDiv);
      m_backdropMat->SetUniform("u_windowClip", windowClip);

      Time now = app->FrameTimestamp();

      AnimBindingLerpFloat *animParam = new AnimBindingLerpFloat(&m_backdropMat->GetUniform1f("u_animParam"));
      animParam->Init(startP, 0.0f, endP, 75.0f, BaseInterpolator::eREPEAT);
      m_animList.Append(animParam);

      const float secsChange = 4.0f;
      const float secsConstant = 60.0f;

      if (m_cycleColors)
      {
         for (uint32_t c = 0; c < 4; c++)
         {
            m_colorSequence[c].Init(&m_animList);

            uint32_t startLabel = m_colorSequence[c].AppendLabel();

            m_colorSequence[c].AppendDelay(secsConstant);

            AnimBindingHermiteVec3 *a1 = new AnimBindingHermiteVec3(&m_currentColors[c]);
            a1->Evaluator()->Init(m_blues[c], m_greens[c]);
            m_colorSequence[c].AppendAnimation(a1, secsChange);

            m_colorSequence[c].AppendDelay(secsConstant);

            AnimBindingHermiteVec3 *a2 = new AnimBindingHermiteVec3(&m_currentColors[c]);
            a2->Evaluator()->Init(m_greens[c], m_purples[c]);
            m_colorSequence[c].AppendAnimation(a2, secsChange);

            m_colorSequence[c].AppendDelay(secsConstant);

            AnimBindingHermiteVec3 *a3 = new AnimBindingHermiteVec3(&m_currentColors[c]);
            a3->Evaluator()->Init(m_purples[c], m_blues[c]);
            m_colorSequence[c].AppendAnimation(a3, secsChange);

            m_colorSequence[c].AppendGoto(startLabel);
            m_colorSequence[c].Start(now);
         }
      }
   }
}

BCMBackdrop::~BCMBackdrop()
{
}

void BCMBackdrop::AddShadedPanel(float bottom, float top)
{
   float yMin = bottom * 2.0f - 1.0f;
   float yMax = top * 2.0f - 1.0f;

   GeometryHandle geom = CreateMyQuad(Vec2(-1.0f, yMin), Vec2(1.0f, yMax), m_zPos, m_windowSizeDiv, m_gradientMat);
   geom->SetSortPriority(m_sortPriority);

   m_root->AppendGeometry(geom);
}

void BCMBackdrop::AddShadedPanel(float bottom, float top,
                                 const Vec3 &blCol, const Vec3 &brCol, const Vec3 &tlCol, const Vec3 &trCol)
{
   float yMin = bottom * 2.0f - 1.0f;
   float yMax = top * 2.0f - 1.0f;

   MaterialHandle mat = MaterialHandle(New);
   mat->SetEffect(m_gradientEffect);

   std::vector<Vec3> colors(4);
   colors[eBCM_BL] = blCol;
   colors[eBCM_BR] = brCol;
   colors[eBCM_TL] = tlCol;
   colors[eBCM_TR] = trCol;

   mat->SetUniformValue("u_colors", colors);

   GeometryHandle geom = CreateMyQuad(Vec2(-1.0f, yMin), Vec2(1.0f, yMax), m_zPos, m_windowSizeDiv, mat);
   geom->SetSortPriority(m_sortPriority);

   m_root->AppendGeometry(geom);
}

void BCMBackdrop::AddCenterPanel(float bottom, float top, Corner coloredCorner, Corner colorToUse)
{
   float yMin = bottom * 2.0f - 1.0f;
   float yMax = top * 2.0f - 1.0f;

   m_centerMat->SetEffect(m_gradientEffect);

   Vec3 black(0.0f, 0.0f, 0.0f);

   std::vector<Vec3> colors(4);
   colors[eBCM_BL] = black;
   colors[eBCM_BR] = black;
   colors[eBCM_TL] = black;
   colors[eBCM_TR] = black;

   m_coloredCorner = coloredCorner;
   m_colorToUse = colorToUse;

   colors[coloredCorner] = m_currentColors[colorToUse];

   m_centerMat->SetUniformValue("u_colors", colors);

   GeometryHandle geom = CreateMyQuad(Vec2(-1.0f, yMin), Vec2(1.0f, yMax), m_zPos, m_windowSizeDiv, m_centerMat);
   geom->SetSortPriority(m_sortPriority);

   m_root->AppendGeometry(geom);
}

SurfaceHandle BCMBackdrop::CreateMyQuad(const Vec2 &bl, const Vec2 &tr, float offset, const Vec2 &tcMult)
{
   const uint32_t FLOAT_STRIDE = 6;

   float x0 = bl.X();
   float x1 = tr.X();
   float y0 = bl.Y();
   float y1 = tr.Y();

   GLfloat quad[] =
   {
      // POSITION       INDEX    TC
      x1,  y1, offset,  eBCM_TR,      tcMult.X(), tcMult.Y(),
      x1,  y0, offset,  eBCM_BR,      tcMult.X(), 0,
      x0,  y1, offset,  eBCM_TL,      0,          tcMult.Y(),
      x0,  y0, offset,  eBCM_BL,      0,          0
   };

   static const GLushort quad_idx[] =
   {
      2, 3, 0, 1
   };

   SurfaceHandle surf(New);

   surf->SetDraw(GL_TRIANGLE_STRIP, 4, sizeof(quad), quad, sizeof(quad_idx), quad_idx);

   surf->SetPointer(EffectSemantics::eVATTR_POSITION,  GLVertexPointer(3, GL_FLOAT, FLOAT_STRIDE * sizeof(GLfloat), 0));
   surf->SetPointer(EffectSemantics::eVATTR_USER1,     GLVertexPointer(1, GL_FLOAT, FLOAT_STRIDE * sizeof(GLfloat), sizeof(GLfloat) * 3));
   surf->SetPointer(EffectSemantics::eVATTR_TEXCOORD1, GLVertexPointer(2, GL_FLOAT, FLOAT_STRIDE * sizeof(GLfloat), sizeof(GLfloat) * 4));

   surf->SetBound(Bound(4, quad, FLOAT_STRIDE));
   surf->SetCull(true);

   return surf;
}

GeometryHandle BCMBackdrop::CreateMyQuad(const Vec2 &bl, const Vec2 &tr, float offset, const Vec2 &tcMult, MaterialHandle material)
{
   GeometryHandle geom(New);

   geom->AppendSurface(CreateMyQuad(bl, tr, offset, tcMult), material);

   return geom;
}

void BCMBackdrop::ResizeHandler(uint32_t width, uint32_t height)
{
   m_backdropMat->SetUniform("u_windowSizeDiv10", Vec4((float)width * 0.1f, (float)height * 0.1f,
                                                   7.0f, ceil((float)height * 0.1f - 7.0f)));

   m_backdropMat->SetUniform("u_windowSizeDiv20", Vec4((float)width * 0.05f, (float)height * 0.05f,
                                                   3.5f, ceil((float)height * 0.05f - 3.5f)));
}

bool BCMBackdrop::UpdateAnimationList(Time t)
{
   m_animList.UpdateTime(t);

   if (m_cycleColors)
{
   // Update all our color uniforms
   Vec3 black(0.0f, 0.0f, 0.0f);

   std::vector<Vec3> colors(4);
   colors[eBCM_BL] = black;
   colors[eBCM_BR] = black;
   colors[eBCM_TL] = black;
   colors[eBCM_TR] = black;
   colors[m_coloredCorner] = m_currentColors[m_colorToUse];

   m_centerMat->SetUniformValue("u_colors", colors);
   m_backdropMat->SetUniformValue("u_colors", m_currentColors);
   m_gradientMat->SetUniformValue("u_colors", m_currentColors);
}

   return true;
}

const SceneNodeHandle BCMBackdrop::RootNode()
{
   return m_root;
}

Vec3 BCMBackdrop::Blues(Corner corner) const
{
   return m_blues[corner];
}

Vec3 BCMBackdrop::Greens(Corner corner) const
{
   return m_greens[corner];
}

Vec3 BCMBackdrop::Purples(Corner corner) const
{
   return m_purples[corner];
}

Vec3 BCMBackdrop::GetCurrentColor(Corner corner) const
{
   return m_currentColors[corner].Get();
}
