/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "DerivedViewportState.h"

#include <cmath>

namespace bvk {

DerivedViewportState::DerivedViewportState(const VkViewport &vp)
{
   Set(vp);
}

void DerivedViewportState::Set(const VkViewport &vp)
{
   internalDepthRangeNear = vp.minDepth;
   internalDepthRangeFar  = vp.maxDepth;
   internalDepthRangeDiff = vp.maxDepth - vp.minDepth;

   // std::round for consistency with CmdBufState::BuildScissorAndViewportCL
   internalScale[0] = 128.0f * std::round(vp.width);
   internalScale[1] = 128.0f * std::round(vp.height); // Note: can be -ve

   internalZOffset  = vp.minDepth;
}

}
