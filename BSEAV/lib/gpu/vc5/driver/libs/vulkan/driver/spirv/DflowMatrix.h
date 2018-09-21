/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vector>
#include <cassert>

#include "DflowScalars.h"

namespace bvk {

///////////////////////////////////////////////////////////////////
// DflowMatrix
//
// Adaptor on a DflowScalars to provide matrix indexing
////////////////////////////////////////////////////////////////////
class DflowMatrix : public NonCopyable
{
public:
   DflowMatrix(const DflowScalars::Allocator &allocator, uint32_t cols, uint32_t rows) :
      m_cols(cols),
      m_rows(rows),
      m_scalars(allocator, m_cols * m_rows)
   {
   }

   DflowMatrix(uint32_t cols, uint32_t rows, const DflowScalars &scalars) :
      m_cols(cols),
      m_rows(rows),
      m_scalars(scalars)
   {
      assert(scalars.Size() == cols * rows);
   }

   DflowMatrix(const DflowScalars::Allocator &allocator,
               const Dflow &x00, const Dflow &x01,
               const Dflow &x10, const Dflow &x11) :
      m_cols(2),
      m_rows(2),
      m_scalars(allocator, { x00, x01,
                             x10, x11 } )
   {}

   DflowMatrix(const DflowScalars::Allocator &allocator,
               const Dflow &x00, const Dflow &x01, const Dflow &x02,
               const Dflow &x10, const Dflow &x11, const Dflow &x12,
               const Dflow &x20, const Dflow &x21, const Dflow &x22) :
      m_cols(3),
      m_rows(3),
      m_scalars(allocator, { x00, x01, x02,
                             x10, x11, x12,
                             x20, x21, x22 } )
   {}

   DflowMatrix(const DflowScalars::Allocator &allocator,
               const Dflow &x00, const Dflow &x01, const Dflow &x02, const Dflow &x03,
               const Dflow &x10, const Dflow &x11, const Dflow &x12, const Dflow &x13,
               const Dflow &x20, const Dflow &x21, const Dflow &x22, const Dflow &x23,
               const Dflow &x30, const Dflow &x31, const Dflow &x32, const Dflow &x33) :
      m_cols(4),
      m_rows(4),
      m_scalars(allocator, { x00, x01, x02, x03,
                             x10, x11, x12, x13,
                             x20, x21, x22, x23,
                             x30, x31, x32, x33 } )
   {}

   const Dflow &operator()(int c, int r) const { return m_scalars[c * m_rows + r]; }
         Dflow &operator()(int c, int r)       { return m_scalars[c * m_rows + r]; }

   uint32_t GetCols() const { return m_cols; }
   uint32_t GetRows() const { return m_rows; }

   const DflowScalars::Allocator &GetAllocator()  const { return m_scalars.GetAllocator(); }
   const DflowScalars            &GetScalars() const    { return m_scalars;                }

private:
   uint32_t m_cols;
   uint32_t m_rows;

   DflowScalars m_scalars;
};

} // namespace bvk
