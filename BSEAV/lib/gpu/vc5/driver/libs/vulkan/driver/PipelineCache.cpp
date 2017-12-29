/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

namespace bvk {

struct PipelineCacheHeader
{
   uint32_t size;
   uint32_t version;
   uint32_t vendorId;
   uint32_t deviceId;
   uint8_t  uuid[VK_UUID_SIZE];
};

// This is a null implementation of pipeline cache that should
// satisfy CTS, but serves no useful purpose.
PipelineCache::PipelineCache(
   const VkAllocationCallbacks      *pCallbacks,
   bvk::Device                      *pDevice,
   const VkPipelineCacheCreateInfo  *pCreateInfo) :
      Allocating(pCallbacks)
{
}

PipelineCache::~PipelineCache() noexcept
{
}

VkResult PipelineCache::GetPipelineCacheData(
   Device         *device,
   size_t         *pDataSize,
   void           *pData) noexcept
{
   VkResult ret = VK_SUCCESS;

   if (pData == nullptr)
      *pDataSize = sizeof(PipelineCacheHeader);
   else
   {
      // Get the info we need for the header
      VkPhysicalDeviceProperties props;
      device->GetPhysicalDevice()->GetPhysicalDeviceProperties(&props);

      PipelineCacheHeader  header{
         sizeof(PipelineCacheHeader),           // m_size
         VK_PIPELINE_CACHE_HEADER_VERSION_ONE,  // m_version
         props.vendorID,                        // m_vendorId
         props.deviceID                         // m_deviceId
      };
      memcpy(header.uuid, props.pipelineCacheUUID, sizeof(header.uuid));

      // "If pDataSize is less than what is necessary to store this header, nothing will be
      // written to pData and zero will be written to pDataSize"
      if (*pDataSize < sizeof(header))
      {
         *pDataSize = 0;
         ret = VK_INCOMPLETE;
      }
      else
      {
         size_t bytesToCopy = std::min(*pDataSize, sizeof(header));
         memcpy(pData, &header, bytesToCopy);
         *pDataSize = bytesToCopy;
      }
   }

   return ret;
}

VkResult PipelineCache::MergePipelineCaches(
   Device                   *device,
   uint32_t                 srcCacheCount,
   const VkPipelineCache   *pSrcCaches) noexcept
{
   return VK_SUCCESS;
}

VkResult PipelineCache::CreateGraphicsPipelines(
   Device                              *device,
   uint32_t                             createInfoCount,
   const VkGraphicsPipelineCreateInfo  *pCreateInfos,
   const VkAllocationCallbacks         *pAllocator,
   VkPipeline                          *pPipelines) noexcept
{
   uint32_t i;
   try
   {
      // Any we fail to create must be set to NULL_HANDLE
      for (i = 0; i < createInfoCount; i++)
         pPipelines[i] = VK_NULL_HANDLE;

      for (i = 0; i < createInfoCount; i++)
      {
         Pipeline *pipe = createObject<GraphicsPipeline, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE>(
                                       pAllocator, &g_defaultAllocCallbacks,
                                       device, &pCreateInfos[i]);

         pPipelines[i] = toHandle<VkPipeline>(pipe);
      }
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (const bvk::bad_device_alloc &)
   {
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;
   }
   return VK_SUCCESS;
}

VkResult PipelineCache::CreateComputePipelines(
   Device                              *device,
   uint32_t                             createInfoCount,
   const VkComputePipelineCreateInfo   *pCreateInfos,
   const VkAllocationCallbacks         *pAllocator,
   VkPipeline                          *pPipelines) noexcept
{
   uint32_t i;
   try
   {
      // Any we fail to create must be set to NULL_HANDLE
      for (i = 0; i < createInfoCount; i++)
         pPipelines[i] = VK_NULL_HANDLE;

      for (i = 0; i < createInfoCount; i++)
      {
         Pipeline *pipe = createObject<ComputePipeline, VK_SYSTEM_ALLOCATION_SCOPE_DEVICE>(
            pAllocator, &g_defaultAllocCallbacks,
            device, &pCreateInfos[i]);

         pPipelines[i] = toHandle<VkPipeline>(pipe);
      }
   }
   catch (const std::bad_alloc &)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (const bvk::bad_device_alloc &)
   {
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;
   }

   return VK_SUCCESS;
}

} // namespace bvk
