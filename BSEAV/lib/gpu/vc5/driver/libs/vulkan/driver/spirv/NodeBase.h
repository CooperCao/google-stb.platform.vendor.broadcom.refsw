/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <cassert>
#include "Spirv.h"

namespace bvk
{

// Include forward decls for every node type
#include "NodeDecls.auto.inc"

class Node;
class NodeType;   // Base class for types
class DflowBuilder;
class NodeVisitor;
class NodeTypeVisitor;
class Module;
class Extractor;

/////////////////////////////////////////////////////////////////////////////
// NodeBase
//
// Abstract interface for all Nodes (SPIR-V operations)
// Note: We choose to have GetInstructionSet and GetOpCode as virtual methods
// rather than holding data in the base node. The concrete classes are auto-
// generated so can very easily provide the methods.
/////////////////////////////////////////////////////////////////////////////
class NodeBase
{
public:
   virtual ~NodeBase() {}

   // Dispatch this node through the given visitor
   virtual void Accept(NodeVisitor &visitor) const = 0;

   // Return the instruction set that this node belongs to
   virtual spv::InstructionSet GetInstructionSet() const = 0;

   // Return the opcode within the instruction set for this node
   virtual spv::Core GetOpCode() const = 0;
};

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
   virtual void Visit(const NodeTypeRuntimeArray *) = 0;
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
   void Visit(const NodeTypeRuntimeArray *)   override { assert(0); }
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
   void Visit(const NodeTypeRuntimeArray *)   override { }
   void Visit(const NodeTypeStruct *)         override { }
   void Visit(const NodeTypePointer *)        override { }
   void Visit(const NodeTypeFunction *)       override { }
};

} // namespace bvk
