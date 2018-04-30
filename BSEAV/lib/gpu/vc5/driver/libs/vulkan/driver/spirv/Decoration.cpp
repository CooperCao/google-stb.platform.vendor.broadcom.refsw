/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Decoration.h"
#include "Extractor.h"
#include "Nodes.h"

namespace bvk {


///////////////////////////////////////////////////////////////////////////////////////////////////
// Decoration
///////////////////////////////////////////////////////////////////////////////////////////////////
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
///////////////////////////////////////////////////////////////////////////////////////////////////
// DecorationQuery
///////////////////////////////////////////////////////////////////////////////////////////////////
DecorationQuery::DecorationQuery(const Node *node) :
   m_decorations(node->GetDecorations())
{}

bool DecorationQuery::Literal(uint32_t *literal, spv::Decoration decoType) const
{
   for (const Decoration *decoration : *m_decorations)
   {
      if (decoration->Is(decoType))
      {
         *literal = decoration->GetLiteral();
         return true;
      }
   }

   return false;
}

uint32_t DecorationQuery::RequireLiteral(spv::Decoration dec) const
{
   uint32_t l = 0;      /* Value never used */
   bool ok = Literal(&l, dec);
   assert(ok);
   return l;
}

bool DecorationQuery::Builtin(spv::BuiltIn *builtin) const
{
   for (const Decoration *decoration : *m_decorations)
   {
      if (decoration->Is(spv::Decoration::BuiltIn))
      {
         if (builtin != nullptr)
            *builtin = decoration->GetBuiltIn();
         return true;
      }
   }

   return false;
}

bool DecorationQuery::Has(spv::Decoration decoType) const
{
   for (const Decoration *decoration : *m_decorations)
      if (decoration->Is(decoType))
         return true;

   return false;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// MemberDecorationQuery
///////////////////////////////////////////////////////////////////////////////////////////////////
MemberDecorationQuery::MemberDecorationQuery(const NodeTypeStruct *node, uint32_t index) :
   m_decorations(node->GetData()->GetMemberDecorations()),
   m_index(index)
{}

bool MemberDecorationQuery::Literal(uint32_t *literal, spv::Decoration decoType) const
{
   for (const Decoration *decoration : *this)
   {
      if (decoration->Is(decoType))
      {
         *literal = decoration->GetLiteral();
         return true;
      }
   }

   return false;
}

uint32_t MemberDecorationQuery::RequireLiteral(spv::Decoration decoType) const
{
   uint32_t ret = 0; // Value never used
   bool ok = Literal(&ret, decoType);
   assert(ok);
   return ret;
}

bool MemberDecorationQuery::Builtin(spv::BuiltIn *builtin) const
{
   for (const Decoration *decoration : *this)
   {
      if (decoration->Is(spv::Decoration::BuiltIn))
      {
         if (builtin != nullptr)
            *builtin = decoration->GetBuiltIn();
         return true;
      }
   }

   return false;
}

bool MemberDecorationQuery::Has(spv::Decoration decoType) const
{
   for (const Decoration *decoration : *this)
      if (decoration->Is(decoType))
         return true;

   return false;
}

} // namespace bvk
