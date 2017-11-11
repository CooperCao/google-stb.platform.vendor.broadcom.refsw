/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

// HeapManager implements an abstract heap without actual memory allocations.
// The heap is treated as a lump of bytes of the given size. All allocations
// are returned relative to 0 (the start of the abstract heap).
class HeapManager : public NonCopyable, public Allocating
{
public:
   enum
   {
      OUT_OF_MEMORY  = -1,
      FRAGMENTED     = -2,
   };

   HeapManager(const VkAllocationCallbacks *pCallbacks, const char *debugName = nullptr) :
      Allocating(pCallbacks),
      m_allChunks(GetAllocator<HeapManager::Chunk>(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT)),
      m_sizeOrderFreeMap(std::less<size_t>(), GetAllocator<std::pair<const size_t,
                         HeapManager::Chunk*>>(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT)),
      m_allocMap(std::less<size_t>(), GetAllocator<std::pair<const size_t,
                 HeapManager::Chunk*>>(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT)),
      m_debugName(debugName)
   {
   }

   void     Initialize(size_t heapBytes, size_t minAlignment);
   size_t   Allocate(size_t numBytes, size_t alignment);
   void     Free(size_t offset);
   void     Reset();

   size_t   BytesFree() const { return m_bytesFree; }
   size_t   BytesUsed() const { return m_bytesUsed; }

private:
   struct Chunk
   {
      Chunk() {}

      Chunk(Chunk *p, Chunk *n, size_t start, size_t size) :
         prev(p), next(n), start(start), size(size) {}

      bool operator==(const Chunk &rhs) const { return this == &rhs; }  // Compare pointers only

      Chunk *prev = nullptr;
      Chunk *next = nullptr;

      size_t start = 0;
      size_t size = 0;
      bool   free = true;
   };

private:
   bool   MergeChunksIfFree(Chunk *c1, Chunk *c2);
   void   RemoveFreeMapChunk(Chunk *c);
   void   Log() const;

private:
   size_t                        m_bytesUsed = 0;
   size_t                        m_bytesFree = 0;
   size_t                        m_alignment = 0;
   bvk::list<Chunk>              m_allChunks;
   bvk::multimap<size_t, Chunk*> m_sizeOrderFreeMap;
   bvk::map<size_t, Chunk*>      m_allocMap;
   const char                   *m_debugName;
};

// MemoryHeap wraps a HeapManager and allows an arbitrary heapBase (of any type).
// Again, no real allocations are done here. You pass the memory for the heap in
// during initialize.
template <typename T>
class MemoryHeap : public NonCopyable
{
public:
   T OUT_OF_MEMORY = (T)HeapManager::OUT_OF_MEMORY;
   T FRAGMENTED    = (T)HeapManager::FRAGMENTED;

   MemoryHeap(const VkAllocationCallbacks *pCallbacks, const char *debugName = nullptr) :
      m_manager(pCallbacks, debugName)
   {
   }

   void Initialize(T heapBase, size_t heapBytes, size_t alignment)
   {
      m_heapBase = heapBase;
      m_manager.Initialize(heapBytes, alignment);
   }

   T Allocate(size_t numBytes, size_t alignment)
   {
      size_t alloc = m_manager.Allocate(numBytes, alignment);

      if (alloc == static_cast<size_t>(m_manager.OUT_OF_MEMORY))
         return OUT_OF_MEMORY;
      else if (alloc == static_cast<size_t>(m_manager.FRAGMENTED))
         return FRAGMENTED;
      else
         return static_cast<T>(alloc + m_heapBase);
   }

   void Free(T block)
   {
      m_manager.Free(static_cast<size_t>(block - m_heapBase));
   }

   void Reset()
   {
      m_manager.Reset();
   }

private:
   HeapManager m_manager;
   T           m_heapBase;
};

} // namespace bvk
