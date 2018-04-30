/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "glsl_symbols.h"
#include "Spirv.h"

namespace bvk {

class DflowBuilder;
class NodeTypeStruct;

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
   class MemberIter
   {
   public:
      virtual ~MemberIter() {}

      virtual uint32_t         Size()           const = 0;
      virtual SymbolTypeHandle Type(uint32_t i) const = 0;
   };

   SymbolTypeHandle() :
      m_symbolType(nullptr)
   {}

   SymbolTypeHandle(const SymbolTypeHandle &rhs) :
      m_symbolType(rhs.m_symbolType)
   {}

   SymbolTypeHandle(SymbolType *symbolType) :
      m_symbolType(symbolType)
   {}

   // Primarily for passing to C interfaces
   operator SymbolType *() { return m_symbolType; }

   // Returns void symbol type
   static SymbolTypeHandle Void();

   // Returns bool symbol type
   static SymbolTypeHandle Bool();

   // Returns signed int symbol type
   static SymbolTypeHandle Int();

   // Returns unsigned int symbol type
   static SymbolTypeHandle UInt();

   // Returns float symbol type
   static SymbolTypeHandle Float();

   // Returns vector symbol type of size elemTypes
   static SymbolTypeHandle Vector(SymbolTypeHandle elemType, uint32_t size);

   // Returns matrix symbol type of col x row
   static SymbolTypeHandle Matrix(uint32_t cols, uint32_t rows);

   // Returns sampler symbol type
   static SymbolTypeHandle Sampler();

   // Returns an array type
   static SymbolTypeHandle Array(const DflowBuilder &builder, SymbolTypeHandle elementType, uint32_t size);

   // Returns a struct symbol type
   static SymbolTypeHandle Struct(const DflowBuilder &builder, const NodeTypeStruct *, const MemberIter &members);

   // Returns a pointer symbol type
   static SymbolTypeHandle Pointer(const DflowBuilder &builder, SymbolTypeHandle targetType);

   // Returns a sampled image symbol type
   static SymbolTypeHandle SampledImage(SymbolTypeHandle sampledType, spv::Dim dim, uint32_t arrayed, uint32_t ms);

   // Returns an image symbol type
   static SymbolTypeHandle Image(SymbolTypeHandle sampledType, spv::Dim dim, uint32_t arrayed);

   // Returns a combined sampled image symbol type
   static SymbolTypeHandle CombinedSampledImage(SymbolTypeHandle sampledType, spv::Dim dim, uint32_t arrayed, uint32_t ms);

   SymbolTypeHandle IndexType();
   SymbolTypeHandle MatrixSubscriptVector(uint32_t i);

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

   SymbolTypeHandle GetStructMemberType(uint32_t index) const
   {
      assert(GetFlavour() == SYMBOL_STRUCT_TYPE);
      return m_symbolType->u.struct_type.member[index].type;
   }

   DataflowType       ToDataflowType(uint32_t index) const;
   PrimitiveTypeIndex GetIndex()                     const;
   bool               IsSampler()                    const;
   bool               IsImage()                      const;

private:
   SymbolTypeHandle(PrimitiveTypeIndex index);

private:
   SymbolType  *m_symbolType;
};

} // namespace bvk
