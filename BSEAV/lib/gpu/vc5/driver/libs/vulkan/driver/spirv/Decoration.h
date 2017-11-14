/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "Extractor.h"
#include <cassert>

namespace bvk {

///////////////////////////////////////////////////////////////////
// Decoration
//
// Some Node types have a set of variable data in the SPIRV.
// This class contains the variable payload data for Decorations.
// The Extractor class defers to this class to import the data.
///////////////////////////////////////////////////////////////////
class Decoration
{
public:
   friend Extractor &operator>>(Extractor &ext, Decoration &deco);

   bool HasLiteral() const;

   spv::Decoration GetKind() const { return m_kind; }

   bool Is(spv::Decoration kind) const
   {
      return m_kind == kind;
   }

   uint32_t GetLiteral() const
   {
      assert(HasLiteral());
      return m_payload;
   }

   spv::BuiltIn GetBuiltIn() const
   {
      assert(m_kind == spv::Decoration::BuiltIn);
      return static_cast<spv::BuiltIn>(m_payload);
   }

   spv::FunctionParameterAttribute GetFuncParamAttr() const
   {
      assert(m_kind == spv::Decoration::FuncParamAttr);
      return static_cast<spv::FunctionParameterAttribute>(m_payload);
   }

   spv::FPRoundingMode GetFPRoundingMode() const
   {
      assert(m_kind == spv::Decoration::FPRoundingMode);
      return static_cast<spv::FPRoundingMode>(m_payload);
   }

   uint32_t GetFPFastMathModeMask() const
   {
      assert(m_kind == spv::Decoration::FPFastMathMode);
      return m_payload;
   }

private:
   spv::Decoration m_kind;
   uint32_t        m_payload = 0;
};

// Extraction operator to build a Decorations object
Extractor &operator>>(Extractor &ext, Decoration &deco);

} // namespace bvk
