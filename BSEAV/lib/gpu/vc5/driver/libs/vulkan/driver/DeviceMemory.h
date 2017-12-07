/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

#include "libs/platform/gmem.h"

namespace bvk {

class DeviceMemory: public NonCopyable, public Allocating
{
public:
   DeviceMemory(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkMemoryAllocateInfo    *pAllocateInfo);

   DeviceMemory(
      const VkAllocationCallbacks   *pCallbacks,
      gmem_handle_t                 externalMemory);

   ~DeviceMemory() noexcept;

   void FreeMemory(
      bvk::Device                   *device,
      const VkAllocationCallbacks   *pAllocator) noexcept;

   VkResult MapMemory(
      bvk::Device       *device,
      VkDeviceSize       offset,
      VkDeviceSize       size,
      VkMemoryMapFlags   flags,
      void*             *ppData) noexcept;

   void UnmapMemory(
      bvk::Device *device) noexcept;

   void GetDeviceMemoryCommitment(
      bvk::Device    *device,
      VkDeviceSize   *pCommittedMemoryInBytes) noexcept;

   bool IsLazy() const { return m_props & VK_MEMORY_PROPERTY_LAZILY_ALLOCATED_BIT; }

   // Implementation specific from this point on
public:
   void AllocateLazyMemory();
   void FlushMappedRange(const VkMappedMemoryRange &range) noexcept;
   void InvalidateMappedRange(const VkMappedMemoryRange &range) noexcept;

   gmem_handle_t Handle() const noexcept { return m_handle; }
   v3d_addr_t    PhysAddr() const noexcept { return gmem_get_addr(m_handle); }

private:
   gmem_handle_t           m_handle = nullptr;
   void                   *m_mapPtr = nullptr;
   VkMemoryPropertyFlags   m_props;
   size_t                  m_size = 0;
   gmem_usage_flags_t      m_usage = GMEM_USAGE_NONE;

};

} // namespace bvk
