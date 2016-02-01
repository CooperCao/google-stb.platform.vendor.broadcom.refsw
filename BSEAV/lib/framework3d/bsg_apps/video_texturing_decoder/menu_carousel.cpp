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

#include "menu_carousel.h"
#include "bsg_application.h"
#include "bsg_shape.h"
#include "bsg_image_png.h"
#include "bsg_animator.h"
#include "bsg_math.h"

#include <fstream>

using namespace bsg;

MenuCarousel::MenuCarousel(uint32_t numNodes,
                           FontHandle font, const Vec2 &textPos) :
   Carousel(),
   m_effect(New),
   m_font(font),
   m_numGeoms(0),
   m_curText(0)
{
   m_effect->Load("null.bfx");

   m_text[0].SetFont(m_font);
   m_text[1].SetFont(m_font);
   m_text[0].Params() = TextParams(textPos, Vec3(1, 1, 1), 1.0f);
   m_text[1].Params() = TextParams(textPos, Vec3(1, 1, 1), 0.0f);

   // Build the carousel nodes the way we want them
   uint32_t s;
   float z = -0.1f;
   float separation = 0.015f;

   numNodes = numNodes | 1;
   uint32_t sideNodes = (numNodes - 1) / 2;

   std::vector<Transform>  positions;

   // Make the lower nodes
   for (s = 0; s < sideNodes; s++)
   {
      Transform       tran;

      tran.SetPosition(Vec3(0.0f, -separation * (sideNodes - s), z));

      positions.push_back(tran);
   }

   // Make the center node
   {
      Transform       tran;

      tran.SetPosition(Vec3(0.0f, 0.0f, z + 0.02f));

      positions.push_back(tran);
   }

   // Make the upper nodes
   for (s = 1; s <= sideNodes; s++)
   {
      Transform       tran;

      tran.SetPosition(Vec3(0.0f, separation * s, z));

      positions.push_back(tran);
   }

   SetLayout(positions);
}

MenuCarousel::~MenuCarousel()
{
   //EffectLibrary::DestroyHandle(m_effect);
}

void MenuCarousel::FadeOut(Text &text, Time start, Time animationTime)
{
   AnimBindingLerpFloat *fadeOut = new AnimBindingLerpFloat;
   fadeOut->Init(text.Params().GetOpacity(), start, 
                            AnimatableFloat(0.0f), start + animationTime);
   fadeOut->Bind(&text.Params().GetOpacity());
   m_animList.Append(fadeOut);
}

void MenuCarousel::FadeOut(bsg::Time start, bsg::Time animationTime)
{
   FadeOut(m_text[m_curText], start, animationTime);
}

void MenuCarousel::FadeIn(Text &text, Time start, Time animationTime)
{
   AnimBindingLerpFloat *fadeIn = new AnimBindingLerpFloat;
   fadeIn->Init(text.Params().GetOpacity(), start, 
                           AnimatableFloat(1.0f), start + animationTime);
   fadeIn->Bind(&text.Params().GetOpacity());
   m_animList.Append(fadeIn);
}

void MenuCarousel::FadeIn(bsg::Time start, bsg::Time animationTime)
{
   FadeIn(m_text[m_curText], start, animationTime);
}

void MenuCarousel::Next(Time now, Time animationTime)
{
   if (Carousel::IsAnimating()) // Can't interrupt the carousel spinning
      return;

   m_animList.Clear();

   Carousel::Next(now, animationTime);

   // Setup a fade out
   m_text[!m_curText].SetString(m_quickText[CurrentIndex()]);

   FadeOut(m_text[m_curText], now, animationTime);
   FadeIn(m_text[!m_curText], now, animationTime);

   m_curText = !m_curText;
}

void MenuCarousel::Prev(Time now, Time animationTime)
{
   if (Carousel::IsAnimating()) // Can't interrupt the carousel spinning
      return;

   m_animList.Clear();

   Carousel::Prev(now, animationTime);

   // Setup a fade out
   m_text[!m_curText].SetString(m_quickText[CurrentIndex()]);

   FadeOut(m_text[m_curText], now, animationTime);
   FadeIn(m_text[!m_curText], now, animationTime);

   m_curText = !m_curText;
}

void MenuCarousel::AddEntry(const std::string &text)
{
   MaterialHandle mat(New);
   mat->SetEffect(m_effect);

   float height = 0.01f;
   float width = 16.0f * height / 9.0f;

   GeometryHandle geom = QuadFactory(width, height, 0.0f, eZ_AXIS).MakeGeometry(mat);
   AddGeometry(geom);

   m_quickText.push_back(text);

   m_numGeoms++;
}

void MenuCarousel::RenderText()
{
   m_text[0].Render();
   m_text[1].Render();
}

bool MenuCarousel::UpdateAnimationList(Time t)
{
   bool changed = false;

   changed = Carousel::UpdateAnimationList(t);
   changed = m_animList.UpdateTime(t) || changed;

   return changed;
}

void MenuCarousel::Prepare()
{
   bool dirty = m_dirty;

   Carousel::Prepare();

   if (dirty)
      m_text[0].SetString(m_quickText[CurrentIndex()]);
}
