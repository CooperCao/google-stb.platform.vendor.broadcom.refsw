/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "HeapManager.h"

namespace bvk {

class DescriptorPool: public NonCopyable, public Allocating
{
// NOTE: constexpr alignment attributes are not supported by the gcc 4.8 toolchain.
#define SysMemAlignment sizeof(uintptr_t)

public:
   DescriptorPool(
      const VkAllocationCallbacks      *pCallbacks,
      bvk::Device                      *pDevice,
      const VkDescriptorPoolCreateInfo *pCreateInfo);

   ~DescriptorPool() noexcept;

   VkResult ResetDescriptorPool(
      bvk::Device                *device,
      VkDescriptorPoolResetFlags  flags) noexcept;

   VkResult FreeDescriptorSets(
      bvk::Device             *device,
      uint32_t                 descriptorSetCount,
      const VkDescriptorSet   *pDescriptorSets) noexcept;

   // Implementation specific from this point on
   VkResult AllocateDescriptorSets(const VkDescriptorSetAllocateInfo *pAllocateInfo,
                                   VkDescriptorSet *pDescriptorSets) noexcept;

   static size_t CalcDescriptorTypeBytes(VkDescriptorType type);
   static size_t CalcDescriptorTypeDevMemBytes(VkDescriptorType type);
   static size_t GetDevMemRecordSize();

   // All descriptor types must be aligned for the allocator to work
   struct ALIGNED(SysMemAlignment) ImageInfo {
      uint32_t   samplerParam;
      ImageView *imageView;
   };

   typedef uint32_t ALIGNED(SysMemAlignment) TexelBufferSize;

   struct ALIGNED(SysMemAlignment) BufferInfo {
      v3d_addr_t addr;
      uint32_t   size;
      uint32_t   range;
   };

private:
   void Allocate(const DescriptorSetLayout &layout, VkDescriptorSet *outputSet);
   void Free(VkDescriptorSet set);
   void ReleaseAllocatedSets();

private:
   uint8_t                      *m_sysMem = nullptr;
   gmem_handle_t                 m_devMem = GMEM_HANDLE_INVALID;

   MemoryHeap<uint8_t*>          m_sysMemHeap;
   size_t                        m_sysMemHeapBytes = 0;

   MemoryHeap<size_t>            m_devMemHeap;
   size_t                        m_devMemHeapBytes = 0;

   // We have to keep a list of active DescriptorSets that have been allocated.
   // The descriptorSet needs to hold most of the data from the descriptorSetLayout since
   // the layout lifetime isn't guaranteed. That means that we have variable data in the
   // descriptorSet that must be released when it goes away. Since the sets are placement
   // newed into a sys-mem block that won't happen automatically, so we need this.
   bvk::set<DescriptorSet*>      m_allocatedSets;
};

} // namespace bvk
