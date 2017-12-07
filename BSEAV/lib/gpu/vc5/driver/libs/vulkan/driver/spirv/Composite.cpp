/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Composite.h"
#include "Module.h"
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

void LayoutInfo::CaptureMemberLayout(const Module &module, const NodeTypeStruct *node, uint32_t index)
{
   module.GetMatrixMemoryLayout(node, index, &m_matrixStride, &m_columnMajor);
}

void LayoutInfo::CaptureMatrixDetail(const NodeTypeMatrix *mat)
{
   auto  columnType = mat->GetColumnType()->As<const NodeTypeVector *>();

   m_inMatrix = true;
   m_columns  = mat->GetColumnCount();
   m_rows     = columnType->GetComponentCount();
}

///////////////////////////////////////////////////////////////////////////////
// MemoryOffset
//
// Calculates Dataflow for the memory offset of a dynamically indexed
// memory-based composite
///////////////////////////////////////////////////////////////////////////////
MemoryOffset::MemoryOffset(DflowBuilder &builder, const NodeType *elem) :
   m_builder(builder),
   m_elem(elem),
   m_indexNode(nullptr),
   m_indexIsConstant(false),
   m_constantIndex(0),
   m_dynOffset(Dflow::ConstantUInt(0)),
   m_constOffset(0)
{
}

uint32_t MemoryOffset::CalculateStatic(DflowBuilder &builder, const NodeType *type,
                                       const spv::vector<const Node *> &indices)
{
   MemoryOffset visitor(builder, type);

   for (const Node *index : indices)
      visitor.AcceptElement(index);

   return visitor.m_constOffset;
}

Dflow MemoryOffset::Calculate(DflowBuilder &builder, const NodeType *type,
                              const spv::vector<const Node *> &indices)
{
   MemoryOffset visitor(builder, type);

   for (const Node *ix : indices)
      visitor.AcceptElement(ix);

   return visitor.Result();
}

void MemoryOffset::AcceptElement(const Node *indexNode)
{
   m_indexNode       = indexNode;
   m_indexIsConstant = m_builder.ConstantInt(indexNode, &m_constantIndex);

   m_elem->AcceptType(*this);
}

Dflow MemoryOffset::Result() const
{
   return m_dynOffset + Dflow::ConstantUInt(m_constOffset);
}

void MemoryOffset::Visit(const NodeTypeVector *node)
{
   uint32_t stride = m_layoutInfo.RowStride();

   m_elem = node->GetComponentType()->As<const NodeType *>();

   if (m_indexIsConstant)
      m_constOffset = m_constOffset + m_constantIndex * stride;
   else
      m_dynOffset   = m_dynOffset + (m_builder.GetDataflow(m_indexNode)[0].As(DF_UINT) *
                                      Dflow::ConstantUInt(stride));
}

void MemoryOffset::Visit(const NodeTypeMatrix *node)
{
   m_layoutInfo.CaptureMatrixDetail(node);

   uint32_t stride = m_layoutInfo.ColumnStride();

   m_elem = node->GetColumnType()->As<const NodeType *>();

   if (m_indexIsConstant)
      m_constOffset = m_constOffset + m_constantIndex * stride;
   else
      m_dynOffset   = m_dynOffset + (m_builder.GetDataflow(m_indexNode)[0].As(DF_UINT) *
                                       Dflow::ConstantUInt(stride));
}

void MemoryOffset::VisitArray(const Node *array)
{
   // Arrays must have strides -- if there isn't one, this must be a block array with zero stride
   uint32_t stride = 0;
   m_builder.GetModule().GetLiteralDecoration(&stride, spv::Decoration::ArrayStride, array);

   if (m_indexIsConstant)
      m_constOffset = m_constOffset + m_constantIndex * stride;
   else
      m_dynOffset = m_dynOffset + (m_builder.GetDataflow(m_indexNode)[0].As(DF_UINT) *
                                     Dflow::ConstantUInt(stride));
}

void MemoryOffset::Visit(const NodeTypeArray *node)
{
   m_elem = node->GetElementType()->As<const NodeType *>();

   VisitArray(node);
}

void MemoryOffset::Visit(const NodeTypeRuntimeArray *node)
{
   m_elem = node->GetElementType()->As<const NodeType *>();

   VisitArray(node);
}

void MemoryOffset::Visit(const NodeTypeStruct *node)
{
   assert(m_indexIsConstant);

   uint32_t offset;
   bool     found = m_builder.GetModule().GetLiteralMemberDecoration(&offset, spv::Decoration::Offset,
                                                                     node, m_constantIndex);

   assert(found);
   m_layoutInfo.CaptureMemberLayout(m_builder.GetModule(), node, m_constantIndex);

   m_elem         = node->GetMemberstype()[m_constantIndex]->As<const NodeType *>();
   m_constOffset += offset;
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
      uint32_t cv;
      builder.ConstantInt(i, &cv);
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
   spv::vector<MemAccess> result(builder.GetArenaAllocator());
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
   m_builder.GetModule().GetLiteralDecoration(&arrayStride, spv::Decoration::ArrayStride, node);

   uint32_t length;
   m_builder.ConstantInt(node->GetLength(), &length);

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
      uint32_t offset;
      bool     foundOffset = m_builder.GetModule().GetLiteralMemberDecoration(&offset, spv::Decoration::Offset,
                                                                              node, memberIndex);
      assert(foundOffset);

      LayoutInfo layoutInfo(m_layoutInfo);
      layoutInfo.CaptureMemberLayout(m_builder.GetModule(), node, memberIndex);

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

void MemoryLayout::Visit(const NodeTypeRuntimeArray *node)
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
   uint32_t memberIndex;

   m_builder.ConstantInt(m_indices[m_curIndex], &memberIndex);

   m_result.CaptureMemberLayout(m_builder.GetModule(), node, memberIndex);
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

   m_scalars = DflowScalars(m_builder, result);
   m_type    = node->GetComponentType()->As<const NodeType *>();
}

void ExtractDynamicScalars::Visit(const NodeTypeMatrix *node)
{
   uint32_t numCols = node->GetColumnCount();
   auto     colType = node->GetColumnType()->As<const NodeTypeVector *>();
   uint32_t numRows = colType->GetComponentCount();

   DflowScalars result = m_scalars.Slice(0, numRows);
   Dflow        index  = m_builder.GetDataflow(m_index)[0];

   for (uint32_t i = 1; i < numCols; ++i)
   {
      Dflow choose = index.Equals(i);

      for (uint32_t j = 0; j < numRows; ++j)
         result[j] = Dflow::CondOp(choose, m_scalars[i * numRows + j], result[j]);
   }

   m_scalars = result;
   m_type    = colType;
}

void ExtractDynamicScalars::Visit(const NodeTypeArray *node)
{
   uint32_t numElems;
   m_builder.ConstantInt(node->GetLength(), &numElems);
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
   uint32_t index;
   m_builder.ConstantInt(m_index, &index);    // Must be a constant
   uint32_t offset     = m_builder.StructureOffset(node, index);

   auto     memberType = node->GetMemberstype()[index]->As<const NodeType *>();
   uint32_t numScalars = m_builder.GetNumScalars(memberType);

   DflowScalars result(m_builder, numScalars);

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
   m_condition(Dflow::ConstantBool(true))
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
   DflowScalars   result(builder);
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

   DflowScalars    result(builder);
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
   uint32_t numElems;

   m_builder.ConstantInt(node->GetLength(), &numElems);

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
   uint32_t index;
   m_builder.ConstantInt(m_accessChain[m_curIndex], &index);

   for (auto &mt : memberTypes)
   {
      Dflow cond = Dflow::ConstantBool(memberIx == index);

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

void IsSigned::Visit(const NodeTypeRuntimeArray *type)
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

   m_builder.ConstantInt(index, &m_index);
}

void BlockIndex::Visit(const NodeTypeStruct *type)
{
   m_index = 0;
}

///////////////////////////////////////////////////////////////////////////////
// IsSSBO
//
// For struct or array, extract the constant block index
///////////////////////////////////////////////////////////////////////////////
bool IsSSBO::Test(const Module &m, const NodeType *type)
{
   IsSSBO   visitor(m);

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
   m_isSSBO = m_module.HasDecoration(spv::Decoration::BufferBlock, type);
}

} // namespace bvk
