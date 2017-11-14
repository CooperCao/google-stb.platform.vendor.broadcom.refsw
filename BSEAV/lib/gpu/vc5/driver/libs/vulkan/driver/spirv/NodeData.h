/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "ModuleAllocator.h"

namespace bvk {

class Node;
class NodeLabel;
class NodeMemberDecorate;
class NodeExecutionMode;
class NodeFunctionParameter;
class NodeEntryPoint;
class NodeFunction;
class NodeTypeStruct;
class NodeVariable;
class Decoration;

//////////////////////////////////////////////////////////////////////////////////////
// Each node type that needs to store extra data, alongside the SPIRV data, has a data
// class in here. If you need to add extra data to another class, add an entry to the
// needsData table in scripts/spirv/spirv.py and regenerate. You can then simply add
// a new Data* class here which will be contained in the Node.
//////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////
// Data held inside NodeEntryPoint
//////////////////////////////////
class DataEntryPoint
{
public:
   DataEntryPoint(spv::ModuleAllocator<uint32_t> &arenaAllocator) :
      m_modes(arenaAllocator)
   {}

   void AddMode(const NodeExecutionMode *mode)
   {
      m_modes.push_back(mode);
   }

   const spv::list<const NodeExecutionMode *> &GetModes() const
   {
      return m_modes;
   }

   void SetId(uint32_t id)
   {
      m_id = id;
   }

   uint32_t GetId() const
   {
      return m_id;
   }

private:
   spv::list<const NodeExecutionMode *>   m_modes;
   uint32_t                               m_id = ~0u;
};

/////////////////////////////
// Data held inside NodeLabel
/////////////////////////////
class DataLabel
{
public:
   DataLabel(spv::ModuleAllocator<uint32_t> &arenaAllocator) :
      m_instructions(arenaAllocator)
   {}

   void AddInstruction(const Node *instruction)
   {
      m_instructions.push_back(instruction);
   }

   const spv::vector<const Node *> &GetInstructions() const
   {
      return m_instructions;
   }

   void SetBlockId(uint32_t id)
   {
      m_id = id;
   }

   uint32_t GetBlockId() const
   {
      return m_id;
   }

private:
   uint32_t                   m_id;
   spv::vector<const Node *>  m_instructions;
};

////////////////////////////////
// Data held inside NodeFunction
////////////////////////////////
class DataFunction
{
public:
   DataFunction(spv::ModuleAllocator<uint32_t> &arenaAllocator) :
      m_parameters(arenaAllocator),
      m_blocks(arenaAllocator)
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

   void SetId(uint32_t id)
   {
      m_id = id;
   }

   uint32_t GetId() const
   {
      return m_id;
   }

private:
   spv::vector<const NodeFunctionParameter *>  m_parameters;
   spv::vector<const NodeLabel *>              m_blocks;
   uint32_t                                    m_id = ~0u;
};

//////////////////////////////////
// Data held inside NodeTypeStruct
//////////////////////////////////
class DataTypeStruct
{
public:
   using DecorationPair = std::pair<uint32_t, const Decoration *>;
   using DecorationList = spv::list<DecorationPair>;

   DataTypeStruct(spv::ModuleAllocator<uint32_t> &arenaAllocator) :
      m_memberDecorations(arenaAllocator)
   {}

   void SetMemberDecoration(uint32_t member, const Decoration *node);

   const DecorationList &GetMemberDecorations() const
   {
      return m_memberDecorations;
   }

private:
   DecorationList m_memberDecorations;
};

//////////////////////////////////////
// Data held inside NodeMemberDecorate
//////////////////////////////////////
class DataMemberDecorate
{
public:
   DataMemberDecorate(spv::ModuleAllocator<uint32_t> &arenaAllocator) :
      m_literals(arenaAllocator)
   {}

   const spv::vector<uint32_t> &GetLiterals() const
   {
      return m_literals;
   }

private:
   spv::vector<uint32_t>   m_literals;
};

////////////////////////////////
// Data held inside NodeVariable
////////////////////////////////
class DataVariable
{
public:
   DataVariable(spv::ModuleAllocator<uint32_t> &arenaAllocator)
   {}

   void SetId(uint32_t id)
   {
      m_id = id;
   }

   uint32_t GetId() const
   {
      return m_id;
   }

private:
   uint32_t    m_id = ~0u;    // Id is used when this node is gathered into an array
};

} // namespace bvk
