/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

namespace bvk {

PipelineLayout::PipelineLayout(
   const VkAllocationCallbacks      *pCallbacks,
   bvk::Device                      *pDevice,
   const VkPipelineLayoutCreateInfo *pCreateInfo) :
      Allocating(pCallbacks)
{
   m_pcTotalBytes = 0;
   for (uint32_t i=0; i<pCreateInfo->pushConstantRangeCount; i++)
   {
      const VkPushConstantRange *pcr = &pCreateInfo->pPushConstantRanges[i];
      m_pcTotalBytes = std::max(m_pcTotalBytes, pcr->offset + pcr->size);
   }
}

PipelineLayout::~PipelineLayout() noexcept
{
}

} // namespace bvk
