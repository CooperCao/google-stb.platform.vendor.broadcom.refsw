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
      Allocating(pCallbacks),
      m_dsLayouts(GetObjScopeAllocator<DescriptorSetLayout*>()),
      m_pcRanges(pCreateInfo->pPushConstantRanges,
                 pCreateInfo->pPushConstantRanges + pCreateInfo->pushConstantRangeCount,
                 GetObjScopeAllocator<VkPushConstantRange>())
{
   m_flags = pCreateInfo->flags;

   m_dsLayouts.resize(pCreateInfo->setLayoutCount);
   for (uint32_t i = 0; i < pCreateInfo->setLayoutCount; i++)
   {
      auto dsl = fromHandle<DescriptorSetLayout>(pCreateInfo->pSetLayouts[i]);
      m_dsLayouts[i] = dsl;
   }

   m_pcTotalBytes = 0;
   for (auto &pcr : m_pcRanges)
      m_pcTotalBytes = std::max(m_pcTotalBytes, pcr.offset + pcr.size);
}

PipelineLayout::~PipelineLayout() noexcept
{
}

} // namespace bvk
