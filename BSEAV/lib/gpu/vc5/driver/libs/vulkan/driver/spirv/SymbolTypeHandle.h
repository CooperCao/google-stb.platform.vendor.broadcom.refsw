/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "glsl_symbols.h"
#include "ModuleAllocator.h"
#include <vector>

namespace bvk {

class Module;

////////////////////////////////////////////////////////
// SymbolTypeHandle
//
// Wraps the glsl 'C' SymbolType with a C++ interface
//
// Types live into the link stage, so are either created
// in static tables by the compiler or are dynamically
// created in the Module memory arena.
////////////////////////////////////////////////////////
class SymbolTypeHandle
{
public:
   SymbolTypeHandle() :
      m_symbolType(nullptr)
   {}

   SymbolTypeHandle(const SymbolTypeHandle &rhs) :
      m_symbolType(rhs.m_symbolType)
   {}

   explicit SymbolTypeHandle(SymbolType *symbolType) :
      m_symbolType(symbolType)
   {}

   operator SymbolType *() { return m_symbolType; }

   static SymbolTypeHandle Primitive(PrimitiveTypeIndex index);
   static SymbolTypeHandle Sampler();
   static SymbolTypeHandle Array(const Module &module, uint32_t size, SymbolTypeHandle elementType);

   template <typename M>
   static SymbolTypeHandle Struct(const Module &module, const char *name, const M &members);

   static SymbolTypeHandle Function(const Module &module, uint32_t numParams, SymbolTypeHandle returnType);
   static SymbolTypeHandle Pointer(const Module &module, SymbolTypeHandle targetType);

   uint32_t           GetNumScalars() const { return m_symbolType->scalar_count; }
   SymbolTypeFlavour  GetFlavour()    const { return m_symbolType->flavour;      }
   const char        *GetName()       const { return m_symbolType->name;         }

   SymbolTypeHandle GetElementType()  const
   {
      assert(GetFlavour() == SYMBOL_ARRAY_TYPE);
      return SymbolTypeHandle(m_symbolType->u.array_type.member_type);
   }

   uint32_t GetElementCount() const
   {
      assert(GetFlavour() == SYMBOL_ARRAY_TYPE);
      return m_symbolType->u.array_type.member_count;
   }

   DataflowType       ToDataflowType(uint32_t index) const;
   PrimitiveTypeIndex GetIndex()                     const;
   bool               IsSampler()                    const;
   bool               IsImage()                      const;

private:
   SymbolType  *m_symbolType;
};

} // namespace bvk
