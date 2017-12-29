/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>
#include <memory>
#include <mutex>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "HeapManager.h"
#include "DevMemRange.h"
#include "Common.h"

namespace bvk {

// Encapsulates a HeapManager and the device memory to back it.
// A list of these is held by DevMemHeap below
class DevMemHeapItem
{
public:
   DevMemHeapItem(size_t heapSize, size_t minAlignment,
                  const VkAllocationCallbacks *pCallbacks, const char *debugName) :
      m_heap(pCallbacks, debugName)
   {
      m_handle = gmem_alloc_and_map(heapSize, minAlignment, GMEM_USAGE_V3D_RW, "DevMemHeapItem");
      if (m_handle == GMEM_HANDLE_INVALID)
         throw bad_device_alloc();

      m_heap.Initialize(heapSize, minAlignment);
   }

   ~DevMemHeapItem()
   {
      gmem_free(m_handle);
   }

   bool operator==(const DevMemHeapItem &rhs) const
   {
      return m_handle == rhs.m_handle;
   }

   bool Allocate(DevMemRange *range, size_t numBytes, size_t alignment)
   {
      size_t allocRet = m_heap.Allocate(numBytes, alignment);
      if (allocRet != static_cast<size_t>(m_heap.OUT_OF_MEMORY) &&
          allocRet != static_cast<size_t>(m_heap.FRAGMENTED))
      {
         range->Set(m_handle, allocRet, numBytes);
         return true;
      }
      return false;
   }

   bool Free(const DevMemRange &range)
   {
      if (m_handle == range.Handle())
      {
         m_heap.Free(range.Offset());
         return true;
      }
      return false;
   }

   bool Unused() const
   {
      return m_heap.BytesUsed() == 0;
   }

private:
   HeapManager     m_heap;
   gmem_handle_t   m_handle;
};

// DevMemHeap behaves like a general heap for device memory.
// It is only really intended for smallish allocations that would be wasteful to have
// an entire 64KB page each. For example, the small allocations inside Pipeline objects.
// DevMemHeap must also be thread-safe, so shouldn't be used in time-critical paths.
//
// Internally, we just maintain a list of heaps. A new heap is added whenever we fail
// to allocate from the existing ones. We don't expect many allocations so we will initially
// avoid any complex heap management.
class DevMemHeap : public NonCopyable, public Allocating
{
public:
   enum
   {
      DEV_MEM_HEAP_SIZE = 64 * 1024,   // 64KB per heap seems to be plenty
      DEV_MEM_ALIGNMENT = 32
   };

   DevMemHeap(const VkAllocationCallbacks *pCallbacks, const char *debugName = nullptr);
   ~DevMemHeap();

   bool Allocate(DevMemRange *range, size_t numBytes, size_t alignment);
   void Free(const DevMemRange &range);

private:
   std::mutex                  m_mutex;
   const char                 *m_debugName;

   bvk::list<DevMemHeapItem>   m_heaps;
};

} // namespace bvk
