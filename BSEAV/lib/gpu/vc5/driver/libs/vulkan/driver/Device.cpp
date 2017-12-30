/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "libs/core/v3d/v3d_barrier.h"

namespace bvk {

ProcAddressFinder Device::m_procAddrFinder;

Device::Device(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::PhysicalDevice           *pPhysicalDevice,
   const VkDeviceCreateInfo      *pCreateInfo) :
      Allocating(pCallbacks),
      m_physicalDevice(pPhysicalDevice),
      m_pnameTable(NameCompareLess(), GetAllocator<const char *>(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE)),
      m_queues(GetAllocator<Queue>(VK_SYSTEM_ALLOCATION_SCOPE_DEVICE)),
      m_devMemHeap(pCallbacks, "DeviceHeap")
{
   // Build the string set for matching valid procAddress names
   m_pnameTable = {
      #include "DeviceProcTable.inc"
#if VK_USE_PLATFORM_ANDROID_KHR
      #include "DeviceAndroidProcTable.inc"
#endif
   };

   // Create all the queues we've been asked for (we only actually support one)
   for (uint32_t i = 0; i < pCreateInfo->queueCreateInfoCount; i++)
   {
      const VkDeviceQueueCreateInfo *qInfo = &pCreateInfo->pQueueCreateInfos[i];

      for (uint32_t q = 0; q < qInfo->queueCount; q++)
      {
         assert(qInfo->queueFamilyIndex == 0); // We have only one queue family

         m_queues.emplace_back(GetCallbacks(), this, qInfo->pQueuePriorities[q]);
      }
   }

   if (pCreateInfo->pEnabledFeatures == nullptr)
      m_requestedFeatures = {};  // No features are enabled
   else
   {
      m_requestedFeatures = *pCreateInfo->pEnabledFeatures;
      if (!m_physicalDevice->ValidateRequestedFeatures(m_requestedFeatures))
         throw VK_ERROR_FEATURE_NOT_PRESENT;
   }

   // Assume compute shaders make following accesses.
   v3d_barrier_flags barrier_flags = V3D_BARRIER_QPU_INSTR_READ
                                   | V3D_BARRIER_QPU_UNIF_READ
                                   | V3D_BARRIER_TMU_CONFIG_READ
                                   | V3D_BARRIER_TMU_DATA_READ
                                   | V3D_BARRIER_TMU_DATA_WRITE;
#if !V3D_USE_CSD
   barrier_flags |= compute_mem_access_flags();
#endif
   bool hasL3C = v3d_scheduler_get_hub_identity()->has_l3c;
   m_computeCacheOps = v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_RW, barrier_flags, false, hasL3C)
                     | v3d_barrier_cache_cleans(barrier_flags, V3D_BARRIER_MEMORY_RW, false, hasL3C);
#if !V3D_USE_CSD
   m_computeCacheOps &= ~V3D_CACHE_CLEAR_VCD; // VCD cache flushed in the control list
#endif
}

Device::~Device() noexcept
{
}

bool Device::IsValidDeviceProcAddrName(const char *pName)
{
   return m_pnameTable.find(pName) != m_pnameTable.end();
}

PFN_vkVoidFunction Device::GetDeviceProcAddr(
   const char  *pName) noexcept
{
   // From section 3.1 : if pName does not match a function which takes VkDevice, VkQueue or VkCommandBuffer as
   // its first argument, we must return null.
   if (!IsValidDeviceProcAddrName(pName))
      return nullptr;

   return m_procAddrFinder.GetProcAddress(pName);
}

void Device::GetDeviceQueue(
   uint32_t  queueFamilyIndex,
   uint32_t  queueIndex,
   VkQueue  *pQueue) noexcept
{
   // We only have one queue family that does everything
   assert(queueFamilyIndex == 0);
   assert(queueIndex < m_queues.size());

   *pQueue = toHandle<VkQueue>(&m_queues[queueIndex]);
}

VkResult Device::DeviceWaitIdle() noexcept
{
   // vkDeviceWaitIdle provides implicit ordering equivalent to using
   // vkQueueWaitIdle on all queues owned by the device.
   for (auto &q : m_queues)
   {
      VkResult res = q.QueueWaitIdle();
      if (res != VK_SUCCESS)
         return res;
   }

   return VK_SUCCESS;
}

VkResult Device::AllocateMemory(
   const VkMemoryAllocateInfo    *pAllocateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkDeviceMemory                *pMemory) noexcept
{
   try
   {
      DeviceMemory *devMem = createObject<DeviceMemory, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE>(
                                          pAllocator, GetCallbacks(), this, pAllocateInfo);
      *pMemory = toHandle<VkDeviceMemory>(devMem);
   }
   catch (const std::bad_alloc &)        { return VK_ERROR_OUT_OF_HOST_MEMORY;   }
   catch (const bvk::bad_device_alloc &) { return VK_ERROR_OUT_OF_DEVICE_MEMORY; }

   return VK_SUCCESS;
}

VkResult Device::FlushMappedMemoryRanges(
   uint32_t                    memoryRangeCount,
   const VkMappedMemoryRange  *pMemoryRanges) noexcept
{
   for (uint32_t r = 0; r < memoryRangeCount; r++)
   {
      const VkMappedMemoryRange &range = pMemoryRanges[r];

      DeviceMemory *mem = fromHandle<DeviceMemory>(range.memory);
      mem->FlushMappedRange(range);
   }

   return VK_SUCCESS;
}

VkResult Device::InvalidateMappedMemoryRanges(
   uint32_t                    memoryRangeCount,
   const VkMappedMemoryRange  *pMemoryRanges) noexcept
{
   for (uint32_t r = 0; r < memoryRangeCount; r++)
   {
      const VkMappedMemoryRange &range = pMemoryRanges[r];

      DeviceMemory *mem = fromHandle<DeviceMemory>(range.memory);
      mem->InvalidateMappedRange(range);
   }

   return VK_SUCCESS;
}

VkResult Device::ResetFences(
   uint32_t        fenceCount,
   const VkFence  *pFences) noexcept
{
   bool ok = true;

   for (uint32_t i = 0; i < fenceCount; i++)
      ok = fromHandle<Fence>(pFences[0])->ResetFence(this) && ok;

   return ok ? VK_SUCCESS : VK_ERROR_OUT_OF_HOST_MEMORY;
}

VkResult Device::WaitForFences(
   uint32_t        fenceCount,
   const VkFence  *pFences,
   VkBool32        waitAll,
   uint64_t        timeout) noexcept
{
   // Defer to the Fence class to keep all fence handling local to Fence
   return Fence::WaitForFences(fenceCount, pFences, waitAll, timeout);
}

VkResult Device::AllocateDescriptorSets(
   const VkDescriptorSetAllocateInfo   *pAllocateInfo,
   VkDescriptorSet                     *pDescriptorSets) noexcept
{
   DescriptorPool *pool = fromHandle<DescriptorPool>(pAllocateInfo->descriptorPool);

   return pool->AllocateDescriptorSets(pAllocateInfo, pDescriptorSets);
}

void Device::UpdateDescriptorSets(
   uint32_t                    descriptorWriteCount,
   const VkWriteDescriptorSet *pDescriptorWrites,
   uint32_t                    descriptorCopyCount,
   const VkCopyDescriptorSet  *pDescriptorCopies) noexcept
{
   for (uint32_t w = 0; w < descriptorWriteCount; w++)
   {
      DescriptorSet *ds = fromHandle<DescriptorSet>(pDescriptorWrites[w].dstSet);
      ds->Write(&pDescriptorWrites[w]);
   }

   for (uint32_t c = 0; c < descriptorCopyCount; c++)
   {
      DescriptorSet *ds = fromHandle<DescriptorSet>(pDescriptorCopies[c].dstSet);
      ds->Copy(&pDescriptorCopies[c]);
   }
}

VkResult Device::AllocateCommandBuffers(
   const VkCommandBufferAllocateInfo   *pAllocateInfo,
   VkCommandBuffer                     *pCommandBuffers) noexcept
{
   try
   {
      CommandPool *pool = fromHandle<CommandPool>(pAllocateInfo->commandPool);
      pool->AllocateCommandBuffers(this, pAllocateInfo, pCommandBuffers);
   }
   catch (const std::bad_alloc &)         { return VK_ERROR_OUT_OF_HOST_MEMORY;   }
   catch (const bvk::bad_device_alloc &)  { return VK_ERROR_OUT_OF_DEVICE_MEMORY; }

   return VK_SUCCESS;
}

VkResult Device::CreateSharedSwapchainsKHR(
   uint32_t                          swapchainCount,
   const VkSwapchainCreateInfoKHR   *pCreateInfos,
   const VkAllocationCallbacks      *pAllocator,
   VkSwapchainKHR                   *pSwapchains) noexcept
{
   VkResult result;

   result = VK_ERROR_INCOMPATIBLE_DRIVER;

   NOT_IMPLEMENTED_YET;
   return result;
}

VkResult Device::DebugMarkerSetObjectTagEXT(
   const VkDebugMarkerObjectTagInfoEXT *pTagInfo) noexcept
{
   VkResult result;

   result = VK_ERROR_INCOMPATIBLE_DRIVER;

   NOT_IMPLEMENTED_YET;
   return result;
}

VkResult Device::DebugMarkerSetObjectNameEXT(
   const VkDebugMarkerObjectNameInfoEXT   *pNameInfo) noexcept
{
   VkResult result;

   result = VK_ERROR_INCOMPATIBLE_DRIVER;

   NOT_IMPLEMENTED_YET;
   return result;
}

} // namespace bvk
