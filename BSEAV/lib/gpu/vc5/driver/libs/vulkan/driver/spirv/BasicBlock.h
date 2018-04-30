/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "glsl_basic_block.h"
#include "glsl_map.h"

#include "SymbolHandle.h"
#include "ModuleAllocator.h"

namespace bvk {

class Dflow;
class BasicBlockData;

class BasicBlockPool
{
public:
   BasicBlockPool(const spv::ModuleAllocator<uint32_t> &allocator);
   ~BasicBlockPool();

   BasicBlockData *ConstructBlockData() const
   {
      m_list->push_back(glsl_basic_block_construct());
      return spv::ModuleAllocator<BasicBlockData>(m_allocator).New(m_list->back());
   }

   void RetainReachableBlocks(BasicBlock *block);

private:
   spv::list<BasicBlock *>              *m_list;
   const spv::ModuleAllocator<uint32_t> &m_allocator;
};

class BasicBlockHandle
{
public:
   BasicBlockHandle() :
      m_blockData(nullptr)
   {}

   BasicBlockHandle(const BasicBlockPool &pool);

   explicit BasicBlockHandle(BasicBlockData *blockData) :
      m_blockData(blockData)
   {}

   bool IsSameBlock(const BasicBlockHandle other) const;

   BasicBlockData *operator->() { return m_blockData; }
   const BasicBlockData *operator->() const { return m_blockData; }

   explicit operator bool() { return m_blockData != nullptr; }

private:
   BasicBlockData *m_blockData;
};

////////////////////////////////////////////////////////
// BasicBlockData
//
// Wraps the glsl 'C' BasicBlock with a C++ interface
// and extends with extra information needed by the
// dataflow builder
////////////////////////////////////////////////////////

class BasicBlockData
{
public:
   BasicBlockData(BasicBlock *block) :
      m_block(block), // The pool will tidy up any blocks not passed to the back-end compiler
      m_redirect()
   {
   }

   BasicBlockData(const BasicBlockData &) = delete;

   void SetBranchTarget(BasicBlockHandle block)      { m_block->branch_target      = block->m_block; }
   void SetFallthroughTarget(BasicBlockHandle block) { m_block->fallthrough_target = block->m_block; }
   void SetRedirect(BasicBlockHandle next)           { m_redirect                  = next;           }

   Dataflow *GetMemoryAccessChain() const            { return m_block->memory_head;   }
   void SetMemoryAccessChain(Dataflow *dflow)        { m_block->memory_head = dflow;  }

   BasicBlock *GetBlock() const                      { return m_block;  }

   void SetBarrier()
   {
      m_block->barrier = true;
   }

   void PutLoad(SymbolHandle symbol, Dataflow **dflow)
   {
      glsl_map_put(m_block->loads, symbol, dflow);
   }

   void PutScalars(SymbolHandle symbol, Dataflow **dflow)
   {
      glsl_map_put(m_block->scalar_values, symbol, dflow);
   }

   Dataflow **GetScalars(SymbolHandle symbol)
   {
      return static_cast<Dataflow **>(glsl_map_get(m_block->scalar_values, symbol));
   }

   bool IsSameBlock(const BasicBlockData *other) const
   {
      return m_block == other->m_block;
   }

   void             SetControl(const Dflow &cond, BasicBlockHandle branch, BasicBlockHandle fallthrough);
   BasicBlockHandle RedirectedBlock();
   void             DebugPrint() const;

private:
   BasicBlock      *m_block;

   // Link to "return block" when a function is called
   // Needed by "phi" instructions to find the last BasicBlock
   // corresponding to a single SPIRV block
   // This will make a linked list of blocks which could become
   // slow to search -- in that case we should keep a "last" pointer instead
   BasicBlockHandle m_redirect;
};

inline bool BasicBlockHandle::IsSameBlock(const BasicBlockHandle other) const
{
   return m_blockData->IsSameBlock(other.m_blockData);
}


} // namespace bvk
