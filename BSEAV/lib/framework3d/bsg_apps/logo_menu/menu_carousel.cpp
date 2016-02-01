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
#include "bsg_image_png.h"
#include "bsg_shape.h"
#include "bsg_animator.h"
#include "bsg_math.h"

#include <fstream>

using namespace bsg;

MenuCarousel::MenuCarousel(float radius, uint32_t numNodes, 
                           float angleStep, float zDistSel, float zDistOthers,
                           FontHandle font, const Vec2 &textPos, const Vec2 &descPos,
                           float startAngle, float yOffset) :
   Carousel(),
   m_effect(New),
   m_disabledEffect(New),
   m_descFont(New),
   m_font(font),
   m_descPos(descPos),
   m_numGeoms(0),
   m_curText(0),
   m_scrollInValue(0),
   m_scrollOutValue(0)
{
   m_descFont->SetFontInPixels(m_font->FontName(), m_font->PointSizePixels() * 0.75f);

   m_effect->Load("menu_icon.bfx");
   m_disabledEffect->Load("menu_icon_disabled.bfx");

   m_text[0].SetFont(m_font);
   m_text[1].SetFont(m_font);
   m_text[0].Params() = TextParams(textPos, Vec3(1, 1, 1), 1.0f);
   m_text[1].Params() = TextParams(textPos, Vec3(1, 1, 1), 0.0f);

   m_desc[0].SetFont(m_descFont);
   m_desc[1].SetFont(m_descFont);
   m_desc[0].Params() = TextParams(m_descPos, Vec3(1, 1, 1), 0.9f);
   m_desc[1].Params() = TextParams(Vec2(-0.55f, m_descPos[1]), Vec3(1, 1, 1), 0.9f);

   // Build the carousel nodes the way we want them
   uint32_t s;
   float    zOff = -radius - zDistOthers;

   numNodes = numNodes | 1;
   uint32_t sideNodes = (numNodes - 1) / 2;

   float startAng = startAngle - angleStep * sideNodes;
   
   std::vector<Transform>  positions;

   // Make the lower nodes
   for (s = 0; s < sideNodes; s++)
   {
      Transform       tran;
      float           ang = (startAng + (s * angleStep));

      tran.SetPosition(Vec3(0.0f, yOffset + radius * sinf(DEG2RAD * ang), radius * cosf(DEG2RAD * ang) + zOff));
      tran.SetRotation(ang, Vec3(-1.0f, 0.0f, 0.0f));

      positions.push_back(tran);
   }

   // Make the center node
   {
      Transform       tran;

      tran.SetPosition(Vec3(0.0f, yOffset, -zDistSel));

      positions.push_back(tran);
   }

   // Make the upper nodes
   for (s = 1; s <= sideNodes; s++)
   {
      Transform       tran;
      float           ang = s * angleStep;

      tran.SetPosition(Vec3(0.0f, yOffset + radius * sinf(DEG2RAD * ang), radius * cosf(DEG2RAD * ang) + zOff));
      tran.SetRotation(ang, Vec3(-1.0f, 0.0f, 0.0f));

      positions.push_back(tran);
   }

   SetLayout(positions);
}

MenuCarousel::~MenuCarousel()
{
   m_effect.Clear();
   m_disabledEffect.Clear();
}

void MenuCarousel::FadeOut(Text &text, Time start, Time animationTime)
{
   AnimBindingLerpFloat *fadeOut = new AnimBindingLerpFloat;
   fadeOut->Init(text.Params().GetOpacity(), start, 
                            AnimatableFloat(0.0f), start + animationTime);
   fadeOut->Bind(&text.Params().GetOpacity());
   m_animList.Append(fadeOut);
}

void MenuCarousel::FadeIn(Text &text, Time start, Time animationTime)
{
   AnimBindingLerpFloat *fadeIn = new AnimBindingLerpFloat;
   fadeIn->Init(text.Params().GetOpacity(), start, 
                           AnimatableFloat(1.0f), start + animationTime);
   fadeIn->Bind(&text.Params().GetOpacity());
   m_animList.Append(fadeIn);
}

void MenuCarousel::AnimateOut(Text &descText, Time start, Time animationTime)
{
   AnimatableVec2 outP(Vec2(-0.55f, m_descPos[1]));

   AnimBindingHermiteVec2 *scrollOut = new AnimBindingHermiteVec2;
   scrollOut->Init(descText.Params().GetPosition(), start, 
      outP, start + animationTime);

   scrollOut->Bind(&descText.Params().GetPosition());
   m_animList.Append(scrollOut);

   AnimBindingHermiteFloat *fout = new AnimBindingHermiteFloat;
   fout->Init(AnimatableFloat(0.0f), start, 
      AnimatableFloat(1.0f), start + animationTime);

   fout->Bind(&m_scrollOutValue);
   m_animList.Append(fout);
}

void MenuCarousel::AnimateIn(Text &descText, Time start, Time animationTime)
{
   AnimatableVec2 outP(m_descPos);

   AnimBindingHermiteVec2 *scrollIn = new AnimBindingHermiteVec2;
   scrollIn->Init(descText.Params().GetPosition(), start, 
      outP, start + animationTime);

   scrollIn->Bind(&descText.Params().GetPosition());
   m_animList.Append(scrollIn);

   AnimBindingHermiteFloat *fout = new AnimBindingHermiteFloat;
   fout->Init(AnimatableFloat(0.0f), start, 
      AnimatableFloat(1.0f), start + animationTime);

   fout->Bind(&m_scrollInValue);
   m_animList.Append(fout);
}

void MenuCarousel::ChangeDesc(Time now, Time animationTime)
{
   // We might be in the middle of a scroll in or out already. Check.
   if (m_scrollInValue > 0.0f && m_scrollInValue < 1.0f)
   {
      // We are scrolling in
      AnimateOut(m_desc[m_curText], now, animationTime * m_scrollInValue);
      AnimateIn(m_desc[!m_curText], now + animationTime, animationTime);
   }
   else if (m_scrollOutValue > 0.0f && m_scrollOutValue < 1.0f)
   {
      // We are scrolling out
      AnimateOut(m_desc[m_curText], now, animationTime * (1.0f - m_scrollOutValue));
      AnimateIn(m_desc[!m_curText], now + animationTime, animationTime);
   }
   else
   {
      AnimateOut(m_desc[m_curText], now, animationTime);
      AnimateIn(m_desc[!m_curText], now + animationTime, animationTime);
   }
}

void MenuCarousel::Next(Time now, Time animationTime)
{
   if (Carousel::IsAnimating()) // Can't interrupt the carousel spinning
      return;

   m_animList.Clear();

   Carousel::Next(now, animationTime);

   // Setup a fade out
   m_text[!m_curText].SetString(m_quickText[CurrentIndex()]);
   m_desc[!m_curText].SetString(m_descText[CurrentIndex()]);

   FadeOut(m_text[m_curText], now, animationTime);
   FadeIn(m_text[!m_curText], now, animationTime);

   ChangeDesc(now, animationTime * 2.0f);

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
   m_desc[!m_curText].SetString(m_descText[CurrentIndex()]);

   FadeOut(m_text[m_curText], now, animationTime);
   FadeIn(m_text[!m_curText], now, animationTime);

   ChangeDesc(now, animationTime * 2.0f);

   m_curText = !m_curText;
}

void MenuCarousel::AddEntry(const std::string &textureFile, const std::string &quickText, 
   const std::string &descText, bool enabled)
{
   TextureHandle texture(New);
   texture->SetAutoMipmap(true);
   texture->TexImage2D(Image(textureFile, Image::eRGB565));

   MaterialHandle mat(New);

   if (enabled)
   {
      mat->SetEffect(m_effect);
   }
   else
   {
      TextureHandle noEntryTex(New);
      noEntryTex->SetAutoMipmap(true);
      noEntryTex->TexImage2D(Image("no_entry.png", Image::eRGBA4444));

      mat->SetEffect(m_disabledEffect);
      mat->SetTexture("u_noEntryTex", noEntryTex);
   }

   mat->SetTexture("u_tex", texture);

   GeometryHandle geom = QuadFactory(1.0f, 1.0f, 0.0f, eZ_AXIS).MakeGeometry(mat);
   AddGeometry(geom);

   m_quickText.push_back(quickText);
   m_descText.push_back(descText);

   m_numGeoms++;
}

void MenuCarousel::RenderText()
{
   m_text[0].Render();
   m_text[1].Render();
   m_desc[0].Render();
   m_desc[1].Render();
}

void MenuCarousel::Resize()
{
   m_descFont->SetFontInPixels(m_font->FontName(), m_font->PointSizePixels() * 0.75f);
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
   {
      m_text[0].SetString(m_quickText[CurrentIndex()]);
      m_desc[0].SetString(m_descText[CurrentIndex()]);
   }
}
