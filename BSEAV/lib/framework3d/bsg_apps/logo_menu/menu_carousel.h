/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
