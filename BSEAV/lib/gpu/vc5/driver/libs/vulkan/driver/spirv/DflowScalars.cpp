/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "DflowScalars.h"
#include "DflowBuilder.h"

#include "glsl_dataflow_print.h"
#include "libs/util/log/log.h"

LOG_DEFAULT_CAT("bvk::comp::DflowScalars");

namespace bvk {

static DataflowFlavour BufferFlavour(DflowScalars::BufferType type)
{
   switch (type)
   {
   case DflowScalars::UBO  : return DATAFLOW_UNIFORM_BUFFER;
   case DflowScalars::SSBO : return DATAFLOW_STORAGE_BUFFER;
   default                 : unreachable();
   }
}

DflowScalars::DflowScalars(const DflowScalars &rhs) :
   m_builder(rhs.m_builder),
   m_scalars(m_builder->NewArray<Dflow>(rhs.m_size)),
   m_size(rhs.m_size)
{
   for (uint32_t i = 0; i < m_size; ++i)
      m_scalars[i] = rhs.m_scalars[i];
}

DflowScalars::DflowScalars(DflowScalars &&rhs) :
   m_builder(rhs.m_builder),
   m_scalars(rhs.m_scalars),
   m_size(rhs.m_size)
{
   rhs.m_scalars = nullptr;
   rhs.m_size    = 0;
}

DflowScalars::DflowScalars(const DflowBuilder &builder, uint32_t size) :
   m_builder(&builder),
   m_scalars(builder.NewArray<Dflow>(size)),
   m_size(size)
{
   Clear();
}

DflowScalars::DflowScalars(const DflowBuilder &builder, uint32_t size, Dataflow **scalars) :
   m_builder(&builder),
   m_scalars(builder.NewArray<Dflow>(size)),
   m_size(size)
{
   for (uint32_t i = 0; i < size; ++i)
      m_scalars[i] = scalars[i];
}

DflowScalars::DflowScalars(const DflowBuilder &builder, const Dflow &dflow) :
   m_builder(&builder),
   m_scalars(builder.NewArray<Dflow>(1)),
   m_size(1)
{
   m_scalars[0] = dflow;
}

DflowScalars::DflowScalars(const DflowBuilder &builder, std::initializer_list<Dflow> dflows) :
   m_builder(&builder),
   m_scalars(builder.NewArray<Dflow>(dflows.size())),
   m_size(dflows.size())
{
   uint32_t i = 0;

   for (const Dflow &dflow : dflows)
   {
      m_scalars[i] = dflow;
      ++i;
   }
}

DflowScalars &DflowScalars::operator=(const DflowScalars &rhs)
{
   if (&rhs != this)
   {
      m_builder = rhs.m_builder;
      m_size    = rhs.m_size;
      m_scalars = m_builder->NewArray<Dflow>(rhs.m_size);

      for (uint32_t i = 0; i < m_size; ++i)
         m_scalars[i] = rhs.m_scalars[i];
   }

   return *this;
}

DflowScalars &DflowScalars::operator=(DflowScalars &&rhs)
{
   if (&rhs != this)
   {
      m_builder = rhs.m_builder;
      m_size    = rhs.m_size;
      m_scalars = rhs.m_scalars;

      rhs.m_scalars = nullptr;
      rhs.m_size    = 0;
   }

   return *this;
}


DflowScalars DflowScalars::Default(const DflowBuilder &builder, SymbolTypeHandle type)
{
   DflowScalars   result(builder, type.GetNumScalars());

   for (uint32_t i = 0; i < result.Size(); ++i)
   {
      DataflowType   dfType = type.ToDataflowType(i);

      switch (dfType)
      {
      case DF_BOOL:
         result[i] = Dflow::ConstantBool(false);
         break;
      case DF_INT:
         result[i] = Dflow::ConstantInt(0);
         break;
      case DF_UINT:
         result[i] = Dflow::ConstantUInt(0);
         break;
      case DF_FLOAT:
         result[i] = Dflow::ConstantFloat(0);
         break;

      case DF_SAMPLER:
      case DF_F_SAMP_IMG:
      case DF_I_SAMP_IMG:
      case DF_U_SAMP_IMG:
      case DF_F_STOR_IMG:
      case DF_I_STOR_IMG:
      case DF_U_STOR_IMG:
         result[i] = Dflow::Uniform(dfType, -1);
         break;

      default:
         assert(0);
      }
   }

   return result;
}

DflowScalars DflowScalars::Initialize(SymbolTypeHandle type, const DflowScalars &init)
{
   if (init.m_size == 0)
      return Default(*init.m_builder, type);

   DflowScalars   result(*init.m_builder, type.GetNumScalars());

   for (uint32_t i = 0; i < result.Size(); ++i)
      result[i] = init[i];

   return result;
}

DflowScalars DflowScalars::Load(DflowBuilder &builder, SymbolHandle symbol)
{
   SymbolTypeHandle type   = symbol.GetType();
   DflowScalars     result = DflowScalars(builder, type.GetNumScalars());

   for (uint32_t i = 0; i < result.Size(); ++i)
      result[i] = Dflow::Load(type.ToDataflowType(i));

   return result;
}

Dflow DflowScalars::CreateBufferAddress(
   uint32_t numElems, uint32_t offset, Dflow dynOff,
   uint32_t descTableIndex, BufferType bufType, bool robust)
{
   // Get dataflow for the buffer address + the const offset
   Dflow addr = Dflow::BufferAddress(DF_UINT, offset, descTableIndex, BufferFlavour(bufType));

   // Add any dynamic offset
   if (robust)
   {
      // Clamp the dynamic offset to prevent reading off the end of the buffer.
      // addr will already have been adjusted if the const offset was too large.
      uint32_t bytesToRead = numElems * sizeof(float);
      Dflow    maxDynOff   = Dflow::BufferSize(DF_UINT, /*subtract=*/offset + bytesToRead,
                                              descTableIndex, BufferFlavour(bufType));
      dynOff = Dflow::BinaryOp(DATAFLOW_MIN, dynOff, maxDynOff);
   }

   addr = addr + dynOff;

   return addr;
}

void DflowScalars::CreateBufferVecLoadDataflow(
   BufferLoadData *result, SymbolTypeHandle type,
   const MemAccess &access, const Dflow &dynOffset,
   uint32_t descTableIndex, BufferType bufType, bool robust)
{
   // Get the address for the load
   Dflow addr = CreateBufferAddress(access.GetSize(), access.GetOffset(), dynOffset, descTableIndex, bufType, robust);

   // Load the vector
   DataflowType dfType = type.ToDataflowType(result->Index());
   Dataflow    *vec    = glsl_dataflow_construct_vector_load(dfType, addr);

   // Access required components one by one
   for (uint32_t e = 0; e < access.GetSize(); e++)
   {
      dfType = type.ToDataflowType(result->Index());
      result->Push(glsl_dataflow_construct_get_vec4_component(e, vec, dfType));
   }
}

static bool ClashingStores(Dataflow *df, const BasicBlockHandle &block)
{
   assert(df->flavour == DATAFLOW_VECTOR_LOAD);

   Dataflow *ldAddress = df->d.unary_op.operand;

   // If it's not a raw address then we have no way of knowing if it clashes
   // with anything
   if (ldAddress->flavour != DATAFLOW_ADDRESS)
      return true;

   Dataflow *ldBuffer = ldAddress->d.unary_op.operand;

   for (Dataflow *store = block->GetMemoryAccessChain(); store != nullptr; store = store->d.addr_store.prev)
   {
      Dataflow *stAddress = store->d.addr_store.addr;

      // If it's not a raw address then all bets are off
      if (stAddress->flavour != DATAFLOW_ADDRESS)
         return true;

      Dataflow *stBuffer = stAddress->d.unary_op.operand;

      // If the store is accessing the same buffer then assume a clash
      // This is clearly overcautious and could be relaxed by looking at the
      // regions being affected (give or take a vec4 offset)
      if (stBuffer->u.buffer.index == ldBuffer->u.buffer.index)
         return true;
   }

   return false;
}

// Large data copies don't rescheule well as the back-end has no information
// on potential aliasing between memory blocks
static Dataflow *RetimeLoad(Dataflow **oldLoad, Dataflow **newLoad, Dataflow *dflow, const BasicBlockHandle &block)
{
   Dataflow *result = dflow;

   // Look for special case of a direct store of a load which covers the
   // OpCopyMemory array case, but is not a general solution to reordering
   // memory operations
   if (dflow->flavour == DATAFLOW_GET_VEC4_COMPONENT)
   {
      Dataflow *df = dflow->d.dependencies[0];

      if (df->flavour == DATAFLOW_VECTOR_LOAD && !ClashingStores(df, block))
      {
         if (df != *oldLoad)
         {
            *newLoad = glsl_dataflow_construct_vector_load(df->type, df->d.dependencies[0]);
            *oldLoad = df;
         }

         result = glsl_dataflow_construct_get_vec4_component(dflow->u.get_vec4_component.component_index, *newLoad, dflow->type);
      }
   }

   return result;
}

void DflowScalars::CreateBufferVecStoreDataflow(
   BufferStoreData *data, const MemAccess &access,
   const Dflow &dynOffset, uint32_t descTableIndex,
   bool robust)
{
   // Get the address for the store
   Dflow addr = CreateBufferAddress(access.GetSize(), access.GetOffset(), dynOffset, descTableIndex, SSBO, robust);

   // Build the vector to store
   DflowScalars vec(data->GetData().GetBuilder(), 4);
   for (uint32_t i = 0; i < access.GetSize(); i++)
   {
      Dataflow *dflow    = data->Pop();
      Dataflow *load     = nullptr;
      Dataflow *prevLoad = nullptr;

      // Maybe pull loads forward in time or just return dflow
      vec[i] = RetimeLoad(&load, &prevLoad, dflow, data->GetBlock());
   }

   // Flush out the store
   Dflow::Atomic(DATAFLOW_ADDRESS_STORE, DF_VOID, addr, Dflow::Vec4(vec), data->GetBlock());
}

DflowScalars DflowScalars::LoadFromBufferAddress(
   const DflowBuilder &builder, SymbolTypeHandle type,
   const spv::vector<MemAccess> &accesses,
   const Dflow &dynOffset, uint32_t descTableIndex,
   BufferType bufType)
{
   BufferLoadData loadData(builder, type.GetNumScalars());
   bool           robust = builder.RobustBufferAccess();

   for (const MemAccess &access : accesses)
      CreateBufferVecLoadDataflow(&loadData, type, access,
                                  dynOffset, descTableIndex, bufType, robust);

   return loadData.GetResult();
}

void DflowScalars::StoreToBufferAddress(
   const DflowBuilder &builder, BasicBlockHandle block,
   SymbolTypeHandle type, const spv::vector<MemAccess> &accesses,
   const Dflow &dynOffset, uint32_t descTableIndex) const
{
   BufferStoreData storeData(block, *this);
   bool            robust = builder.RobustBufferAccess();

   for (const MemAccess &access : accesses)
      CreateBufferVecStoreDataflow(&storeData, access,
                                   dynOffset, descTableIndex, robust);
}

DflowScalars DflowScalars::AtomicAtAddress(
   DataflowFlavour op, BasicBlockHandle block,
   const DflowScalars &data, const Dflow &addr,
   SymbolTypeHandle type)
{
   // Build the vector to store
   const DflowBuilder &builder = data.GetBuilder();

   DflowScalars vec(builder, 4);
   if (op == DATAFLOW_ATOMIC_CMPXCHG)
   {
      vec[0] = data[1];
      vec[1] = data[0];
   }
   else
      vec[0] = data[0];

   DataflowType   dfType = op == DATAFLOW_ADDRESS_STORE ? DF_VOID : type.ToDataflowType(0);

   return DflowScalars(builder, Dflow::Atomic(op, dfType, addr, Dflow::Vec4(vec), block));
}

DflowScalars DflowScalars::AtomicAtBufferAddress(
   DataflowFlavour op, BasicBlockHandle block,
   const DflowScalars &data, SymbolTypeHandle type,
   const MemAccess &access,
   const Dflow &dynOffset, uint32_t descTableIndex)
{
   assert(access.GetSize() == 1);

   const DflowBuilder   &builder = data.GetBuilder();
   // Get the address for the store
   Dflow addr = CreateBufferAddress(1, access.GetOffset(), dynOffset, descTableIndex,
                                    SSBO, builder.RobustBufferAccess());

   return AtomicAtAddress(op, block, data, addr, type);
}

DflowScalars DflowScalars::LoadFromPushConstants(
   const DflowBuilder &builder, SymbolTypeHandle type,
   const spv::vector<MemAccess> &accesses, uint32_t constOffset)
{
   DflowScalars   result(builder, type.GetNumScalars());
   uint32_t       index = 0;

   for (const MemAccess &access : accesses)
   {
      uint32_t offset = constOffset + access.GetOffset();

      for (uint32_t i = 0; i < access.GetSize(); i++)
      {
         result[index] = glsl_dataflow_construct_buffer(DATAFLOW_UNIFORM, type.ToDataflowType(index),
                                                        /*index=*/0, offset + i * 4);
         index++;
      }
   }

   return result;
}

DflowScalars DflowScalars::ImageSampler(
   DflowBuilder &builder, const SymbolTypeHandle type,
   uint32_t descriptorSet, uint32_t binding, int *ids)
{
   uint32_t         numScalars = type.GetNumScalars();
   SymbolTypeHandle elemType   = type;
   uint32_t         numElems   = 1;
   uint32_t         elemSize   = numScalars;

   if (type.GetFlavour() == SYMBOL_ARRAY_TYPE)
   {
      elemType = type.GetElementType();
      numElems = type.GetElementCount();
      elemSize = elemType.GetNumScalars();
   }

   assert(elemSize == 1 || elemSize == 2);

   DflowScalars result = DflowScalars(builder, numScalars);

   for (uint32_t i = 0; i < numElems; i++)
   {
      DescriptorInfo descInfo(descriptorSet, binding, i);

      // Combined image sampler?
      if (elemSize == 2)
      {
         uint32_t imgTableIndex = builder.GetDescriptorMaps().FindImageEntry(descInfo);
         result.m_scalars[i] = Dflow::Uniform(type.ToDataflowType(0), imgTableIndex);
         ids[i] = imgTableIndex;

         uint32_t samplerTableIndex = builder.GetDescriptorMaps().FindSamplerEntry(descInfo);
         result.m_scalars[numElems + i] = Dflow::Uniform(DF_SAMPLER, samplerTableIndex);
         ids[numElems + i] = samplerTableIndex;
      }
      else if (elemType.IsSampler())
      {
         uint32_t descTableIndex = builder.GetDescriptorMaps().FindSamplerEntry(descInfo);
         result.m_scalars[i] = Dflow::Uniform(DF_SAMPLER, descTableIndex);
         ids[i] = descTableIndex;
      }
      else
      {
         // Should be an image
         uint32_t descTableIndex = builder.GetDescriptorMaps().FindImageEntry(descInfo);
         result.m_scalars[i] = Dflow::Uniform(type.ToDataflowType(0), descTableIndex);
         ids[i] = descTableIndex;
      }
   }

   return result;
}

void DflowScalars::DebugPrint() const
{
   if (log_trace_enabled())
   {
      for (uint32_t i = 0; i < m_size; i++)
      {
         glsl_print_dataflow(stderr, m_scalars[i]);
         fprintf(stderr, "\n");
      }
   }
}

} // namespace bvk
