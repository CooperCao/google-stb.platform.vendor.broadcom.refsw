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
#include "NodeIndex.h"
#include "Extractor.h"
#include "ModuleAllocator.h"
#include "NodeData.h"
#include "Decoration.h"
#include "ImageOperands.h"

#ifdef WIN32
#undef GetObject
#endif

namespace bvk
{

class DflowBuilder;
class Module;
class Extractor;
class Decoration;

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

protected:
   uint32_t        m_resultId   = 0;
   const NodeType *m_resultType = nullptr;
};

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

   const NodeIndex &GetEntryPoint() const
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
   NodeIndex               m_entryPoint;
   spv::ExecutionMode      m_mode;
   spv::vector<uint32_t>   m_literals;
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

   spv::Core GetOperation() const                     { return m_opcode;    }
   const spv::vector<NodeIndex> &GetOperands() const  { return m_operands;  }
   const spv::vector<uint32_t> &GetIndices() const    { return m_indices;   }
   const NodeIndex &GetVector1() const                { return m_vector1;   }
   const NodeIndex &GetVector2() const                { return m_vector2;   }
   const NodeIndex &GetObject() const                 { return m_object;    }
   const NodeIndex &GetComposite() const              { return m_composite; }

private:
   spv::Core                 m_opcode;
   spv::vector<NodeIndex>    m_operands;  // Operands for non-vector instructions

   // Vector op args
   NodeIndex                 m_vector1;   // For shuffle
   NodeIndex                 m_vector2;   // For shuffle
   NodeIndex                 m_object;    // for insert
   NodeIndex                 m_composite; // For insert and extract

   spv::vector<uint32_t>     m_indices;   // Common to vector ops
};

class NodeAccessChain : public Node
{
public:
   NodeAccessChain(Extractor &ext, bool inBounds)
      : Node(),
        m_indices(ext.GetAllocator()),
        m_inBounds(inBounds)
   {
      ext >> m_resultType >> m_resultId >> m_base >> m_indices;
   }

   void Accept(NodeVisitor &visitor) const override
   {
      visitor.Visit(this);
   }

   const NodeIndex &GetBase() const
   {
      return m_base;
   }

   const spv::vector<NodeIndex> &GetIndices() const
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
   NodeIndex                 m_base;
   spv::vector<NodeIndex>    m_indices;
   bool                      m_inBounds;
};

} // namespace bvk
