/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class DebugReportCallbackEXT: public NonCopyable, public Allocating
{
public:
   DebugReportCallbackEXT(
      const VkAllocationCallbacks               *pCallbacks,
      bvk::Instance                             *pInstance,
      const VkDebugReportCallbackCreateInfoEXT  *pCreateInfo);

   ~DebugReportCallbackEXT() noexcept;

   // Implementation specific from this point on

};

} // namespace bvk
