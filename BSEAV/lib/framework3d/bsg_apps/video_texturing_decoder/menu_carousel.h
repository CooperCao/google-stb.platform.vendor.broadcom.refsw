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
   MenuCarousel(uint32_t numNodes, bsg::FontHandle font, const bsg::Vec2 &textPos);
   virtual ~MenuCarousel();

   virtual void RenderText();
   virtual bool UpdateAnimationList(bsg::Time t);

   virtual void AddEntry(const std::string &text);
   virtual void Next(bsg::Time now, bsg::Time animationTime);
   virtual void Prev(bsg::Time now, bsg::Time animationTime);

   void FadeOut(bsg::Time start, bsg::Time animationTime);
   void FadeIn(bsg::Time start, bsg::Time animationTime);

protected:
   virtual void Prepare();

private:
   void SwapText(bsg::Time now, bsg::Time animationTime);
   void FadeOut(bsg::Text &text, bsg::Time start, bsg::Time animationTime);
   void FadeIn(bsg::Text &text, bsg::Time start, bsg::Time animationTime);

protected:
   bsg::EffectHandle          m_effect;
   bsg::FontHandle            m_font;
   bsg::Text                  m_text[2];
   std::vector<std::string>   m_quickText;
   uint32_t                   m_numGeoms;
   uint32_t                   m_curText;
   bsg::AnimationList         m_animList;
};

#endif /* __MENU_CAROUSEL_H__ */
