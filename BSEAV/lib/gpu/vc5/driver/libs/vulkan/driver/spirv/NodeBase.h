/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <cassert>
#include <list>
#include "Spirv.h"
#include "ModuleAllocator.h"

namespace bvk
{

class Node;
class NodeType;

class NodeVisitor;
class NodeTypeVisitor;
class Decoration;
class DecorationQuery;
class Extractor;

// Include forward decls for every node type
#include "NodeDecls.auto.inc"

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

//////////////////////////////////////////////
// Node
//
// Common functionality to all concrete nodes
//////////////////////////////////////////////
class Node : public NodeBase
{
public:
   Node() = default;
   Node(const Node &) = delete;

   // Return the instruction set that this node belongs to
   spv::InstructionSet GetInstructionSet() const override
   {
      return spv::InstructionSet::Core;
   }

   // Result Id
   uint32_t GetResultId() const
   {
      return m_resultId;
   }

   const NodeType *GetResultType() const
   {
      return m_resultType;
   }

   bool IsPointer() const;

   template <typename T>
   T TryAs() const
   {
      static_assert(std::is_base_of<NodeBase, typename std::remove_pointer<T>::type>(),
               "TryAs() converts to 'const NodeBase *' derivatives only");
      T ret = dynamic_cast<T>(this);
      return ret;
   }

   template <typename T>
   T As() const
   {
      T ret = TryAs<T>();
      assert(ret != nullptr);
      return ret;
   }

   const spv::list<const Decoration *> *GetDecorations() const
   {
      return m_decorations;
   }

   void SetDecorations(const spv::list<const Decoration *> *decorations)
   {
      m_decorations = decorations;
   }

protected:
   uint32_t        m_resultId    = 0;
   const NodeType *m_resultType  = nullptr;

private:
   // Set by the module after Node has been constructed.
   const spv::list<const Decoration *> *m_decorations = nullptr;
};

using NodeConstPtr = const Node *;

// GLSL450 extended instructions derive from this
class NodeGLSL : public Node
{
public:
   NodeGLSL() = default;

   // Always an OpExtInst
   spv::Core GetOpCode() const override
   {
      return spv::Core::OpExtInst;
   }

   // Return the instruction set that this node belongs to
   spv::InstructionSet GetInstructionSet() const override
   {
      return spv::InstructionSet::GLSL;
   }
};

// Dummy node used for unsupported Ops
class NodeDummy : public Node
{
public:
   NodeDummy(Extractor &extr) : Node()
   {}

   void Accept(NodeVisitor &visitor) const override
   {}

   // Return the opcode within the instruction set for this node
   spv::Core GetOpCode() const override
   {
      return spv::Core::OpNop;
   }
};

///////////////////////////////////////////////////////
// NodeType
//
// Base class for all nodes that represent a SPIRV type
///////////////////////////////////////////////////////
class NodeType : public Node
{
public:
   NodeType() = default;

   virtual void AcceptType(NodeTypeVisitor &visitor) const = 0;

   uint32_t GetTypeId() const
   {
      return m_typeId;
   }

   void SetTypeId(uint32_t id) const
   {
      m_typeId = id;
   }

private:
   // TODO: this could be held in a data field, but the effect is the same
   // TODO: alternatively we could generate the type ids during parsing
   mutable uint32_t m_typeId;
};

} // namespace bvk
