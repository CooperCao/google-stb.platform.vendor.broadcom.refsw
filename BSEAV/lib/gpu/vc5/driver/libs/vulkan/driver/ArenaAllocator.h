/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "Pool.h"

#include <functional>

namespace bvk {

template<class T, typename Res>
class ArenaAllocator : public Allocating, public NonCopyable
{
public:
   // Create an arena allocator and an underlying pool
   ArenaAllocator(const VkAllocationCallbacks *pCallbacks) :
      Allocating(pCallbacks),
      m_blocks(GetObjScopeAllocator<T*>()),
      m_fullBlocks(GetObjScopeAllocator<T*>())
   {
      m_pool = createObject<Pool<T>, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
                                          GetCallbacks(), GetCallbacks());
      m_ourPool = true;
   }

   // Create an arena allocator against an existing pool
   ArenaAllocator(Pool<T> *pool, const VkAllocationCallbacks *pCallbacks) :
      Allocating(pCallbacks),
      m_pool(pool),
      m_ourPool(false),
      m_blocks(GetObjScopeAllocator<T*>()),
      m_fullBlocks(GetObjScopeAllocator<T*>())
   {
   }

   virtual ~ArenaAllocator()
   {
      FreeAll();

      if (m_ourPool)
         destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(m_pool, GetCallbacks());
   }

   void FreeAll()
   {
      for (T *block : m_blocks)
         m_pool->ReleaseBlock(block);

      for (T *block : m_fullBlocks)
         m_pool->ReleaseBlock(block);

      m_blocks.clear();
      m_fullBlocks.clear();
      m_lastBlock = nullptr;
      m_smallestAllocSeen = ~0;
   }

   void Allocate(Res *result, size_t size, size_t align)
   {
      m_smallestAllocSeen = std::min(m_smallestAllocSeen, size);

      if (m_lastBlock == nullptr)
         NewBlock(size + align);

      if (!m_lastBlock->Allocate(result, size, align))
      {
         // Won't fit in current block
         if (size + align > T::eDefaultBlockSize)
         {
            // Over-sized, make a new block (which will be full by definition)
            T *oldLastBlock = m_lastBlock;
            NewBlock(size + align);
            bool ok = m_lastBlock->Allocate(result, size, align);   // Must fit (or throw)
            assert(ok);
            m_fullBlocks.push_back(m_lastBlock);
            m_blocks.remove(m_lastBlock);
            m_lastBlock = oldLastBlock;
         }
         else
         {
            // Try to find a block it might fit in
            for (T *b : m_blocks)
               if (b->Allocate(result, size, align))
                  return;

            SweepFullBlocks();

            // No blocks fit, make a new one
            NewBlock(size + align);
            bool ok = m_lastBlock->Allocate(result, size, align);   // Must fit (or throw)
            assert(ok);
         }
      }
   }

   // Keep track of what you would have deleted. Useful for stats
   void RecordWastedDelete(size_t bytes)
   {
      m_wastedDeletedBytes += bytes;
   }

   // Visit all blocks calling the method given in fn
   void VisitBlocks(const std::function<void(T*)>& fn)
   {
      for (T *block : m_fullBlocks)
         fn(block);

      for (T *block : m_blocks)
         fn(block);
   }

   void GetUsageData(size_t *numBlocks, size_t *capacity, size_t *used, size_t *wastedDeletedBytes) const
   {
      *numBlocks = m_fullBlocks.size() + m_blocks.size();
      *capacity = 0;
      *used = 0;
      *wastedDeletedBytes = m_wastedDeletedBytes;

      for (T *block : m_fullBlocks)
      {
         *capacity += block->Size();
         *used += block->Used();
      }

      for (T *block : m_blocks)
      {
         *capacity += block->Size();
         *used += block->Used();
      }
   }

private:
   void NewBlock(size_t atLeast)
   {
      // Get a new block
      T *block = m_pool->AcquireBlock(atLeast); // Will throw on alloc failure
      m_blocks.push_back(block);
      m_lastBlock = block;
   }

   void SweepFullBlocks()
   {
      // Move any full blocks into the m_fullBlocks list so we don't keep
      // checking them
      auto iter = m_blocks.begin();
      while (iter != m_blocks.end())
      {
         T *b = *iter;
         size_t freeBytes = b->Size() - b->Used();
         if (m_smallestAllocSeen != ~0U && freeBytes < m_smallestAllocSeen && freeBytes < 256U)
         {
            m_fullBlocks.push_back(b);
            iter = m_blocks.erase(iter);
         }
         else
            ++iter;
      }
   }

protected:
   Pool<T>       *m_pool = nullptr;
   bool           m_ourPool = false;
   bvk::list<T*>  m_blocks;
   bvk::list<T*>  m_fullBlocks;
   T             *m_lastBlock = nullptr;
   size_t         m_smallestAllocSeen = ~0;
   size_t         m_wastedDeletedBytes = 0;
};

} // namespace bvk
