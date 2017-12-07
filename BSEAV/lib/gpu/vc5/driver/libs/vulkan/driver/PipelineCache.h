/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class PipelineCache: public NonCopyable, public Allocating
{
public:
   PipelineCache(
      const VkAllocationCallbacks      *pCallbacks,
      bvk::Device                      *pDevice,
      const VkPipelineCacheCreateInfo  *pCreateInfo);

   ~PipelineCache() noexcept;

   VkResult GetPipelineCacheData(
      bvk::Device *device,
      size_t      *pDataSize,
      void        *pData) noexcept;

   VkResult MergePipelineCaches(
      bvk::Device             *device,
      uint32_t                 srcCacheCount,
      const VkPipelineCache   *pSrcCaches) noexcept;

   VkResult CreateGraphicsPipelines(
      bvk::Device                         *device,
      uint32_t                             createInfoCount,
      const VkGraphicsPipelineCreateInfo  *pCreateInfos,
      const VkAllocationCallbacks         *pAllocator,
      VkPipeline                          *pPipelines) noexcept;

   VkResult CreateComputePipelines(
      bvk::Device                         *device,
      uint32_t                             createInfoCount,
      const VkComputePipelineCreateInfo   *pCreateInfos,
      const VkAllocationCallbacks         *pAllocator,
      VkPipeline                          *pPipelines) noexcept;

   // Implementation specific from this point on
private:
};

} // namespace bvk
