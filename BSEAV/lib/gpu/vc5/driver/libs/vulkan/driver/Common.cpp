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

v3d_compare_func_t TranslateCompareFunc(VkCompareOp op)
{
   assert(op >= VK_COMPARE_OP_NEVER && op <= VK_COMPARE_OP_ALWAYS);
   return static_cast<v3d_compare_func_t>(op);
}

v3d_stencil_op_t TranslateStencilOp(VkStencilOp op)
{
   switch (op)
   {
   case VK_STENCIL_OP_ZERO:                  return V3D_STENCIL_OP_ZERO;
   case VK_STENCIL_OP_KEEP:                  return V3D_STENCIL_OP_KEEP;
   case VK_STENCIL_OP_REPLACE:               return V3D_STENCIL_OP_REPLACE;
   case VK_STENCIL_OP_INCREMENT_AND_CLAMP:   return V3D_STENCIL_OP_INCR;
   case VK_STENCIL_OP_DECREMENT_AND_CLAMP:   return V3D_STENCIL_OP_DECR;
   case VK_STENCIL_OP_INVERT:                return V3D_STENCIL_OP_INVERT;
   case VK_STENCIL_OP_INCREMENT_AND_WRAP:    return V3D_STENCIL_OP_INCWRAP;
   case VK_STENCIL_OP_DECREMENT_AND_WRAP:    return V3D_STENCIL_OP_DECWRAP;
   default:
      unreachable();
      return V3D_STENCIL_OP_INVALID;
   }
}

} // namespace bvk
