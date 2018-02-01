/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vector>
#include <algorithm>

#include "Dflow.h"
#include "ModuleAllocator.h"
#include "SymbolTypeHandle.h"
#include "BasicBlock.h"

namespace bvk {

class DflowBuilder;

class BufferLoadData;
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

   MemAccess Add(uint32_t offset) const
   {
      return MemAccess(m_offset + offset, m_size);
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
   enum BufferType
   {
      UBO,
      SSBO
   };

   DflowScalars() :
      m_builder(nullptr),
      m_scalars(nullptr),
      m_size(0)
   {}

   DflowScalars(const DflowBuilder &builder) :
      m_builder(&builder),
      m_scalars(nullptr),
      m_size(0)
   {}

   DflowScalars(const DflowScalars &rhs);
   DflowScalars(DflowScalars &&rhs);

   DflowScalars(const DflowBuilder &builder, uint32_t size);
   DflowScalars(const DflowBuilder &builder, const Dflow &dflow);
   DflowScalars(const DflowBuilder &builder, uint32_t size, Dataflow **scalars);
   DflowScalars(const DflowBuilder &builder, std::initializer_list<Dflow> dflow);

   DflowScalars &operator=(const DflowScalars &rhs);
   DflowScalars &operator=(DflowScalars &&rhs);

   const Dflow  &operator[](uint32_t i) const { assert(i < m_size); return m_scalars[i]; }
   Dflow        &operator[](uint32_t i)       { assert(i < m_size); return m_scalars[i]; }

   bool IsNull() const { return m_scalars == nullptr; }

   Dataflow **Data() const { return reinterpret_cast<Dataflow **>(m_scalars); }

   static DflowScalars Initialize(const SymbolTypeHandle type, const DflowScalars &init);
   static DflowScalars Default(const DflowBuilder &builder, SymbolTypeHandle type);

   void Clear()
   {
      for (uint32_t i = 0; i < Size(); ++i)
         m_scalars[i] = nullptr;
   }

   static DflowScalars ConstantValue(const DflowBuilder &builder, const SymbolTypeHandle type, uint32_t value)
   {
      DflowScalars   result(builder, type.GetNumScalars());
      for (uint32_t i = 0; i < result.Size(); ++i)
         result[i] = Dflow::ConstantValue(type.ToDataflowType(i), value);
      return result;
   }

   static DflowScalars ConstantInt(const DflowBuilder &builder, int32_t value, uint32_t count = 1)
   {
      DflowScalars   result(builder, count);
      for (uint32_t i = 0; i < count; ++i)
         result[i] = Dflow::ConstantInt(value);
      return result;
   }

   static DflowScalars ConstantUInt(const DflowBuilder &builder, uint32_t value, uint32_t count = 1)
   {
      DflowScalars   result(builder, count);
      for (uint32_t i = 0; i < count; ++i)
         result[i] = Dflow::ConstantUInt(value);
      return result;
   }

   static DflowScalars ConstantBool(const DflowBuilder &builder, bool value, uint32_t count = 1)
   {
      DflowScalars   result(builder, count);
      for (uint32_t i = 0; i < count; ++i)
         result[i] = Dflow::ConstantBool(value);
      return result;
   }

   static DflowScalars ConstantFloat(const DflowBuilder &builder, float value, uint32_t count = 1)
   {
      DflowScalars   result(builder, count);
      for (uint32_t i = 0; i < count; ++i)
         result[i] = Dflow::ConstantFloat(value);
      return result;
   }

   static DflowScalars In(const DflowBuilder &builder, const SymbolTypeHandle type,
                          uint32_t startRow)
   {
      DflowScalars   result(builder, type.GetNumScalars());

      for (uint32_t i = 0; i < result.Size(); ++i)
         result.m_scalars[i] = Dflow::In(type.ToDataflowType(i), startRow++);

      return result;
   }

   static DflowScalars InLoad(const DflowScalars &scalars)
   {
      DflowScalars s(scalars.GetBuilder(), scalars.Size());

      for (unsigned i = 0; i < s.Size(); i++)
         s[i] = Dflow::InLoad(scalars[i]);

      return s;
   }

   static DflowScalars ImageSampler(DflowBuilder &builder, const SymbolTypeHandle type,
                                    uint32_t descriptorSet, uint32_t binding, int *ids);

   static Dflow CreateBufferAddress(
      uint32_t numElems, uint32_t offset, Dflow dynOff,
      uint32_t descTableIndex, BufferType bufType, bool robust);

   static DflowScalars CreateImageVecLoadDataflow(const DflowScalars &coord, uint32_t descTableIndex,
                                                  bool robust);

   static void CreateBufferVecLoadDataflow(
      BufferLoadData *result, SymbolTypeHandle type,
      const MemAccess &access, const Dflow &dynOffset,
      uint32_t descTableIndex, BufferType bufType,
      bool robust);

   static void CreateBufferVecStoreDataflow(
      BufferStoreData *data, const MemAccess &access,
      const Dflow &dynOffset, uint32_t descTableIndex,
      bool robust);

   static DflowScalars LoadFromBufferAddress(
      const DflowBuilder &builder, const SymbolTypeHandle type,
      const spv::vector<MemAccess> &accesses,
      const Dflow &dynOffset, uint32_t descTableIndex,
      BufferType bufType);

   static DflowScalars LoadFromPushConstants(
      const DflowBuilder &builder, const SymbolTypeHandle type,
      const spv::vector<MemAccess> &accesses, uint32_t constOffset);

   void StoreToBufferAddress(
      const DflowBuilder &builder, BasicBlockHandle block,
      SymbolTypeHandle type, const spv::vector<MemAccess> &accesses,
      const Dflow &dynOffset, uint32_t descTableIndex) const;

   static DflowScalars AtomicAtAddress(
      DataflowFlavour op, BasicBlockHandle block,
      const DflowScalars &data, const Dflow &addr,
      SymbolTypeHandle type);

   static DflowScalars AtomicAtBufferAddress(
      DataflowFlavour op, BasicBlockHandle block,
      const DflowScalars &data, SymbolTypeHandle type,
      const MemAccess &access,
      const Dflow &dynOffset, uint32_t descTableIndex);

   static DflowScalars NullaryOp(const DflowBuilder &builder, DataflowFlavour flavour)
   {
      auto result = DflowScalars(builder, 1);
      result[0] = Dflow::NullaryOp(flavour);
      return result;
   }

   static DflowScalars NullaryOp(const DflowBuilder &builder, std::initializer_list<DataflowFlavour> flavours)
   {
      auto result = DflowScalars(builder, flavours.size());

      uint32_t i = 0;
      for (auto f : flavours)
      {
         result[i] = Dflow::NullaryOp(f);
         ++i;
      }

      return result;
   }

   static DflowScalars UnaryOp(DataflowFlavour flavour, const DflowScalars &operand)
   {
      auto result = DflowScalars(*operand.m_builder, operand.Size());

      for (uint32_t i = 0; i < operand.Size(); ++i)
         result[i] = Dflow::UnaryOp(flavour, operand[i]);

      return result;
   }

   static DflowScalars Reinterpret(DataflowType newType, const DflowScalars &operand)
   {
      auto result = DflowScalars(*operand.m_builder, operand.Size());

      for (uint32_t i = 0; i < operand.Size(); ++i)
         result[i] = operand[i].As(newType);

      return result;
   }

   DflowScalars As(DataflowType type) const
   {
      return Reinterpret(type, *this);
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
      {
         auto result = DflowScalars(lhs.GetBuilder(), lhs.Size());
         for (uint32_t i = 0; i < lhs.Size(); ++i)
            result[i] = Dflow::BinaryOp(flavour, lhs[i], rhs[i]);
         return result;
      }
      else if (lhs.Size() == 1)
      {
         auto result = DflowScalars(lhs.GetBuilder(), rhs.Size());
         for (uint32_t i = 0; i < rhs.Size(); ++i)
            result[i] = Dflow::BinaryOp(flavour, lhs[0], rhs[i]);
         return result;
      }
      else if (rhs.Size() == 1)
      {
         auto result = DflowScalars(lhs.GetBuilder(), lhs.Size());
         for (uint32_t i = 0; i < lhs.Size(); ++i)
            result[i] = Dflow::BinaryOp(flavour, lhs[i], rhs[0]);
         return result;
      }

      unreachable();
   }

   static DflowScalars TypedBinaryOp(DataflowType type, DataflowFlavour flavour,
      const DflowScalars &lhs, const DflowScalars &rhs)
   {
      return BinaryOp(flavour, Reinterpret(type, lhs), Reinterpret(type, rhs));
   }

   static DflowScalars CondOp(const DflowScalars &cond,
                              const DflowScalars &t, const DflowScalars &f)
   {
      uint32_t tsize = t.Size();
      uint32_t fsize = f.Size();
      uint32_t csize = cond.Size();

      uint32_t size  = std::max(std::max(csize, tsize), fsize);

      auto result = DflowScalars(*cond.m_builder, size);

      for (uint32_t i = 0; i < size; ++i)
         result[i] = Dflow::CondOp(cond[i >= csize ? 0 : i], t[i >= tsize ? 0 : i], f[i >= fsize ? 0 : i]);

      return result;
   }

   static DflowScalars Join(const DflowScalars &op1, const DflowScalars &op2)
   {
      auto     result = DflowScalars(*op1.m_builder, op1.Size() + op2.Size());
      uint32_t size1  = op1.Size();

      for (uint32_t i = 0; i < result.Size(); ++i)
         result[i] = i < size1 ? op1[i] : op2[i - size1];

      return result;
   }

   static DflowScalars Zip(const DflowScalars &op1, const DflowScalars &op2)
   {
      assert(op1.Size() == op2.Size());
      auto     result = DflowScalars(*op1.m_builder, op1.Size() * 2);

      uint32_t j = 0;
      for (uint32_t i = 0; i < op1.Size(); ++i)
      {
         result[j++] = op1[i];
         result[j++] = op2[i];
      }

      return result;
   }

   static DflowScalars IsNaN(const DflowScalars &x)
   {
      return DflowScalars::UnaryOp(DATAFLOW_ISNAN, x);
   }

   static DflowScalars Texture(const DflowBuilder &builder, const DflowScalars &coords, const Dflow &depthRef,
                               const Dflow &bias, const Dflow &offset, const Dflow &image, const Dflow &sampler,
                               uint32_t bits)
   {
      bool scalar = (depthRef.m_dflow && !(bits & DF_TEXBITS_GATHER));
      uint32_t numResultScalars = scalar ? 1 : 4;

#if V3D_VER_AT_LEAST(4,2,13,0)
      bool q = (bits & DF_TEXBITS_LOD_QUERY);
      if (q) numResultScalars = 2;
#endif

      DflowScalars result = DflowScalars(builder, numResultScalars);

      // required_components gets filled in the later compiler stages, when the usage of the texture is known
      glsl_dataflow_construct_texture_lookup(result.Data(), numResultScalars, bits,
                                             image.m_dflow, Dflow::Vec4(coords).m_dflow, depthRef.m_dflow,
                                             bias.m_dflow,  offset.m_dflow, sampler.m_dflow);

      return result;
   }

   static DflowScalars TextureSize(const DflowBuilder &builder, uint32_t numScalars, const Dflow &image)
   {
      DflowScalars result = DflowScalars(builder, numScalars);
      Dflow        tSize  = Dflow::TextureSize(image);

      for (uint32_t i = 0; i < numScalars; i++)
         result[i] = glsl_dataflow_construct_get_vec4_component(i, tSize, DF_INT);

      return result;
   }

   static DflowScalars TextureNumLevels(const DflowBuilder &builder, const Dflow &sampler)
   {
      DflowScalars result = DflowScalars(builder, 1);
      result[0] = Dflow::TextureNumLevels(sampler);
      return result;
   }

   static DflowScalars Load(DflowBuilder &builder, SymbolHandle symbol);

   DflowScalars Slice(uint32_t offset, uint32_t numScalars) const
   {
      assert(offset < m_size);
      assert(offset + numScalars <= m_size);

      auto result = DflowScalars(*m_builder, numScalars);

      for (uint32_t i = 0; i < numScalars; ++i)
         result[i] = m_scalars[i + offset];

      return result;
   }

   // Easier single slicers
   DflowScalars X() const   { return Slice(0, 1); }
   DflowScalars Y() const   { return Slice(1, 1); }
   DflowScalars Z() const   { return Slice(2, 1); }
   DflowScalars W() const   { return Slice(3, 1); }
   DflowScalars XY() const  { return Slice(0, 2); }
   DflowScalars XYZ() const { return Slice(0, 3); }
   DflowScalars ZW() const  { return Slice(2, 2); }

   DflowScalars Swizzle(uint32_t x, uint32_t y) const
   {
      DflowScalars result(*m_builder, 2);

      result[0] = m_scalars[x];
      result[1] = m_scalars[y];

      return result;
   }

   DflowScalars Swizzle(uint32_t x, uint32_t y, uint32_t z) const
   {
      DflowScalars result(*m_builder, 3);

      result[0] = m_scalars[x];
      result[1] = m_scalars[y];
      result[2] = m_scalars[z];

      return result;
   }

   DflowScalars Swizzle(uint32_t x, uint32_t y, uint32_t z, uint32_t w) const
   {
      DflowScalars result(*m_builder, 4);

      result[0] = m_scalars[x];
      result[1] = m_scalars[y];
      result[2] = m_scalars[z];
      result[3] = m_scalars[w];

      return result;
   }

   void SetSlice(uint32_t offset, const DflowScalars &sliceData) const
   {
      assert(offset < m_size);
      assert(offset + sliceData.Size() <= m_size);

      for (uint32_t i = 0; i < sliceData.Size(); ++i)
          m_scalars[i + offset] = sliceData[i];
   }

   // Easier single setters
   void SetX(const DflowScalars &v)   const { assert(v.Size() == 1); SetSlice(0, v); }
   void SetY(const DflowScalars &v)   const { assert(v.Size() == 1); SetSlice(1, v); }
   void SetZ(const DflowScalars &v)   const { assert(v.Size() == 1); SetSlice(2, v); }
   void SetW(const DflowScalars &v)   const { assert(v.Size() == 1); SetSlice(3, v); }
   void SetXY(const DflowScalars &v)  const { assert(v.Size() == 2); SetSlice(0, v); }
   void SetXYZ(const DflowScalars &v) const { assert(v.Size() == 3); SetSlice(0, v); }

   void Set4(const DflowScalars &x, const DflowScalars &y, const DflowScalars &z, const DflowScalars &w) const
   {
      assert(x.Size() == 1);
      assert(y.Size() == 1);
      assert(z.Size() == 1);
      assert(w.Size() == 1);

      SetX(x);
      SetY(y);
      SetZ(z);
      SetW(w);
   }

   uint32_t Size() const
   {
      return m_size;
   }

   void DebugPrint() const;

   const DflowBuilder &GetBuilder() const { return *m_builder; }

private:
   const DflowBuilder  *m_builder;  // Can be null
   Dflow               *m_scalars;
   uint32_t             m_size;
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
   return DflowScalars::BinaryOp(DATAFLOW_MUL, DflowScalars::ConstantFloat(rhs.GetBuilder(), lhs), rhs);
}
static inline DflowScalars operator*(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_MUL, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator/(float lhs, const DflowScalars &rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_DIV, DflowScalars::ConstantFloat(rhs.GetBuilder(), lhs), rhs);
}
static inline DflowScalars operator/(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_DIV, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator+(float lhs, const DflowScalars &rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_ADD, DflowScalars::ConstantFloat(rhs.GetBuilder(), lhs), rhs);
}
static inline DflowScalars operator+(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_ADD, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator-(float lhs, const DflowScalars &rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_SUB, DflowScalars::ConstantFloat(rhs.GetBuilder(), lhs), rhs);
}
static inline DflowScalars operator-(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_SUB, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator==(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_EQUAL, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator==(const DflowScalars &lhs, int rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_EQUAL, lhs.As(DF_INT), DflowScalars::ConstantInt(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator!=(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_NOT_EQUAL, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator<(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_LESS_THAN, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator>(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_GREATER_THAN, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator<=(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_LESS_THAN_EQUAL, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

static inline DflowScalars operator>=(const DflowScalars &lhs, float rhs)
{
   return DflowScalars::BinaryOp(DATAFLOW_GREATER_THAN_EQUAL, lhs, DflowScalars::ConstantFloat(lhs.GetBuilder(), rhs));
}

// BufferLoadData
//
// Results from buffer loads are accumulated into
// this buffer.
class BufferLoadData
{
public:
   BufferLoadData(const DflowBuilder &builder, uint32_t numScalars) :
      m_result(builder, numScalars)
   {}

   const DflowScalars &GetResult() const { return m_result; }

   void     Push(const Dflow &dflow) { m_result[m_index++] = dflow; }
   uint32_t Index() const            { return m_index;              }

private:
   DflowScalars m_result;
   uint32_t     m_index = 0;
};

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
