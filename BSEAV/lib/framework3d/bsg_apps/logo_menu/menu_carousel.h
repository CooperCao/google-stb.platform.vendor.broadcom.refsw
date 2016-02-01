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

#ifndef __MENU_CAROUSEL_H__
#define __MENU_CAROUSEL_H__

#include "bsg_carousel.h"
#include "bsg_text.h"

#include <string>
#include <vector>

class MenuCarousel : public bsg::Carousel
{
public:
   MenuCarousel(float radius, uint32_t numNodes, float angleStep, 
                float zDistSel, float zDistOthers, bsg::FontHandle font, 
                const bsg::Vec2 &textPos, const bsg::Vec2 &descPos, float startAngle, float yOffset);
   virtual ~MenuCarousel();

   virtual void RenderText();
   virtual bool UpdateAnimationList(bsg::Time t);

   virtual void AddEntry(const std::string &textureFile, const std::string &quickText, const std::string &descText, bool enabled);
   virtual void Next(bsg::Time now, bsg::Time animationTime);
   virtual void Prev(bsg::Time now, bsg::Time animationTime);

   // Resize the description font
   void Resize();

protected:
   virtual void Prepare();

private:
   void SwapText(bsg::Time now, bsg::Time animationTime);
   void FadeOut(bsg::Text &text, bsg::Time start, bsg::Time animationTime);
   void FadeIn(bsg::Text &text, bsg::Time start, bsg::Time animationTime);
   void AnimateOut(bsg::Text &descText, bsg::Time start, bsg::Time animationTime);
   void AnimateIn(bsg::Text &descText, bsg::Time start, bsg::Time animationTime);
   void ChangeDesc(bsg::Time now, bsg::Time animationTime);

protected:
   bsg::EffectHandle          m_effect;
   bsg::EffectHandle          m_disabledEffect;
   bsg::FontHandle            m_descFont;
   bsg::FontHandle            m_font;
   bsg::Text                  m_text[2];
   bsg::Text                  m_desc[2];
   bsg::Vec2                  m_descPos;
   std::vector<std::string>   m_quickText;
   std::vector<std::string>   m_descText;
   uint32_t                   m_numGeoms;
   uint32_t                   m_curText;
   bsg::AnimationList         m_animList;
   bsg::AnimatableFloat       m_scrollInValue;
   bsg::AnimatableFloat       m_scrollOutValue;
};

#endif /* __MENU_CAROUSEL_H__ */

