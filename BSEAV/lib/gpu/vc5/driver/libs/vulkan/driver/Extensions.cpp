/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Extensions.h"

#include <string.h>
#include <algorithm>

namespace bvk {

// Helper method to correctly handle extension reporting.
// Used by both Instance and PhysicalDevice.
VkResult Extensions::EnumerateExtensionProperties(
   VkExtensionProperties   *extsIn,
   uint32_t                 extsInCount,
   const char              *pLayerName,
   uint32_t                *pPropertyCount,
   VkExtensionProperties   *pProperties)
{
   VkResult result = VK_SUCCESS;

   if (pLayerName != nullptr) // No layer specific extensions
   {
      *pPropertyCount = 0;
   }
   else
   {
      result = sizedListQuery<VkExtensionProperties>(pProperties, pPropertyCount,
                                                     extsIn, extsInCount);
   }

   return result;
}

} // namespace bvk
