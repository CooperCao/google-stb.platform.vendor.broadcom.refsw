/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "DflowScalars.h"
#include "DflowBuilder.h"

#include "glsl_dataflow_print.h"
#include "libs/util/log/log.h"

LOG_DEFAULT_CAT("bvk::comp::DflowScalars");

namespace bvk {

// Copy constructor
DflowScalars::DflowScalars(const DflowScalars &rhs) :
   m_allocator(rhs.m_allocator),
   m_scalars(NewArray(rhs.m_size)),
   m_size(rhs.m_size)
{
   std::copy(rhs.begin(), rhs.end(), begin());
}

// Move constructor
DflowScalars::DflowScalars(DflowScalars &&rhs) :
   m_allocator(rhs.m_allocator),
   m_scalars(rhs.m_scalars),
   m_size(rhs.m_size)
{
   rhs.m_scalars = nullptr;
   rhs.m_size    = 0;
}

// Constructor -- creates size null entries
DflowScalars::DflowScalars(const Allocator &allocator, uint32_t size) :
   m_allocator(&allocator),
   m_scalars(NewArray(size)),
   m_size(size)
{
}

DflowScalars::DflowScalars(const Allocator &allocator, uint32_t size, const Dflow &dflow) :
   m_allocator(&allocator),
   m_scalars(NewArray(size)),
   m_size(size)
{
   std::fill(begin(), end(), dflow);
}

// Constructor from Dataflow**
DflowScalars::DflowScalars(const Allocator &allocator, uint32_t size, Dataflow **scalars) :
   m_allocator(&allocator),
   m_scalars(NewArray(size)),
   m_size(size)
{
   for (uint32_t i = 0; i < size; ++i)
      m_scalars[i] = scalars[i];
}

// Constructor for single element
DflowScalars::DflowScalars(const Allocator &allocator, const Dflow &dflow) :
   m_allocator(&allocator),
   m_scalars(NewArray(1)),
   m_size(1)
{
   m_scalars[0] = dflow;
}

// Constructor from initializer list
DflowScalars::DflowScalars(const Allocator &allocator, std::initializer_list<Dflow> dflows) :
   m_allocator(&allocator),
   m_scalars(NewArray(dflows.size())),
   m_size(dflows.size())
{
   std::copy(dflows.begin(), dflows.end(), begin());
}

// Copy assignment
DflowScalars &DflowScalars::operator=(const DflowScalars &rhs)
{
   if (&rhs != this)
   {
      m_allocator = rhs.m_allocator;
      m_size      = rhs.m_size;
      m_scalars   = NewArray(rhs.m_size);

      std::copy(rhs.begin(), rhs.end(), begin());
   }

   return *this;
}

// Move assignment
DflowScalars &DflowScalars::operator=(DflowScalars &&rhs)
{
   if (&rhs != this)
   {
      m_allocator = rhs.m_allocator;
      m_size      = rhs.m_size;
      m_scalars   = rhs.m_scalars;

      rhs.m_scalars = nullptr;
      rhs.m_size    = 0;
   }

   return *this;
}

DflowScalars DflowScalars::Default(const Allocator &allocator, SymbolTypeHandle type)
{
   return ForeachIndex(allocator, type.GetNumScalars(),
      [&type](uint32_t i) { return Dflow::Default(type.ToDataflowType(i)); });
}

DflowScalars DflowScalars::Load(const Allocator &allocator, SymbolHandle symbol)
{
   SymbolTypeHandle type = symbol.GetType();

   return ForeachIndex(allocator, type.GetNumScalars(),
      [&type](uint32_t i) { return Dflow::Load(type.ToDataflowType(i)); });
}

Dflow DflowScalars::OffsetBufferAddress(const Dflow &buf, uint32_t numElems, Dflow offset, bool robust)
{
   if (robust)
   {
      Dflow maxDynOff = Dflow::BufferSize(buf, /*subtract=*/numElems * sizeof(float));
      offset = Dflow::BinaryOp(DATAFLOW_MIN, offset, maxDynOff);
   }

   return Dflow::BufferAddress(buf) + offset;
}

void DflowScalars::CreateBufferVecLoadDataflow(
   DflowScalars &result, uint32_t index, const Dflow &buf,
   const Dflow &offset, SymbolTypeHandle type, uint32_t numElems, bool robust)
{
   // Get the address for the load
   Dflow addr = OffsetBufferAddress(buf, numElems, offset, robust);

   Dataflow *vec = glsl_dataflow_construct_vector_load(addr);

   // Access required components one by one
   for (uint32_t e = 0; e < numElems; e++)
   {
      DataflowType dfType = type.ToDataflowType(index + e);
      result[index + e] = glsl_dataflow_construct_get_vec4_component(e, vec, dfType);
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
            *newLoad = glsl_dataflow_construct_vector_load(df->d.dependencies[0]);
            *oldLoad = df;
         }

         result = glsl_dataflow_construct_get_vec4_component(dflow->u.get_vec4_component.component_index, *newLoad, dflow->type);
      }
   }

   return result;
}

void DflowScalars::CreateBufferVecStoreDataflow(
   BufferStoreData *data, const Dflow &buf, const Dflow &dynOffset,
   uint32_t numElems, bool robust)
{
   // Get the address for the store
   Dflow addr = OffsetBufferAddress(buf, numElems, dynOffset, robust);

   // Build the vector to store
   DflowScalars vec(data->GetData().GetAllocator(), 4);
   for (uint32_t i = 0; i < numElems; i++)
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
   const Allocator &allocator, SymbolTypeHandle type,
   const spv::vector<MemAccess> &accesses,
   const Dflow &buf, const Dflow &dynOffset, bool robust)
{
   DflowScalars result(allocator, type.GetNumScalars());

   uint32_t index = 0u;
   for (const MemAccess &access : accesses)
   {
      CreateBufferVecLoadDataflow(result, index, buf, dynOffset + Dflow::UInt(access.GetOffset()), type, access.GetSize(), robust);
      index += access.GetSize();
   }

   return result;
}

void DflowScalars::StoreToBufferAddress(
   BasicBlockHandle block, const spv::vector<MemAccess> &accesses,
   const Dflow &buf, const Dflow &dynOffset,
   bool robust) const
{
   BufferStoreData storeData(block, *this);

   for (const MemAccess &access : accesses)
      CreateBufferVecStoreDataflow(&storeData, buf, dynOffset + Dflow::UInt(access.GetOffset()), access.GetSize(), robust);
}

DflowScalars DflowScalars::AtomicAtAddress(
   DataflowFlavour op, BasicBlockHandle block,
   const DflowScalars &data, const Dflow &addr,
   SymbolTypeHandle type)
{
   // Build the vector to store
   const DflowScalars::Allocator &alloc = data.GetAllocator();

   DflowScalars vec = data;
   if (op == DATAFLOW_ATOMIC_CMPXCHG)
      std::swap(vec[0], vec[1]);

   DataflowType dfType = op == DATAFLOW_ADDRESS_STORE ? DF_VOID : type.ToDataflowType(0);

   return DflowScalars(alloc, Dflow::Atomic(op, dfType, addr, Dflow::Vec4(vec), block));
}

DflowScalars DflowScalars::AtomicAtBufferAddress(
   DataflowFlavour op, BasicBlockHandle block,
   const DflowScalars &data, SymbolTypeHandle type,
   const Dflow &buf, const Dflow &offset,
   bool robust)
{
   Dflow addr = OffsetBufferAddress(buf, 1, offset, robust);
   return AtomicAtAddress(op, block, data, addr, type);
}

DflowScalars DflowScalars::LoadFromPushConstants(
   const Allocator &allocator, SymbolTypeHandle type,
   const spv::vector<MemAccess> &accesses, uint32_t constOffset)
{
   DflowScalars   result(allocator, type.GetNumScalars());
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
