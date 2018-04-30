/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Pointer.h"
#include "SymbolHandle.h"
#include "DflowBuilder.h"
#include "Composite.h"
#include "TextureLookup.h"
#include "Module.h"

extern "C"
{
// Import SPIR-V interface to glsl_dataflow_builder
extern void glsl_buffer_load_calculate_dataflow(
   BasicBlock *ctx, Dataflow **result,
   Dataflow *address, Dataflow *offset,
   MemLayout *layout, SymbolType *type,
   StorageQualifier sq);

extern void glsl_buffer_store_calculate_dataflow(
   BasicBlock *ctx, Dataflow **result,
   Dataflow *address, Dataflow *offset,
   MemLayout *layout, SymbolType *type,
   StorageQualifier sq);
}

namespace bvk {

// Flattens access chains
//
// Returns the pointer to the base of the composite (this will usually be a variable)
// Puts the indices into the accessChain list
static const Node *UnrollAccessChains(const Node *pointer, spv::vector<const Node *> *accessChain)
{
   assert(accessChain->size() == 0);

   const Node *result = pointer;

   spv::Core opCode = pointer->GetOpCode();

   if (opCode == spv::Core::OpAccessChain)
   {
      auto chain = pointer->As<const NodeAccessChain *>();

      result = UnrollAccessChains(chain->GetBase(), accessChain);

      for (const NodeConstPtr &ni : chain->GetIndices())
         accessChain->push_back(ni);
   }

   return result;
}

static const NodeType *TypeOf(const Node *pointer)
{
   auto pointerType = pointer->GetResultType()->As<const NodeTypePointer *>();
   return pointerType->GetType()->As<const NodeType *>();
}

class SharedAccess
{
public:
   SharedAccess() = delete;
   SharedAccess(const SharedAccess &) = delete;
   SharedAccess(DflowBuilder &builder, SymbolHandle symbol, const spv::vector<const Node *> &accessChain);

   const Dflow       &GetOffset() const { return m_offset; }
   MemLayout        *GetLayout()  const { return m_layout; }
   SymbolTypeHandle  GetType()    const { return m_type;   }

private:
   Dflow             m_offset;
   MemLayout        *m_layout = nullptr;
   SymbolTypeHandle  m_type;
};

SharedAccess::SharedAccess(DflowBuilder &builder, SymbolHandle symbol, const spv::vector<const Node *> &accessChain)
{
   Dflow offset = Dflow::UInt(0);
   MemLayout *layout = symbol.GetMemLayout();
   SymbolTypeHandle type = symbol.GetType();

   for (uint32_t i = 0; i < accessChain.size(); ++i)
   {
      const DflowScalars   &index = builder.GetDataflow(accessChain[i]);
      Dflow index0 = index[0].As(DF_UINT);

      switch (layout->flavour)
      {
      case LAYOUT_PRIM_NONMATRIX:
      {
         offset = offset + index0 * layout->u.prim_nonmatrix_layout.stride;
         layout = glsl_mem_prim_nonmatrix_layout(4);
         type   = type.IndexType();
         break;
      }

      case LAYOUT_PRIM_MATRIX :
      {
         // We chose the layout for shared memory and its stride so make some assumptions here.
         assert(!layout->u.prim_matrix_layout.row_major);
         offset = offset + index0 * layout->u.prim_matrix_layout.stride;
         layout = glsl_mem_prim_nonmatrix_layout(4);
         type   = type.MatrixSubscriptVector(1);
         break;
      }

      case LAYOUT_ARRAY :
      {
         offset = offset + index0 * layout->u.array_layout.stride;
         layout = layout->u.array_layout.member_layout;
         type   = type.GetElementType();
         break;
      }

      case LAYOUT_STRUCT :
      {
         uint32_t constIndex = index0.GetConstantInt(); // Must be a constant
         offset = offset + layout->u.struct_layout.member_offsets[constIndex];
         layout = &layout->u.struct_layout.member_layouts[constIndex];
         type   = type.GetStructMemberType(constIndex);
         break;
      }

      default :
         assert(0);
         break;
      }
   }

   m_offset = offset;
   m_layout = layout;
   m_type   = type;
}

//////////////////////////////////////////////////////////////////
// LoadScalars
//////////////////////////////////////////////////////////////////

// INTERFACE
DflowScalars LoadScalars::Load(DflowBuilder &builder, BasicBlockHandle block, const Node *pointer)
{
   auto pointerType = pointer->GetResultType()->As<const NodeTypePointer *>();
   auto type        = pointerType->GetType()->As<const NodeType *>();

   LoadScalars visitor(builder, block, type);

   pointer->Accept(visitor);

   return visitor.m_result;
}

// IMPLEMENTATION
LoadScalars::LoadScalars(DflowBuilder &builder, BasicBlockHandle block, const NodeType *resultType) :
   m_builder(builder),
   m_block(block),
   m_resultType(resultType),
   m_accessChain(builder.GetArenaAllocator()),
   m_result(builder.GetArenaAllocator())
{
}

DflowScalars LoadScalars::SelectScalarsForChain(const DflowScalars &scalars, const NodeType *resType,
                                                const NodeType *rootType) const
{
   if (m_builder.IsConstant(m_accessChain))
   {
      uint32_t offset     = ScalarOffsetStatic::Calculate(m_builder, rootType, m_accessChain);
      uint32_t numScalars = m_builder.GetNumScalars(resType);

      // Slice out the bit we want
      return scalars.Slice(offset, numScalars);
   }
   else
      return ExtractDynamicScalars::Calculate(m_builder, scalars, rootType, m_accessChain);
}

// Returns an enclosing type only if it is required for special indexing code
const NodeType *LoadScalars::GetEnclosingType(const NodeType *rootType)
{
   if (m_accessChain.size() == 0)
      return nullptr;

   // Remove the last link of the access chain and find the type. Restore the chain afterwards
   const Node *index = m_accessChain.back();
   m_accessChain.pop_back();
   const NodeType *enclosingType = TypeFind::FindType(m_builder, rootType, m_accessChain);
   SymbolTypeHandle sType = m_builder.GetSymbolType(enclosingType);

   bool ret = sType.GetFlavour() == SYMBOL_ARRAY_TYPE &&
              sType.GetElementType().GetFlavour() == SYMBOL_PRIMITIVE_TYPE &&
              glsl_prim_is_prim_comb_sampler_type(sType.GetElementType());

   m_accessChain.push_back(index);
   return ret ? enclosingType : nullptr;
}

void LoadScalars::LoadVariable(const DflowScalars &scalars, const NodeType *rootType)
{
   const NodeType *enclosingType = GetEnclosingType(rootType);
   if (enclosingType)
   {
      const Node *index = m_accessChain.back();
      m_accessChain.pop_back();
      SymbolTypeHandle sType = m_builder.GetSymbolType(enclosingType);
      DflowScalars arrScalars = SelectScalarsForChain(scalars, enclosingType, rootType);

      uint32_t count = sType.GetElementCount();
      uint32_t ix = 0;
      if (m_builder.ConstantInt(index, &ix))
         m_result = arrScalars.Swizzle(ix, count + ix);
      else
      {
         m_result = arrScalars.Swizzle(0, count);
         Dflow indx = m_builder.GetDataflow(index)[0];

         for (uint32_t i = 1; i < count; ++i)
         {
            Dflow choose = indx.Equals(i);

            m_result[0] = Dflow::CondOp(choose, arrScalars[i],         m_result[0]);
            m_result[1] = Dflow::CondOp(choose, arrScalars[count + i], m_result[1]);
         }
      }
   }
   else
      m_result = SelectScalarsForChain(scalars, m_resultType, rootType);
}

void LoadScalars::LoadFromMemory(const NodeVariable *var, const NodeType *rootType)
{
   bool ssbo = IsSSBO::Test(var);

   DecorationQuery   query(var);

   uint32_t descriptorSet = query.RequireLiteral(spv::Decoration::DescriptorSet);
   uint32_t binding       = query.RequireLiteral(spv::Decoration::Binding);

   uint32_t    arrayElement = BlockIndex::Get(m_builder, rootType, m_accessChain);
   LayoutInfo  layoutInfo   = MemoryLayout::Calculate(m_builder, rootType, m_accessChain);

   // Calculate a list of offsets for each loadable item in result
   spv::vector<MemAccess> accesses = MemoryOffsetsOfScalars::Calculate(m_builder, m_resultType,
                                                                       layoutInfo);

   // Find or create a new table entry for the (set, binding, element) tuple
   DescriptorInfo dInfo(descriptorSet, binding, arrayElement);
   uint32_t descTableIndex = m_builder.GetDescriptorMaps().FindBuffer(ssbo, dInfo);

   DataflowFlavour bufType = ssbo ? DATAFLOW_STORAGE_BUFFER : DATAFLOW_UNIFORM_BUFFER;
   Dflow buf = Dflow::Buffer(bufType, descTableIndex, 0);
   Dflow dynOffset = MemoryOffset::Calculate(m_builder, rootType, m_accessChain, buf);

   // Calculate the types of all the scalars in the composite
   SymbolTypeHandle type = m_builder.GetSymbolType(m_resultType);
   m_result = DflowScalars::LoadFromBufferAddress(m_builder.GetArenaAllocator(), type, accesses,
                                                  buf, dynOffset, m_builder.RobustBufferAccess());
}

void LoadScalars::LoadFromWorkgroup(const NodeVariable *var, const NodeType *rootType)
{
   SymbolHandle symbol = m_builder.GetVariableSymbol(var);
   Dflow        addr   = m_builder.WorkgroupAddress();
   SharedAccess access(m_builder, symbol, m_accessChain);

   m_result = DflowScalars(m_builder.GetArenaAllocator(), access.GetType().GetNumScalars());

   glsl_buffer_load_calculate_dataflow(m_block->GetBlock(), m_result.Data(),
                                       addr, access.GetOffset(),
                                       access.GetLayout(), access.GetType(), STORAGE_SHARED);
}

void LoadScalars::LoadPushConstant(const NodeVariable *var, const NodeType *rootType)
{
   // Calculate the types of all the scalars in the composite
   SymbolTypeHandle type = m_builder.GetSymbolType(m_resultType);

   LayoutInfo  layoutInfo = MemoryLayout::Calculate(m_builder, rootType, m_accessChain);

   spv::vector<MemAccess> accesses = MemoryOffsetsOfScalars::Calculate(m_builder, m_resultType,
                                                                       layoutInfo);

   Dflow buf = Dflow::Buffer(DATAFLOW_UNIFORM_BUFFER, DriverLimits::ePushConstantIdentifier, 0);
   Dflow offset = MemoryOffset::Calculate(m_builder, rootType, m_accessChain, buf);

   if (offset.GetFlavour() == DATAFLOW_CONST)
   {
      uint32_t constOffset = offset.GetConstantInt();
      assert((constOffset & 3) == 0);

      m_result = DflowScalars::LoadFromPushConstants(m_builder.GetArenaAllocator(), type, accesses, constOffset);
   }
   else
   {
      m_result = DflowScalars::LoadFromBufferAddress(m_builder.GetArenaAllocator(), type, accesses, buf, offset,
                                                     m_builder.RobustBufferAccess());
   }
}

void LoadScalars::Visit(const NodeVariable *var)
{
   const NodeType *rootType = TypeOf(var);

   switch (var->GetStorageClass())
   {
   case spv::StorageClass::Function:
   case spv::StorageClass::Output:
   case spv::StorageClass::Private:
   case spv::StorageClass::UniformConstant:
      {
         SymbolHandle symbol = m_builder.GetVariableSymbol(var);
         LoadVariable(m_builder.LoadFromSymbol(m_block, symbol), rootType);
      }
      break;
   case spv::StorageClass::Input:
      {
         SymbolHandle symbol = m_builder.GetVariableSymbol(var);

         if (!strncmp(symbol.GetName(), "gl_", 3))
         {
            DflowScalars scalars = m_builder.LoadFromSymbol(m_block, symbol);
            LoadVariable(scalars, rootType);
         }
         else
         {
            DflowScalars scalars = InputLoader::Load(m_builder, var, m_block);
            LoadVariable(DflowScalars::InLoad(scalars), rootType);
         }
      }
      break;

   case spv::StorageClass::Uniform:
   case spv::StorageClass::StorageBuffer:
      LoadFromMemory(var, rootType);
      break;

   case spv::StorageClass::PushConstant:
      LoadPushConstant(var, rootType);
      break;

   case spv::StorageClass::Workgroup:
      LoadFromWorkgroup(var, rootType);
      break;

   case spv::StorageClass::CrossWorkgroup:
   case spv::StorageClass::Generic:
   case spv::StorageClass::AtomicCounter:
   case spv::StorageClass::Image:
   default:
      // Not handled or invalid
      assert(0);
      break;
   }
}

void LoadScalars::Visit(const NodeFunctionParameter *node)
{
   auto pointerType = node->GetResultType()->As<const NodeTypePointer *>();
   assert(pointerType != nullptr);

   SymbolHandle symbol = m_builder.GetParameterSymbol(node);

   LoadVariable(m_builder.LoadFromSymbol(m_block, symbol), TypeOf(node));
}

void LoadScalars::Visit(const NodeAccessChain *node)
{
   const Node *basePointer = UnrollAccessChains(node, &m_accessChain);

   basePointer->Accept(*this);
}

void LoadScalars::Visit(const NodeImageTexelPointer *node)
{
   // Not allowed
   assert(0);
}

void LoadScalars::Visit(const NodeCopyObject *node)
{
   // Not implemented yet
   assert(0);
}

//////////////////////////////////////////////////////////////////
// StoreScalars
//////////////////////////////////////////////////////////////////

// INTERFACE
void StoreScalars::Store(DflowBuilder &builder, BasicBlockHandle block, const Node *pointer,
                         const DflowScalars &data)
{
   StoreScalars   visitor(builder, block, TypeOf(pointer), data);

   pointer->Accept(visitor);
}

void StoreScalars::Store(DflowBuilder &builder, BasicBlockHandle block, const Node *pointer,
                         const Node *dataObject)
{
   const DflowScalars &scalars = builder.GetDataflow(dataObject);

   Store(builder, block, pointer, scalars);
}

// IMPLEMENTATION
StoreScalars::StoreScalars(DflowBuilder &builder, BasicBlockHandle block, const NodeType *storeType,
                           const DflowScalars &scalars) :
   m_builder(builder),
   m_block(block),
   m_storeType(storeType),
   m_scalars(scalars),
   m_accessChain(builder.GetArenaAllocator())
{
}

void StoreScalars::StoreSymbol(SymbolHandle symbol, const Node *pointer) const
{
   auto pointerType = pointer->GetResultType()->As<const NodeTypePointer *>();
   auto rootType    = pointerType->GetType()->As<const NodeType *>();

   if (m_builder.IsConstant(m_accessChain))
   {
      uint32_t offset = ScalarOffsetStatic::Calculate(m_builder, rootType, m_accessChain);
      m_builder.StoreToSymbol(m_block, symbol, m_scalars, offset);
   }
   else
   {
      DflowScalars oldScalars = m_builder.LoadFromSymbol(m_block, symbol);
      DflowScalars result = InsertDynamicScalars::Calculate(m_builder, oldScalars, rootType,
                                                            m_accessChain, m_scalars);
      m_builder.StoreToSymbol(m_block, symbol, result);
   }
}

void StoreScalars::StoreToMemory(const NodeVariable *var) const
{
   const NodeType *rootType = TypeOf(var);

   bool ssbo = IsSSBO::Test(var);

   DecorationQuery   query(var);

   uint32_t descriptorSet = query.RequireLiteral(spv::Decoration::DescriptorSet);
   uint32_t binding       = query.RequireLiteral(spv::Decoration::Binding);

   uint32_t    arrayElement = BlockIndex::Get(m_builder, rootType, m_accessChain);
   LayoutInfo  layoutInfo   = MemoryLayout::Calculate(m_builder, rootType, m_accessChain);

   // Calculate a list of offsets for each loadable item in result
   spv::vector<MemAccess> accesses = MemoryOffsetsOfScalars::Calculate(m_builder, m_storeType,
                                                                       layoutInfo);

   // Find or create a new table entry for the (set, binding, element) tuple
   DescriptorInfo dInfo(descriptorSet, binding, arrayElement);
   uint32_t descTableIndex = m_builder.GetDescriptorMaps().FindBuffer(ssbo, dInfo);

   Dflow buf = Dflow::Buffer(DATAFLOW_STORAGE_BUFFER, descTableIndex, 0);
   Dflow dynOffset = MemoryOffset::Calculate(m_builder, rootType, m_accessChain, buf);
   m_scalars.StoreToBufferAddress(m_block, accesses, buf, dynOffset,
                                  m_builder.RobustBufferAccess());
}

void StoreScalars::StoreToWorkgroup(const NodeVariable *var) const
{
   SymbolHandle symbol = m_builder.GetVariableSymbol(var);
   Dflow        addr   = m_builder.WorkgroupAddress();
   SharedAccess access(m_builder, symbol, m_accessChain);

   glsl_buffer_store_calculate_dataflow(m_block->GetBlock(), m_scalars.Data(),
                                        addr, access.GetOffset(),
                                        access.GetLayout(), access.GetType(), STORAGE_SHARED);
}

void StoreScalars::Visit(const NodeVariable *var)
{
   switch (var->GetStorageClass())
   {
   case spv::StorageClass::Output:
   case spv::StorageClass::Function:
   case spv::StorageClass::Private:
      StoreSymbol(m_builder.GetVariableSymbol(var), var);
      break;

   case spv::StorageClass::Uniform:
   case spv::StorageClass::StorageBuffer:
      StoreToMemory(var);
      break;

   case spv::StorageClass::Workgroup:
      StoreToWorkgroup(var);
      break;

   case spv::StorageClass::UniformConstant:
   case spv::StorageClass::Input:
   case spv::StorageClass::CrossWorkgroup:
   case spv::StorageClass::Generic:
   case spv::StorageClass::PushConstant:
   case spv::StorageClass::AtomicCounter:
   case spv::StorageClass::Image:
   default:
      assert(0); // Not handled or invalid
      break;
   }
}

void StoreScalars::Visit(const NodeAccessChain *node)
{
   const Node *basePointer = UnrollAccessChains(node, &m_accessChain);

   basePointer->Accept(*this);
}

void StoreScalars::Visit(const NodeFunctionParameter *node)
{
   SymbolHandle symbol = m_builder.GetParameterSymbol(node);
   StoreSymbol(symbol, node);
}

void StoreScalars::Visit(const NodeImageTexelPointer *node)
{
   // Not allowed
   assert(0);
}

void StoreScalars::Visit(const NodeCopyObject *node)
{
   assert(0);
}

//////////////////////////////////////////////////////////////////
// AtomicAccess
//////////////////////////////////////////////////////////////////

// INTERFACE
DflowScalars AtomicAccess::Access(DflowBuilder &builder, DataflowFlavour atomicOp, BasicBlockHandle block,
                                  const Node *pointer, const DflowScalars &value, Dflow comp)
{
   AtomicAccess   visitor(builder, atomicOp, block, TypeOf(pointer), value, comp);

   pointer->Accept(visitor);

   return visitor.m_result;
}

// IMPLEMENTATION
AtomicAccess::AtomicAccess(DflowBuilder &builder, DataflowFlavour atomicOp, BasicBlockHandle block,
                           const NodeType *storeType, const DflowScalars &value, Dflow comp) :
   m_builder(builder),
   m_atomicOp(atomicOp),
   m_block(block),
   m_storeType(storeType),
   m_vec4(builder.GetArenaAllocator(), { value[0], comp, Dflow(), Dflow() }),
   m_accessChain(builder.GetArenaAllocator())
{
}

void AtomicAccess::Visit(const NodeVariable *node)
{
   if (node->GetStorageClass() == spv::StorageClass::Workgroup)
      m_result = ReadWriteWorkgroup(node);
   else
      m_result = ReadWriteMemory(node);
}

void AtomicAccess::Visit(const NodeAccessChain *node)
{
   const Node *basePointer = UnrollAccessChains(node, &m_accessChain);

   basePointer->Accept(*this);
}

void AtomicAccess::Visit(const NodeImageTexelPointer *node)
{
   m_result = ReadWriteTexel(node);
}

void AtomicAccess::Visit(const NodeFunctionParameter *node)
{
   assert(0);
}

void AtomicAccess::Visit(const NodeCopyObject *node)
{
   assert(0);
}

DflowScalars AtomicAccess::ReadWriteMemory(const NodeVariable *var) const
{
   // The type of the composite
   const NodeType *rootType = TypeOf(var);

   SymbolTypeHandle type = m_builder.GetSymbolType(m_storeType);

   DecorationQuery   query(var);

   uint32_t descriptorSet = query.RequireLiteral(spv::Decoration::DescriptorSet);
   uint32_t binding       = query.RequireLiteral(spv::Decoration::Binding);

   uint32_t arrayElement  = BlockIndex::Get(m_builder, rootType, m_accessChain);

   // Find or create a new table entry for the (set, binding, element) tuple
   DescriptorInfo dInfo(descriptorSet, binding, arrayElement);
   uint32_t descTableIndex = m_builder.GetDescriptorMaps().FindBuffer(/*ssbo=*/true, dInfo);

   Dflow buf    = Dflow::Buffer(DATAFLOW_STORAGE_BUFFER, descTableIndex, 0);
   Dflow offset = MemoryOffset::Calculate(m_builder, rootType, m_accessChain, buf);
   return DflowScalars::AtomicAtBufferAddress(m_atomicOp, m_block, m_vec4, type,
                                              buf, offset, m_builder.RobustBufferAccess());
}

DflowScalars AtomicAccess::ReadWriteWorkgroup(const NodeVariable *var) const
{
   SymbolHandle symbol = m_builder.GetVariableSymbol(var);
   Dflow        addr   = m_builder.WorkgroupAddress();
   SharedAccess access(m_builder, symbol, m_accessChain);

   return DflowScalars::AtomicAtAddress(m_atomicOp, m_block, m_vec4,
                                        addr + access.GetOffset(), access.GetType());
}

DflowScalars AtomicAccess::ReadWriteTexel(const NodeImageTexelPointer *node) const
{
   const Node *imagePtr = node->GetImage();

   auto imagePtrType = imagePtr->GetResultType()->As<const NodeTypePointer *>();
   auto imageType    = imagePtrType->GetType()->As<const NodeTypeImage *>();

   DflowScalars image = LoadScalars::Load(m_builder, m_block, imagePtr);

   TextureLookup tl(m_builder, nullptr,
                    m_builder.GetDataflow(node->GetCoordinate()), image,
                    imageType,
                    /*dref=*/nullptr, /*project=*/false, /*fetch=*/true);

   return tl.Atomic(m_atomicOp, m_vec4, m_block);
}

} // namespace bvk
