/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "SymbolTypeHandle.h"
#include "ModuleAllocator.h"
#include "Module.h"
#include "PrimitiveTypes.h"
#include "TypeBuilder.h"

namespace bvk {

PrimitiveTypeIndex SymbolTypeHandle::GetIndex() const
{
   assert(m_symbolType->flavour == SYMBOL_PRIMITIVE_TYPE);
   return m_symbolType->u.primitive_type.index;
}

SymbolTypeHandle::SymbolTypeHandle(PrimitiveTypeIndex pti) :
   m_symbolType(&primitiveTypes[pti])
{
}

SymbolTypeHandle SymbolTypeHandle::Void()
{
   return PRIM_VOID;
}

SymbolTypeHandle SymbolTypeHandle::Bool()
{
   return PRIM_BOOL;
}

SymbolTypeHandle SymbolTypeHandle::Int()
{
   return PRIM_INT;
}

SymbolTypeHandle SymbolTypeHandle::UInt()
{
   return PRIM_UINT;
}

SymbolTypeHandle SymbolTypeHandle::Float()
{
   return PRIM_FLOAT;
}

SymbolTypeHandle SymbolTypeHandle::Vector(SymbolTypeHandle elemType, uint32_t size)
{
   return glsl_prim_vector_type(elemType.GetIndex(), size);
}

SymbolTypeHandle SymbolTypeHandle::Matrix(uint32_t cols, uint32_t rows)
{
   return primitiveMatrixTypeIndices[cols][rows];
}

SymbolTypeHandle SymbolTypeHandle::Sampler()
{
   return PRIM_SAMPLER;
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
                                 uint32_t numScalars)
{
   SymbolType *type = module.New<SymbolType>();

   type->flavour      = flavour;
   type->name         = name;
   type->scalar_count = numScalars;

   return type;
}

SymbolTypeHandle SymbolTypeHandle::Array(const Module &module, SymbolTypeHandle elementType, uint32_t size)
{
   SymbolType *type = NewSymbolType(module, SYMBOL_ARRAY_TYPE, "array", size * elementType.GetNumScalars());

   type->u.array_type.member_count = size;
   type->u.array_type.member_type  = elementType.m_symbolType;

   return type;
}

template <class M>
SymbolTypeHandle SymbolTypeHandle::Struct(const Module &module, const M &members)
{
   uint32_t scalarCount = 0;
   uint32_t numMembers  = members.size();

   StructMember *memb = module.NewArray<StructMember>(numMembers);

   for (uint32_t i = 0; i < numMembers; ++i)
   {
      StructMember &structMember = memb[i];

      structMember.layout = nullptr;        // TODO
      structMember.memq   = MEMORY_NONE;    // TODO
      structMember.name   = "field";        // TODO
      structMember.prec   = PREC_NONE;      // TODO
      structMember.interp = INTERP_SMOOTH;  // TODO
      structMember.auxq   = AUXILIARY_NONE; // TODO
      structMember.type   = members[i];

      scalarCount += structMember.type->scalar_count;
   }

   SymbolType *type = NewSymbolType(module, SYMBOL_STRUCT_TYPE, "struct", scalarCount);
   type->u.struct_type.member_count = numMembers;
   type->u.struct_type.member       = memb;

   return type;
}

template SymbolTypeHandle SymbolTypeHandle::Struct<MemberIter>(const Module &module, const MemberIter &members);

SymbolTypeHandle SymbolTypeHandle::Pointer(const Module &module, SymbolTypeHandle targetType)
{
   SymbolType *type = NewSymbolType(module, SYMBOL_ARRAY_TYPE, "pointer", 1);

   type->u.array_type.member_count = 1;
   type->u.array_type.member_type  = targetType.m_symbolType;

   return type;
}

static PrimitiveTypeIndex SampledImageTypeIndex(PrimitiveTypeIndex sampledType, spv::Dim dim, uint32_t arrayed, uint32_t ms)
{
   PrimitiveTypeIndex   pti = PRIMITIVE_TYPE_UNDEFINED;

   switch (dim)
   {
   case spv::Dim::Dim1D:          // TODO : is treating 1D as 2D correct??
   case spv::Dim::Dim2D:
      if (arrayed)
         pti = ms ? PRIM_TEXTURE2DMSARRAY : PRIM_TEXTURE2DARRAY;
      else
         pti = ms ? PRIM_TEXTURE2DMS : PRIM_TEXTURE2D;
      break;
   case spv::Dim::Dim3D:
      pti = PRIM_TEXTURE3D;
      break;
   case spv::Dim::Cube:
      if (arrayed)
         pti = PRIM_TEXTURECUBEARRAY;
      else
         pti = PRIM_TEXTURECUBE;
      break;
   case spv::Dim::Buffer:
      pti = PRIM_TEXTUREBUFFER;
      break;
   case spv::Dim::Rect:        // TODO : is this required?
   case spv::Dim::SubpassData: // TODO : input attachment related
   default:
      unreachable();
   }

   if (sampledType == PRIM_INT)
   {
      switch (pti)
      {
      case PRIM_TEXTURE2DARRAY:     pti = PRIM_ITEXTURE2DARRAY;   break;
      case PRIM_TEXTURE2D:          pti = PRIM_ITEXTURE2D;        break;
      case PRIM_TEXTURE2DMSARRAY:   pti = PRIM_ITEXTURE2DMSARRAY; break;
      case PRIM_TEXTURE2DMS:        pti = PRIM_ITEXTURE2DMS;      break;
      case PRIM_TEXTURE3D:          pti = PRIM_ITEXTURE3D;        break;
      case PRIM_TEXTURECUBE:        pti = PRIM_ITEXTURECUBE;      break;
      case PRIM_TEXTURECUBEARRAY:   pti = PRIM_ITEXTURECUBEARRAY; break;
      case PRIM_TEXTUREBUFFER:      pti = PRIM_ITEXTUREBUFFER;    break;
      default:                      unreachable();
      }
   }
   else if (sampledType == PRIM_UINT)
   {
      switch (pti)
      {
      case PRIM_TEXTURE2DARRAY:     pti = PRIM_UTEXTURE2DARRAY;   break;
      case PRIM_TEXTURE2D:          pti = PRIM_UTEXTURE2D;        break;
      case PRIM_TEXTURE2DMSARRAY:   pti = PRIM_UTEXTURE2DMSARRAY; break;
      case PRIM_TEXTURE2DMS:        pti = PRIM_UTEXTURE2DMS;      break;
      case PRIM_TEXTURE3D:          pti = PRIM_UTEXTURE3D;        break;
      case PRIM_TEXTURECUBE:        pti = PRIM_UTEXTURECUBE;      break;
      case PRIM_TEXTURECUBEARRAY:   pti = PRIM_UTEXTURECUBEARRAY; break;
      case PRIM_TEXTUREBUFFER:      pti = PRIM_UTEXTUREBUFFER;    break;
      default:                      unreachable();
      }
   }
   else
      assert(sampledType == PRIM_FLOAT);

   return pti;
}

SymbolTypeHandle SymbolTypeHandle::SampledImage(SymbolTypeHandle sampledType, spv::Dim dim, uint32_t arrayed, uint32_t ms)
{
   PrimitiveTypeIndex   sampledIndex = sampledType.GetIndex();

   return SampledImageTypeIndex(sampledIndex, dim, arrayed, ms);
}

static PrimitiveTypeIndex ImageTypeIndex(PrimitiveTypeIndex sampledType, spv::Dim dim, uint32_t arrayed)
{
   PrimitiveTypeIndex   pti = PRIMITIVE_TYPE_UNDEFINED;

   switch (dim)
   {
   case spv::Dim::Dim1D:          // TODO : is treating 1D as 2D correct??
   case spv::Dim::Dim2D:
   case spv::Dim::SubpassData:
      if (arrayed)
         pti = PRIM_IMAGE2DARRAY;
      else
         pti = PRIM_IMAGE2D;
      break;
   case spv::Dim::Dim3D:
      pti = PRIM_IMAGE3D;
      break;
   case spv::Dim::Cube:
      if (arrayed)
         pti = PRIM_IMAGECUBEARRAY;
      else
         pti = PRIM_IMAGECUBE;
      break;
   case spv::Dim::Buffer:
      pti = PRIM_IMAGEBUFFER;
      break;
   case spv::Dim::Rect:        // TODO : is this required?
   default:
      unreachable();
   }

   if (sampledType == PRIM_INT)
   {
      switch (pti)
      {
      case PRIM_IMAGE2DARRAY:    pti = PRIM_IIMAGE2DARRAY;     break;
      case PRIM_IMAGE2D:         pti = PRIM_IIMAGE2D;          break;
      case PRIM_IMAGE3D:         pti = PRIM_IIMAGE3D;          break;
      case PRIM_IMAGECUBE:       pti = PRIM_IIMAGECUBE;        break;
      case PRIM_IMAGECUBEARRAY:  pti = PRIM_IIMAGECUBEARRAY;   break;
      case PRIM_IMAGEBUFFER:     pti = PRIM_IIMAGEBUFFER;      break;
      default:                   unreachable();
      }
   }
   else if (sampledType == PRIM_UINT)
   {
      switch (pti)
      {
      case PRIM_IMAGE2DARRAY:    pti = PRIM_UIMAGE2DARRAY;     break;
      case PRIM_IMAGE2D:         pti = PRIM_UIMAGE2D;          break;
      case PRIM_IMAGE3D:         pti = PRIM_UIMAGE3D;          break;
      case PRIM_IMAGECUBE:       pti = PRIM_UIMAGECUBE;        break;
      case PRIM_IMAGECUBEARRAY:  pti = PRIM_UIMAGECUBEARRAY;   break;
      case PRIM_IMAGEBUFFER:     pti = PRIM_UIMAGEBUFFER;      break;
      default:                   unreachable();
      }
   }
   else
      assert(sampledType == PRIM_FLOAT);

   return pti;
}

SymbolTypeHandle SymbolTypeHandle::Image(SymbolTypeHandle sampledType, spv::Dim dim, uint32_t arrayed)
{
   PrimitiveTypeIndex   sampledIndex = sampledType.GetIndex();

   return ImageTypeIndex(sampledIndex, dim, arrayed);
}

static PrimitiveTypeIndex CombinedSampledImageTypeIndex(PrimitiveTypeIndex sampledType, spv::Dim dim, uint32_t arrayed, uint32_t ms)
{
   PrimitiveTypeIndex   pti = PRIMITIVE_TYPE_UNDEFINED;

   switch (dim)
   {
   case spv::Dim::Dim1D:          // TODO : is treating 1D as 2D correct??
   case spv::Dim::Dim2D:
      if (arrayed)
         pti = ms ? PRIM_SAMPLER2DMSARRAY : PRIM_SAMPLER2DARRAY;
      else
         pti = ms ? PRIM_SAMPLER2DMS : PRIM_SAMPLER2D;
      break;
   case spv::Dim::Dim3D:
      pti = PRIM_SAMPLER3D;
      break;
   case spv::Dim::Cube:
      if (arrayed)
         pti = PRIM_SAMPLERCUBEARRAY;
      else
         pti = PRIM_SAMPLERCUBE;
      break;
   case spv::Dim::Buffer:
      pti = PRIM_SAMPLERBUFFER;
      break;
   case spv::Dim::Rect:        // TODO : is this required?
   case spv::Dim::SubpassData: // TODO : input attachment related
   default:
      unreachable();
   }

   if (sampledType == PRIM_INT)
   {
      switch (pti)
      {
      case PRIM_SAMPLER2DARRAY:     pti = PRIM_ISAMPLER2DARRAY;   break;
      case PRIM_SAMPLER2D:          pti = PRIM_ISAMPLER2D;        break;
      case PRIM_SAMPLER2DMSARRAY:   pti = PRIM_ISAMPLER2DMSARRAY; break;
      case PRIM_SAMPLER2DMS:        pti = PRIM_ISAMPLER2DMS;      break;
      case PRIM_SAMPLER3D:          pti = PRIM_ISAMPLER3D;        break;
      case PRIM_SAMPLERCUBE:        pti = PRIM_ISAMPLERCUBE;      break;
      case PRIM_SAMPLERCUBEARRAY:   pti = PRIM_ISAMPLERCUBEARRAY; break;
      case PRIM_SAMPLERBUFFER:      pti = PRIM_ISAMPLERBUFFER;    break;
      default:                      unreachable();
      }
   }
   else if (sampledType == PRIM_UINT)
   {
      switch (pti)
      {
      case PRIM_SAMPLER2DARRAY:     pti = PRIM_USAMPLER2DARRAY;   break;
      case PRIM_SAMPLER2D:          pti = PRIM_USAMPLER2D;        break;
      case PRIM_SAMPLER2DMSARRAY:   pti = PRIM_USAMPLER2DMSARRAY; break;
      case PRIM_SAMPLER2DMS:        pti = PRIM_USAMPLER2DMS;      break;
      case PRIM_SAMPLER3D:          pti = PRIM_USAMPLER3D;        break;
      case PRIM_SAMPLERCUBE:        pti = PRIM_USAMPLERCUBE;      break;
      case PRIM_SAMPLERCUBEARRAY:   pti = PRIM_USAMPLERCUBEARRAY; break;
      case PRIM_SAMPLERBUFFER:      pti = PRIM_USAMPLERBUFFER;    break;
      default:                      unreachable();
      }
   }
   else
      assert(sampledType == PRIM_FLOAT);

   return pti;
}

SymbolTypeHandle SymbolTypeHandle::CombinedSampledImage(SymbolTypeHandle sampledType, spv::Dim dim, uint32_t arrayed, uint32_t ms)
{
   PrimitiveTypeIndex   sampledIndex = sampledType.GetIndex();

   return CombinedSampledImageTypeIndex(sampledIndex, dim, arrayed, ms);
}

SymbolTypeHandle SymbolTypeHandle::IndexType()
{
   return primitiveTypeSubscriptTypes[GetIndex()];
}

SymbolTypeHandle SymbolTypeHandle::MatrixSubscriptVector(uint32_t i)
{
   return glsl_prim_matrix_type_subscript_vector(GetIndex(), i);
}

DataflowType SymbolTypeHandle::ToDataflowType(uint32_t index) const
{
   PrimitiveTypeIndex  type       = glsl_get_scalar_value_type_index(m_symbolType, index);
   const SymbolType   *symbolType = &primitiveTypes[type];

   if (!glsl_prim_is_opaque_type(symbolType))
      return glsl_prim_index_to_df_type(type);
   else
   {
      if (glsl_prim_is_prim_atomic_type(symbolType))
         return DF_UINT;
      else if (glsl_prim_is_prim_sampler_type(symbolType))
         return DF_SAMPLER;
      else
      {
         PrimSamplerInfo *psi = glsl_prim_get_image_info(type);
         PrimitiveTypeIndex retBasicType = primitiveScalarTypeIndices[psi->return_type];
         bool storage = glsl_prim_is_prim_image_type(symbolType);
         switch (retBasicType)
         {
         case PRIM_FLOAT:    return storage ? DF_F_STOR_IMG : DF_F_SAMP_IMG;
         case PRIM_INT:      return storage ? DF_I_STOR_IMG : DF_I_SAMP_IMG;
         case PRIM_UINT:     return storage ? DF_U_STOR_IMG : DF_U_SAMP_IMG;
         default:            unreachable();
         }
      }
   }
}

} // namespace bvk
