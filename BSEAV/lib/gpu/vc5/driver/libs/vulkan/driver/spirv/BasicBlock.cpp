/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "BasicBlock.h"
#include "SymbolHandle.h"
#include "Dflow.h"

#include "libs/util/log/log.h"
#include "glsl_dataflow_print.h"

LOG_DEFAULT_CAT("bvk::comp::BasicBlock");

namespace bvk {

///////////////////////////////////////////////////////////////////////////////////////////////////

BasicBlockPool::BasicBlockPool(const SpvAllocator &allocator) :
   m_allocator(allocator)
{
   m_list = allocator.New<spv::list<BasicBlock *>>(allocator);
}

BasicBlockPool::~BasicBlockPool()
{
   for (BasicBlock *block : *m_list)
      glsl_basic_block_delete(block);
}

void BasicBlockPool::RetainReachableBlocks(BasicBlock *entry)
{
   spv::set<BasicBlock *>    used(std::less<BasicBlock *>(), m_allocator);
   spv::vector<BasicBlock *> stack(m_allocator);

   if (entry != nullptr)
      stack.push_back(entry);

   while (stack.size() != 0)
   {
      BasicBlock  *tos = stack.back();
      stack.pop_back();

      used.insert(tos);

      if (tos->branch_target != nullptr)
         if (used.find(tos->branch_target) == used.end())
            stack.push_back(tos->branch_target);

      if (tos->fallthrough_target != nullptr)
         if (used.find(tos->fallthrough_target) == used.end())
            stack.push_back(tos->fallthrough_target);
   }

   m_list->remove_if([&used](BasicBlock *block)
                     {
                        return used.find(block) != used.end();
                     });
}

///////////////////////////////////////////////////////////////////////////////////////////////////

void BasicBlockData::SetControl(const Dflow &cond, BasicBlockHandle branch, BasicBlockHandle fallthrough)
{
   m_block->branch_cond        = cond;
   m_block->branch_target      = branch->m_block;
   m_block->fallthrough_target = fallthrough->m_block;
}

BasicBlockHandle BasicBlockData::RedirectedBlock()
{
   BasicBlockHandle last(this);

   while (last->m_redirect)
      last = last->m_redirect;

   return last;
}

void DebugSymDataflowMap(Map *map)
{
   GLSL_MAP_FOREACH(e, map)
   {
      auto k = static_cast<const Symbol *>(e->k);
      auto v = static_cast<Dataflow **>(e->v);

      SymbolHandle(const_cast<Symbol *>(k)).DebugPrint();

      for (uint32_t i = 0; i < k->type->scalar_count; i++)
      {
         glsl_print_dataflow(stderr, v[i]);
         fprintf(stderr, "\n");
      }
   }
}

void BasicBlockData::DebugPrint() const
{
   if (log_trace_enabled())
   {
      // Debug the load map
      log_trace("LOADS");
      DebugSymDataflowMap(m_block->loads);

      // Debug the scalar_values map
      log_trace("SCALAR_VALUES");
      DebugSymDataflowMap(m_block->scalar_values);
   }
}

///////////////////////////////////////////////////////////////////////////////////////////////////

BasicBlockHandle::BasicBlockHandle(const BasicBlockPool &pool):
   m_blockData(pool.ConstructBlockData())
{
}

///////////////////////////////////////////////////////////////////////////////////////////////////

} // namespace bvk
