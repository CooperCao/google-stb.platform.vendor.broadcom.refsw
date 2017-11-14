/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "ImageOperands.h"

namespace bvk {

Extractor &operator>>(Extractor &ext, ImageOperands &ops)
{
   ext >> ops.m_imageOperandsMask;

   for (uint32_t i = 0; i < 8; i++)
   {
      if ((ops.m_imageOperandsMask & (1 << i)) != 0)
      {
         ext >> ops.m_operands[i][0];
         if ((1 << i) == spv::ImageOperandsGradMask)
            ext >> ops.m_operands[i][1];
      }
   }

   return ext;
}

} // namespace bvk
