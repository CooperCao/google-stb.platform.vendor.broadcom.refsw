/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

#include "libs/core/v3d/v3d_align.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::DeviceMemory");

DeviceMemory::DeviceMemory(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkMemoryAllocateInfo    *pAllocateInfo) :
      Allocating(pCallbacks)
{
   VkMemoryType memType = pDevice->GetPhysicalDevice()->GetMemoryType(
                                         pAllocateInfo->memoryTypeIndex);
   m_props = memType.propertyFlags;

   assert(m_props & VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   m_usage = GMEM_USAGE_V3D_RW;

   log_trace("%s: mem = %p size = %zu, mem type index = %u", __FUNCTION__, this,
         (size_t)pAllocateInfo->allocationSize, pAllocateInfo->memoryTypeIndex);

   bool roundToNonCoherentAtomSize = true;
   if (m_props & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
   {
      assert((m_props & VK_MEMORY_PROPERTY_HOST_CACHED_BIT) == 0);
      m_usage |= GMEM_USAGE_COHERENT;
      roundToNonCoherentAtomSize = false;
      log_trace("   Coherent allocation");
   }

   // To avoid the TFU reading into unmapped pages beyond the end of buffers
   // add 64 bytes to the allocation size. See GFXH-1585.
   constexpr size_t padding = 64;

   m_size = (size_t)pAllocateInfo->allocationSize + padding;
   if (roundToNonCoherentAtomSize)
   {
      size_t nonCoherentAtomSize = pDevice->GetPhysicalDevice()->Limits().nonCoherentAtomSize;
      m_size = gfx_zround_up(m_size, nonCoherentAtomSize);
   }

   if (!IsLazy())
   {
      // From the spec "Allocations returned by vkAllocateMemory are guaranteed to meet any
      // alignment requirement by the implementation". So, we use 4KB to be safe.
      constexpr size_t align = V3D_MAX_ALIGN;

      m_handle = gmem_alloc(m_size, align, m_usage, "vkAllocateMemory");

      if (m_handle == nullptr)
         throw bvk::bad_device_alloc();

      log_trace("   Phys = %08X", gmem_get_addr(m_handle));
   }
   else
      log_trace("   Lazy allocation");
}

DeviceMemory::DeviceMemory(
   const VkAllocationCallbacks   *pCallbacks,
   gmem_handle_t                 externalMemory) :
      Allocating(pCallbacks),
      m_props(0)
{
   assert(externalMemory);
   m_handle = externalMemory;
}

DeviceMemory::~DeviceMemory() noexcept
{
   if (m_handle)
      log_trace("Free DeviceMemory : mem = %p, phys = %08X, mapPtr = %p", this,
                gmem_get_addr(m_handle), m_mapPtr);
   gmem_free(m_handle);
}

void DeviceMemory::AllocateLazyMemory()
{
   if (IsLazy() && m_handle == GMEM_HANDLE_INVALID)
   {
      log_trace("Filling lazy allocation of %zu bytes", m_size);

      constexpr size_t align = V3D_MAX_ALIGN; // See comments in constructor

      m_handle = gmem_alloc(m_size, align, m_usage, "vkAllocateLazyMemory");

      if (m_handle == nullptr)
         throw bvk::bad_device_alloc();
   }
}

void DeviceMemory::FreeMemory(
   bvk::Device                   *device,
   const VkAllocationCallbacks   *pAllocator) noexcept
{
   // Destroy ourself
   destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_DEVICE>(this, pAllocator);
}

VkResult DeviceMemory::MapMemory(
   bvk::Device       *device,
   VkDeviceSize       offset,
   VkDeviceSize       size,
   VkMemoryMapFlags   flags,
   void*             *ppData) noexcept
{
   if (m_mapPtr == nullptr)
   {
      m_mapPtr = gmem_map_and_get_ptr(m_handle);
      if (m_mapPtr == nullptr)
         return VK_ERROR_MEMORY_MAP_FAILED;
   }

   log_trace("%s: mem = %p, phys = %08X, mapPtr = %p", __FUNCTION__, this,
             gmem_get_addr(m_handle), m_mapPtr);

   // We can safely ignore size (& VK_WHOLE_SIZE) & flags
   *ppData = (void*)((uintptr_t)m_mapPtr + offset);

   return VK_SUCCESS;
}

void DeviceMemory::UnmapMemory(
   bvk::Device *device) noexcept
{
   log_trace("%s (no-op): mem = %p, phys = %08X, mapPtr = %p", __FUNCTION__, this,
             gmem_get_addr(m_handle), m_mapPtr);

   assert(m_mapPtr != nullptr);
}

void DeviceMemory::GetDeviceMemoryCommitment(
   bvk::Device    *device,
   VkDeviceSize   *pCommittedMemoryInBytes) noexcept
{
   if (m_handle == nullptr)
      *pCommittedMemoryInBytes = 0;
   else
      *pCommittedMemoryInBytes = gmem_get_size(m_handle);
}

static inline size_t GetRangeSize(
   gmem_handle_t              handle,
   const VkMappedMemoryRange  &range)
{
   // From the spec: "size is either the size of range, or VK_WHOLE_SIZE to affect
   // the range from offset to the end of the current mapping of the allocation".
   if (range.size == VK_WHOLE_SIZE)
      return gmem_get_size(handle) - (size_t)range.offset;
   else
      return (size_t)range.size;
}

void DeviceMemory::FlushMappedRange(
   const VkMappedMemoryRange &range) noexcept
{
   if ((m_usage & GMEM_USAGE_COHERENT) == 0)
   {
      log_trace("%s: mem = %p, base_phys = %08X, offset = %zu, size = %zu", __FUNCTION__, this,
                gmem_get_addr(m_handle), (size_t)range.offset, GetRangeSize(m_handle, range));
      if (range.size > 0)
         gmem_flush_mapped_range(m_handle, (size_t)range.offset,
            GetRangeSize(m_handle, range));
   }
}

void DeviceMemory::InvalidateMappedRange(
   const VkMappedMemoryRange &range) noexcept
{
   if ((m_usage & GMEM_USAGE_COHERENT) == 0)
   {
      log_trace("%s: mem = %p, base_phys = %08X, offset = %zu, size = %zu", __FUNCTION__, this,
                gmem_get_addr(m_handle), (size_t)range.offset, GetRangeSize(m_handle, range));
      if (range.size > 0)
         gmem_invalidate_mapped_range(m_handle, (size_t)range.offset,
                                        GetRangeSize(m_handle, range));
   }
}

} // namespace bvk
