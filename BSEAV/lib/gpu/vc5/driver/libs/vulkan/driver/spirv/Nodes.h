/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <stdint.h>
#include <assert.h>

#include "Optional.h"
#include "NodeBase.h"
#include "PoolAllocator.h"
#include "NodeData.h"
#include "Decoration.h"
#include "Extractor.h"
#include "ImageOperands.h"

#ifdef WIN32
#undef GetObject
#endif

namespace bvk
{

class Extractor;

///////////////////////////////////////////////////////
// NodeVisitor
//
// Base class for all visitors over nodes.
// abstract declarations for every node
///////////////////////////////////////////////////////
class NodeVisitor
{
public:
   virtual ~NodeVisitor() {}
   // Has a visitor for every type of node, in this form:
   // virtual void Visit(const NodeNop *) = 0;
   #include "NodeVisitorAbstract.auto.inc"
};

template <class I>
class NodeVisitorDefault : public NodeVisitor
{
public:
   NodeVisitorDefault(I &impl) :
      m_impl(impl)
   {}

   #include "NodeVisitorDefault.auto.inc"

private:
   I &m_impl;
};

///////////////////////////////////////////////////////
// NodeEmptyVisitor
//
// Has empty implementations for every node
///////////////////////////////////////////////////////
class NodeEmptyVisitor : public NodeVisitorDefault<NodeEmptyVisitor>
{
public:
   NodeEmptyVisitor() :
      NodeVisitorDefault<NodeEmptyVisitor>(*this)
   {}

   void Default(const Node *node) {}
};

///////////////////////////////////////////////////////
// NodeAssertVisitor
//
// Has implementations for every node that assert(0)
///////////////////////////////////////////////////////
class NodeAssertVisitor : public NodeVisitorDefault<NodeAssertVisitor>
{
public:
   NodeAssertVisitor() :
      NodeVisitorDefault<NodeAssertVisitor>(*this)
   {}

   void Default(const Node *node) { assert(0); }
};

/////////////////////////////////////////////////////////
// NodeTypeVisitor
//
// A visitor that only visits nodes derived from NodeType
/////////////////////////////////////////////////////////
class NodeTypeVisitor
{
public:
   virtual ~NodeTypeVisitor() {}

   virtual void Visit(const NodeTypeVoid *) = 0;
   virtual void Visit(const NodeTypeBool *) = 0;
   virtual void Visit(const NodeTypeInt *) = 0;
   virtual void Visit(const NodeTypeFloat *) = 0;
   virtual void Visit(const NodeTypeVector *) = 0;
   virtual void Visit(const NodeTypeMatrix *) = 0;
   virtual void Visit(const NodeTypeImage *) = 0;
   virtual void Visit(const NodeTypeSampler *) = 0;
   virtual void Visit(const NodeTypeSampledImage *) = 0;
   virtual void Visit(const NodeTypeArray *) = 0;
   virtual void Visit(const NodeTypeStruct *) = 0;
   virtual void Visit(const NodeTypePointer *) = 0;
   virtual void Visit(const NodeTypeFunction *) = 0;
};

/////////////////////////////////////////////////////////
// NodeTypeVisitorAssert
//
// A visitor that only visits nodes derived from NodeType
// and asserts for all types.  Helpful as a base-class
// where the expected set of types is small.
/////////////////////////////////////////////////////////
class NodeTypeVisitorAssert : public NodeTypeVisitor
{
public:
   void Visit(const NodeTypeVoid *)           override { assert(0); }
   void Visit(const NodeTypeBool *)           override { assert(0); }
   void Visit(const NodeTypeInt *)            override { assert(0); }
   void Visit(const NodeTypeFloat *)          override { assert(0); }
   void Visit(const NodeTypeVector *)         override { assert(0); }
   void Visit(const NodeTypeMatrix *)         override { assert(0); }
   void Visit(const NodeTypeImage *)          override { assert(0); }
   void Visit(const NodeTypeSampler *)        override { assert(0); }
   void Visit(const NodeTypeSampledImage *)   override { assert(0); }
   void Visit(const NodeTypeArray *)          override { assert(0); }
   void Visit(const NodeTypeStruct *)         override { assert(0); }
   void Visit(const NodeTypePointer *)        override { assert(0); }
   void Visit(const NodeTypeFunction *)       override { assert(0); }
};

/////////////////////////////////////////////////////////
// NodeTypeVisitorEmpty
/////////////////////////////////////////////////////////
class NodeTypeVisitorEmpty : public NodeTypeVisitor
{
public:
   void Visit(const NodeTypeVoid *)           override { }
   void Visit(const NodeTypeBool *)           override { }
   void Visit(const NodeTypeInt *)            override { }
   void Visit(const NodeTypeFloat *)          override { }
   void Visit(const NodeTypeVector *)         override { }
   void Visit(const NodeTypeMatrix *)         override { }
   void Visit(const NodeTypeImage *)          override { }
   void Visit(const NodeTypeSampler *)        override { }
   void Visit(const NodeTypeSampledImage *)   override { }
   void Visit(const NodeTypeArray *)          override { }
   void Visit(const NodeTypeStruct *)         override { }
   void Visit(const NodeTypePointer *)        override { }
   void Visit(const NodeTypeFunction *)       override { }
};

//////////////////////////////////////////////////////////
// Node*
//
// Auto-generated class implementation of every SPIRV node
// These will all derive either from Node or NodeType
//////////////////////////////////////////////////////////
#include "Nodes.auto.inc"

//////////////////////////////////////////////////////////
// Bespoke Nodes
//
// Nodes whose grammar is inadequate to describe them
// or who need special consideration are defined here
//////////////////////////////////////////////////////////
class NodeExecutionMode : public Node
{
public:
   NodeExecutionMode(Extractor &ext) :
      Node(),
      m_literals(ext.GetAllocator())
   {
      ext >> m_entryPoint >> m_mode >> m_literals;
   }

   void Accept(NodeVisitor &visitor) const override
   {
      visitor.Visit(this);
   }

   const spv::vector<uint32_t> &GetLiterals() const
   {
      return m_literals;
   }

   const NodeConstPtr &GetEntryPoint() const
   {
      return m_entryPoint;
   }

   spv::ExecutionMode GetMode() const
   {
      return m_mode;
   }

   spv::Core GetOpCode() const override
   {
      return spv::Core::OpExecutionMode;
   }

private:
   NodeConstPtr            m_entryPoint;
   spv::ExecutionMode      m_mode;
   spv::vector<uint32_t>   m_literals;
};

class NodeExecutionModeId : public Node
{
public:
   NodeExecutionModeId(Extractor &ext) :
      Node(),
      m_ids(ext.GetAllocator())
   {
      ext >> m_entryPoint >> m_mode >> m_ids;
   }

   void Accept(NodeVisitor &visitor) const override
   {
      visitor.Visit(this);
   }

   const spv::vector<NodeConstPtr> &GetIds() const
   {
      return m_ids;
   }

   const NodeConstPtr &GetEntryPoint() const
   {
      return m_entryPoint;
   }

   spv::ExecutionMode GetMode() const
   {
      return m_mode;
   }

   spv::Core GetOpCode() const override
   {
      return spv::Core::OpExecutionMode;
   }

private:
   NodeConstPtr               m_entryPoint;
   spv::ExecutionMode         m_mode;
   spv::vector<NodeConstPtr>  m_ids;
};

class NodeSpecConstantOp : public Node
{
public:
   NodeSpecConstantOp(Extractor &ext) :
      Node(),
      m_operands(ext.GetAllocator()),
      m_indices(ext.GetAllocator())
   {
      ext >> m_resultType >> m_resultId >> m_opcode;

      switch (m_opcode)
      {
      case spv::Core::OpVectorShuffle:
         ext >> m_vector1 >> m_vector2 >> m_indices;
         break;

      case spv::Core::OpCompositeExtract:
         ext >> m_composite >> m_indices;
         break;

      case spv::Core::OpCompositeInsert:
         ext >> m_object >> m_composite >> m_indices;
         break;

      default:
         ext >> m_operands;
         break;
      }
   }

   void Accept(NodeVisitor &visitor) const override
   {
      visitor.Visit(this);
   }

   spv::Core GetOpCode() const override
   {
      return spv::Core::OpSpecConstantOp;
   }

   spv::Core GetOperation() const                        { return m_opcode;    }
   const spv::vector<NodeConstPtr> &GetOperands() const  { return m_operands;  }
   const spv::vector<uint32_t> &GetIndices() const       { return m_indices;   }
   const NodeConstPtr &GetVector1() const                { return m_vector1;   }
   const NodeConstPtr &GetVector2() const                { return m_vector2;   }
   const NodeConstPtr &GetObject() const                 { return m_object;    }
   const NodeConstPtr &GetComposite() const              { return m_composite; }

private:
   spv::Core                 m_opcode;
   spv::vector<NodeConstPtr> m_operands;  // Operands for non-vector instructions

   // Vector op args
   NodeConstPtr              m_vector1;   // For shuffle
   NodeConstPtr              m_vector2;   // For shuffle
   NodeConstPtr              m_object;    // for insert
   NodeConstPtr              m_composite; // For insert and extract

   spv::vector<uint32_t>     m_indices;   // Common to vector ops
};

class NodeAccessChain : public Node
{
public:
   NodeAccessChain(Extractor &ext, bool inBounds) :
      Node(),
      m_indices(ext.GetAllocator()),
      m_inBounds(inBounds)
   {
      ext >> m_resultType >> m_resultId >> m_base >> m_indices;
   }

   void Accept(NodeVisitor &visitor) const override
   {
      visitor.Visit(this);
   }

   const NodeConstPtr &GetBase() const
   {
      return m_base;
   }

   const spv::vector<NodeConstPtr> &GetIndices() const
   {
      return m_indices;
   }

   spv::Core GetOpCode() const override
   {
      return spv::Core::OpAccessChain;
   }

   bool InBounds() const
   {
      return m_inBounds;
   }

private:
   NodeConstPtr              m_base;
   spv::vector<NodeConstPtr> m_indices;
   bool                      m_inBounds;
};

class NodeTypeArray : public NodeType
{
public:
   NodeTypeArray(Extractor &ext, bool runtime)
      : NodeType()
   {
      ext >> m_resultId >> m_elementType;

      if (!runtime)
         ext >> m_length;
      else
         m_length = nullptr;
   }

   void AcceptType(NodeTypeVisitor &visitor) const override
   {
      visitor.Visit(this);
   }

   void Accept(NodeVisitor &visitor) const override
   {
      visitor.Visit(this);
   }

   const NodeConstPtr &GetElementType() const
   {
      return m_elementType;
   }

   const NodeConstPtr &GetLength() const
   {
      assert(m_length);
      return m_length;
   }

   spv::Core GetOpCode() const override
   {
      return spv::Core::OpTypeArray;
   }

   bool IsRuntime() const
   {
      return !m_length;
   }

private:
   NodeConstPtr m_elementType;
   NodeConstPtr m_length;
};

} // namespace bvk
