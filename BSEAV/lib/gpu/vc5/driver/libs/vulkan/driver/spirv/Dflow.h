/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "DriverLimits.h"
#include "BasicBlock.h"

#include "glsl_dataflow.h"
#include "glsl_dataflow_image.h"

#include <type_traits>

namespace bvk {

class DflowScalars;

//////////////////////////////////////////////////////////////////////
// Dflow
//
// Wrapper over a Dataflow
// Provides helper methods for constructing Dataflow of various types
//////////////////////////////////////////////////////////////////////

class Dflow
{
   friend class DflowScalars;

private:
   // Constructs a dataflow wrapper -- the pointer is not owned
   Dflow(Dataflow *dflow) :
      m_dflow(dflow)
   {}

public:
   Dflow() :
      m_dflow(nullptr)
   {}

   operator Dataflow *() const { return m_dflow; }
   bool IsNull() const { return m_dflow == nullptr; }

   // The template here prevents any implicit casting to uint32_t at the call-sites
   template <typename T>
   static inline Dflow Value(DataflowType type, T value)
   {
      static_assert(std::is_same<T, uint32_t>::value, "Value only accepts uint32_t");
      return Dflow(glsl_dataflow_construct_const_value(type, value));
   }

   static inline Dflow Bool(bool value)
   {
      return Dflow(glsl_dataflow_construct_const_bool(value));
   }

   static inline Dflow Int(int32_t i)
   {
      return Dflow(glsl_dataflow_construct_const_int(i));
   }

   static inline Dflow UInt(uint32_t u)
   {
      return Dflow(glsl_dataflow_construct_const_uint(u));
   }

   static Dflow Default(DataflowType dfType);

   template <typename I>
   static inline Dflow TypedInt(const Dflow &proto, I val)
   {
      assert(proto.GetType() == DF_INT || proto.GetType() == DF_UINT);
      return Dflow(glsl_dataflow_construct_const_value(proto.GetType(), val));
   }

   static inline Dflow Float(float f)
   {
      return Dflow(glsl_dataflow_construct_const_float(f));
   }

   static inline Dflow Reinterpret(DataflowType type, const Dflow &operand)
   {
      return Dflow(glsl_dataflow_construct_reinterp(operand.m_dflow, type));
   }

   static inline Dflow In(DataflowType type, uint32_t row)
   {
      return glsl_dataflow_construct_linkable_value(DATAFLOW_IN, type, row);
   }

   static inline Dflow InLoad(const Dflow &in)
   {
      DataflowType t = in.GetType();
      Dataflow *ret = glsl_dataflow_construct_address(in);
      ret = glsl_dataflow_construct_vec4(ret, NULL, NULL, NULL);
      return glsl_dataflow_construct_address_load(DATAFLOW_IN_LOAD, t, ret);
   }

   static inline Dflow Sampler(uint32_t descTableIndex)
   {
      return Dflow(glsl_dataflow_construct_linkable_value(DATAFLOW_CONST_SAMPLER, DF_SAMPLER, descTableIndex));
   }

   static inline Dflow ImageUniform(DataflowType type, bool relaxed, uint32_t descTableIndex)
   {
      assert(glsl_dataflow_type_is_sampled_image(type) || glsl_dataflow_type_is_storage_image(type));
      return Dflow(glsl_dataflow_construct_const_image(type, descTableIndex, !relaxed));
   }

   static inline Dflow Buffer(DataflowFlavour bufFlavour, uint32_t id, uint32_t offset)
   {
      return glsl_dataflow_construct_buffer(bufFlavour, DF_UINT, id, offset);
   }

   // Get dataflow for the start address of the buffer plus offset
   static inline Dflow BufferAddress(const Dflow &buf)
   {
      return Dflow(glsl_dataflow_construct_address(buf));
   }

   static Dflow CreateImageWriteAddress(const Dflow &image, const DflowScalars &coord);

   // Get dataflow for the size of the buffer (minus subtractOffset)
   static inline Dflow BufferSize(const Dflow &buf, uint32_t subtractOffset)
   {
      return glsl_dataflow_construct_buf_size(buf, subtractOffset);
   }

   static inline Dflow BufferArrayLength(const Dflow &buf, uint32_t subtractOffset)
   {
      return glsl_dataflow_construct_buf_array_length(buf, subtractOffset);
   }

   static inline Dflow NullaryOp(DataflowFlavour flavour)
   {
      return Dflow(glsl_dataflow_construct_nullary_op(flavour));
   }

   static inline Dflow UnaryOp(DataflowFlavour flavour, const Dflow &operand)
   {
      return Dflow(glsl_dataflow_construct_unary_op(flavour, operand.m_dflow));
   }

   static inline Dflow BinaryOp(DataflowFlavour flavour, const Dflow &operand1, const Dflow &operand2)
   {
      return Dflow(glsl_dataflow_construct_binary_op(flavour, operand1.m_dflow, operand2.m_dflow));
   }

   static inline Dflow CondOp(const Dflow &cond, const Dflow &t, const Dflow &f)
   {
      return Dflow(glsl_dataflow_construct_ternary_op(DATAFLOW_CONDITIONAL, cond.m_dflow, t.m_dflow, f.m_dflow));
   }

   static Dflow Vec4(const DflowScalars &coord);

   static inline Dflow Load(DataflowType type)
   {
      return Dflow(glsl_dataflow_construct_load(type));
   }

   static inline Dflow TextureSize(const Dflow &image)
   {
      // Returns a vec4 with x,y,z,0 sizes
      return Dflow(glsl_dataflow_construct_texture_size(image));
   }

   static inline Dflow TextureNumLevels(const Dflow &image)
   {
      return Dflow(glsl_dataflow_construct_texture_num_levels(image));
   }

   static Dflow PackImageData(FormatQualifier f, const DflowScalars &data);

   static Dflow Atomic(DataflowFlavour flavour, DataflowType type, const Dflow &addr,
                       const Dflow &data, BasicBlockHandle block);

   template <typename T> Dflow Equals(T n) const;

   DataflowType    GetType()        const { return m_dflow->type; }
   DataflowFlavour GetFlavour()     const { return m_dflow->flavour; }
   uint32_t        GetConstantInt() const { return m_dflow->u.constant.value; }

   Dflow As(DataflowType type) const
   {
      if (GetType() == type)
         return *this;

      return Dflow(glsl_dataflow_construct_reinterp(m_dflow, type));
   }

   bool IsSigned() const
   {
      assert(GetType() == DF_INT || GetType() == DF_UINT);
      return GetType() == DF_INT;
   }

   bool IsUnsigned() const
   {
      assert(GetType() == DF_INT || GetType() == DF_UINT);
      return GetType() == DF_UINT;
   }

private:
   Dataflow *m_dflow;
};

inline Dflow operator*(const Dflow &lhs, const Dflow &rhs)  { return Dflow::BinaryOp(DATAFLOW_MUL, lhs, rhs);    }
inline Dflow operator/(const Dflow &lhs, const Dflow &rhs)  { return Dflow::BinaryOp(DATAFLOW_DIV, lhs, rhs);    }
inline Dflow operator+(const Dflow &lhs, const Dflow &rhs)  { return Dflow::BinaryOp(DATAFLOW_ADD, lhs, rhs);    }
inline Dflow operator-(const Dflow &lhs, const Dflow &rhs)  { return Dflow::BinaryOp(DATAFLOW_SUB, lhs, rhs);    }

inline Dflow operator-(const Dflow &arg)                    { return Dflow::UnaryOp(DATAFLOW_ARITH_NEGATE, arg); }
inline Dflow operator!(const Dflow &arg)                    { return Dflow::UnaryOp(DATAFLOW_LOGICAL_NOT,  arg); }

inline Dflow operator&(const Dflow &lhs, const Dflow &rhs)  { return Dflow::BinaryOp(DATAFLOW_BITWISE_AND, lhs, rhs); }
inline Dflow operator|(const Dflow &lhs, const Dflow &rhs)  { return Dflow::BinaryOp(DATAFLOW_BITWISE_OR,  lhs, rhs); }
inline Dflow operator^(const Dflow &lhs, const Dflow &rhs)  { return Dflow::BinaryOp(DATAFLOW_BITWISE_XOR, lhs, rhs); }

inline Dflow operator&&(const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_LOGICAL_AND, lhs, rhs); }
inline Dflow operator||(const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_LOGICAL_OR,  lhs, rhs); }
inline Dflow operator<<(const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_SHL,         lhs, rhs); }
inline Dflow operator>>(const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_SHR,         lhs, rhs); }

inline Dflow operator==(const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_EQUAL,              lhs, rhs); }
inline Dflow operator!=(const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_NOT_EQUAL,          lhs, rhs); }
inline Dflow operator< (const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_LESS_THAN,          lhs, rhs); }
inline Dflow operator> (const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_GREATER_THAN,       lhs, rhs); }
inline Dflow operator<=(const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_LESS_THAN_EQUAL,    lhs, rhs); }
inline Dflow operator>=(const Dflow &lhs, const Dflow &rhs) { return Dflow::BinaryOp(DATAFLOW_GREATER_THAN_EQUAL, lhs, rhs); }

inline Dflow operator+(const Dflow &lhs, uint32_t rhs)      { return lhs + Dflow::TypedInt(lhs, rhs); }
inline Dflow operator-(const Dflow &lhs, uint32_t rhs)      { return lhs - Dflow::TypedInt(lhs, rhs); }
inline Dflow operator*(const Dflow &lhs, uint32_t rhs)      { return lhs * Dflow::TypedInt(lhs, rhs); }
inline Dflow operator/(const Dflow &lhs, uint32_t rhs)      { return lhs / Dflow::TypedInt(lhs, rhs); }

inline Dflow operator+(const Dflow &lhs, int32_t rhs)       { return lhs + Dflow::TypedInt(lhs, rhs); }
inline Dflow operator-(const Dflow &lhs, int32_t rhs)       { return lhs - Dflow::TypedInt(lhs, rhs); }
inline Dflow operator*(const Dflow &lhs, int32_t rhs)       { return lhs * Dflow::TypedInt(lhs, rhs); }
inline Dflow operator/(const Dflow &lhs, int32_t rhs)       { return lhs / Dflow::TypedInt(lhs, rhs); }

inline Dflow operator+(const Dflow &lhs, float rhs)         { return lhs + Dflow::Float(rhs); }
inline Dflow operator-(const Dflow &lhs, float rhs)         { return lhs - Dflow::Float(rhs); }
inline Dflow operator*(const Dflow &lhs, float rhs)         { return lhs * Dflow::Float(rhs); }
inline Dflow operator/(const Dflow &lhs, float rhs)         { return lhs / Dflow::Float(rhs); }

inline Dflow operator+(uint32_t lhs, const Dflow &rhs)      { return Dflow::TypedInt(rhs, lhs) + rhs; }
inline Dflow operator-(uint32_t lhs, const Dflow &rhs)      { return Dflow::TypedInt(rhs, lhs) - rhs; }
inline Dflow operator*(uint32_t lhs, const Dflow &rhs)      { return Dflow::TypedInt(rhs, lhs) * rhs; }
inline Dflow operator/(uint32_t lhs, const Dflow &rhs)      { return Dflow::TypedInt(rhs, lhs) / rhs; }

inline Dflow operator+(int32_t lhs, const Dflow &rhs)       { return Dflow::TypedInt(rhs, lhs) + rhs; }
inline Dflow operator-(int32_t lhs, const Dflow &rhs)       { return Dflow::TypedInt(rhs, lhs) - rhs; }
inline Dflow operator*(int32_t lhs, const Dflow &rhs)       { return Dflow::TypedInt(rhs, lhs) * rhs; }
inline Dflow operator/(int32_t lhs, const Dflow &rhs)       { return Dflow::TypedInt(rhs, lhs) / rhs; }

inline Dflow operator+(float lhs, const Dflow &rhs)         { return Dflow::Float(lhs) + rhs; }
inline Dflow operator-(float lhs, const Dflow &rhs)         { return Dflow::Float(lhs) - rhs; }
inline Dflow operator*(float lhs, const Dflow &rhs)         { return Dflow::Float(lhs) * rhs; }
inline Dflow operator/(float lhs, const Dflow &rhs)         { return Dflow::Float(lhs) / rhs; }

inline Dflow operator&(const Dflow &lhs, uint32_t rhs)      { assert(lhs.GetType() == DF_UINT); return lhs & Dflow::UInt(rhs); }
inline Dflow operator&(const Dflow &lhs, int32_t rhs)       { assert(lhs.GetType() == DF_INT);  return lhs & Dflow::Int(rhs);  }
inline Dflow operator|(const Dflow &lhs, uint32_t rhs)      { assert(lhs.GetType() == DF_UINT); return lhs | Dflow::UInt(rhs); }
inline Dflow operator|(const Dflow &lhs, int32_t rhs)       { assert(lhs.GetType() == DF_INT);  return lhs | Dflow::Int(rhs);  }
inline Dflow operator^(const Dflow &lhs, uint32_t rhs)      { assert(lhs.GetType() == DF_UINT); return lhs ^ Dflow::UInt(rhs); }
inline Dflow operator^(const Dflow &lhs, int32_t rhs)       { assert(lhs.GetType() == DF_INT);  return lhs ^ Dflow::Int(rhs);  }

template <typename T> Dflow Dflow::Equals(T n) const
{
   switch (m_dflow->type)
   {
   case DF_BOOL  : return *this == Dflow::Bool(static_cast<bool>(n != (T)0));
   case DF_INT   : return *this == Dflow::Int(static_cast<int32_t>(n));
   case DF_UINT  : return *this == Dflow::UInt(static_cast<uint32_t>(n));
   case DF_FLOAT : return *this == Dflow::Float(static_cast<float>(n));
   default       : assert(0); return *this;
   }
}

} // namespace bvk
