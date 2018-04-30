/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

#include <utility>

#include "libs/util/log/log.h"
#include "libs/core/v3d/v3d_align.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::DescriptorPool");

// Calculate the largest dev mem record size we will need.
// All records will be this size to ensure that alignment and size constraints are
// easily met. If this ever becomes a memory limitation (which I highly doubt) we
// could extend the HeapManager to handle per-alloc alignments.
constexpr size_t DevMemRecordSize =
      GFX_MAX(V3D_TMU_TEX_STATE_ALIGN,
      GFX_MAX(V3D_TMU_EXTENDED_TEX_STATE_ALIGN,
      GFX_MAX(V3D_TMU_SAMPLER_ALIGN,
      GFX_MAX(V3D_TMU_EXTENDED_SAMPLER_ALIGN,
      GFX_MAX(V3D_TMU_SAMPLER_PACKED_SIZE,
              V3D_TMU_TEX_STATE_PACKED_SIZE + V3D_TMU_TEX_EXTENSION_PACKED_SIZE
   )))));

constexpr size_t DevMemAlignment = DevMemRecordSize;

static const size_t DescriptorTypeSysMemSize[VK_DESCRIPTOR_TYPE_RANGE_SIZE] = {
   sizeof(DescriptorPool::ImageInfo),           // VK_DESCRIPTOR_TYPE_SAMPLER
   sizeof(DescriptorPool::ImageInfo),           // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER
   sizeof(DescriptorPool::ImageInfo),           // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
   sizeof(DescriptorPool::ImageInfo),           // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
   sizeof(DescriptorPool::TexelBufferSize),     // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
   sizeof(DescriptorPool::TexelBufferSize),     // VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
   sizeof(DescriptorPool::BufferInfo),          // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
   sizeof(DescriptorPool::BufferInfo),          // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
   sizeof(DescriptorPool::BufferInfo),          // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
   sizeof(DescriptorPool::BufferInfo),          // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
   sizeof(DescriptorPool::ImageInfo),           // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
};

// v4 h/w has separate sampler and texture records
static const size_t DescriptorTypeDevMemSize[VK_DESCRIPTOR_TYPE_RANGE_SIZE] = {
   DevMemRecordSize,         // VK_DESCRIPTOR_TYPE_SAMPLER
   DevMemRecordSize * 2,     // VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER (Separate tex & sampler records)
   DevMemRecordSize,         // VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE
   DevMemRecordSize,         // VK_DESCRIPTOR_TYPE_STORAGE_IMAGE
   DevMemRecordSize,         // VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER
   DevMemRecordSize,         // VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER
   0,                        // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER
   0,                        // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
   0,                        // VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC
   0,                        // VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC
   DevMemRecordSize,         // VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT
};


size_t DescriptorPool::CalcDescriptorTypeBytes(VkDescriptorType type)
{
   assert(type <= VK_DESCRIPTOR_TYPE_END_RANGE);
   // Ensure we account for the alignment
   return gfx_zround_up(DescriptorTypeSysMemSize[type], SysMemAlignment);
}

size_t DescriptorPool::CalcDescriptorTypeDevMemBytes(VkDescriptorType type)
{
   assert(type <= VK_DESCRIPTOR_TYPE_END_RANGE);
   return DescriptorTypeDevMemSize[type];
}

size_t DescriptorPool::GetDevMemRecordSize()
{
   return DevMemRecordSize;
}

DescriptorPool::DescriptorPool(
   const VkAllocationCallbacks      *pCallbacks,
   bvk::Device                      *pDevice,
   const VkDescriptorPoolCreateInfo *pCreateInfo) :
      Allocating(pCallbacks),
      m_sysMemHeap(pCallbacks, "DescPool SysMem Heap"),
      m_devMemHeap(pCallbacks, "DescPool DevMem Heap"),
      m_allocatedSets(std::less<DescriptorSet*>(), GetObjScopeAllocator<DescriptorSet*>())
{
   try
   {
      // Calculate the total system memory and device memory sizes of the pool
      m_sysMemHeapBytes = pCreateInfo->maxSets * sizeof(DescriptorSet);
      m_devMemHeapBytes = 0;

      for (uint32_t i = 0; i < pCreateInfo->poolSizeCount; i++)
      {
         const auto &sizeInfo = pCreateInfo->pPoolSizes[i];
         m_sysMemHeapBytes += sizeInfo.descriptorCount * CalcDescriptorTypeBytes(sizeInfo.type);
         m_devMemHeapBytes += sizeInfo.descriptorCount * CalcDescriptorTypeDevMemBytes(sizeInfo.type);
      }

      log_trace("Construct descriptor pool %p (sysMemBytes=%zu, devMemBytes=%zu)",
                this, m_sysMemHeapBytes, m_devMemHeapBytes);

      // Allocate system memory
      if (m_sysMemHeapBytes > 0)
      {
         auto byteAllocator = GetAllocator<uint8_t>(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
         m_sysMem = byteAllocator.allocate(m_sysMemHeapBytes);

         // Initialize heap manager with the allocated memory
         m_sysMemHeap.Initialize(m_sysMem, m_sysMemHeapBytes, SysMemAlignment);
      }

      // Allocate device memory
      if (m_devMemHeapBytes > 0)
      {
         m_devMem = gmem_alloc_and_map(m_devMemHeapBytes, DevMemAlignment, GMEM_USAGE_V3D_READ,
                                       "DescriptorPool");
         if (m_devMem == GMEM_HANDLE_INVALID)
            throw bvk::bad_device_alloc();

         // Initialize device heap manager with the allocated memory
         m_devMemHeap.Initialize(0, m_devMemHeapBytes, DevMemAlignment);
      }
   }
   catch (...)
   {
      log_warn("Construct descriptor pool FAILED");

      if (m_sysMem != nullptr)
      {
         auto byteAllocator = GetAllocator<uint8_t>(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
         byteAllocator.deallocate(m_sysMem, m_sysMemHeapBytes);
      }

      if (m_devMem != nullptr)
         gmem_free(m_devMem);

      throw;
   }
}

DescriptorPool::~DescriptorPool() noexcept
{
   log_trace("Destruct descriptor pool %p", this);

   ReleaseAllocatedSets();

   if (m_sysMem != nullptr)
   {
      auto byteAllocator = GetAllocator<uint8_t>(VK_SYSTEM_ALLOCATION_SCOPE_OBJECT);
      byteAllocator.deallocate(m_sysMem, m_sysMemHeapBytes);
   }

   if (m_devMem != nullptr)
      gmem_free(m_devMem);
}

void DescriptorPool::Allocate(const DescriptorSetLayout &layout, VkDescriptorSet *outputSet)
{
   size_t numBytes = sizeof(DescriptorSet) + layout.State()->BytesRequired();
   size_t devBytes = layout.State()->DeviceBytesRequired();

   log_trace("DescriptorPool %p Allocate (sysBytes=%zu, devBytes=%zu)", this, numBytes, devBytes);

   uint8_t *ptr = m_sysMemHeap.Allocate(numBytes, SysMemAlignment);
   if (ptr == m_sysMemHeap.OUT_OF_MEMORY)
   {
      log_warn("DescriptorPool %p Allocate failed : VK_ERROR_OUT_OF_POOL_MEMORY_KHR", this);
      throw VK_ERROR_OUT_OF_POOL_MEMORY_KHR;
   }
   else if (ptr == m_sysMemHeap.FRAGMENTED)
   {
      log_warn("DescriptorPool %p Allocate failed : VK_ERROR_FRAGMENTED_POOL (system)", this);
      throw VK_ERROR_FRAGMENTED_POOL;
   }

   size_t devHeapOffset = 0;
   if (devBytes > 0)
   {
      devHeapOffset = m_devMemHeap.Allocate(devBytes, DevMemAlignment);
      if (devHeapOffset == m_devMemHeap.OUT_OF_MEMORY)
      {
         log_warn("DescriptorPool %p Allocate failed : VK_ERROR_OUT_OF_POOL_MEMORY_KHR", this);
         m_sysMemHeap.Free(ptr);
         throw VK_ERROR_OUT_OF_POOL_MEMORY_KHR;
      }
      else if (devHeapOffset == m_devMemHeap.FRAGMENTED)
      {
         log_warn("DescriptorPool %p Allocate failed : VK_ERROR_FRAGMENTED_POOL (device)", this);
         m_sysMemHeap.Free(ptr);
         throw VK_ERROR_FRAGMENTED_POOL;
      }
   }

   DescriptorSet *ds = new (ptr) DescriptorSet(&layout, ptr + sizeof(DescriptorSet), numBytes,
                                               m_devMem, devHeapOffset, devBytes);

   try
   {
      m_allocatedSets.insert(ds);
   }
   catch (const std::bad_alloc&)
   {
      log_warn("DescriptorPool %p m_allocatedSets insert failed : VK_ERROR_OUT_OF_HOST_MEMORY", this);
      ds->~DescriptorSet();   // Do this first, since the Free calls below might also throw
      m_devMemHeap.Free(devHeapOffset);
      m_sysMemHeap.Free(ptr);
      throw VK_ERROR_OUT_OF_HOST_MEMORY;
   }

   *outputSet = toHandle<VkDescriptorSet>(ds);
}

void DescriptorPool::Free(VkDescriptorSet set)
{
   if (set == VK_NULL_HANDLE)
      return;

   DescriptorSet *ds = fromHandle<DescriptorSet>(set);

   m_allocatedSets.erase(ds); // Remove from allocated list
   ds->~DescriptorSet();      // Call the destructor

   size_t devOffset = 0, devSize = 0;
   if (ds->GetDevMemDetails(&devOffset, &devSize))
      m_devMemHeap.Free(devOffset);

   m_sysMemHeap.Free(reinterpret_cast<uint8_t*>(ds));
}

VkResult DescriptorPool::AllocateDescriptorSets(
   const VkDescriptorSetAllocateInfo   *pAllocateInfo,
   VkDescriptorSet                     *pDescriptorSets) noexcept
{
   VkResult res = VK_SUCCESS;

   memset(pDescriptorSets, 0, pAllocateInfo->descriptorSetCount * sizeof(VkDescriptorSet));

   try
   {
      for (uint32_t s = 0; s < pAllocateInfo->descriptorSetCount; s++)
      {
         DescriptorSetLayout *layout = fromHandle<DescriptorSetLayout>(pAllocateInfo->pSetLayouts[s]);
         Allocate(*layout, &pDescriptorSets[s]);
      }
   }
   catch (const std::bad_alloc &)
   {
      res = VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      res = e; // Could be VK_ERROR_OUT_OF_POOL_MEMORY_KHR or VK_ERROR_FRAGMENTED_POOL
   }

   if (res != VK_SUCCESS)
   {
      // KHR_maintenance1 states we must clean up anything we successfully made
      try
      {
         for (uint32_t s = 0; s < pAllocateInfo->descriptorSetCount; s++)
            Free(pDescriptorSets[s]);
      }
      catch (const std::bad_alloc &)
      {
         // Free tries to add something to a 'free list' so can actually throw bad_alloc.
         // There are no handlers above this, so trap it here
         res = VK_ERROR_OUT_OF_HOST_MEMORY;
      }

      memset(pDescriptorSets, 0, pAllocateInfo->descriptorSetCount * sizeof(VkDescriptorSet));
   }

   return res;
}


void DescriptorPool::ReleaseAllocatedSets()
{
   // Cleanup the allocated descriptor sets
   for (auto ds : m_allocatedSets)
      ds->~DescriptorSet();      // Call the destructor
   m_allocatedSets.clear();
}

VkResult DescriptorPool::ResetDescriptorPool(
   bvk::Device                *device,
   VkDescriptorPoolResetFlags  flags) noexcept
{
   ReleaseAllocatedSets();

   m_sysMemHeap.Reset();
   m_devMemHeap.Reset();

   return VK_SUCCESS;
}

VkResult DescriptorPool::FreeDescriptorSets(
   bvk::Device             *device,
   uint32_t                 descriptorSetCount,
   const VkDescriptorSet   *pDescriptorSets) noexcept
{
   for (uint32_t s = 0; s < descriptorSetCount; s++)
      Free(pDescriptorSets[s]);

   return VK_SUCCESS;
}

} // namespace bvk
