/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "DevMemHeap.h"

namespace bvk {

DevMemHeap::DevMemHeap(const VkAllocationCallbacks *pCallbacks, const char *debugName) :
   Allocating(pCallbacks),
   m_debugName(debugName),
   m_heaps(GetAllocator<DevMemHeapItem>(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE))
{
}

DevMemHeap::~DevMemHeap()
{
   std::lock_guard<std::mutex> guard(m_mutex);

   // At most one heap should exist at destruction and it should be unused
   assert(m_heaps.size() <= 1);
   for (DevMemHeapItem &heap : m_heaps)
      assert(heap.Unused());
}

bool DevMemHeap::Allocate(DevMemRange *range, size_t numBytes, size_t alignment)
{
   std::lock_guard<std::mutex> guard(m_mutex);

   if (numBytes == 0)
   {
      *range = DevMemRange(); // Set the invalid range
      return true;
   }

   assert(alignment <= DEV_MEM_ALIGNMENT);

   try
   {
      // Is the requested allocation larger than our standard heap size?
      if (numBytes > DEV_MEM_HEAP_SIZE)
      {
         // Make a heap at least as big as the allocation requested
         m_heaps.emplace_front(gfx_zround_up(numBytes, alignment), DEV_MEM_ALIGNMENT,
                               GetCallbacks(), m_debugName);
         return m_heaps.front().Allocate(range, numBytes, alignment);
      }

      assert(numBytes <= DEV_MEM_HEAP_SIZE);

      // Try each heap in order - newer heaps are at the front
      for (DevMemHeapItem &heap : m_heaps)
      {
         if (heap.Allocate(range, numBytes, alignment))
            return true;
      }

      // No existing heap can supply the memory, make a new one and allocate from it.
      // This cannot be over-size, since we dealt with that above
      m_heaps.emplace_front(DEV_MEM_HEAP_SIZE, DEV_MEM_ALIGNMENT, GetCallbacks(), m_debugName);
      if (m_heaps.front().Allocate(range, numBytes, alignment))
         return true;

      // Cannot allocate at all
      return false;
   }
   catch (const bad_device_alloc&)
   {
      return false;
   }

   return true;
}


void DevMemHeap::Free(const DevMemRange &range)
{
   if (!range.IsValid())
      return;

   std::lock_guard<std::mutex> guard(m_mutex);

   // Find the heap that has the handle and free it
   for (DevMemHeapItem &heap : m_heaps)
   {
      if (heap.Free(range))
      {
         // Remove heap if it's now unused (and isn't the only one)
         if (m_heaps.size() > 1 && heap.Unused())
            m_heaps.remove(heap);

         return;
      }
   }

   // We didn't find a heap which contained that range
   assert(0);
}

} // namespace bvk
