/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Common.h"

namespace bvk {

class Extensions
{
public:
   static VkResult EnumerateExtensionProperties(
      VkExtensionProperties *extsIn,
      uint32_t               extsInCount,
      const char            *pLayerName,
      uint32_t              *pPropertyCount,
      VkExtensionProperties *pProperties);
};

} // namespace bvk
