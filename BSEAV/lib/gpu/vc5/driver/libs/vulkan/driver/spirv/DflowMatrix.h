/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vector>
#include <cassert>

#include "Dflow.h"
#include "ModuleAllocator.h"

namespace bvk {

///////////////////////////////////////////////////////////////////
// DflowMatrix
//
// Adaptor on a DflowScalars to provide matrix indexing
////////////////////////////////////////////////////////////////////
class DflowMatrix : public NonCopyable
{
public:
   DflowMatrix(const DflowBuilder &builder, uint32_t cols, uint32_t rows) :
      m_cols(cols),
      m_rows(rows),
      m_scalars(builder, m_cols * m_rows)
   {
   }

   DflowMatrix(uint32_t cols, uint32_t rows, const DflowScalars &scalars) :
      m_cols(cols),
      m_rows(rows),
      m_scalars(scalars)
   {
      assert(scalars.Size() == cols * rows);
   }

   DflowMatrix(const DflowBuilder &builder,
               const Dflow &x00, const Dflow &x01,
               const Dflow &x10, const Dflow &x11) :
      m_cols(2),
      m_rows(2),
      m_scalars(builder, m_cols * m_rows)
   {
      (*this)(0, 0) = x00; (*this)(0, 1) = x01;
      (*this)(1, 0) = x10; (*this)(1, 1) = x11;
   }

   DflowMatrix(const DflowBuilder &builder,
               const Dflow &x00, const Dflow &x01, const Dflow &x02,
               const Dflow &x10, const Dflow &x11, const Dflow &x12,
               const Dflow &x20, const Dflow &x21, const Dflow &x22) :
      m_cols(3),
      m_rows(3),
      m_scalars(builder, m_cols * m_rows)
   {
      (*this)(0, 0) = x00; (*this)(0, 1) = x01; (*this)(0, 2) = x02;
      (*this)(1, 0) = x10; (*this)(1, 1) = x11; (*this)(1, 2) = x12;
      (*this)(2, 0) = x20; (*this)(2, 1) = x21; (*this)(2, 2) = x22;
   }

   DflowMatrix(const DflowBuilder &builder,
               const Dflow &x00, const Dflow &x01, const Dflow &x02, const Dflow &x03,
               const Dflow &x10, const Dflow &x11, const Dflow &x12, const Dflow &x13,
               const Dflow &x20, const Dflow &x21, const Dflow &x22, const Dflow &x23,
               const Dflow &x30, const Dflow &x31, const Dflow &x32, const Dflow &x33) :
      m_cols(4),
      m_rows(4),
      m_scalars(builder, m_cols * m_rows)
   {
      (*this)(0, 0) = x00; (*this)(0, 1) = x01; (*this)(0, 2) = x02; (*this)(0, 3) = x03;
      (*this)(1, 0) = x10; (*this)(1, 1) = x11; (*this)(1, 2) = x12; (*this)(1, 3) = x13;
      (*this)(2, 0) = x20; (*this)(2, 1) = x21; (*this)(2, 2) = x22; (*this)(2, 3) = x23;
      (*this)(3, 0) = x30; (*this)(3, 1) = x31; (*this)(3, 2) = x32; (*this)(3, 3) = x33;
   }

   const Dflow &operator()(int c, int r) const { return m_scalars[c * m_rows + r]; }
         Dflow &operator()(int c, int r)       { return m_scalars[c * m_rows + r]; }

   uint32_t GetCols() const { return m_cols; }
   uint32_t GetRows() const { return m_rows; }

   const DflowBuilder &Builder()    const { return m_scalars.GetBuilder(); }
   const DflowScalars &GetScalars() const { return m_scalars;              }

private:
   uint32_t m_cols;
   uint32_t m_rows;

   DflowScalars m_scalars;
};

} // namespace bvk
