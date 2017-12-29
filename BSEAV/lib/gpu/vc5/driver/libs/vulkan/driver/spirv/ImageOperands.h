/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Extractor.h"
#include <array>

namespace bvk {

///////////////////////////////////////////////////////////////////
// ImageOperands
//
// Some Node types have a set of variable data in the SPIRV.
// This class contains the variable payload data for ImageOperands.
// The Extractor class defers to this class to import the data.
///////////////////////////////////////////////////////////////////
class ImageOperands
{
public:
   using Payload = std::array<NodeIndex, 2>;

   friend Extractor &operator>>(Extractor &ext, ImageOperands &ops);

   // Accessors for possible payloads - will return nullptr if not present
   const Node *GetBias() const
   {
      if (m_imageOperandsMask & spv::ImageOperandsBiasMask)
         return m_operands[spv::ImageOperandsBiasShift][0];
      return nullptr;
   }

   const Node *GetLod() const
   {
      if (m_imageOperandsMask & spv::ImageOperandsLodMask)
         return m_operands[spv::ImageOperandsLodShift][0];
      return nullptr;
   }

   const Node *GetGradDx() const
   {
      if (m_imageOperandsMask & spv::ImageOperandsGradMask)
         return m_operands[spv::ImageOperandsGradShift][0];
      return nullptr;
   }

   const Node *GetGradDy() const
   {
      if (m_imageOperandsMask & spv::ImageOperandsGradMask)
         return m_operands[spv::ImageOperandsGradShift][1];
      return nullptr;
   }

   const Node *GetConstOffset() const
   {
      if (m_imageOperandsMask & spv::ImageOperandsConstOffsetMask)
         return m_operands[spv::ImageOperandsConstOffsetShift][0];
      return nullptr;
   }

   const Node *GetOffset() const
   {
      if (m_imageOperandsMask & spv::ImageOperandsOffsetMask)
         return m_operands[spv::ImageOperandsOffsetShift][0];
      return nullptr;
   }

   const Node *GetConstOffsets() const
   {
      if (m_imageOperandsMask & spv::ImageOperandsConstOffsetsMask)
         return m_operands[spv::ImageOperandsConstOffsetsShift][0];
      return nullptr;
   }

   const Node *GetSample() const
   {
      if (m_imageOperandsMask & spv::ImageOperandsSampleMask)
         return m_operands[spv::ImageOperandsSampleShift][0];
      return nullptr;
   }

   const Node *GetMinLod() const
   {
      if (m_imageOperandsMask & spv::ImageOperandsMinLodMask)
         return m_operands[spv::ImageOperandsMinLodShift][0];
      return nullptr;
   }

private:
   uint32_t                m_imageOperandsMask = 0;
   std::array<Payload, 8>  m_operands;
};

// Extraction operator to build an ImageOperands
Extractor &operator>>(Extractor &ext, ImageOperands &ops);

} // namespace bvk
