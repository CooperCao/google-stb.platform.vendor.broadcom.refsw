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
   MenuCarousel(float radius, float selScale, const std::vector<bsg::SceneNodeHandle> &nodes);
   virtual ~MenuCarousel();
};

#endif /* __MENU_CAROUSEL_H__ */
