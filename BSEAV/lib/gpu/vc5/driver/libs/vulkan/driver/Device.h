/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>
#include "Queue.h"

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "Dispatchable.h"
#include "ProcAddressFinder.h"
#include "PhysicalDevice.h"
#include "DevMemHeap.h"

#include <string.h>

namespace bvk {

class Device: public NonCopyable, public Allocating, public Dispatchable
{
public:
   Device(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::PhysicalDevice           *pPhysicalDevice,
      const VkDeviceCreateInfo      *pCreateInfo);

   ~Device() noexcept;

   PFN_vkVoidFunction GetDeviceProcAddr(
      const char  *pName) noexcept;

   void GetDeviceQueue(
      uint32_t  queueFamilyIndex,
      uint32_t  queueIndex,
      VkQueue  *pQueue) noexcept;

   VkResult DeviceWaitIdle() noexcept;

   VkResult AllocateMemory(
      const VkMemoryAllocateInfo    *pAllocateInfo,
      const VkAllocationCallbacks   *pAllocator,
      VkDeviceMemory                *pMemory) noexcept;

   VkResult FlushMappedMemoryRanges(
      uint32_t                    memoryRangeCount,
      const VkMappedMemoryRange  *pMemoryRanges) noexcept;

   VkResult InvalidateMappedMemoryRanges(
      uint32_t                    memoryRangeCount,
      const VkMappedMemoryRange  *pMemoryRanges) noexcept;

   VkResult ResetFences(
      uint32_t        fenceCount,
      const VkFence  *pFences) noexcept;

   VkResult WaitForFences(
      uint32_t        fenceCount,
      const VkFence  *pFences,
      VkBool32        waitAll,
      uint64_t        timeout) noexcept;

   VkResult AllocateDescriptorSets(
      const VkDescriptorSetAllocateInfo   *pAllocateInfo,
      VkDescriptorSet                     *pDescriptorSets) noexcept;

   void UpdateDescriptorSets(
      uint32_t                    descriptorWriteCount,
      const VkWriteDescriptorSet *pDescriptorWrites,
      uint32_t                    descriptorCopyCount,
      const VkCopyDescriptorSet  *pDescriptorCopies) noexcept;

   VkResult AllocateCommandBuffers(
      const VkCommandBufferAllocateInfo   *pAllocateInfo,
      VkCommandBuffer                     *pCommandBuffers) noexcept;

   VkResult CreateSharedSwapchainsKHR(
      uint32_t                          swapchainCount,
      const VkSwapchainCreateInfoKHR   *pCreateInfos,
      const VkAllocationCallbacks      *pAllocator,
      VkSwapchainKHR                   *pSwapchains) noexcept;

   VkResult DebugMarkerSetObjectTagEXT(
      const VkDebugMarkerObjectTagInfoEXT *pTagInfo) noexcept;

   VkResult DebugMarkerSetObjectNameEXT(
      const VkDebugMarkerObjectNameInfoEXT   *pNameInfo) noexcept;

   // Implementation specific from this point on
public:
   PhysicalDevice *GetPhysicalDevice() { return m_physicalDevice; }
   V3DPlatform &GetPlatform() { return m_physicalDevice->Platform(); }
   const VkPhysicalDeviceFeatures &GetRequestedFeatures() const { return m_requestedFeatures; }
   v3d_cache_ops ComputeCacheOps() const { return m_computeCacheOps; }
   DevMemHeap *GetDevMemHeap() { return &m_devMemHeap; }

private:
   bool IsValidDeviceProcAddrName(const char *pName);

   struct NameCompareLess
   {
      NameCompareLess() = default;
      bool operator()(const char *lhs, const char *rhs) const { return strcmp(lhs, rhs) < 0; }
   };

private:
   bvk::PhysicalDevice                      *m_physicalDevice;
   bvk::set<const char *, NameCompareLess>   m_pnameTable;
   static ProcAddressFinder                  m_procAddrFinder;

   bvk::vector<Queue>                        m_queues;
   VkPhysicalDeviceFeatures                  m_requestedFeatures;

   v3d_cache_ops                             m_computeCacheOps;

   DevMemHeap                                m_devMemHeap;
};

} // namespace bvk
