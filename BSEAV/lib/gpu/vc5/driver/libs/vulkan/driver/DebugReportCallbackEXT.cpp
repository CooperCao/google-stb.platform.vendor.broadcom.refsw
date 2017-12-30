/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

namespace bvk {

DebugReportCallbackEXT::DebugReportCallbackEXT(
   const VkAllocationCallbacks               *pCallbacks,
   bvk::Instance                             *pInstance,
   const VkDebugReportCallbackCreateInfoEXT  *pCreateInfo) :
      Allocating(pCallbacks)
{

}

DebugReportCallbackEXT::~DebugReportCallbackEXT() noexcept
{

}

} // namespace bvk
