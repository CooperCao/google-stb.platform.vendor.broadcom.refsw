/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "vulkan.h"

namespace bvk {

struct Viewport
{
   Viewport() {}
   Viewport(const VkViewport &vp);

   void Set(const VkViewport &vp);

   int x;
   int y;
   int w;
   int h;

   float depthNear;
   float depthFar;
   float depthDiff;

   float internalScale[2];
};

}
