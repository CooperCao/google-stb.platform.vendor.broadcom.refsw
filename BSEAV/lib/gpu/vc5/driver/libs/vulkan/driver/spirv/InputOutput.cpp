/******************************************************************************************************
 * Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "InputOutput.h"
#include "Module.h"
#include "DflowBuilder.h"
#include "Nodes.h"

#include "glsl_fastmem.h"     // For malloc_fast

namespace bvk
{

///////////////////////////////////////////////////////////////////////////////////////////////////
// IOMap
//
// A bag of state associated with input and output variables.
// Input and output variables, in the SPIRV sense, are not directly mapped to
// input and output symbols, in the compiler sense, because we have to handle
// component decorations which allow two variables to share a location
// The compiler only expects one symbol per location and has no notion
// of component.
///////////////////////////////////////////////////////////////////////////////////////////////////

void IOMap::Record(uint32_t location, uint32_t component, int *currentId, SymbolTypeHandle type, const QualifierDecorations &q)
{
   Location &loc = m_locs[location];

   // If there is no entry for this location, create one
   if (loc.m_maxComponent == -1)
   {
      loc.m_type       = type;
      loc.m_qualifiers = q;
      loc.m_ids        = static_cast<int *>(malloc_fast(4 * sizeof(int)));

      memset(loc.m_ids, 0xff, 4 *sizeof(int));
   }

   if ((int32_t)component > loc.m_maxComponent)
      loc.m_maxComponent = component;

   if (loc.m_ids[component] == -1)
   {
      loc.m_ids[component] = *currentId;
      *currentId = *currentId + 1;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// IOVisitor
//
// Implements generic code for matrices, arrays and structures used by the visitors below.
// Having the aggregate code common means that all visitors should behave the same with respect
// to them.
///////////////////////////////////////////////////////////////////////////////////////////////////
IOVisitor::IOVisitor(DflowBuilder &builder, const NodeVariable *var) :
   m_builder(builder),
   m_location(0),
   m_component(0)
{
   DecorationQuery query(var);

   m_location = query.RequireLiteral(spv::Decoration::Location);
   query.Literal(&m_component, spv::Decoration::Component);
}

void IOVisitor::Visit(const NodeTypeMatrix *type)
{
   uint32_t colCount = type->GetColumnCount();
   auto     colType  = type->GetColumnType()->As<const NodeTypeVector *>();

   for (uint32_t c = 0; c < colCount; ++c)
      colType->AcceptType(*this);
}

void IOVisitor::Visit(const NodeTypeArray  *type)
{
   uint32_t length   = m_builder.RequireConstantInt(type->GetLength());
   auto     elemType = type->GetElementType()->As<const NodeType *>();

   for (uint32_t i = 0; i < length; ++i)
      elemType->AcceptType(*this);
}

void IOVisitor::Visit(const NodeTypeStruct *type)
{
   auto     &memberTypes = type->GetMemberstype();
   uint32_t  memberCount = memberTypes.size();

   for (uint32_t i = 0; i < memberCount; ++i)
   {
      auto memberType = memberTypes[i]->As<const NodeType *>();

      MemberDecorationQuery   query(type, i);
      query.Literal(&m_location,  spv::Decoration::Location);
      query.Literal(&m_component, spv::Decoration::Component);

      memberType->AcceptType(*this);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// IORecorder
//
// Records variable usage according to its type into an IOMap
///////////////////////////////////////////////////////////////////////////////////////////////////
IORecorder::IORecorder(DflowBuilder &builder, IOMap *map, uint32_t size,
                       int *currentId, const NodeVariable *var) :
   IOVisitor(builder, var),
   m_map(*map),
   m_resultIndex(0),
   m_qualifiers(var),
   m_currentId(currentId)
{}

void IORecorder::Record(DflowBuilder &builder, IOMap *map, uint32_t size, int *currentId,
                        const NodeVariable *var)
{
   IORecorder    visitor(builder, map, size, currentId, var);
   const NodeType *type = var->TypeOfTarget();

   type->AcceptType(visitor);
}

void IORecorder::Scalar(SymbolTypeHandle type)
{
   m_map.Record(m_location, m_component, m_currentId, type, m_qualifiers);
   m_location++;
}

void IORecorder::Visit(const NodeTypeBool *type)
{
   Scalar(SymbolTypeHandle::Bool());
}

void IORecorder::Visit(const NodeTypeInt *type)
{
   Scalar(type->GetSignedness() == 0 ? SymbolTypeHandle::UInt() : SymbolTypeHandle::Int());
}

void IORecorder::Visit(const NodeTypeFloat *type)
{
   Scalar(SymbolTypeHandle::Float());
}

void IORecorder::Visit(const NodeTypeVector *type)
{
   uint32_t componentCount = type->GetComponentCount();
   auto     componentType  = type->GetComponentType()->As<const NodeType *>();
   uint32_t startComponent = m_component;

   for (uint32_t c = 0; c < componentCount; ++c)
   {
      componentType->AcceptType(*this);
      m_location--;      // We want to stay at the same location, but the Scalar() method will bump it on
      m_component++;
   }

   m_location++;
   m_component = startComponent;
}

void IORecorder::Visit(const NodeTypeStruct *type)
{
   auto     &memberTypes = type->GetMemberstype();
   uint32_t  memberCount = memberTypes.size();

   for (uint32_t i = 0; i < memberCount; ++i)
   {
      auto memberType = memberTypes[i]->As<const NodeType *>();

      MemberDecorationQuery query(type, i);

      query.Literal(&m_location,  spv::Decoration::Location);
      query.Literal(&m_component, spv::Decoration::Component);

      QualifierDecorations  oldQuals = m_qualifiers;
      QualifierDecorations &newQuals = m_qualifiers;

      for (const Decoration *dec : query)
         newQuals.UpdateWith(dec);

      memberType->AcceptType(*this);
      m_qualifiers = oldQuals;
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// InputLoader
//
// Gets the dataflow for accessing an input variable by indirecting through the symbols
// held in the IOMap for input variables.
///////////////////////////////////////////////////////////////////////////////////////////////////

// Create dataflow to load an input variable
InputLoader::InputLoader(DflowBuilder &builder, uint32_t numScalars, const NodeVariable *var, BasicBlockHandle block) :
   IOVisitor(builder, var),
   m_resultIndex(0),
   m_result(m_builder.GetAllocator(), numScalars),
   m_block(block)
{}

DflowScalars InputLoader::Load(DflowBuilder &builder, const NodeVariable *var, BasicBlockHandle block)
{
   SymbolHandle symbol     = builder.GetVariableSymbol(var);
   uint32_t     numScalars = symbol.GetType().GetNumScalars();

   InputLoader     loader(builder, numScalars, var, block);

   const NodeType *type = var->TypeOfTarget();

   type->AcceptType(loader);

   return loader.m_result;
}

void InputLoader::LoadScalar()
{
   DflowScalars   scalars = m_builder.LoadFromInputLocation(m_location, m_block);

   m_result[m_resultIndex++] = scalars[m_component];
   m_location++;
}

void InputLoader::Visit(const NodeTypeBool *type)
{
   LoadScalar();
}

void InputLoader::Visit(const NodeTypeInt *type)
{
   LoadScalar();
}

void InputLoader::Visit(const NodeTypeFloat *type)
{
   LoadScalar();
}

void InputLoader::Visit(const NodeTypeVector *type)
{
   DflowScalars   scalars        = m_builder.LoadFromInputLocation(m_location, m_block);

   for (uint32_t i = 0; i < type->GetComponentCount(); ++i)
      m_result[m_resultIndex++] = scalars[m_component + i];

   m_location++;
}

///////////////////////////////////////////////////////////////////////////////////////////////////
// OutputStorer
//
// Sets the dataflow for into an output variable by indirecting through the symbols
// held in the IOMap for output variables.
///////////////////////////////////////////////////////////////////////////////////////////////////

OutputStorer::OutputStorer(DflowBuilder &builder, const DflowScalars &scalars, const NodeVariable *var, BasicBlockHandle block) :
   IOVisitor(builder, var),
   m_scalarsIndex(0),
   m_scalars(scalars),
   m_block(block)
{}

void OutputStorer::Store(DflowBuilder &builder, const DflowScalars &scalars, const NodeVariable *var, BasicBlockHandle block)
{
   OutputStorer   storer(builder, scalars, var, block);
   const NodeType *type = var->TypeOfTarget();

   type->AcceptType(storer);
}

void OutputStorer::StoreScalar()
{
   m_builder.StoreToOutputLocation(m_scalars.Slice(m_scalarsIndex, 1), m_location, m_component, m_block);
   m_scalarsIndex++;
   m_location++;
}

void OutputStorer::Visit(const NodeTypeBool *type)
{
   StoreScalar();
}

void OutputStorer::Visit(const NodeTypeInt *type)
{
   StoreScalar();
}

void OutputStorer::Visit(const NodeTypeFloat *type)
{
   StoreScalar();
}

void OutputStorer::Visit(const NodeTypeVector *type)
{
   uint32_t size = type->GetComponentCount();

   m_builder.StoreToOutputLocation(m_scalars.Slice(m_scalarsIndex, size), m_location, m_component, m_block);
   m_scalarsIndex += size;
   m_location++;
}

}
