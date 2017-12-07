/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "vulkan.h"

namespace bvk {

struct DerivedViewportState
{
   DerivedViewportState() {}
   DerivedViewportState(const VkViewport &vp);

   void Set(const VkViewport &vp);

   float internalDepthRangeNear;
   float internalDepthRangeFar;
   float internalDepthRangeDiff;
   float internalScale[2];
   float internalZOffset;
};

}
