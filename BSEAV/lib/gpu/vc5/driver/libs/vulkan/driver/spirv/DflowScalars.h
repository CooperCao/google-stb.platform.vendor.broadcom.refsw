/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vector>
#include <algorithm>

#include "Dflow.h"
#include "PoolAllocator.h"
#include "SymbolTypeHandle.h"
#include "BasicBlock.h"

namespace bvk {

class BufferStoreData;

// Because we rudely pun an array of Dflows to an array of Dataflow *
static_assert(sizeof(Dflow) == sizeof(Dataflow *), "Dflow is not the same size as a pointer");

// Put this here for now -- might not be the best place for it
class MemAccess
{
public:
   MemAccess(uint32_t offset, uint32_t size) :
      m_offset(offset),
      m_size(size)
   {
      assert(size <= 4);
   }

   uint32_t GetOffset() const { return m_offset; }
   uint32_t GetSize()   const { return m_size;   }

private:
   uint32_t m_offset;
   uint32_t m_size;
};

////////////////////////////////////////////////////////////////////
// DflowScalars
//
// Wraps an array of Dataflow pointers.
// We do not own the array or the pointers - they will be allocated
// from a pool and deleted after compilation has completed.
// Copying/assigning does a shallow copy, so passing these
// around is relatively lightweight.
////////////////////////////////////////////////////////////////////
class DflowScalars
{
public:
   // TODO (maybe) make the allocators more flexible so we can use
   // different allocators in different contexts e.g. use a temporary
   // allocator for intermediate results
   using Allocator = SpvAllocator;

   DflowScalars() :
      m_allocator(nullptr),
      m_scalars(nullptr),
      m_size(0)
   {}

   DflowScalars(const Allocator &allocator) :
      m_allocator(&allocator),
      m_scalars(nullptr),
      m_size(0)
   {}

   DflowScalars(const DflowScalars &rhs);
   DflowScalars(DflowScalars &&rhs);

   DflowScalars(const Allocator &allocator, uint32_t size);
   DflowScalars(const Allocator &allocator, uint32_t size, const Dflow &dflow);
   DflowScalars(const Allocator &allocator, const Dflow &dflow);
   DflowScalars(const Allocator &allocator, uint32_t size, Dataflow **scalars);
   DflowScalars(const Allocator &allocator, std::initializer_list<Dflow> dflow);

   DflowScalars &operator=(const DflowScalars &rhs);
   DflowScalars &operator=(DflowScalars &&rhs);

   const Dflow  &operator[](uint32_t i) const { assert(i < m_size); return m_scalars[i]; }
   Dflow        &operator[](uint32_t i)       { assert(i < m_size); return m_scalars[i]; }

   Dflow       *begin()       { return m_scalars;          }
   Dflow       *end()         { return m_scalars + m_size; }
   const Dflow *begin() const { return m_scalars;          }
   const Dflow *end()   const { return m_scalars + m_size; }

   bool IsNull() const { return m_scalars == nullptr; }

   Dataflow **Data() const { return reinterpret_cast<Dataflow **>(m_scalars); }

   // Creates a DflowScalars objects of size, using fn :: uint32_t -> Dflow
   template <typename FN>
   static DflowScalars ForeachIndex(const Allocator &allocator, uint32_t size, const FN &fn)
   {
      DflowScalars   ret(allocator, size);

      for (uint32_t i = 0; i < size; ++i)
         ret.m_scalars[i] = fn(i);

      return ret;
   }

   // Fold elements together with fn :: (Dflow, Dflow) -> Dflow
   template <typename FN>
   Dflow Fold(const FN &fn) const
   {
      Dflow res = m_scalars[0];

      for (uint32_t i = 1; i < m_size; ++i)
         res = fn(res, m_scalars[i]);

      return res;
   }

   static DflowScalars Default(const Allocator &allocator, SymbolTypeHandle type);

   static DflowScalars Value(const Allocator &allocator, const SymbolTypeHandle type, uint32_t value)
   {
      return DflowScalars(allocator, type.GetNumScalars(), Dflow::Value(type.ToDataflowType(0), value));
   }

   static DflowScalars Int(const Allocator &allocator, int32_t value, uint32_t count = 1)
   {
      return DflowScalars(allocator, count, Dflow::Int(value));
   }

   static DflowScalars UInt(const Allocator &allocator, uint32_t value, uint32_t count = 1)
   {
      return DflowScalars(allocator, count, Dflow::UInt(value));
   }

   static DflowScalars Bool(const Allocator &allocator, bool value, uint32_t count = 1)
   {
      return DflowScalars(allocator, count, Dflow::Bool(value));
   }

   static DflowScalars Float(const Allocator &allocator, float value, uint32_t count = 1)
   {
      return DflowScalars(allocator, count, Dflow::Float(value));
   }

   static DflowScalars In(const Allocator &allocator, const SymbolTypeHandle type, int *ids)
   {
      return ForeachIndex(allocator, type.GetNumScalars(),
         [&type, ids](uint32_t i) { return Dflow::In(type.ToDataflowType(i), ids[i]); });
   }

   static DflowScalars InLoad(const DflowScalars &scalars)
   {
      return ForeachIndex(scalars.GetAllocator(), scalars.Size(),
         [&scalars](uint32_t i) { return Dflow::InLoad(scalars[i]); });
   }

   static Dflow OffsetBufferAddress(const Dflow &buf, uint32_t numElems, Dflow offset, bool clamp);

   static void CreateBufferVecLoadDataflow(
      DflowScalars &result, uint32_t index, const Dflow &buf,
      const Dflow &offset, SymbolTypeHandle type, uint32_t numElems,
      bool robust);

   static void CreateBufferVecStoreDataflow(
      BufferStoreData *data, const Dflow &buf, const Dflow &dynOffset,
      uint32_t numElems, bool robust);

   static DflowScalars LoadFromBufferAddress(
      const Allocator &allocator, const SymbolTypeHandle type,
      const spv::vector<MemAccess> &accesses,
      const Dflow &buf, const Dflow &dynOffset, bool robust);

   static DflowScalars LoadFromPushConstants(
      const Allocator &allocator, const SymbolTypeHandle type,
      const spv::vector<MemAccess> &accesses, uint32_t constOffset);

   void StoreToBufferAddress(
      BasicBlockHandle block, const spv::vector<MemAccess> &accesses,
      const Dflow &buf, const Dflow &dynOffset, bool robust) const;

   static DflowScalars AtomicAtAddress(
      DataflowFlavour op, BasicBlockHandle block,
      const DflowScalars &data, const Dflow &addr,
      SymbolTypeHandle type);

   static DflowScalars AtomicAtBufferAddress(
      DataflowFlavour op, BasicBlockHandle block,
      const DflowScalars &data, SymbolTypeHandle type,
      const Dflow &buf, const Dflow &dynOffset,
      bool robust);

   static DflowScalars NullaryOp(const Allocator &allocator, DataflowFlavour flavour)
   {
      return DflowScalars(allocator, Dflow::NullaryOp(flavour));
   }

   static DflowScalars NullaryOp(const Allocator &allocator, std::initializer_list<DataflowFlavour> flavours)
   {
      auto fIter = flavours.begin();

      return ForeachIndex(allocator, flavours.size(),
         [&fIter](uint32_t i) { return Dflow::NullaryOp(*fIter++); } );
   }

   static DflowScalars UnaryOp(DataflowFlavour f, const DflowScalars &op)
   {
      return ForeachIndex(op.GetAllocator(), op.Size(),
         [f, &op](uint32_t i) { return Dflow::UnaryOp(f, op[i]); });
   }

   DflowScalars As(DataflowType type) const
   {
      return ForeachIndex(*m_allocator, Size(),
         [this, type](uint32_t i) { return m_scalars[i].As(type); } );
   }

   DflowScalars Signed() const
   {
      return As(DF_INT);
   }

   DflowScalars Unsigned() const
   {
      return As(DF_UINT);
   }

   static DflowScalars BinaryOp(DataflowFlavour flavour,
                                const DflowScalars &lhs, const DflowScalars &rhs)
   {
      if (lhs.Size() == rhs.Size())
         return ForeachIndex(lhs.GetAllocator(), lhs.Size(),
            [flavour, &lhs, &rhs](uint32_t i) { return Dflow::BinaryOp(flavour, lhs[i], rhs[i]); });

      else if (lhs.Size() == 1)
         return ForeachIndex(lhs.GetAllocator(), rhs.Size(),
            [flavour, &lhs, &rhs](uint32_t i) { return Dflow::BinaryOp(flavour, lhs[0], rhs[i]); });

      else if (rhs.Size() == 1)
         return ForeachIndex(lhs.GetAllocator(), lhs.Size(),
            [flavour, &lhs, &rhs](uint32_t i) { return Dflow::BinaryOp(flavour, lhs[i], rhs[0]); });

      unreachable();
   }

   static DflowScalars TypedBinaryOp(DataflowType type, DataflowFlavour flavour,
      const DflowScalars &lhs, const DflowScalars &rhs)
   {
      return BinaryOp(flavour, lhs.As(type), rhs.As(type));
   }

   static DflowScalars CondOp(const DflowScalars &cond, const DflowScalars &t, const DflowScalars &f)
   {
      uint32_t tsize = t.Size();
      uint32_t fsize = f.Size();
      uint32_t csize = cond.Size();
      uint32_t size  = std::max(std::max(csize, tsize), fsize);

      return ForeachIndex(cond.GetAllocator(), size,
         [&](uint32_t i)
         {
            return Dflow::CondOp(cond[i >= csize ? 0 : i], t[i >= tsize ? 0 : i], f[i >= fsize ? 0 : i]);
         });
   }

   static DflowScalars CondOp(const Dflow &cond, const DflowScalars &t, const DflowScalars &f)
   {
      assert(t.Size() == f.Size());

      return ForeachIndex(t.GetAllocator(), t.Size(),
         [&t, &f, &cond](uint32_t i) { return Dflow::CondOp(cond, t[i], f[i]); });

   }

   static DflowScalars Join(const DflowScalars &op1, const DflowScalars &op2)
   {
      uint32_t size1  = op1.Size();

      return ForeachIndex(op1.GetAllocator(), op1.Size() + op2.Size(),
         [&op1, &op2, size1](uint32_t i) { return i < size1 ? op1[i] : op2[i - size1]; });
   }

   static DflowScalars Texture(const Allocator &allocator, const DflowScalars &coords, const Dflow &depthRef,
                               const Dflow &bias, const Dflow &offset, const Dflow &image, const Dflow &sampler,
                               uint32_t bits)
   {
      bool scalar = (depthRef.m_dflow && !(bits & DF_TEXBITS_GATHER));
      uint32_t numResultScalars = scalar ? 1 : 4;

#if V3D_VER_AT_LEAST(4,2,13,0)
      bool q = (bits & DF_TEXBITS_LOD_QUERY);
      if (q) numResultScalars = 2;
#endif

      DflowScalars result = DflowScalars(allocator, numResultScalars);

      glsl_dataflow_construct_texture_lookup(result.Data(), numResultScalars, bits,
                                             image.m_dflow, Dflow::Vec4(coords).m_dflow, depthRef.m_dflow,
                                             bias.m_dflow,  offset.m_dflow, sampler.m_dflow);

      return result;
   }

   static DflowScalars TextureSize(const Allocator &allocator, uint32_t numScalars, const Dflow &image)
   {
      Dflow tSize  = Dflow::TextureSize(image);

      return ForeachIndex(allocator, numScalars,
         [&tSize](uint32_t i) { return glsl_dataflow_construct_get_vec4_component(i, tSize, DF_INT); } );
   }

   static DflowScalars TextureNumLevels(const Allocator &allocator, const Dflow &sampler)
   {
      return DflowScalars(allocator, Dflow::TextureNumLevels(sampler));
   }

   static DflowScalars Load(const Allocator &allocator, SymbolHandle symbol);

   DflowScalars Slice(uint32_t offset, uint32_t numScalars) const
   {
      assert(offset < m_size);
      assert(offset + numScalars <= m_size);

      return ForeachIndex(*m_allocator, numScalars,
         [this, offset](uint32_t i) { return m_scalars[i + offset]; } );
   }

   // Easier single slicers
   DflowScalars X() const   { return Slice(0, 1); }
   DflowScalars Y() const   { return Slice(1, 1); }
   DflowScalars Z() const   { return Slice(2, 1); }
   DflowScalars W() const   { return Slice(3, 1); }
   DflowScalars XY() const  { return Slice(0, 2); }
   DflowScalars XYZ() const { return Slice(0, 3); }
   DflowScalars ZW() const  { return Slice(2, 2); }

   const Dflow &x() const   { return m_scalars[0]; }
   const Dflow &y() const   { return m_scalars[1]; }
   const Dflow &z() const   { return m_scalars[2]; }
   const Dflow &w() const   { return m_scalars[3]; }

   uint32_t Size()  const   { return m_size; }

   DflowScalars Swizzle(uint32_t x, uint32_t y) const
   {
      return DflowScalars(*m_allocator, { m_scalars[x], m_scalars[y] });
   }

   DflowScalars Swizzle(uint32_t x, uint32_t y, uint32_t z) const
   {
      return DflowScalars(*m_allocator, { m_scalars[x], m_scalars[y], m_scalars[z] });
   }

   DflowScalars Swizzle(uint32_t x, uint32_t y, uint32_t z, uint32_t w) const
   {
      return DflowScalars(*m_allocator, { m_scalars[x], m_scalars[y], m_scalars[z], m_scalars[w] });
   }

   void SetSlice(uint32_t offset, const DflowScalars &sliceData) const
   {
      assert(offset < m_size);
      assert(offset + sliceData.Size() <= m_size);

      for (uint32_t i = 0; i < sliceData.Size(); ++i)
          m_scalars[i + offset] = sliceData[i];
   }

   void DebugPrint() const;

   const Allocator &GetAllocator() const { return *m_allocator; }

private:
   Dflow *NewArray(uint32_t numElems) const
   {
      return m_allocator->NewArray<Dflow>(numElems);
   }

private:
   const Allocator *m_allocator;  // Can be null
   Dflow           *m_scalars;
   uint32_t         m_size;
};

static inline DflowScalars operator*(const DflowScalars &lhs, const DflowScalars &rhs)  { return DflowScalars::BinaryOp(DATAFLOW_MUL, lhs, rhs); }
static inline DflowScalars operator/(const DflowScalars &lhs, const DflowScalars &rhs)  { return DflowScalars::BinaryOp(DATAFLOW_DIV, lhs, rhs); }
static inline DflowScalars operator+(const DflowScalars &lhs, const DflowScalars &rhs)  { return DflowScalars::BinaryOp(DATAFLOW_ADD, lhs, rhs); }
static inline DflowScalars operator-(const DflowScalars &lhs, const DflowScalars &rhs)  { return DflowScalars::BinaryOp(DATAFLOW_SUB, lhs, rhs); }
static inline DflowScalars operator-(const DflowScalars &arg)                           { return DflowScalars::UnaryOp(DATAFLOW_ARITH_NEGATE, arg); }
static inline DflowScalars operator!(const DflowScalars &arg)                           { return DflowScalars::UnaryOp(DATAFLOW_LOGICAL_NOT, arg); }
static inline DflowScalars operator~(const DflowScalars &arg)                           { return DflowScalars::UnaryOp(DATAFLOW_BITWISE_NOT, arg); }
static inline DflowScalars operator&(const DflowScalars &lhs, const DflowScalars &rhs)  { return DflowScalars::BinaryOp(DATAFLOW_BITWISE_AND, lhs, rhs); }
static inline DflowScalars operator|(const DflowScalars &lhs, const DflowScalars &rhs)  { return DflowScalars::BinaryOp(DATAFLOW_BITWISE_OR, lhs, rhs); }
static inline DflowScalars operator^(const DflowScalars &lhs, const DflowScalars &rhs)  { return DflowScalars::BinaryOp(DATAFLOW_BITWISE_XOR, lhs, rhs); }
static inline DflowScalars operator&&(const DflowScalars &lhs, const DflowScalars &rhs) { return DflowScalars::BinaryOp(DATAFLOW_LOGICAL_AND, lhs, rhs); }
static inline DflowScalars operator||(const DflowScalars &lhs, const DflowScalars &rhs) { return DflowScalars::BinaryOp(DATAFLOW_LOGICAL_OR, lhs, rhs); }
static inline DflowScalars operator<<(const DflowScalars &lhs, const DflowScalars &rhs) { return DflowScalars::BinaryOp(DATAFLOW_SHL, lhs, rhs); }
static inline DflowScalars operator>>(const DflowScalars &lhs, const DflowScalars &rhs) { return DflowScalars::BinaryOp(DATAFLOW_SHR, lhs, rhs); }
static inline DflowScalars operator==(const DflowScalars &lhs, const DflowScalars &rhs) { return DflowScalars::BinaryOp(DATAFLOW_EQUAL, lhs, rhs); }
static inline DflowScalars operator!=(const DflowScalars &lhs, const DflowScalars &rhs) { return DflowScalars::BinaryOp(DATAFLOW_NOT_EQUAL, lhs, rhs); }
static inline DflowScalars operator<(const DflowScalars &lhs, const DflowScalars &rhs)  { return DflowScalars::BinaryOp(DATAFLOW_LESS_THAN, lhs, rhs); }
static inline DflowScalars operator>(const DflowScalars &lhs, const DflowScalars &rhs)  { return DflowScalars::BinaryOp(DATAFLOW_GREATER_THAN, lhs, rhs); }
static inline DflowScalars operator<=(const DflowScalars &lhs, const DflowScalars &rhs) { return DflowScalars::BinaryOp(DATAFLOW_LESS_THAN_EQUAL, lhs, rhs); }
static inline DflowScalars operator>=(const DflowScalars &lhs, const DflowScalars &rhs) { return DflowScalars::BinaryOp(DATAFLOW_GREATER_THAN_EQUAL, lhs, rhs); }

static inline DflowScalars operator*(float lhs, const DflowScalars &rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_MUL, DflowScalars::Float(rhs.GetAllocator(), lhs), rhs);
}
static inline DflowScalars operator*(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_MUL, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}

static inline DflowScalars operator/(float lhs, const DflowScalars &rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_DIV, DflowScalars::Float(rhs.GetAllocator(), lhs), rhs);
}
static inline DflowScalars operator/(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_DIV, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}

static inline DflowScalars operator+(float lhs, const DflowScalars &rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_ADD, DflowScalars::Float(rhs.GetAllocator(), lhs), rhs);
}
static inline DflowScalars operator+(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_ADD, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}

static inline DflowScalars operator-(float lhs, const DflowScalars &rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_SUB, DflowScalars::Float(rhs.GetAllocator(), lhs), rhs);
}
static inline DflowScalars operator-(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_SUB, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}

static inline DflowScalars operator==(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_EQUAL, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}
static inline DflowScalars operator==(const DflowScalars &lhs, int rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_EQUAL, lhs.As(DF_INT), DflowScalars::Int(lhs.GetAllocator(), rhs));
}

static inline DflowScalars operator!=(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_NOT_EQUAL, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}
static inline DflowScalars operator<(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_LESS_THAN, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}

static inline DflowScalars operator>(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_GREATER_THAN, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}

static inline DflowScalars operator<=(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_LESS_THAN_EQUAL, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}

static inline DflowScalars operator>=(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_GREATER_THAN_EQUAL, lhs, DflowScalars::Float(lhs.GetAllocator(), rhs));
}

class BufferStoreData
{
public:
   BufferStoreData(BasicBlockHandle blk, const DflowScalars &data) :
      m_block(blk),
      m_srcData(data)
   {}

   BasicBlockHandle    GetBlock() const { return m_block;   }
   const DflowScalars &GetData()  const { return m_srcData; }

   Dflow Pop() { return m_srcData[m_index++]; }

private:
   BasicBlockHandle    m_block;
   const DflowScalars &m_srcData;
   uint32_t            m_index = 0;
};

} // namespace bvk
