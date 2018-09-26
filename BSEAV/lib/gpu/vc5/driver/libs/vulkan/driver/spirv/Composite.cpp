/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Composite.h"
#include "DflowBuilder.h"
#include "DflowLibrary.h"

namespace bvk {

///////////////////////////////////////////////////////////////////////////////
// LayoutInfo
//
// Properties of a memory resident matrix.
// Obtained in part from the matrix itself and in part from the containing
// structure's decorations.
///////////////////////////////////////////////////////////////////////////////

void LayoutInfo::CaptureMemberLayout(const NodeTypeStruct *node, uint32_t index)
{
   // Only do anything for matrices and arrays of matrices. This is equivalent
   // to being decorated with MatrixStride.
   MemberDecorationQuery   query(node, index);

   if (query.Literal(&m_matrixStride, spv::Decoration::MatrixStride))
      m_columnMajor = query.Has(spv::Decoration::ColMajor);
}

void LayoutInfo::CaptureMatrixDetail(const NodeTypeMatrix *mat)
{
   m_inMatrix = true;

   NodeTypeMatrix::Size sz = mat->GetSize();
   m_columns = sz.cols;
   m_rows    = sz.rows;
}

///////////////////////////////////////////////////////////////////////////////
// TypeFind
//
// Applies an access chain to find the result type
///////////////////////////////////////////////////////////////////////////////
TypeFind::TypeFind(DflowBuilder &builder, const NodeType *elem) :
   m_builder(builder),
   m_elem(elem)
{
}

const NodeType *TypeFind::FindType(DflowBuilder &builder, const NodeType *type,
                                   const spv::vector<const Node *> &indices)
{
   TypeFind visitor(builder, type);

   for (const Node *index : indices)
      visitor.AcceptElement(index);

   return visitor.m_elem;
}

void TypeFind::AcceptElement(const Node *indexNode)
{
   m_index = indexNode;
   m_elem->AcceptType(*this);
}

void TypeFind::Visit(const NodeTypeVector *node)
{
   m_elem = node->GetComponentType()->As<const NodeType *>();
}

void TypeFind::Visit(const NodeTypeMatrix *node)
{
   m_elem = node->GetColumnType()->As<const NodeType *>();
}

void TypeFind::Visit(const NodeTypeArray  *node)
{
   m_elem = node->GetElementType()->As<const NodeType *>();
}

void TypeFind::Visit(const NodeTypeStruct *node)
{
   uint32_t constIndex = m_builder.RequireConstantInt(m_index);
   m_elem = node->GetMemberstype()[constIndex]->As<const NodeType *>();
}

///////////////////////////////////////////////////////////////////////////////
// MemoryOffset
//
// Calculates Dataflow for the memory offset of a dynamically indexed
// memory-based composite
///////////////////////////////////////////////////////////////////////////////
MemoryOffset::MemoryOffset(DflowBuilder &builder, const NodeType *elem, const Dflow &buf) :
   m_builder(builder),
   m_buf(buf),
   m_elem(elem),
   m_index(nullptr),
   m_offset(Dflow::UInt(0))
{
}

Dflow MemoryOffset::Calculate(DflowBuilder &builder, const NodeType *type,
                              const spv::vector<const Node *> &indices, const Dflow &buf)
{
   MemoryOffset visitor(builder, type, buf);

   for (const Node *ix : indices)
      visitor.AcceptElement(ix);

   return visitor.m_offset;
}

void MemoryOffset::AcceptElement(const Node *indexNode)
{
   m_index = indexNode;

   m_elem->AcceptType(*this);
}

void MemoryOffset::ApplyIndex(uint32_t stride, const Dflow &last)
{
   const Dflow &index = m_builder.GetDataflow(m_index)[0].As(DF_UINT);

   Dflow clampedIndex = Dflow::BinaryOp(DATAFLOW_MIN, index, last);

   m_offset = m_offset + clampedIndex * stride;
}

void MemoryOffset::Visit(const NodeTypeVector *node)
{
   m_elem = node->GetComponentType()->As<const NodeType *>();
   ApplyIndex(m_layoutInfo.RowStride(), Dflow::UInt(node->GetComponentCount() - 1));
}

void MemoryOffset::Visit(const NodeTypeMatrix *node)
{
   m_layoutInfo.CaptureMatrixDetail(node);

   m_elem = node->GetColumnType()->As<const NodeType *>();
   ApplyIndex(m_layoutInfo.ColumnStride(), Dflow::UInt(node->GetColumnCount() - 1));
}

void MemoryOffset::Visit(const NodeTypeArray *node)
{
   m_elem = node->GetElementType()->As<const NodeType *>();

   // Arrays must have strides -- if there isn't one, this must be a block array with zero stride
   uint32_t stride = 0;
   DecorationQuery(node).Literal(&stride, spv::Decoration::ArrayStride);

   Dflow last;

   if (node->IsRuntime())
      last = Dflow::BufferArrayLength(m_buf, 1);
   else
      last = m_builder.GetDataflow(node->GetLength())[0].As(DF_UINT) - 1;

   ApplyIndex(stride, last);
}

void MemoryOffset::Visit(const NodeTypeStruct *node)
{
   uint32_t constIndex = m_builder.RequireConstantInt(m_index);

   MemberDecorationQuery query(node, constIndex);
   uint32_t offset = query.RequireLiteral(spv::Decoration::Offset);

   m_layoutInfo.CaptureMemberLayout(node, constIndex);

   m_elem   = node->GetMemberstype()[constIndex]->As<const NodeType *>();
   m_offset = m_offset + offset;
}

///////////////////////////////////////////////////////////////////////////////
// ScalarOffsetStatic
//
// Calculate the position of an element in a composite within an internal
// scalar array (i.e. a DflowScalars)
///////////////////////////////////////////////////////////////////////////////
ScalarOffsetStatic::ScalarOffsetStatic(DflowBuilder &builder, const NodeType *elem) :
   m_builder(builder),
   m_elem(elem),
   m_offset(0)
{
}

uint32_t ScalarOffsetStatic::Calculate(
   DflowBuilder &builder, const NodeType *type, const spv::vector<uint32_t> &indices)
{
   ScalarOffsetStatic  visitor(builder, type);

   for (uint32_t i : indices)
      visitor.AcceptElement(i);

   return visitor.m_offset;
}

uint32_t ScalarOffsetStatic::Calculate(
   DflowBuilder &builder, const NodeType *type, const spv::vector<const Node *> &indices)
{
   ScalarOffsetStatic  visitor(builder, type);

   for (const Node *i : indices)
   {
      uint32_t cv = builder.RequireConstantInt(i);
      visitor.AcceptElement(cv);
   }

   return visitor.m_offset;
}

void ScalarOffsetStatic::AcceptElement(uint32_t index)
{
   m_index = index;
   m_elem->AcceptType(*this);
}

void ScalarOffsetStatic::Visit(const NodeTypeStruct *node)
{
   m_elem    = node->GetMemberstype()[m_index]->As<const NodeType *>();
   m_offset += m_builder.StructureOffset(node, m_index);
}

void ScalarOffsetStatic::Visit(const NodeTypeVector *node)
{
   m_elem    = node->GetComponentType()->As<const NodeType *>();
   m_offset += m_index;
}

void ScalarOffsetStatic::Visit(const NodeTypeMatrix *node)
{
   auto colVecType = node->GetColumnType()->As<const NodeTypeVector *>();

   m_elem    = colVecType;
   m_offset += colVecType->GetComponentCount() * m_index;
}

void ScalarOffsetStatic::Visit(const NodeTypeArray *node)
{
   m_elem    = node->GetElementType()->As<const NodeType *>();
   m_offset += m_index * m_builder.GetNumScalars(m_elem);
}

///////////////////////////////////////////////////////////////////////////////
// MemoryOffsetsOfScalars
//
// Calculate the memory offsets of scalars within a composite.  The memory
// offsets need not be sequential or compact.
///////////////////////////////////////////////////////////////////////////////
MemoryOffsetsOfScalars::MemoryOffsetsOfScalars(DflowBuilder &builder, spv::vector<MemAccess> &result,
                                               uint32_t curOffset, const LayoutInfo &layoutInfo) :
   m_builder(builder),
   m_result(result),
   m_curOffset(curOffset),
   m_layoutInfo(layoutInfo)
{}

// Calculate
//
// Calculate the memory offsets for scalars within a composite.
// This static method is the only way to use this class
//
spv::vector<MemAccess> MemoryOffsetsOfScalars::Calculate(
   DflowBuilder &builder, const NodeType *node, const LayoutInfo &layoutInfo)
{
   spv::vector<MemAccess> result(builder.GetAllocator());
   result.reserve(builder.GetNumScalars(node));

   MemoryOffsetsOfScalars visitor(builder, result, 0, layoutInfo);

   node->AcceptType(visitor);

   return result;
}

void MemoryOffsetsOfScalars::Visit(const NodeTypeBool *node)
{
   m_result.emplace_back(m_curOffset, 1);
}

void MemoryOffsetsOfScalars::Visit(const NodeTypeInt *node)
{
   m_result.emplace_back(m_curOffset, 1);
}

void MemoryOffsetsOfScalars::Visit(const NodeTypeFloat *node)
{
   m_result.emplace_back(m_curOffset, 1);
}

void MemoryOffsetsOfScalars::Visit(const NodeTypeVector *node)
{
   uint32_t stride     = m_layoutInfo.RowStride();
   uint32_t components = node->GetComponentCount();

   // Is the vector contiguous in memory?
   if (stride == 4)
   {
      // Load in one lump
      m_result.emplace_back(m_curOffset, components);
   }
   else
   {
      for (uint32_t i = 0; i < components; i++)
         m_result.emplace_back(m_curOffset + i * stride, 1);
   }
}

void MemoryOffsetsOfScalars::Visit(const NodeTypeMatrix *node)
{
   auto columnType = node->GetColumnType()->As<const NodeTypeVector *>();

   LayoutInfo  layoutInfo(m_layoutInfo);
   layoutInfo.CaptureMatrixDetail(node);

   uint32_t stride = layoutInfo.ColumnStride();

   for (uint32_t i = 0; i < layoutInfo.GetNumColumns(); i++)
   {
      MemoryOffsetsOfScalars  visitor(m_builder, m_result,
                                      m_curOffset + i * stride, layoutInfo);

      columnType->AcceptType(visitor);
   }
}

void MemoryOffsetsOfScalars::Visit(const NodeTypeArray *node)
{
   // Arrays must have strides -- if there isn't one, this must be a block array with zero stride
   uint32_t arrayStride = 0;
   DecorationQuery(node).Literal(&arrayStride, spv::Decoration::ArrayStride);

   uint32_t length = m_builder.RequireConstantInt(node->GetLength());

   const NodeType *elemType = node->GetElementType()->As<const NodeType *>();

   for (uint32_t i = 0; i < length; i++)
   {
      MemoryOffsetsOfScalars visitor(m_builder, m_result,
                                     m_curOffset + (i * arrayStride), m_layoutInfo);

      elemType->AcceptType(visitor);
   }
}

void MemoryOffsetsOfScalars::Visit(const NodeTypeStruct *node)
{
   auto &members = node->GetMemberstype();

   uint32_t memberIndex = 0;
   for (auto &member : members)
   {
      MemberDecorationQuery query(node, memberIndex);

      uint32_t offset = query.RequireLiteral(spv::Decoration::Offset);

      LayoutInfo layoutInfo(m_layoutInfo);
      layoutInfo.CaptureMemberLayout(node, memberIndex);

      MemoryOffsetsOfScalars visitor(m_builder, m_result, m_curOffset + offset, layoutInfo);

      member->As<const NodeType *>()->AcceptType(visitor);

      memberIndex++;
   }
}

///////////////////////////////////////////////////////////////////////////////
// MemoryLayout
///////////////////////////////////////////////////////////////////////////////
MemoryLayout::MemoryLayout(DflowBuilder &builder, const spv::vector<const Node *> &indices) :
   m_builder(builder),
   m_indices(indices)
{
}

LayoutInfo MemoryLayout::Calculate(DflowBuilder &builder, const NodeType *node,
                                   const spv::vector<const Node *> &indices)
{
   MemoryLayout visitor(builder, indices);

   visitor.m_elem = node;

   for (visitor.m_curIndex = 0; visitor.m_curIndex < indices.size(); ++visitor.m_curIndex)
      visitor.m_elem->AcceptType(visitor);

   return visitor.m_result;
}

void MemoryLayout::Visit(const NodeTypeArray *node)
{
   // Burrow into array -- it could contain structures
   m_elem = node->GetElementType()->As<const NodeType *>();
}

void MemoryLayout::Visit(const NodeTypeMatrix *node)
{
   m_elem = node->GetColumnType()->As<const NodeType *>();
   m_result.CaptureMatrixDetail(node);
}

void MemoryLayout::Visit(const NodeTypeStruct *node)
{
   // Indices into structures must be constant
   uint32_t memberIndex = m_builder.RequireConstantInt(m_indices[m_curIndex]);

   m_result.CaptureMemberLayout(node, memberIndex);
   m_elem   = node->GetMemberstype()[memberIndex]->As<const NodeType *>();
}

///////////////////////////////////////////////////////////////////////////////
// ExtractDynamicScalars
//
// Generate the dataflow to extract the elements from the scalar array
// pointed to by the accessChain
///////////////////////////////////////////////////////////////////////////////
ExtractDynamicScalars::ExtractDynamicScalars(DflowBuilder &builder, const DflowScalars &scalars,
                                             const NodeType *compositeType) :
   m_builder(builder),
   m_scalars(scalars),
   m_type(compositeType)
{
}

void ExtractDynamicScalars::Visit(const NodeTypeBool *node)
{
}

void ExtractDynamicScalars::Visit(const NodeTypeInt *node)
{
}

void ExtractDynamicScalars::Visit(const NodeTypeFloat *node)
{
}

void ExtractDynamicScalars::Visit(const NodeTypeVector *node)
{
   Dflow result(m_scalars[0]);
   Dflow index = m_builder.GetDataflow(m_index)[0];

   for (uint32_t i = 1; i < node->GetComponentCount(); ++i)
   {
      Dflow choose = index.Equals(i);
      result = Dflow::CondOp(choose, m_scalars[i], result);
   }

   m_scalars = DflowScalars(m_builder.GetAllocator(), result);
   m_type    = node->GetComponentType()->As<const NodeType *>();
}

void ExtractDynamicScalars::Visit(const NodeTypeMatrix *node)
{
   NodeTypeMatrix::Size sz = node->GetSize();

   DflowScalars result = m_scalars.Slice(0, sz.rows);
   Dflow        index  = m_builder.GetDataflow(m_index)[0];

   for (uint32_t i = 1; i < sz.cols; ++i)
   {
      Dflow choose = index.Equals(i);

      for (uint32_t j = 0; j < sz.rows; ++j)
         result[j] = Dflow::CondOp(choose, m_scalars[i * sz.rows + j], result[j]);
   }

   m_scalars = result;
   m_type    = node->GetColumnType()->As<const NodeTypeVector *>();
}

void ExtractDynamicScalars::Visit(const NodeTypeArray *node)
{
   uint32_t numElems = m_builder.RequireConstantInt(node->GetLength());
   auto     elemType = node->GetElementType()->As<const NodeType *>();
   uint32_t elemSize = m_builder.GetNumScalars(elemType);

   DflowScalars result = m_scalars.Slice(0, elemSize);
   Dflow        index  = m_builder.GetDataflow(m_index)[0];

   for (uint32_t i = 1; i < numElems; ++i)
   {
      Dflow choose = index.Equals(i);

      for (uint32_t j = 0; j < elemSize; ++j)
      {
         const Dflow   &trueFlow  = m_scalars[i * elemSize + j];
         const Dflow   &falseFlow = result[j];

         result[j] = Dflow::CondOp(choose, trueFlow, falseFlow);
      }
   }

   m_scalars = result;
   m_type    = elemType;
}

void ExtractDynamicScalars::Visit(const NodeTypeStruct *node)
{
   uint32_t index  = m_builder.RequireConstantInt(m_index);
   uint32_t offset = m_builder.StructureOffset(node, index);

   auto     memberType = node->GetMemberstype()[index]->As<const NodeType *>();
   uint32_t numScalars = m_builder.GetNumScalars(memberType);

   DflowScalars result(m_builder.GetAllocator(), numScalars);

   for (uint32_t i = 0; i < numScalars; ++i)
      result[i] = m_scalars[offset + i];

   m_scalars = result;
   m_type    = memberType;
}

void ExtractDynamicScalars::AcceptElement(const Node *index)
{
   m_index = index;
   m_type->AcceptType(*this);
}

DflowScalars ExtractDynamicScalars::Calculate(DflowBuilder &builder, const DflowScalars &scalars,
   const NodeType *compositeType, const spv::vector<const Node *> &accessChain)
{
   ExtractDynamicScalars visitor(builder, scalars, compositeType);

   for (const Node *i : accessChain)
      visitor.AcceptElement(i);

   return visitor.m_scalars;
}

DflowScalars ExtractDynamicScalars::Calculate(DflowBuilder &builder, const DflowScalars &scalars,
   const NodeType *compositeType, const Node *index)
{
   ExtractDynamicScalars visitor(builder, scalars, compositeType);

   visitor.AcceptElement(index);

   return visitor.m_scalars;
}

///////////////////////////////////////////////////////////////////////////////
// InsertDynamicScalars
//
// Generate the dataflow to extract the elements from the scalar array
// pointed to by the accessChain
///////////////////////////////////////////////////////////////////////////////

InsertDynamicScalars::InsertDynamicScalars(
      DflowBuilder &builder, DflowScalars &result, uint32_t offset,
      const spv::vector<const Node *> &accessChain, uint32_t curIndex, const DflowScalars &data) :
   m_builder(builder),
   m_data(data),
   m_accessChain(accessChain),
   m_result(result),
   m_curOffset(offset),
   m_curIndex(curIndex),
   m_condition(Dflow::Bool(true))
{
}

InsertDynamicScalars::InsertDynamicScalars(
      const InsertDynamicScalars &visitor, uint32_t index,
      uint32_t offset, const Dflow &cond) :
   m_builder(visitor.m_builder),
   m_data(visitor.m_data),
   m_accessChain(visitor.m_accessChain),
   m_result(visitor.m_result),
   m_curOffset(offset),
   m_curIndex(index),
   m_condition(cond)
{
}

DflowScalars InsertDynamicScalars::Calculate(DflowBuilder &builder, const DflowScalars &scalars,
   const NodeType *compositeType, const spv::vector<const Node *> &accessChain, const DflowScalars &data)
{
   DflowScalars   result(builder.GetAllocator());
   result = scalars;

   InsertDynamicScalars visitor(builder, result, 0, accessChain, 0, data);

   compositeType->AcceptType(visitor);

   return result;
}

DflowScalars InsertDynamicScalars::Calculate(DflowBuilder &builder, const DflowScalars &scalars,
   const NodeType *compositeType, const DflowScalars &index, const DflowScalars &component)
{
   auto            vectorType    = compositeType->As<const NodeTypeVector *>();
   uint32_t        numComponents = vectorType->GetComponentCount();

   DflowScalars    result(builder.GetAllocator());
   result = scalars;

   for (uint32_t i = 0; i < numComponents; ++i)
   {
      Dflow cond  = index[0].Equals(i);

      result[i] = Dflow::CondOp(cond, component[0], result[i]);
   }

   return result;
}

void InsertDynamicScalars::VisitRecursive(const NodeType *type, uint32_t offset, const Dflow cond)
{
   uint32_t newIndex = m_curIndex + 1;
   Dflow    newCond  = cond && m_condition;

   // Have we hit the end of the access chain?
   if (newIndex == m_accessChain.size())
   {
      for (uint32_t i = 0; i < m_data.Size(); ++i)
         m_result[offset + i] = Dflow::CondOp(newCond, m_data[i], m_result[offset + i]);
   }
   else
   {
      InsertDynamicScalars visitor(*this, newIndex, offset, newCond);

      type->AcceptType(visitor);
   }
}

void InsertDynamicScalars::Visit(const NodeTypeBool *node)
{
}

void InsertDynamicScalars::Visit(const NodeTypeInt *node)
{
}

void InsertDynamicScalars::Visit(const NodeTypeFloat *node)
{
}

void InsertDynamicScalars::Visit(const NodeTypeVector *node)
{
   uint32_t        numComponents = node->GetComponentCount();
   const NodeType *compType      = node->GetComponentType()->As<const NodeType *>();
   uint32_t        offset        = m_curOffset;
   Dflow           index         = m_builder.GetDataflow(m_accessChain[m_curIndex])[0];

   for (uint32_t ci = 0; ci < numComponents; ++ci)
   {
      Dflow cond = index.Equals(ci);

      VisitRecursive(compType, offset + ci, cond);
   }
}

void InsertDynamicScalars::Visit(const NodeTypeMatrix *node)
{
   const NodeType *colType = node->GetColumnType()->As<const NodeType *>();
   uint32_t        colSize = m_builder.GetNumScalars(colType);
   uint32_t        numCols = node->GetColumnCount();
   uint32_t        offset  = m_curOffset;
   Dflow           index   = m_builder.GetDataflow(m_accessChain[m_curIndex])[0];

   for (uint32_t i = 0; i < numCols; ++i)
   {
      Dflow cond = index.Equals(i);

      VisitRecursive(colType, offset, cond);

      offset += colSize;
   }
}

void InsertDynamicScalars::Visit(const NodeTypeArray *node)
{
   auto     elemType = node->GetElementType()->As<const NodeType *>();
   uint32_t numElems = m_builder.RequireConstantInt(node->GetLength());

   uint32_t elemSize = m_builder.GetNumScalars(elemType);
   uint32_t offset   = m_curOffset;
   Dflow    index    = m_builder.GetDataflow(m_accessChain[m_curIndex])[0];

   for (uint32_t i = 0; i < numElems; ++i)
   {
      Dflow cond = index.Equals(i);

      VisitRecursive(elemType, offset, cond);

      offset += elemSize;
   }
}

void InsertDynamicScalars::Visit(const NodeTypeStruct *node)
{
   auto &memberTypes = node->GetMemberstype();

   uint32_t offset   = m_curOffset;
   uint32_t memberIx = 0;
   uint32_t index = m_builder.RequireConstantInt(m_accessChain[m_curIndex]);

   for (auto &mt : memberTypes)
   {
      Dflow cond = Dflow::Bool(memberIx == index);

      auto memberType = mt->As<const NodeType *>();

      VisitRecursive(memberType, offset, cond);

      offset = offset + m_builder.GetNumScalars(memberType);

      memberIx += 1;
   }
}

///////////////////////////////////////////////////////////////////////////////
// IsSigned
//
// Test if a type is signed
///////////////////////////////////////////////////////////////////////////////

bool IsSigned::Get(const NodeType *type)
{
   IsSigned visitor;

   type->AcceptType(visitor);

   return visitor.m_result;
}

void IsSigned::Visit(const NodeTypeInt *type)
{
   m_result = type->GetSignedness() != 0;
}

void IsSigned::Visit(const NodeTypeVector *type)
{
   type->GetComponentType()->As<const NodeType *>()->AcceptType(*this);
}

void IsSigned::Visit(const NodeTypeArray *type)
{
   type->GetElementType()->As<const NodeType *>()->AcceptType(*this);
}

///////////////////////////////////////////////////////////////////////////////
// BlockIndex
//
// For struct or array, extract the constant block index
///////////////////////////////////////////////////////////////////////////////

uint32_t BlockIndex::Get(DflowBuilder &builder, const NodeType *type,
                         const spv::vector<const Node *> &accessChain)
{
   BlockIndex  visitor(builder, accessChain);

   type->AcceptType(visitor);

   return visitor.m_index;
}

void BlockIndex::Visit(const NodeTypeArray *type)
{
   const Node *index = m_accessChain[0];

   m_index = m_builder.RequireConstantInt(index);
}

void BlockIndex::Visit(const NodeTypeStruct *type)
{
   m_index = 0;
}

///////////////////////////////////////////////////////////////////////////////
// IsSSBO
//
// Return whether or not a variable is in an SSBO
///////////////////////////////////////////////////////////////////////////////
bool IsSSBO::Test(const NodeVariable *v)
{
   spv::StorageClass s = v->GetStorageClass();
   if (s == spv::StorageClass::StorageBuffer)
      return true;
   if (s != spv::StorageClass::Uniform)
      return false;

   const NodeType *type = v->TypeOfTarget();

   IsSSBO   visitor;
   type->AcceptType(visitor);

   return visitor.m_isSSBO;
}

void IsSSBO::Visit(const NodeTypeArray *type)
{
   auto elemType = type->GetElementType()->As<const NodeType *>();

   elemType->AcceptType(*this);
}

void IsSSBO::Visit(const NodeTypeStruct *type)
{
   m_isSSBO = DecorationQuery(type).Has(spv::Decoration::BufferBlock);
}

} // namespace bvk
