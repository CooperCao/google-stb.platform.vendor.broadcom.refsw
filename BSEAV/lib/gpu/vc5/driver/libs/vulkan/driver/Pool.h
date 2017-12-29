/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

template<class T>
class Pool : public Allocating, public NonCopyable
{
public:
   Pool(const VkAllocationCallbacks *pCallbacks) :
      Allocating(pCallbacks),
      m_usedBlocks(GetObjScopeAllocator<T*>()),
      m_freeBlocks(GetObjScopeAllocator<T*>())
   {
   }

   virtual ~Pool()
   {
      CleanupAllBlocks();
   }

   T *AcquireBlock(size_t atLeast = 0)
   {
      T *block = nullptr;

      // See if we have a free block large enough
      for (auto &freeBlock : m_freeBlocks)
      {
         if (freeBlock->Size() >= atLeast)
         {
            // Re-use a free block
            block = freeBlock;
            m_usedBlocks.push_back(block);
            m_freeBlocks.remove(block);
            break;
         }
      }

      if (block == nullptr)
      {
         // Need a new block
         size_t size = std::max(atLeast, (size_t)T::eDefaultBlockSize);
         m_usedBlocks.push_back(nullptr); // May throw OOM, so do up-front
         block = createObject<T, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(GetCallbacks(), nullptr, size);
         m_usedBlocks.back() = block;
      }

      block->Acquire();
      return block;
   }

   void ReleaseBlock(T *block)
   {
      try
      {
         m_freeBlocks.push_back(block);
         m_usedBlocks.remove(block);
         block->Release();
      }
      catch (const std::bad_alloc &)
      {
         // Don't rethrow during destruction
      }
   }

   void CleanupFreeBlocks()
   {
      for (T *block : m_freeBlocks)
         destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(block, GetCallbacks());

      m_freeBlocks.clear();
   }

   void CleanupAllBlocks()
   {
      // There shouldn't be any usedBlocks at this point in normal use, but if we had hit OOM
      // exceptions we may have some. This happens in the CTS tests for example.
      for (T *block : m_usedBlocks)
      {
         if (block != nullptr)
         {
            block->Release();
            destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(block, GetCallbacks());
         }
      }
      m_usedBlocks.clear();

      CleanupFreeBlocks();
   }

protected:
   bvk::list<T*>  m_usedBlocks;
   bvk::list<T*>  m_freeBlocks;
};

} // namespace bvk
