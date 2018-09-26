/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "PoolAllocator.h"

namespace bvk {

class Node;
class NodeLabel;
class NodeFunctionParameter;
class Decoration;

//////////////////////////////////////////////////////////////////////////////////////
// Each node type that needs to store extra data, alongside the SPIRV data, has a data
// class in here. If you need to add extra data to another class, add an entry to the
// needsData table in scripts/spirv/spirv.py and regenerate. You can then simply add
// a new Data* class here which will be contained in the Node.
//////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////
// Data held inside NodeLabel
/////////////////////////////
class DataLabel
{
public:
   DataLabel(const SpvAllocator &allocator) :
      m_instructions(allocator)
   {}

   void AddInstruction(const Node *instruction)
   {
      m_instructions.push_back(instruction);
   }

   const spv::vector<const Node *> &GetInstructions() const
   {
      return m_instructions;
   }

private:
   spv::vector<const Node *>  m_instructions;
};

////////////////////////////////
// Data held inside NodeFunction
////////////////////////////////
class DataFunction
{
public:
   DataFunction(const SpvAllocator &allocator) :
      m_parameters(allocator),
      m_blocks(allocator)
   {}

   void AddParameter(const NodeFunctionParameter *parameter)
   {
      m_parameters.push_back(parameter);
   }

   void AddInstruction(const Node *instruction);

   const spv::vector<const NodeFunctionParameter *> &GetParameters() const
   {
      return m_parameters;
   }

   void StartBlock(const NodeLabel *label)
   {
      m_blocks.push_back(label);
   }

   const spv::vector<const NodeLabel *> &GetBlocks() const
   {
      return m_blocks;
   }

private:
   spv::vector<const NodeFunctionParameter *>  m_parameters;
   spv::vector<const NodeLabel *>              m_blocks;
};

//////////////////////////////////
// Data held inside NodeTypeStruct
//////////////////////////////////
class DataTypeStruct
{
public:
   using DecorationPair = std::pair<uint32_t, const Decoration *>;
   using DecorationList = spv::list<DecorationPair>;

   DataTypeStruct(const SpvAllocator &allocator) :
      m_memberDecorations(allocator)
   {}

   void SetMemberDecoration(uint32_t member, const Decoration *node);

   const DecorationList &GetMemberDecorations() const
   {
      return m_memberDecorations;
   }

private:
   DecorationList m_memberDecorations;
};

} // namespace bvk
