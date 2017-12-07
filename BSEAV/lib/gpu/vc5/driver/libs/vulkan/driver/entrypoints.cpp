/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

/*
 * DO NOT EDIT.
 * This file is generated from the Khronos Vulkan XML API Registry.
 */

#include <vulkan.h>

#include "Allocating.h"
#include "Allocator.h"
#include "AllObjects.h"
#include "Common.h"

extern "C" {


VKAPI_ATTR VkResult VKAPI_CALL vkCreateInstance(
   const VkInstanceCreateInfo    *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkInstance                    *pInstance)
{
   bvk::APIScoper scope;
   bvk::Instance *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::Instance, VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(
         pAllocator,
         &bvk::g_defaultAllocCallbacks,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pInstance = bvk::toHandle<VkInstance>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyInstance(
   VkInstance                     instance,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_INSTANCE>(
      instanceObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumeratePhysicalDevices(
   VkInstance         instance,
   uint32_t          *pPhysicalDeviceCount,
   VkPhysicalDevice  *pPhysicalDevices)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   return instanceObj->EnumeratePhysicalDevices(
      pPhysicalDeviceCount,
      pPhysicalDevices
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures(
   VkPhysicalDevice            physicalDevice,
   VkPhysicalDeviceFeatures   *pFeatures)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceFeatures(
      pFeatures
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties(
   VkPhysicalDevice      physicalDevice,
   VkFormat              format,
   VkFormatProperties   *pFormatProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceFormatProperties(
      format,
      pFormatProperties
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties(
   VkPhysicalDevice         physicalDevice,
   VkFormat                 format,
   VkImageType              type,
   VkImageTiling            tiling,
   VkImageUsageFlags        usage,
   VkImageCreateFlags       flags,
   VkImageFormatProperties *pImageFormatProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceImageFormatProperties(
      format,
      type,
      tiling,
      usage,
      flags,
      pImageFormatProperties
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties(
   VkPhysicalDevice            physicalDevice,
   VkPhysicalDeviceProperties *pProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceProperties(
      pProperties
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties(
   VkPhysicalDevice         physicalDevice,
   uint32_t                *pQueueFamilyPropertyCount,
   VkQueueFamilyProperties *pQueueFamilyProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceQueueFamilyProperties(
      pQueueFamilyPropertyCount,
      pQueueFamilyProperties
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties(
   VkPhysicalDevice                  physicalDevice,
   VkPhysicalDeviceMemoryProperties *pMemoryProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceMemoryProperties(
      pMemoryProperties
   );
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetInstanceProcAddr(
   VkInstance   instance,
   const char  *pName)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   return bvk::Instance::GetInstanceProcAddr(
      instanceObj,
      pName
   );
}

VKAPI_ATTR PFN_vkVoidFunction VKAPI_CALL vkGetDeviceProcAddr(
   VkDevice     device,
   const char  *pName)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->GetDeviceProcAddr(
      pName
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDevice(
   VkPhysicalDevice               physicalDevice,
   const VkDeviceCreateInfo      *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkDevice                      *pDevice)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   bvk::Device *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::Device, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE>(
         pAllocator,
         &bvk::g_defaultAllocCallbacks,
         physicalDeviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pDevice = bvk::toHandle<VkDevice>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDevice(
   VkDevice                       device,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_DEVICE>(
      deviceObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceExtensionProperties(
   const char              *pLayerName,
   uint32_t                *pPropertyCount,
   VkExtensionProperties   *pProperties)
{
   bvk::APIScoper scope;
   return bvk::Instance::EnumerateInstanceExtensionProperties(
      pLayerName,
      pPropertyCount,
      pProperties
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceExtensionProperties(
   VkPhysicalDevice         physicalDevice,
   const char              *pLayerName,
   uint32_t                *pPropertyCount,
   VkExtensionProperties   *pProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->EnumerateDeviceExtensionProperties(
      pLayerName,
      pPropertyCount,
      pProperties
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateInstanceLayerProperties(
   uint32_t          *pPropertyCount,
   VkLayerProperties *pProperties)
{
   bvk::APIScoper scope;
   return bvk::Instance::EnumerateInstanceLayerProperties(
      pPropertyCount,
      pProperties
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkEnumerateDeviceLayerProperties(
   VkPhysicalDevice   physicalDevice,
   uint32_t          *pPropertyCount,
   VkLayerProperties *pProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->EnumerateDeviceLayerProperties(
      pPropertyCount,
      pProperties
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceQueue(
   VkDevice  device,
   uint32_t  queueFamilyIndex,
   uint32_t  queueIndex,
   VkQueue  *pQueue)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   deviceObj->GetDeviceQueue(
      queueFamilyIndex,
      queueIndex,
      pQueue
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueSubmit(
   VkQueue               queue,
   uint32_t              submitCount,
   const VkSubmitInfo   *pSubmits,
   VkFence               fence)
{
   bvk::APIScoper scope;
   auto queueObj = bvk::fromHandle<bvk::Queue>(queue);
   auto fenceObj = bvk::fromHandle<bvk::Fence>(fence);

   return queueObj->QueueSubmit(
      submitCount,
      pSubmits,
      fenceObj
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueWaitIdle(
   VkQueue   queue)
{
   bvk::APIScoper scope;
   auto queueObj = bvk::fromHandle<bvk::Queue>(queue);

   return queueObj->QueueWaitIdle();
}

VKAPI_ATTR VkResult VKAPI_CALL vkDeviceWaitIdle(
   VkDevice  device)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->DeviceWaitIdle();
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateMemory(
   VkDevice                       device,
   const VkMemoryAllocateInfo    *pAllocateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkDeviceMemory                *pMemory)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->AllocateMemory(
      pAllocateInfo,
      pAllocator,
      pMemory
   );
}

VKAPI_ATTR void VKAPI_CALL vkFreeMemory(
   VkDevice                       device,
   VkDeviceMemory                 memory,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto memoryObj = bvk::fromHandle<bvk::DeviceMemory>(memory);

   if (memory == VK_NULL_HANDLE) return;

   memoryObj->FreeMemory(
      deviceObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkMapMemory(
   VkDevice           device,
   VkDeviceMemory     memory,
   VkDeviceSize       offset,
   VkDeviceSize       size,
   VkMemoryMapFlags   flags,
   void*             *ppData)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto memoryObj = bvk::fromHandle<bvk::DeviceMemory>(memory);

   return memoryObj->MapMemory(
      deviceObj,
      offset,
      size,
      flags,
      ppData
   );
}

VKAPI_ATTR void VKAPI_CALL vkUnmapMemory(
   VkDevice        device,
   VkDeviceMemory  memory)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto memoryObj = bvk::fromHandle<bvk::DeviceMemory>(memory);

   memoryObj->UnmapMemory(
      deviceObj
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkFlushMappedMemoryRanges(
   VkDevice                    device,
   uint32_t                    memoryRangeCount,
   const VkMappedMemoryRange  *pMemoryRanges)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->FlushMappedMemoryRanges(
      memoryRangeCount,
      pMemoryRanges
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkInvalidateMappedMemoryRanges(
   VkDevice                    device,
   uint32_t                    memoryRangeCount,
   const VkMappedMemoryRange  *pMemoryRanges)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->InvalidateMappedMemoryRanges(
      memoryRangeCount,
      pMemoryRanges
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetDeviceMemoryCommitment(
   VkDevice        device,
   VkDeviceMemory  memory,
   VkDeviceSize   *pCommittedMemoryInBytes)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto memoryObj = bvk::fromHandle<bvk::DeviceMemory>(memory);

   memoryObj->GetDeviceMemoryCommitment(
      deviceObj,
      pCommittedMemoryInBytes
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindBufferMemory(
   VkDevice        device,
   VkBuffer        buffer,
   VkDeviceMemory  memory,
   VkDeviceSize    memoryOffset)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto bufferObj = bvk::fromHandle<bvk::Buffer>(buffer);
   auto memoryObj = bvk::fromHandle<bvk::DeviceMemory>(memory);

   return bufferObj->BindBufferMemory(
      deviceObj,
      memoryObj,
      memoryOffset
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkBindImageMemory(
   VkDevice        device,
   VkImage         image,
   VkDeviceMemory  memory,
   VkDeviceSize    memoryOffset)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto imageObj = bvk::fromHandle<bvk::Image>(image);
   auto memoryObj = bvk::fromHandle<bvk::DeviceMemory>(memory);

   return imageObj->BindImageMemory(
      deviceObj,
      memoryObj,
      memoryOffset
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetBufferMemoryRequirements(
   VkDevice              device,
   VkBuffer              buffer,
   VkMemoryRequirements *pMemoryRequirements)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto bufferObj = bvk::fromHandle<bvk::Buffer>(buffer);

   bufferObj->GetBufferMemoryRequirements(
      deviceObj,
      pMemoryRequirements
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetImageMemoryRequirements(
   VkDevice              device,
   VkImage               image,
   VkMemoryRequirements *pMemoryRequirements)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto imageObj = bvk::fromHandle<bvk::Image>(image);

   imageObj->GetImageMemoryRequirements(
      deviceObj,
      pMemoryRequirements
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSparseMemoryRequirements(
   VkDevice                          device,
   VkImage                           image,
   uint32_t                         *pSparseMemoryRequirementCount,
   VkSparseImageMemoryRequirements  *pSparseMemoryRequirements)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto imageObj = bvk::fromHandle<bvk::Image>(image);

   imageObj->GetImageSparseMemoryRequirements(
      deviceObj,
      pSparseMemoryRequirementCount,
      pSparseMemoryRequirements
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties(
   VkPhysicalDevice               physicalDevice,
   VkFormat                       format,
   VkImageType                    type,
   VkSampleCountFlagBits          samples,
   VkImageUsageFlags              usage,
   VkImageTiling                  tiling,
   uint32_t                      *pPropertyCount,
   VkSparseImageFormatProperties *pProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceSparseImageFormatProperties(
      format,
      type,
      samples,
      usage,
      tiling,
      pPropertyCount,
      pProperties
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueueBindSparse(
   VkQueue                  queue,
   uint32_t                 bindInfoCount,
   const VkBindSparseInfo  *pBindInfo,
   VkFence                  fence)
{
   bvk::APIScoper scope;
   auto queueObj = bvk::fromHandle<bvk::Queue>(queue);
   auto fenceObj = bvk::fromHandle<bvk::Fence>(fence);

   return queueObj->QueueBindSparse(
      bindInfoCount,
      pBindInfo,
      fenceObj
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateFence(
   VkDevice                       device,
   const VkFenceCreateInfo       *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkFence                       *pFence)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::Fence *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::Fence, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pFence = bvk::toHandle<VkFence>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyFence(
   VkDevice                       device,
   VkFence                        fence,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto fenceObj = bvk::fromHandle<bvk::Fence>(fence);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      fenceObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetFences(
   VkDevice        device,
   uint32_t        fenceCount,
   const VkFence  *pFences)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->ResetFences(
      fenceCount,
      pFences
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetFenceStatus(
   VkDevice  device,
   VkFence   fence)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto fenceObj = bvk::fromHandle<bvk::Fence>(fence);

   return fenceObj->GetFenceStatus(
      deviceObj
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkWaitForFences(
   VkDevice        device,
   uint32_t        fenceCount,
   const VkFence  *pFences,
   VkBool32        waitAll,
   uint64_t        timeout)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->WaitForFences(
      fenceCount,
      pFences,
      waitAll,
      timeout
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSemaphore(
   VkDevice                       device,
   const VkSemaphoreCreateInfo   *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkSemaphore                   *pSemaphore)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::Semaphore *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::Semaphore, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pSemaphore = bvk::toHandle<VkSemaphore>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySemaphore(
   VkDevice                       device,
   VkSemaphore                    semaphore,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto semaphoreObj = bvk::fromHandle<bvk::Semaphore>(semaphore);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      semaphoreObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateEvent(
   VkDevice                       device,
   const VkEventCreateInfo       *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkEvent                       *pEvent)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::Event *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::Event, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pEvent = bvk::toHandle<VkEvent>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyEvent(
   VkDevice                       device,
   VkEvent                        event,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto eventObj = bvk::fromHandle<bvk::Event>(event);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      eventObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetEventStatus(
   VkDevice  device,
   VkEvent   event)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto eventObj = bvk::fromHandle<bvk::Event>(event);

   return eventObj->GetEventStatus(
      deviceObj
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkSetEvent(
   VkDevice  device,
   VkEvent   event)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto eventObj = bvk::fromHandle<bvk::Event>(event);

   return eventObj->SetEvent(
      deviceObj
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetEvent(
   VkDevice  device,
   VkEvent   event)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto eventObj = bvk::fromHandle<bvk::Event>(event);

   return eventObj->ResetEvent(
      deviceObj
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateQueryPool(
   VkDevice                       device,
   const VkQueryPoolCreateInfo   *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkQueryPool                   *pQueryPool)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::QueryPool *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::QueryPool, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pQueryPool = bvk::toHandle<VkQueryPool>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyQueryPool(
   VkDevice                       device,
   VkQueryPool                    queryPool,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto queryPoolObj = bvk::fromHandle<bvk::QueryPool>(queryPool);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      queryPoolObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetQueryPoolResults(
   VkDevice           device,
   VkQueryPool        queryPool,
   uint32_t           firstQuery,
   uint32_t           queryCount,
   size_t             dataSize,
   void              *pData,
   VkDeviceSize       stride,
   VkQueryResultFlags flags)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto queryPoolObj = bvk::fromHandle<bvk::QueryPool>(queryPool);

   return queryPoolObj->GetQueryPoolResults(
      deviceObj,
      firstQuery,
      queryCount,
      dataSize,
      pData,
      stride,
      flags
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateBuffer(
   VkDevice                       device,
   const VkBufferCreateInfo      *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkBuffer                      *pBuffer)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::Buffer *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::Buffer, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pBuffer = bvk::toHandle<VkBuffer>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyBuffer(
   VkDevice                       device,
   VkBuffer                       buffer,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto bufferObj = bvk::fromHandle<bvk::Buffer>(buffer);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      bufferObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateBufferView(
   VkDevice                       device,
   const VkBufferViewCreateInfo  *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkBufferView                  *pView)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::BufferView *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::BufferView, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pView = bvk::toHandle<VkBufferView>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyBufferView(
   VkDevice                       device,
   VkBufferView                   bufferView,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto bufferViewObj = bvk::fromHandle<bvk::BufferView>(bufferView);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      bufferViewObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateImage(
   VkDevice                       device,
   const VkImageCreateInfo       *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkImage                       *pImage)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::Image *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::Image, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pImage = bvk::toHandle<VkImage>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyImage(
   VkDevice                       device,
   VkImage                        image,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto imageObj = bvk::fromHandle<bvk::Image>(image);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      imageObj,
      pAllocator
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetImageSubresourceLayout(
   VkDevice                    device,
   VkImage                     image,
   const VkImageSubresource   *pSubresource,
   VkSubresourceLayout        *pLayout)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto imageObj = bvk::fromHandle<bvk::Image>(image);

   imageObj->GetImageSubresourceLayout(
      deviceObj,
      pSubresource,
      pLayout
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateImageView(
   VkDevice                       device,
   const VkImageViewCreateInfo   *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkImageView                   *pView)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::ImageView *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::ImageView, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pView = bvk::toHandle<VkImageView>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyImageView(
   VkDevice                       device,
   VkImageView                    imageView,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto imageViewObj = bvk::fromHandle<bvk::ImageView>(imageView);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      imageViewObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateShaderModule(
   VkDevice                          device,
   const VkShaderModuleCreateInfo   *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkShaderModule                   *pShaderModule)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::ShaderModule *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::ShaderModule, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pShaderModule = bvk::toHandle<VkShaderModule>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyShaderModule(
   VkDevice                       device,
   VkShaderModule                 shaderModule,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto shaderModuleObj = bvk::fromHandle<bvk::ShaderModule>(shaderModule);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      shaderModuleObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineCache(
   VkDevice                          device,
   const VkPipelineCacheCreateInfo  *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkPipelineCache                  *pPipelineCache)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::PipelineCache *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::PipelineCache, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pPipelineCache = bvk::toHandle<VkPipelineCache>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineCache(
   VkDevice                       device,
   VkPipelineCache                pipelineCache,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto pipelineCacheObj = bvk::fromHandle<bvk::PipelineCache>(pipelineCache);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      pipelineCacheObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPipelineCacheData(
   VkDevice        device,
   VkPipelineCache pipelineCache,
   size_t         *pDataSize,
   void           *pData)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto pipelineCacheObj = bvk::fromHandle<bvk::PipelineCache>(pipelineCache);

   return pipelineCacheObj->GetPipelineCacheData(
      deviceObj,
      pDataSize,
      pData
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkMergePipelineCaches(
   VkDevice                 device,
   VkPipelineCache          dstCache,
   uint32_t                 srcCacheCount,
   const VkPipelineCache   *pSrcCaches)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto dstCacheObj = bvk::fromHandle<bvk::PipelineCache>(dstCache);

   return dstCacheObj->MergePipelineCaches(
      deviceObj,
      srcCacheCount,
      pSrcCaches
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateGraphicsPipelines(
   VkDevice                             device,
   VkPipelineCache                      pipelineCache,
   uint32_t                             createInfoCount,
   const VkGraphicsPipelineCreateInfo  *pCreateInfos,
   const VkAllocationCallbacks         *pAllocator,
   VkPipeline                          *pPipelines)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto pipelineCacheObj = bvk::fromHandle<bvk::PipelineCache>(pipelineCache);

   return pipelineCacheObj->CreateGraphicsPipelines(
      deviceObj,
      createInfoCount,
      pCreateInfos,
      pAllocator,
      pPipelines
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateComputePipelines(
   VkDevice                             device,
   VkPipelineCache                      pipelineCache,
   uint32_t                             createInfoCount,
   const VkComputePipelineCreateInfo   *pCreateInfos,
   const VkAllocationCallbacks         *pAllocator,
   VkPipeline                          *pPipelines)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto pipelineCacheObj = bvk::fromHandle<bvk::PipelineCache>(pipelineCache);

   return pipelineCacheObj->CreateComputePipelines(
      deviceObj,
      createInfoCount,
      pCreateInfos,
      pAllocator,
      pPipelines
   );
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipeline(
   VkDevice                       device,
   VkPipeline                     pipeline,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto pipelineObj = bvk::fromHandle<bvk::Pipeline>(pipeline);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      pipelineObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreatePipelineLayout(
   VkDevice                          device,
   const VkPipelineLayoutCreateInfo *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkPipelineLayout                 *pPipelineLayout)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::PipelineLayout *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::PipelineLayout, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pPipelineLayout = bvk::toHandle<VkPipelineLayout>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyPipelineLayout(
   VkDevice                       device,
   VkPipelineLayout               pipelineLayout,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto pipelineLayoutObj = bvk::fromHandle<bvk::PipelineLayout>(pipelineLayout);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      pipelineLayoutObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateSampler(
   VkDevice                       device,
   const VkSamplerCreateInfo     *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkSampler                     *pSampler)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::Sampler *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::Sampler, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pSampler = bvk::toHandle<VkSampler>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySampler(
   VkDevice                       device,
   VkSampler                      sampler,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto samplerObj = bvk::fromHandle<bvk::Sampler>(sampler);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      samplerObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorSetLayout(
   VkDevice                                device,
   const VkDescriptorSetLayoutCreateInfo  *pCreateInfo,
   const VkAllocationCallbacks            *pAllocator,
   VkDescriptorSetLayout                  *pSetLayout)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::DescriptorSetLayout *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::DescriptorSetLayout, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pSetLayout = bvk::toHandle<VkDescriptorSetLayout>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorSetLayout(
   VkDevice                       device,
   VkDescriptorSetLayout          descriptorSetLayout,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto descriptorSetLayoutObj = bvk::fromHandle<bvk::DescriptorSetLayout>(descriptorSetLayout);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      descriptorSetLayoutObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDescriptorPool(
   VkDevice                          device,
   const VkDescriptorPoolCreateInfo *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkDescriptorPool                 *pDescriptorPool)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::DescriptorPool *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::DescriptorPool, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pDescriptorPool = bvk::toHandle<VkDescriptorPool>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDescriptorPool(
   VkDevice                       device,
   VkDescriptorPool               descriptorPool,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto descriptorPoolObj = bvk::fromHandle<bvk::DescriptorPool>(descriptorPool);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      descriptorPoolObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetDescriptorPool(
   VkDevice                    device,
   VkDescriptorPool            descriptorPool,
   VkDescriptorPoolResetFlags  flags)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto descriptorPoolObj = bvk::fromHandle<bvk::DescriptorPool>(descriptorPool);

   return descriptorPoolObj->ResetDescriptorPool(
      deviceObj,
      flags
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateDescriptorSets(
   VkDevice                             device,
   const VkDescriptorSetAllocateInfo   *pAllocateInfo,
   VkDescriptorSet                     *pDescriptorSets)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->AllocateDescriptorSets(
      pAllocateInfo,
      pDescriptorSets
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkFreeDescriptorSets(
   VkDevice                 device,
   VkDescriptorPool         descriptorPool,
   uint32_t                 descriptorSetCount,
   const VkDescriptorSet   *pDescriptorSets)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto descriptorPoolObj = bvk::fromHandle<bvk::DescriptorPool>(descriptorPool);

   return descriptorPoolObj->FreeDescriptorSets(
      deviceObj,
      descriptorSetCount,
      pDescriptorSets
   );
}

VKAPI_ATTR void VKAPI_CALL vkUpdateDescriptorSets(
   VkDevice                    device,
   uint32_t                    descriptorWriteCount,
   const VkWriteDescriptorSet *pDescriptorWrites,
   uint32_t                    descriptorCopyCount,
   const VkCopyDescriptorSet  *pDescriptorCopies)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   deviceObj->UpdateDescriptorSets(
      descriptorWriteCount,
      pDescriptorWrites,
      descriptorCopyCount,
      pDescriptorCopies
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateFramebuffer(
   VkDevice                       device,
   const VkFramebufferCreateInfo *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkFramebuffer                 *pFramebuffer)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::Framebuffer *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::Framebuffer, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pFramebuffer = bvk::toHandle<VkFramebuffer>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyFramebuffer(
   VkDevice                       device,
   VkFramebuffer                  framebuffer,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto framebufferObj = bvk::fromHandle<bvk::Framebuffer>(framebuffer);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      framebufferObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateRenderPass(
   VkDevice                       device,
   const VkRenderPassCreateInfo  *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkRenderPass                  *pRenderPass)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::RenderPass *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::RenderPass, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pRenderPass = bvk::toHandle<VkRenderPass>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyRenderPass(
   VkDevice                       device,
   VkRenderPass                   renderPass,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto renderPassObj = bvk::fromHandle<bvk::RenderPass>(renderPass);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      renderPassObj,
      pAllocator
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetRenderAreaGranularity(
   VkDevice     device,
   VkRenderPass renderPass,
   VkExtent2D  *pGranularity)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto renderPassObj = bvk::fromHandle<bvk::RenderPass>(renderPass);

   renderPassObj->GetRenderAreaGranularity(
      deviceObj,
      pGranularity
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateCommandPool(
   VkDevice                       device,
   const VkCommandPoolCreateInfo *pCreateInfo,
   const VkAllocationCallbacks   *pAllocator,
   VkCommandPool                 *pCommandPool)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::CommandPool *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::CommandPool, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pCommandPool = bvk::toHandle<VkCommandPool>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyCommandPool(
   VkDevice                       device,
   VkCommandPool                  commandPool,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto commandPoolObj = bvk::fromHandle<bvk::CommandPool>(commandPool);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      commandPoolObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandPool(
   VkDevice                 device,
   VkCommandPool            commandPool,
   VkCommandPoolResetFlags  flags)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto commandPoolObj = bvk::fromHandle<bvk::CommandPool>(commandPool);

   return commandPoolObj->ResetCommandPool(
      deviceObj,
      flags
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkAllocateCommandBuffers(
   VkDevice                             device,
   const VkCommandBufferAllocateInfo   *pAllocateInfo,
   VkCommandBuffer                     *pCommandBuffers)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->AllocateCommandBuffers(
      pAllocateInfo,
      pCommandBuffers
   );
}

VKAPI_ATTR void VKAPI_CALL vkFreeCommandBuffers(
   VkDevice                 device,
   VkCommandPool            commandPool,
   uint32_t                 commandBufferCount,
   const VkCommandBuffer   *pCommandBuffers)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto commandPoolObj = bvk::fromHandle<bvk::CommandPool>(commandPool);

   commandPoolObj->FreeCommandBuffers(
      deviceObj,
      commandBufferCount,
      pCommandBuffers
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkBeginCommandBuffer(
   VkCommandBuffer                   commandBuffer,
   const VkCommandBufferBeginInfo   *pBeginInfo)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   return commandBufferObj->BeginCommandBuffer(
      pBeginInfo
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkEndCommandBuffer(
   VkCommandBuffer commandBuffer)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   return commandBufferObj->EndCommandBuffer();
}

VKAPI_ATTR VkResult VKAPI_CALL vkResetCommandBuffer(
   VkCommandBuffer             commandBuffer,
   VkCommandBufferResetFlags   flags)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   return commandBufferObj->ResetCommandBuffer(
      flags
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindPipeline(
   VkCommandBuffer       commandBuffer,
   VkPipelineBindPoint   pipelineBindPoint,
   VkPipeline            pipeline)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto pipelineObj = bvk::fromHandle<bvk::Pipeline>(pipeline);

   commandBufferObj->CmdBindPipeline(
      pipelineBindPoint,
      pipelineObj
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetViewport(
   VkCommandBuffer    commandBuffer,
   uint32_t           firstViewport,
   uint32_t           viewportCount,
   const VkViewport  *pViewports)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdSetViewport(
      firstViewport,
      viewportCount,
      pViewports
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetScissor(
   VkCommandBuffer commandBuffer,
   uint32_t        firstScissor,
   uint32_t        scissorCount,
   const VkRect2D *pScissors)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdSetScissor(
      firstScissor,
      scissorCount,
      pScissors
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetLineWidth(
   VkCommandBuffer commandBuffer,
   float           lineWidth)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdSetLineWidth(
      lineWidth
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBias(
   VkCommandBuffer commandBuffer,
   float           depthBiasConstantFactor,
   float           depthBiasClamp,
   float           depthBiasSlopeFactor)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdSetDepthBias(
      depthBiasConstantFactor,
      depthBiasClamp,
      depthBiasSlopeFactor
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetBlendConstants(
   VkCommandBuffer commandBuffer,
   const float     blendConstants[4])
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdSetBlendConstants(
      blendConstants
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetDepthBounds(
   VkCommandBuffer commandBuffer,
   float           minDepthBounds,
   float           maxDepthBounds)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdSetDepthBounds(
      minDepthBounds,
      maxDepthBounds
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilCompareMask(
   VkCommandBuffer    commandBuffer,
   VkStencilFaceFlags faceMask,
   uint32_t           compareMask)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdSetStencilCompareMask(
      faceMask,
      compareMask
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilWriteMask(
   VkCommandBuffer    commandBuffer,
   VkStencilFaceFlags faceMask,
   uint32_t           writeMask)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdSetStencilWriteMask(
      faceMask,
      writeMask
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetStencilReference(
   VkCommandBuffer    commandBuffer,
   VkStencilFaceFlags faceMask,
   uint32_t           reference)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdSetStencilReference(
      faceMask,
      reference
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindDescriptorSets(
   VkCommandBuffer          commandBuffer,
   VkPipelineBindPoint      pipelineBindPoint,
   VkPipelineLayout         layout,
   uint32_t                 firstSet,
   uint32_t                 descriptorSetCount,
   const VkDescriptorSet   *pDescriptorSets,
   uint32_t                 dynamicOffsetCount,
   const uint32_t          *pDynamicOffsets)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto layoutObj = bvk::fromHandle<bvk::PipelineLayout>(layout);

   commandBufferObj->CmdBindDescriptorSets(
      pipelineBindPoint,
      layoutObj,
      firstSet,
      descriptorSetCount,
      pDescriptorSets,
      dynamicOffsetCount,
      pDynamicOffsets
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindIndexBuffer(
   VkCommandBuffer commandBuffer,
   VkBuffer        buffer,
   VkDeviceSize    offset,
   VkIndexType     indexType)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto bufferObj = bvk::fromHandle<bvk::Buffer>(buffer);

   commandBufferObj->CmdBindIndexBuffer(
      bufferObj,
      offset,
      indexType
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdBindVertexBuffers(
   VkCommandBuffer       commandBuffer,
   uint32_t              firstBinding,
   uint32_t              bindingCount,
   const VkBuffer       *pBuffers,
   const VkDeviceSize   *pOffsets)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdBindVertexBuffers(
      firstBinding,
      bindingCount,
      pBuffers,
      pOffsets
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdDraw(
   VkCommandBuffer commandBuffer,
   uint32_t        vertexCount,
   uint32_t        instanceCount,
   uint32_t        firstVertex,
   uint32_t        firstInstance)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdDraw(
      vertexCount,
      instanceCount,
      firstVertex,
      firstInstance
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexed(
   VkCommandBuffer commandBuffer,
   uint32_t        indexCount,
   uint32_t        instanceCount,
   uint32_t        firstIndex,
   int32_t         vertexOffset,
   uint32_t        firstInstance)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdDrawIndexed(
      indexCount,
      instanceCount,
      firstIndex,
      vertexOffset,
      firstInstance
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndirect(
   VkCommandBuffer commandBuffer,
   VkBuffer        buffer,
   VkDeviceSize    offset,
   uint32_t        drawCount,
   uint32_t        stride)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto bufferObj = bvk::fromHandle<bvk::Buffer>(buffer);

   commandBufferObj->CmdDrawIndirect(
      bufferObj,
      offset,
      drawCount,
      stride
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdDrawIndexedIndirect(
   VkCommandBuffer commandBuffer,
   VkBuffer        buffer,
   VkDeviceSize    offset,
   uint32_t        drawCount,
   uint32_t        stride)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto bufferObj = bvk::fromHandle<bvk::Buffer>(buffer);

   commandBufferObj->CmdDrawIndexedIndirect(
      bufferObj,
      offset,
      drawCount,
      stride
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatch(
   VkCommandBuffer commandBuffer,
   uint32_t        groupCountX,
   uint32_t        groupCountY,
   uint32_t        groupCountZ)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdDispatch(
      groupCountX,
      groupCountY,
      groupCountZ
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdDispatchIndirect(
   VkCommandBuffer commandBuffer,
   VkBuffer        buffer,
   VkDeviceSize    offset)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto bufferObj = bvk::fromHandle<bvk::Buffer>(buffer);

   commandBufferObj->CmdDispatchIndirect(
      bufferObj,
      offset
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBuffer(
   VkCommandBuffer       commandBuffer,
   VkBuffer              srcBuffer,
   VkBuffer              dstBuffer,
   uint32_t              regionCount,
   const VkBufferCopy   *pRegions)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto srcBufferObj = bvk::fromHandle<bvk::Buffer>(srcBuffer);
   auto dstBufferObj = bvk::fromHandle<bvk::Buffer>(dstBuffer);

   commandBufferObj->CmdCopyBuffer(
      srcBufferObj,
      dstBufferObj,
      regionCount,
      pRegions
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImage(
   VkCommandBuffer    commandBuffer,
   VkImage            srcImage,
   VkImageLayout      srcImageLayout,
   VkImage            dstImage,
   VkImageLayout      dstImageLayout,
   uint32_t           regionCount,
   const VkImageCopy *pRegions)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto srcImageObj = bvk::fromHandle<bvk::Image>(srcImage);
   auto dstImageObj = bvk::fromHandle<bvk::Image>(dstImage);

   commandBufferObj->CmdCopyImage(
      srcImageObj,
      srcImageLayout,
      dstImageObj,
      dstImageLayout,
      regionCount,
      pRegions
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdBlitImage(
   VkCommandBuffer    commandBuffer,
   VkImage            srcImage,
   VkImageLayout      srcImageLayout,
   VkImage            dstImage,
   VkImageLayout      dstImageLayout,
   uint32_t           regionCount,
   const VkImageBlit *pRegions,
   VkFilter           filter)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto srcImageObj = bvk::fromHandle<bvk::Image>(srcImage);
   auto dstImageObj = bvk::fromHandle<bvk::Image>(dstImage);

   commandBufferObj->CmdBlitImage(
      srcImageObj,
      srcImageLayout,
      dstImageObj,
      dstImageLayout,
      regionCount,
      pRegions,
      filter
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyBufferToImage(
   VkCommandBuffer          commandBuffer,
   VkBuffer                 srcBuffer,
   VkImage                  dstImage,
   VkImageLayout            dstImageLayout,
   uint32_t                 regionCount,
   const VkBufferImageCopy *pRegions)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto srcBufferObj = bvk::fromHandle<bvk::Buffer>(srcBuffer);
   auto dstImageObj = bvk::fromHandle<bvk::Image>(dstImage);

   commandBufferObj->CmdCopyBufferToImage(
      srcBufferObj,
      dstImageObj,
      dstImageLayout,
      regionCount,
      pRegions
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyImageToBuffer(
   VkCommandBuffer          commandBuffer,
   VkImage                  srcImage,
   VkImageLayout            srcImageLayout,
   VkBuffer                 dstBuffer,
   uint32_t                 regionCount,
   const VkBufferImageCopy *pRegions)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto srcImageObj = bvk::fromHandle<bvk::Image>(srcImage);
   auto dstBufferObj = bvk::fromHandle<bvk::Buffer>(dstBuffer);

   commandBufferObj->CmdCopyImageToBuffer(
      srcImageObj,
      srcImageLayout,
      dstBufferObj,
      regionCount,
      pRegions
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdUpdateBuffer(
   VkCommandBuffer commandBuffer,
   VkBuffer        dstBuffer,
   VkDeviceSize    dstOffset,
   VkDeviceSize    dataSize,
   const void     *pData)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto dstBufferObj = bvk::fromHandle<bvk::Buffer>(dstBuffer);

   commandBufferObj->CmdUpdateBuffer(
      dstBufferObj,
      dstOffset,
      dataSize,
      pData
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdFillBuffer(
   VkCommandBuffer commandBuffer,
   VkBuffer        dstBuffer,
   VkDeviceSize    dstOffset,
   VkDeviceSize    size,
   uint32_t        data)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto dstBufferObj = bvk::fromHandle<bvk::Buffer>(dstBuffer);

   commandBufferObj->CmdFillBuffer(
      dstBufferObj,
      dstOffset,
      size,
      data
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearColorImage(
   VkCommandBuffer                commandBuffer,
   VkImage                        image,
   VkImageLayout                  imageLayout,
   const VkClearColorValue       *pColor,
   uint32_t                       rangeCount,
   const VkImageSubresourceRange *pRanges)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto imageObj = bvk::fromHandle<bvk::Image>(image);

   commandBufferObj->CmdClearColorImage(
      imageObj,
      imageLayout,
      pColor,
      rangeCount,
      pRanges
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearDepthStencilImage(
   VkCommandBuffer                   commandBuffer,
   VkImage                           image,
   VkImageLayout                     imageLayout,
   const VkClearDepthStencilValue   *pDepthStencil,
   uint32_t                          rangeCount,
   const VkImageSubresourceRange    *pRanges)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto imageObj = bvk::fromHandle<bvk::Image>(image);

   commandBufferObj->CmdClearDepthStencilImage(
      imageObj,
      imageLayout,
      pDepthStencil,
      rangeCount,
      pRanges
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdClearAttachments(
   VkCommandBuffer          commandBuffer,
   uint32_t                 attachmentCount,
   const VkClearAttachment *pAttachments,
   uint32_t                 rectCount,
   const VkClearRect       *pRects)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdClearAttachments(
      attachmentCount,
      pAttachments,
      rectCount,
      pRects
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdResolveImage(
   VkCommandBuffer       commandBuffer,
   VkImage               srcImage,
   VkImageLayout         srcImageLayout,
   VkImage               dstImage,
   VkImageLayout         dstImageLayout,
   uint32_t              regionCount,
   const VkImageResolve *pRegions)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto srcImageObj = bvk::fromHandle<bvk::Image>(srcImage);
   auto dstImageObj = bvk::fromHandle<bvk::Image>(dstImage);

   commandBufferObj->CmdResolveImage(
      srcImageObj,
      srcImageLayout,
      dstImageObj,
      dstImageLayout,
      regionCount,
      pRegions
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdSetEvent(
   VkCommandBuffer       commandBuffer,
   VkEvent               event,
   VkPipelineStageFlags  stageMask)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto eventObj = bvk::fromHandle<bvk::Event>(event);

   commandBufferObj->CmdSetEvent(
      eventObj,
      stageMask
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetEvent(
   VkCommandBuffer       commandBuffer,
   VkEvent               event,
   VkPipelineStageFlags  stageMask)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto eventObj = bvk::fromHandle<bvk::Event>(event);

   commandBufferObj->CmdResetEvent(
      eventObj,
      stageMask
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdWaitEvents(
   VkCommandBuffer                commandBuffer,
   uint32_t                       eventCount,
   const VkEvent                 *pEvents,
   VkPipelineStageFlags           srcStageMask,
   VkPipelineStageFlags           dstStageMask,
   uint32_t                       memoryBarrierCount,
   const VkMemoryBarrier         *pMemoryBarriers,
   uint32_t                       bufferMemoryBarrierCount,
   const VkBufferMemoryBarrier   *pBufferMemoryBarriers,
   uint32_t                       imageMemoryBarrierCount,
   const VkImageMemoryBarrier    *pImageMemoryBarriers)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdWaitEvents(
      eventCount,
      pEvents,
      srcStageMask,
      dstStageMask,
      memoryBarrierCount,
      pMemoryBarriers,
      bufferMemoryBarrierCount,
      pBufferMemoryBarriers,
      imageMemoryBarrierCount,
      pImageMemoryBarriers
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdPipelineBarrier(
   VkCommandBuffer                commandBuffer,
   VkPipelineStageFlags           srcStageMask,
   VkPipelineStageFlags           dstStageMask,
   VkDependencyFlags              dependencyFlags,
   uint32_t                       memoryBarrierCount,
   const VkMemoryBarrier         *pMemoryBarriers,
   uint32_t                       bufferMemoryBarrierCount,
   const VkBufferMemoryBarrier   *pBufferMemoryBarriers,
   uint32_t                       imageMemoryBarrierCount,
   const VkImageMemoryBarrier    *pImageMemoryBarriers)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdPipelineBarrier(
      srcStageMask,
      dstStageMask,
      dependencyFlags,
      memoryBarrierCount,
      pMemoryBarriers,
      bufferMemoryBarrierCount,
      pBufferMemoryBarriers,
      imageMemoryBarrierCount,
      pImageMemoryBarriers
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginQuery(
   VkCommandBuffer       commandBuffer,
   VkQueryPool           queryPool,
   uint32_t              query,
   VkQueryControlFlags   flags)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto queryPoolObj = bvk::fromHandle<bvk::QueryPool>(queryPool);

   commandBufferObj->CmdBeginQuery(
      queryPoolObj,
      query,
      flags
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndQuery(
   VkCommandBuffer commandBuffer,
   VkQueryPool     queryPool,
   uint32_t        query)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto queryPoolObj = bvk::fromHandle<bvk::QueryPool>(queryPool);

   commandBufferObj->CmdEndQuery(
      queryPoolObj,
      query
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdResetQueryPool(
   VkCommandBuffer commandBuffer,
   VkQueryPool     queryPool,
   uint32_t        firstQuery,
   uint32_t        queryCount)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto queryPoolObj = bvk::fromHandle<bvk::QueryPool>(queryPool);

   commandBufferObj->CmdResetQueryPool(
      queryPoolObj,
      firstQuery,
      queryCount
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdWriteTimestamp(
   VkCommandBuffer          commandBuffer,
   VkPipelineStageFlagBits  pipelineStage,
   VkQueryPool              queryPool,
   uint32_t                 query)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto queryPoolObj = bvk::fromHandle<bvk::QueryPool>(queryPool);

   commandBufferObj->CmdWriteTimestamp(
      pipelineStage,
      queryPoolObj,
      query
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdCopyQueryPoolResults(
   VkCommandBuffer    commandBuffer,
   VkQueryPool        queryPool,
   uint32_t           firstQuery,
   uint32_t           queryCount,
   VkBuffer           dstBuffer,
   VkDeviceSize       dstOffset,
   VkDeviceSize       stride,
   VkQueryResultFlags flags)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto queryPoolObj = bvk::fromHandle<bvk::QueryPool>(queryPool);
   auto dstBufferObj = bvk::fromHandle<bvk::Buffer>(dstBuffer);

   commandBufferObj->CmdCopyQueryPoolResults(
      queryPoolObj,
      firstQuery,
      queryCount,
      dstBufferObj,
      dstOffset,
      stride,
      flags
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdPushConstants(
   VkCommandBuffer    commandBuffer,
   VkPipelineLayout   layout,
   VkShaderStageFlags stageFlags,
   uint32_t           offset,
   uint32_t           size,
   const void        *pValues)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);
   auto layoutObj = bvk::fromHandle<bvk::PipelineLayout>(layout);

   commandBufferObj->CmdPushConstants(
      layoutObj,
      stageFlags,
      offset,
      size,
      pValues
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdBeginRenderPass(
   VkCommandBuffer                commandBuffer,
   const VkRenderPassBeginInfo   *pRenderPassBegin,
   VkSubpassContents              contents)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdBeginRenderPass(
      pRenderPassBegin,
      contents
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdNextSubpass(
   VkCommandBuffer    commandBuffer,
   VkSubpassContents  contents)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdNextSubpass(
      contents
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdEndRenderPass(
   VkCommandBuffer commandBuffer)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdEndRenderPass();
}

VKAPI_ATTR void VKAPI_CALL vkCmdExecuteCommands(
   VkCommandBuffer          commandBuffer,
   uint32_t                 commandBufferCount,
   const VkCommandBuffer   *pCommandBuffers)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdExecuteCommands(
      commandBufferCount,
      pCommandBuffers
   );
}



VKAPI_ATTR void VKAPI_CALL vkDestroySurfaceKHR(
   VkInstance                     instance,
   VkSurfaceKHR                   surface,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   instanceObj->DestroySurfaceKHR(
      surface,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceSupportKHR(
   VkPhysicalDevice   physicalDevice,
   uint32_t           queueFamilyIndex,
   VkSurfaceKHR       surface,
   VkBool32          *pSupported)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceSurfaceSupportKHR(
      queueFamilyIndex,
      surface,
      pSupported
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
   VkPhysicalDevice            physicalDevice,
   VkSurfaceKHR                surface,
   VkSurfaceCapabilitiesKHR   *pSurfaceCapabilities)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceSurfaceCapabilitiesKHR(
      surface,
      pSurfaceCapabilities
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceFormatsKHR(
   VkPhysicalDevice      physicalDevice,
   VkSurfaceKHR          surface,
   uint32_t             *pSurfaceFormatCount,
   VkSurfaceFormatKHR   *pSurfaceFormats)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceSurfaceFormatsKHR(
      surface,
      pSurfaceFormatCount,
      pSurfaceFormats
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfacePresentModesKHR(
   VkPhysicalDevice   physicalDevice,
   VkSurfaceKHR       surface,
   uint32_t          *pPresentModeCount,
   VkPresentModeKHR  *pPresentModes)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceSurfacePresentModesKHR(
      surface,
      pPresentModeCount,
      pPresentModes
   );
}



VKAPI_ATTR VkResult VKAPI_CALL vkCreateSwapchainKHR(
   VkDevice                          device,
   const VkSwapchainCreateInfoKHR   *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkSwapchainKHR                   *pSwapchain)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   bvk::SwapchainKHR *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::SwapchainKHR, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         deviceObj->GetCallbacks(),
         deviceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pSwapchain = bvk::toHandle<VkSwapchainKHR>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroySwapchainKHR(
   VkDevice                       device,
   VkSwapchainKHR                 swapchain,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto swapchainObj = bvk::fromHandle<bvk::SwapchainKHR>(swapchain);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      swapchainObj,
      pAllocator
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainImagesKHR(
   VkDevice        device,
   VkSwapchainKHR  swapchain,
   uint32_t       *pSwapchainImageCount,
   VkImage        *pSwapchainImages)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto swapchainObj = bvk::fromHandle<bvk::SwapchainKHR>(swapchain);

   return swapchainObj->GetSwapchainImagesKHR(
      deviceObj,
      pSwapchainImageCount,
      pSwapchainImages
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireNextImageKHR(
   VkDevice        device,
   VkSwapchainKHR  swapchain,
   uint64_t        timeout,
   VkSemaphore     semaphore,
   VkFence         fence,
   uint32_t       *pImageIndex)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto swapchainObj = bvk::fromHandle<bvk::SwapchainKHR>(swapchain);
   auto semaphoreObj = bvk::fromHandle<bvk::Semaphore>(semaphore);
   auto fenceObj = bvk::fromHandle<bvk::Fence>(fence);

   return swapchainObj->AcquireNextImageKHR(
      deviceObj,
      timeout,
      semaphoreObj,
      fenceObj,
      pImageIndex
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkQueuePresentKHR(
   VkQueue                  queue,
   const VkPresentInfoKHR  *pPresentInfo)
{
   bvk::APIScoper scope;
   auto queueObj = bvk::fromHandle<bvk::Queue>(queue);

   return queueObj->QueuePresentKHR(
      pPresentInfo
   );
}



VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayPropertiesKHR(
   VkPhysicalDevice         physicalDevice,
   uint32_t                *pPropertyCount,
   VkDisplayPropertiesKHR  *pProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceDisplayPropertiesKHR(
      pPropertyCount,
      pProperties
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceDisplayPlanePropertiesKHR(
   VkPhysicalDevice               physicalDevice,
   uint32_t                      *pPropertyCount,
   VkDisplayPlanePropertiesKHR   *pProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceDisplayPlanePropertiesKHR(
      pPropertyCount,
      pProperties
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayPlaneSupportedDisplaysKHR(
   VkPhysicalDevice   physicalDevice,
   uint32_t           planeIndex,
   uint32_t          *pDisplayCount,
   VkDisplayKHR      *pDisplays)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetDisplayPlaneSupportedDisplaysKHR(
      planeIndex,
      pDisplayCount,
      pDisplays
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayModePropertiesKHR(
   VkPhysicalDevice            physicalDevice,
   VkDisplayKHR                display,
   uint32_t                   *pPropertyCount,
   VkDisplayModePropertiesKHR *pProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetDisplayModePropertiesKHR(
      display,
      pPropertyCount,
      pProperties
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayModeKHR(
   VkPhysicalDevice                  physicalDevice,
   VkDisplayKHR                      display,
   const VkDisplayModeCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkDisplayModeKHR                 *pMode)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->CreateDisplayModeKHR(
      display,
      pCreateInfo,
      pAllocator,
      pMode
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetDisplayPlaneCapabilitiesKHR(
   VkPhysicalDevice               physicalDevice,
   VkDisplayModeKHR               mode,
   uint32_t                       planeIndex,
   VkDisplayPlaneCapabilitiesKHR *pCapabilities)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetDisplayPlaneCapabilitiesKHR(
      mode,
      planeIndex,
      pCapabilities
   );
}

#ifdef VK_USE_PLATFORM_DISPLAY_KHR

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDisplayPlaneSurfaceKHR(
   VkInstance                           instance,
   const VkDisplaySurfaceCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks         *pAllocator,
   VkSurfaceKHR                        *pSurface)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   return instanceObj->CreateDisplayPlaneSurfaceKHR(
      pCreateInfo,
      pAllocator,
      pSurface
   );
}

#endif // VK_USE_PLATFORM_DISPLAY_KHR



VKAPI_ATTR VkResult VKAPI_CALL vkCreateSharedSwapchainsKHR(
   VkDevice                          device,
   uint32_t                          swapchainCount,
   const VkSwapchainCreateInfoKHR   *pCreateInfos,
   const VkAllocationCallbacks      *pAllocator,
   VkSwapchainKHR                   *pSwapchains)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->CreateSharedSwapchainsKHR(
      swapchainCount,
      pCreateInfos,
      pAllocator,
      pSwapchains
   );
}



#ifdef VK_USE_PLATFORM_XLIB_KHR

VKAPI_ATTR VkResult VKAPI_CALL vkCreateXlibSurfaceKHR(
   VkInstance                        instance,
   const VkXlibSurfaceCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkSurfaceKHR                     *pSurface)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   return instanceObj->CreateXlibSurfaceKHR(
      pCreateInfo,
      pAllocator,
      pSurface
   );
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceXlibPresentationSupportKHR(
   VkPhysicalDevice   physicalDevice,
   uint32_t           queueFamilyIndex,
   Display           *dpy,
   VisualID           visualID)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceXlibPresentationSupportKHR(
      queueFamilyIndex,
      dpy,
      visualID
   );
}


#endif // VK_USE_PLATFORM_XLIB_KHR


#ifdef VK_USE_PLATFORM_XCB_KHR

VKAPI_ATTR VkResult VKAPI_CALL vkCreateXcbSurfaceKHR(
   VkInstance                        instance,
   const VkXcbSurfaceCreateInfoKHR  *pCreateInfo,
   const VkAllocationCallbacks      *pAllocator,
   VkSurfaceKHR                     *pSurface)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   return instanceObj->CreateXcbSurfaceKHR(
      pCreateInfo,
      pAllocator,
      pSurface
   );
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceXcbPresentationSupportKHR(
   VkPhysicalDevice   physicalDevice,
   uint32_t           queueFamilyIndex,
   xcb_connection_t  *connection,
   xcb_visualid_t     visual_id)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceXcbPresentationSupportKHR(
      queueFamilyIndex,
      connection,
      visual_id
   );
}


#endif // VK_USE_PLATFORM_XCB_KHR


#ifdef VK_USE_PLATFORM_WAYLAND_KHR

VKAPI_ATTR VkResult VKAPI_CALL vkCreateWaylandSurfaceKHR(
   VkInstance                           instance,
   const VkWaylandSurfaceCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks         *pAllocator,
   VkSurfaceKHR                        *pSurface)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   return instanceObj->CreateWaylandSurfaceKHR(
      pCreateInfo,
      pAllocator,
      pSurface
   );
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceWaylandPresentationSupportKHR(
   VkPhysicalDevice   physicalDevice,
   uint32_t           queueFamilyIndex,
   struct wl_display *display)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceWaylandPresentationSupportKHR(
      queueFamilyIndex,
      display
   );
}


#endif // VK_USE_PLATFORM_WAYLAND_KHR


#ifdef VK_USE_PLATFORM_ANDROID_KHR

VKAPI_ATTR VkResult VKAPI_CALL vkCreateAndroidSurfaceKHR(
   VkInstance                           instance,
   const VkAndroidSurfaceCreateInfoKHR *pCreateInfo,
   const VkAllocationCallbacks         *pAllocator,
   VkSurfaceKHR                        *pSurface)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   return instanceObj->CreateAndroidSurfaceKHR(
      pCreateInfo,
      pAllocator,
      pSurface
   );
}


#endif // VK_USE_PLATFORM_ANDROID_KHR


#ifdef VK_USE_PLATFORM_WIN32_KHR

VKAPI_ATTR VkResult VKAPI_CALL vkCreateWin32SurfaceKHR(
   VkInstance                           instance,
   const VkWin32SurfaceCreateInfoKHR   *pCreateInfo,
   const VkAllocationCallbacks         *pAllocator,
   VkSurfaceKHR                        *pSurface)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   return instanceObj->CreateWin32SurfaceKHR(
      pCreateInfo,
      pAllocator,
      pSurface
   );
}

VKAPI_ATTR VkBool32 VKAPI_CALL vkGetPhysicalDeviceWin32PresentationSupportKHR(
   VkPhysicalDevice   physicalDevice,
   uint32_t           queueFamilyIndex)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceWin32PresentationSupportKHR(
      queueFamilyIndex
   );
}


#endif // VK_USE_PLATFORM_WIN32_KHR


VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFeatures2KHR(
   VkPhysicalDevice               physicalDevice,
   VkPhysicalDeviceFeatures2KHR  *pFeatures)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceFeatures2KHR(
      pFeatures
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceProperties2KHR(
   VkPhysicalDevice                  physicalDevice,
   VkPhysicalDeviceProperties2KHR   *pProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceProperties2KHR(
      pProperties
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceFormatProperties2KHR(
   VkPhysicalDevice         physicalDevice,
   VkFormat                 format,
   VkFormatProperties2KHR  *pFormatProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceFormatProperties2KHR(
      format,
      pFormatProperties
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceImageFormatProperties2KHR(
   VkPhysicalDevice                           physicalDevice,
   const VkPhysicalDeviceImageFormatInfo2KHR *pImageFormatInfo,
   VkImageFormatProperties2KHR               *pImageFormatProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   return physicalDeviceObj->GetPhysicalDeviceImageFormatProperties2KHR(
      pImageFormatInfo,
      pImageFormatProperties
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceQueueFamilyProperties2KHR(
   VkPhysicalDevice               physicalDevice,
   uint32_t                      *pQueueFamilyPropertyCount,
   VkQueueFamilyProperties2KHR   *pQueueFamilyProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceQueueFamilyProperties2KHR(
      pQueueFamilyPropertyCount,
      pQueueFamilyProperties
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceMemoryProperties2KHR(
   VkPhysicalDevice                        physicalDevice,
   VkPhysicalDeviceMemoryProperties2KHR   *pMemoryProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceMemoryProperties2KHR(
      pMemoryProperties
   );
}

VKAPI_ATTR void VKAPI_CALL vkGetPhysicalDeviceSparseImageFormatProperties2KHR(
   VkPhysicalDevice                                 physicalDevice,
   const VkPhysicalDeviceSparseImageFormatInfo2KHR *pFormatInfo,
   uint32_t                                        *pPropertyCount,
   VkSparseImageFormatProperties2KHR               *pProperties)
{
   bvk::APIScoper scope;
   auto physicalDeviceObj = bvk::fromHandle<bvk::PhysicalDevice>(physicalDevice);

   physicalDeviceObj->GetPhysicalDeviceSparseImageFormatProperties2KHR(
      pFormatInfo,
      pPropertyCount,
      pProperties
   );
}



VKAPI_ATTR void VKAPI_CALL vkTrimCommandPoolKHR(
   VkDevice                    device,
   VkCommandPool               commandPool,
   VkCommandPoolTrimFlagsKHR   flags)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);
   auto commandPoolObj = bvk::fromHandle<bvk::CommandPool>(commandPool);

   commandPoolObj->TrimCommandPoolKHR(
      deviceObj,
      flags
   );
}




VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
   VkInstance                                 instance,
   const VkDebugReportCallbackCreateInfoEXT  *pCreateInfo,
   const VkAllocationCallbacks               *pAllocator,
   VkDebugReportCallbackEXT                  *pCallback)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   bvk::DebugReportCallbackEXT *obj = nullptr;
   try
   {
      obj = bvk::createObject<
         bvk::DebugReportCallbackEXT, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
         pAllocator,
         instanceObj->GetCallbacks(),
         instanceObj,
         pCreateInfo
      );
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (VkResult e)
   {
      return e;
   }
   catch (...)
   {
      std::terminate();
   }
   *pCallback = bvk::toHandle<VkDebugReportCallbackEXT>(obj);
   return VK_SUCCESS;
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(
   VkInstance                     instance,
   VkDebugReportCallbackEXT       callback,
   const VkAllocationCallbacks   *pAllocator)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);
   auto callbackObj = bvk::fromHandle<bvk::DebugReportCallbackEXT>(callback);

   bvk::destroyObject<VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>(
      callbackObj,
      pAllocator
   );
}

VKAPI_ATTR void VKAPI_CALL vkDebugReportMessageEXT(
   VkInstance                  instance,
   VkDebugReportFlagsEXT       flags,
   VkDebugReportObjectTypeEXT  objectType,
   uint64_t                    object,
   size_t                      location,
   int32_t                     messageCode,
   const char                 *pLayerPrefix,
   const char                 *pMessage)
{
   bvk::APIScoper scope;
   auto instanceObj = bvk::fromHandle<bvk::Instance>(instance);

   instanceObj->DebugReportMessageEXT(
      flags,
      objectType,
      object,
      location,
      messageCode,
      pLayerPrefix,
      pMessage
   );
}



VKAPI_ATTR VkResult VKAPI_CALL vkDebugMarkerSetObjectTagEXT(
   VkDevice                             device,
   const VkDebugMarkerObjectTagInfoEXT *pTagInfo)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->DebugMarkerSetObjectTagEXT(
      pTagInfo
   );
}

VKAPI_ATTR VkResult VKAPI_CALL vkDebugMarkerSetObjectNameEXT(
   VkDevice                                device,
   const VkDebugMarkerObjectNameInfoEXT   *pNameInfo)
{
   bvk::APIScoper scope;
   auto deviceObj = bvk::fromHandle<bvk::Device>(device);

   return deviceObj->DebugMarkerSetObjectNameEXT(
      pNameInfo
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerBeginEXT(
   VkCommandBuffer                   commandBuffer,
   const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdDebugMarkerBeginEXT(
      pMarkerInfo
   );
}

VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerEndEXT(
   VkCommandBuffer commandBuffer)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdDebugMarkerEndEXT();
}

VKAPI_ATTR void VKAPI_CALL vkCmdDebugMarkerInsertEXT(
   VkCommandBuffer                   commandBuffer,
   const VkDebugMarkerMarkerInfoEXT *pMarkerInfo)
{
   bvk::APIScoper scope;
   auto commandBufferObj = bvk::fromHandle<bvk::CommandBuffer>(commandBuffer);

   commandBufferObj->CmdDebugMarkerInsertEXT(
      pMarkerInfo
   );
}


} // extern "C"
