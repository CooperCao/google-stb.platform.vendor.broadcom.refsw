/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Viewport.h"

#include <cmath>
#include <cstdlib>

namespace bvk {

Viewport::Viewport(const VkViewport &vp)
{
   Set(vp);
}

void Viewport::Set(const VkViewport &vp)
{
   x = std::lround(vp.x);
   y = std::lround(vp.y);

   // The Vulkan spec says that the application should move the viewport origin to
   // the bottom left if using a -ve viewport height. We need to undo that to do the
   // correct thing.
   if (vp.height < 0.0f)
      y = std::lround(vp.y + vp.height);

   w =           std::lround(vp.width);
   h = std::labs(std::lround(vp.height));   // Ensure +ve height is used here

   depthNear = vp.minDepth;
   depthFar  = vp.maxDepth;
   depthDiff = vp.maxDepth - vp.minDepth;

   internalScale[0] = 128.0f * std::lround(vp.width);
   internalScale[1] = 128.0f * std::lround(vp.height); // Note: can be -ve
}

}
