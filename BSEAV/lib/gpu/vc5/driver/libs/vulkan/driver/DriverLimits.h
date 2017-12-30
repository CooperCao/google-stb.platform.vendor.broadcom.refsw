/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

namespace bvk {

// Limitations of the driver implementation, or that we want to impose, not h/w limits
class DriverLimits
{
public:
   enum
   {
      eMaxBoundDescriptorSets    = 16,         // Must be at least 4

      // We use 27-bits to encode the uniform byte offset
      eBufferOffsetBits          = 27,
      eBufferOffsetMask          = (1 << eBufferOffsetBits) - 1,
      eMaxBufferByte             = (1 << eBufferOffsetBits) - 1,

      // We use 5-bits to encode the UBO/SSBO descriptor table index
      eBufferTableIndexBits      = 5,
      eBufferTableIndexMask      = (1 << eBufferTableIndexBits) - 1,
      eMaxBufferTableIndex       = (1 << eBufferTableIndexBits) - 1,

      // We use 5-bits to encode the sampler descriptor table index
      eSamplerTableIndexBits     = 5,
      eSamplerTableIndexMask     = (1 << eSamplerTableIndexBits) - 1,
      eMaxSamplerTableIndex      = (1 << eSamplerTableIndexBits) - 1,

      // We use 16-bits to encode the image descriptor table index
      eImageTableIndexBits       = 16,
      eImageTableIndexMask       = (1 << eImageTableIndexBits) - 1,
      eMaxImageTableIndex        = (1 << eImageTableIndexBits) - 1,

      // We use UBO index slot 31 to represent a push constant lookup
      ePushConstantIdentifier    = eMaxBufferTableIndex,
   };
};

} // namespace bvk
