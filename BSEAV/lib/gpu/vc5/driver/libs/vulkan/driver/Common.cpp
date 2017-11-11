/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Common.h"

#include <algorithm>

namespace bvk {

void Intersect(VkRect2D *result, const VkRect2D &r1, const VkRect2D &r2)
{
   VkOffset2D r1End = { r1.offset.x + static_cast<int32_t>(r1.extent.width)  - 1,
                        r1.offset.y + static_cast<int32_t>(r1.extent.height) - 1 };
   VkOffset2D r2End = { r2.offset.x + static_cast<int32_t>(r2.extent.width)  - 1,
                        r2.offset.y + static_cast<int32_t>(r2.extent.height) - 1 };

   VkOffset2D resStart;
   resStart.x = std::max(r1.offset.x, r2.offset.x);
   resStart.y = std::max(r1.offset.y, r2.offset.y);

   VkOffset2D resEnd;
   resEnd.x = std::min(r1End.x, r2End.x);
   resEnd.y = std::min(r1End.y, r2End.y);

   if (resEnd.x - resStart.x >= 0 && resEnd.y - resStart.y >= 0)
   {
      result->offset = resStart;
      result->extent.width = resEnd.x - resStart.x + 1;
      result->extent.height = resEnd.y - resStart.y + 1;
   }
   else
   {
      *result = {};
   }
}

void Union(VkRect2D *result, const VkRect2D &r1, const VkRect2D &r2)
{
   // Don't calculate directly into result in case it's also r1 or r2
   VkRect2D res;

   VkOffset2D r1End = { r1.offset.x + static_cast<int32_t>(r1.extent.width)  - 1,
                        r1.offset.y + static_cast<int32_t>(r1.extent.height) - 1 };
   VkOffset2D r2End = { r2.offset.x + static_cast<int32_t>(r2.extent.width)  - 1,
                        r2.offset.y + static_cast<int32_t>(r2.extent.height) - 1 };

   res.offset.x = std::min(r1.offset.x, r2.offset.x);
   res.offset.y = std::min(r1.offset.y, r2.offset.y);

   res.extent.width  = std::max(r1End.x - res.offset.x + 1, r2End.x - res.offset.x + 1);
   res.extent.height = std::max(r1End.y - res.offset.y + 1, r2End.y - res.offset.y + 1);

   *result = res;
}

} // namespace bvk
