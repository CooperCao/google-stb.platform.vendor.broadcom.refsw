/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "SymbolTypeHandle.h"
#include "Module.h"
#include "PrimitiveTypes.h"
#include "TypeBuilder.h"

namespace bvk {

PrimitiveTypeIndex SymbolTypeHandle::GetIndex() const
{
   assert(m_symbolType->flavour == SYMBOL_PRIMITIVE_TYPE);
   return m_symbolType->u.primitive_type.index;
}

SymbolTypeHandle SymbolTypeHandle::Primitive(PrimitiveTypeIndex index)
{
   return SymbolTypeHandle(&primitiveTypes[index]);
}

SymbolTypeHandle SymbolTypeHandle::Sampler()
{
   return SymbolTypeHandle(&primitiveTypes[PRIM_SAMPLER]);
}

bool SymbolTypeHandle::IsSampler() const
{
   return glsl_get_scalar_value_type_index(m_symbolType, 0) == PRIM_SAMPLER;
}

bool SymbolTypeHandle::IsImage() const
{
   if (m_symbolType->flavour != SYMBOL_PRIMITIVE_TYPE)
      return false;
   return (primitiveTypeFlags[m_symbolType->u.primitive_type.index] & PRIM_STOR_IMAGE_TYPE) != 0;
}

static SymbolType *NewSymbolType(const Module &module, SymbolTypeFlavour flavour, const char *name,
                                 uint32_t numScalars = ~0u)
{
   SymbolType *type = module.New<SymbolType>();

   type->flavour      = flavour;
   type->name         = name;
   type->scalar_count = numScalars;

   return type;
}

SymbolTypeHandle SymbolTypeHandle::Array(const Module &module, uint32_t size, SymbolTypeHandle elementType)
{
   SymbolType *type = NewSymbolType(module, SYMBOL_ARRAY_TYPE, "array", size * elementType.GetNumScalars());

   type->u.array_type.member_count = size;
   type->u.array_type.member_type  = elementType.m_symbolType;

   return SymbolTypeHandle(type);
}

template <class M>
SymbolTypeHandle SymbolTypeHandle::Struct(const Module &module, const char *name, const M &members)
{
   uint32_t scalarCount = 0;
   uint32_t numMembers  = members.size();

   SymbolType *type = NewSymbolType(module, SYMBOL_STRUCT_TYPE, name);

   type->u.struct_type.member_count = numMembers;
   type->u.struct_type.member = module.NewArray<StructMember>(numMembers);

   for (uint32_t i = 0; i < numMembers; ++i)
   {
      StructMember &structMember = type->u.struct_type.member[i];

      structMember.layout = nullptr;        // TODO
      structMember.memq   = MEMORY_NONE;    // TODO
      structMember.name   = "field";        // TODO
      structMember.prec   = PREC_NONE;      // TODO
      structMember.interp = INTERP_SMOOTH;  // TODO
      structMember.auxq   = AUXILIARY_NONE; // TODO
      structMember.type   = members[i];

      scalarCount += structMember.type->scalar_count;
   }

   type->scalar_count = scalarCount;

   return SymbolTypeHandle(type);
}

template SymbolTypeHandle SymbolTypeHandle::Struct<MemberIter>(const Module &module, const char *name,
                                                               const MemberIter &members);

SymbolTypeHandle SymbolTypeHandle::Function(const Module &module, uint32_t numParams,
                                            SymbolTypeHandle returnType)
{
   SymbolType *type = NewSymbolType(module, SYMBOL_FUNCTION_TYPE, "function");

   type->u.function_type.param_count = numParams;
   type->u.function_type.return_type = returnType.m_symbolType;

   // TODO add parameters

   return SymbolTypeHandle(type);
}

SymbolTypeHandle SymbolTypeHandle::Pointer(const Module &module, SymbolTypeHandle targetType)
{
   SymbolType *type = NewSymbolType(module, SYMBOL_ARRAY_TYPE, "pointer", 1);

   type->u.array_type.member_count = 1;
   type->u.array_type.member_type  = targetType.m_symbolType;

   return SymbolTypeHandle(type);
}

DataflowType SymbolTypeHandle::ToDataflowType(uint32_t index) const
{
   PrimitiveTypeIndex  type       = glsl_get_scalar_value_type_index(m_symbolType, index);
   const SymbolType   *symbolType = &primitiveTypes[type];

   if (!glsl_prim_is_opaque_type(symbolType))
      return glsl_prim_index_to_df_type(type);
   else
   {
      if (glsl_prim_is_prim_texture_type(symbolType) ||
          glsl_prim_is_prim_comb_sampler_type(symbolType))
      {
         PrimSamplerInfo *psi = glsl_prim_get_image_info(type);
         PrimitiveTypeIndex retBasicType = primitiveScalarTypeIndices[psi->return_type];
         switch (retBasicType)
         {
         case PRIM_FLOAT:    return DF_F_SAMP_IMG;
         case PRIM_INT:      return DF_I_SAMP_IMG;
         case PRIM_UINT:     return DF_U_SAMP_IMG;
         default:            unreachable();
         }
      }
      else if (glsl_prim_is_prim_atomic_type(symbolType))
         return DF_UINT;
      else if (glsl_prim_is_prim_sampler_type(symbolType))
         return DF_SAMPLER;
      else
      {
         PrimSamplerInfo *psi = glsl_prim_get_image_info(type);
         PrimitiveTypeIndex retBasicType = primitiveScalarTypeIndices[psi->return_type];
         switch (retBasicType)
         {
         case PRIM_FLOAT:    return DF_F_STOR_IMG;
         case PRIM_INT:      return DF_I_STOR_IMG;
         case PRIM_UINT:     return DF_U_STOR_IMG;
         default:            unreachable();
         }
      }
   }
}

} // namespace bvk
