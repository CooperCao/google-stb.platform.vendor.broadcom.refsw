/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "NodeBase.h"
#include "NodeIndex.h"
#include "DflowScalars.h"

namespace bvk {

class Module;
class DflowBuilder;

///////////////////////////////////////////////////////////////////////////////
// LayoutInfo
//
// Properties of a memory resident matrix.
// Obtained in part from the matrix itself and in part from the containing
// structure's decorations.
///////////////////////////////////////////////////////////////////////////////
class LayoutInfo
{
public:
   // Fills in the stride and layout
   void CaptureMemberLayout(const Module &builder, const NodeTypeStruct *node, uint32_t index);

   // Fills in rows and columns and sets "in matrix" flag
   void CaptureMatrixDetail(const NodeTypeMatrix *mat);

   bool StrideIsSpecified() const
   {
      return m_matrixStride != UNSPECIFIED;
   }

   uint32_t ColumnStride() const
   {
      assert(m_inMatrix);

      if (m_columnMajor)
         return StrideIsSpecified() ? m_matrixStride : m_rows * sizeof(float);

      return sizeof(float);
   }

   uint32_t RowStride() const
   {
      if (!m_inMatrix || m_columnMajor)
         return sizeof(float);

      return StrideIsSpecified() ? m_matrixStride : m_columns * sizeof(float);
   }

   uint32_t GetNumColumns() const { return m_columns; }
   uint32_t GetNumRows()   const  { return m_rows;    }

   enum
   {
      UNSPECIFIED = ~0u
   };

private:
   bool     m_inMatrix     = false;
   bool     m_columnMajor  = true;
   uint32_t m_matrixStride = UNSPECIFIED;
   uint32_t m_columns      = 0;
   uint32_t m_rows         = 0;
};

///////////////////////////////////////////////////////////////////////////////
// MemoryOffset
//
// Calculates the memory offset of a pointer from an access chain
///////////////////////////////////////////////////////////////////////////////
class MemoryOffset : public NodeTypeVisitorAssert
{
public:
   static Dflow Calculate(DflowBuilder &builder, const NodeType *type,
                          const spv::vector<const Node *> &indices);
   static uint32_t CalculateStatic(DflowBuilder &builder, const NodeType *type,
                                   const spv::vector<const Node *> &indices);

   void Visit(const NodeTypeVector       *node) override;
   void Visit(const NodeTypeMatrix       *node) override;
   void Visit(const NodeTypeArray        *node) override;
   void Visit(const NodeTypeRuntimeArray *node) override;
   void Visit(const NodeTypeStruct       *node) override;

private:
   MemoryOffset(DflowBuilder &builder, const NodeType *elem);

   void VisitArray(const Node *array);
   void AcceptElement(const Node *index);
   Dflow Result() const;

private:
   DflowBuilder    &m_builder;          // The SPIRV module
   LayoutInfo      m_layoutInfo;
   const NodeType *m_elem;             // Current element
   const Node     *m_indexNode;        // Current index node
   bool            m_indexIsConstant;  // True if the index is a constant
   uint32_t        m_constantIndex;    // If index is constant this is the value

   Dflow           m_dynOffset;        // Accumulated dynamic offset (result)
   uint32_t        m_constOffset;      // Accumulated constant offset (result)
};

///////////////////////////////////////////////////////////////////////////////
// ScalarOffsetStatic
//
// Calculate the position of an element in a composite within an internal
// scalar array (i.e. a DflowScalars)
///////////////////////////////////////////////////////////////////////////////
class ScalarOffsetStatic : public NodeTypeVisitorAssert
{
public:
   static uint32_t Calculate(DflowBuilder &builder, const NodeType *type, const spv::vector<uint32_t> &indices);
   static uint32_t Calculate(DflowBuilder &builder, const NodeType *type, const spv::vector<const Node *> &indices);

   void Visit(const NodeTypeVector *node) override;
   void Visit(const NodeTypeMatrix *node) override;
   void Visit(const NodeTypeArray  *node) override;
   void Visit(const NodeTypeStruct *node) override;

private:
   ScalarOffsetStatic(DflowBuilder &builder, const NodeType *elem);
   void AcceptElement(uint32_t index);

private:
   DflowBuilder       &m_builder;     // Needed to query type info
   const NodeType     *m_elem;        // Current element
   uint32_t            m_index;       // Index of next element
   uint32_t            m_offset;      // Accumulated constant offset (result)
};

///////////////////////////////////////////////////////////////////////////////
// MemoryOffsetsOfScalars
//
// Calculate the memory offsets of scalars within a composite.  The memory
// offsets need not be sequential or compact.
///////////////////////////////////////////////////////////////////////////////

class MemoryOffsetsOfScalars : public NodeTypeVisitorAssert
{
public:
   static spv::vector<MemAccess> Calculate(DflowBuilder &builder, const NodeType *node,
                                           const LayoutInfo &layoutInfo);

   void Visit(const NodeTypeBool         *node) override;
   void Visit(const NodeTypeInt          *node) override;
   void Visit(const NodeTypeFloat        *node) override;
   void Visit(const NodeTypeVector       *node) override;
   void Visit(const NodeTypeMatrix       *node) override;
   void Visit(const NodeTypeArray        *node) override;
   void Visit(const NodeTypeStruct       *node) override;

private:
   MemoryOffsetsOfScalars(DflowBuilder &builder, spv::vector<MemAccess> &result,
                          uint32_t curOffset, const LayoutInfo &layoutInfo);

private:
   DflowBuilder           &m_builder;         // Needed to query type info
   spv::vector<MemAccess> &m_result;          // Result list
   uint32_t                m_curOffset;       // Memory offset of the current element
   const LayoutInfo       &m_layoutInfo;
};

///////////////////////////////////////////////////////////////////////////////
// MemoryLayout
//
// Finds the memory layout of a sub-component of an aggregate
///////////////////////////////////////////////////////////////////////////////
class MemoryLayout : public NodeTypeVisitorEmpty
{
public:
   MemoryLayout(DflowBuilder &builder, const spv::vector<const Node *> &indices);

   static LayoutInfo Calculate(DflowBuilder &builder, const NodeType *node,
                               const spv::vector<const Node *> &indices);

   void Visit(const NodeTypeArray        *node) override;
   void Visit(const NodeTypeRuntimeArray *node) override;
   void Visit(const NodeTypeMatrix       *node) override;
   void Visit(const NodeTypeStruct       *node) override;

private:
   DflowBuilder                    &m_builder;
   const spv::vector<const Node *> &m_indices;
   uint32_t                         m_curIndex = 0;
   const NodeType                  *m_elem     = nullptr;

   LayoutInfo  m_result;
};

///////////////////////////////////////////////////////////////////////////////
// ExtractDynamicScalars
//
// Generate the dataflow to extract the elements from the scalar array
// pointed to by the accessChain
///////////////////////////////////////////////////////////////////////////////
class ExtractDynamicScalars : public NodeTypeVisitorAssert
{
public:
   static DflowScalars Calculate(DflowBuilder &builder, const DflowScalars &scalars,
      const NodeType *compositeType, const spv::vector<const Node *> &accessChain);

   static DflowScalars Calculate(DflowBuilder &builder, const DflowScalars &scalars,
      const NodeType *compositeType, const Node *index);

   void Visit(const NodeTypeBool   *node) override;
   void Visit(const NodeTypeInt    *node) override;
   void Visit(const NodeTypeFloat  *node) override;
   void Visit(const NodeTypeVector *node) override;
   void Visit(const NodeTypeMatrix *node) override;
   void Visit(const NodeTypeArray  *node) override;
   void Visit(const NodeTypeStruct *node) override;

private:
   ExtractDynamicScalars(DflowBuilder &builder, const DflowScalars &scalars,
                  const NodeType *compositeType);

   void AcceptElement(const Node *index);

private:
   DflowBuilder   &m_builder;
   DflowScalars    m_scalars;

   const Node     *m_index;
   const NodeType *m_type;
};

///////////////////////////////////////////////////////////////////////////////
// InsertDynamicScalars
//
// Generate the dataflow to extract the elements from the scalar array
// pointed to by the accessChain
///////////////////////////////////////////////////////////////////////////////
class InsertDynamicScalars : public NodeTypeVisitorAssert
{
public:
   static DflowScalars Calculate(DflowBuilder &builder, const DflowScalars &scalars,
      const NodeType *compositeType, const spv::vector<const Node *> &accessChain, const DflowScalars &data);

   static DflowScalars Calculate(DflowBuilder &builder, const DflowScalars &scalars,
      const NodeType *compositeType, const DflowScalars &component, const DflowScalars &data);

   void Visit(const NodeTypeBool   *node) override;
   void Visit(const NodeTypeInt    *node) override;
   void Visit(const NodeTypeFloat  *node) override;
   void Visit(const NodeTypeVector *node) override;
   void Visit(const NodeTypeMatrix *node) override;
   void Visit(const NodeTypeArray  *node) override;
   void Visit(const NodeTypeStruct *node) override;

private:
   InsertDynamicScalars(DflowBuilder &builder, DflowScalars &result, uint32_t offset,
                  const spv::vector<const Node *> &accessChain, uint32_t curIndex,
                  const DflowScalars &data);

   InsertDynamicScalars(const InsertDynamicScalars &visitor, uint32_t index, uint32_t offset, const Dflow &cond);

   void VisitRecursive(const NodeType *type, uint32_t offset, const Dflow cond);

private:
   DflowBuilder                    &m_builder;
   const DflowScalars              &m_data;      // Data to store
   const spv::vector<const Node *> &m_accessChain;

   DflowScalars   &m_result;
   uint32_t        m_curOffset; // Current offset into result
   const NodeType *m_type;      // Type being processed
   uint32_t        m_curIndex;  // Index into access chain TODO combine with access chain?
   const Dflow     m_condition; // Holds the conjunction of conditions
};

///////////////////////////////////////////////////////////////////////////////
// IsSigned
//
// Test if a type is signed
///////////////////////////////////////////////////////////////////////////////
class IsSigned : public NodeTypeVisitorAssert
{
public:
   static bool Get(const NodeType *type);

   void Visit(const NodeTypeInt *type)           override;
   void Visit(const NodeTypeVector *type)        override;
   void Visit(const NodeTypeArray *type)         override;
   void Visit(const NodeTypeRuntimeArray *type)  override;

private:
   IsSigned() {}

private:
   bool m_result = false;
};

///////////////////////////////////////////////////////////////////////////////
// BlockIndex
//
// Find the constant block index for a UBO/SSBO
///////////////////////////////////////////////////////////////////////////////
class BlockIndex : public NodeTypeVisitorEmpty
{
public:
   static uint32_t Get(DflowBuilder &builder, const NodeType *type,
                       const spv::vector<const Node *> &accessChain);

   void Visit(const NodeTypeArray  *type) override;
   void Visit(const NodeTypeStruct *type) override;

private:
   BlockIndex(DflowBuilder &builder, const spv::vector<const Node *> &accessChain) :
      m_builder(builder),
      m_accessChain(accessChain)
   {}

private:
   DflowBuilder                    &m_builder;
   uint32_t                         m_index = 0;
   const spv::vector<const Node *> &m_accessChain;
};

///////////////////////////////////////////////////////////////////////////////
// IsSSBO
//
// Is this type a struct or array of structs decorated with buffer block
///////////////////////////////////////////////////////////////////////////////
class IsSSBO : public NodeTypeVisitorEmpty
{
public:
   static bool Test(const Module &m, const NodeType *type);

   void Visit(const NodeTypeArray  *type) override;
   void Visit(const NodeTypeStruct *type) override;

private:
   IsSSBO(const Module &m) :
      m_module(m),
      m_isSSBO(false)
   {}

private:
   const Module &m_module;
   bool          m_isSSBO;
};

} // namespace bvk
