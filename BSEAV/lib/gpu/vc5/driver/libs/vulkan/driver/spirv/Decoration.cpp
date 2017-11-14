/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Decoration.h"

namespace bvk {

Extractor &operator>>(Extractor &ext, Decoration &deco)
{
   ext >> deco.m_kind;

   switch (deco.m_kind)
   {
   case spv::Decoration::SpecId:
   case spv::Decoration::ArrayStride:
   case spv::Decoration::MatrixStride:
   case spv::Decoration::Stream:
   case spv::Decoration::Location:
   case spv::Decoration::Component:
   case spv::Decoration::Index:
   case spv::Decoration::Binding:
   case spv::Decoration::DescriptorSet:
   case spv::Decoration::Offset:
   case spv::Decoration::XfbBuffer:
   case spv::Decoration::XfbStride:
   case spv::Decoration::InputAttachmentIndex:
   case spv::Decoration::Alignment:

   case spv::Decoration::BuiltIn:
   case spv::Decoration::FuncParamAttr:
   case spv::Decoration::FPRoundingMode:
   case spv::Decoration::FPFastMathMode:
      ext >> deco.m_payload;
      break;
   case spv::Decoration::LinkageAttributes:
      // Ignoring
      break;
   default:
      break;
   }

   return ext;
}

bool Decoration::HasLiteral() const
{
   switch (m_kind)
   {
   case spv::Decoration::SpecId:
   case spv::Decoration::ArrayStride:
   case spv::Decoration::MatrixStride:
   case spv::Decoration::Stream:
   case spv::Decoration::Location:
   case spv::Decoration::Component:
   case spv::Decoration::Index:
   case spv::Decoration::Binding:
   case spv::Decoration::DescriptorSet:
   case spv::Decoration::Offset:
   case spv::Decoration::XfbBuffer:
   case spv::Decoration::XfbStride:
   case spv::Decoration::InputAttachmentIndex:
   case spv::Decoration::Alignment:
      return true;
   default:
      return false;
   }
}

} // namespace bvk
