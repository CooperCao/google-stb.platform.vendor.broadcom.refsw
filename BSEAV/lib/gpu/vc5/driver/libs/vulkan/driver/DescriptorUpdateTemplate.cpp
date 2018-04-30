/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

namespace bvk {

DescriptorUpdateTemplate::DescriptorUpdateTemplate(
   const VkAllocationCallbacks                *pCallbacks,
   bvk::Device                                *pDevice,
   const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo) :
      Allocating(pCallbacks),
      m_updateEntries(pCreateInfo->descriptorUpdateEntryCount,
                      VkDescriptorUpdateTemplateEntry(),
                      GetObjScopeAllocator<VkDescriptorUpdateTemplateEntry>())
{
   for (uint32_t i = 0; i < pCreateInfo->descriptorUpdateEntryCount; i++)
      m_updateEntries[i] = pCreateInfo->pDescriptorUpdateEntries[i];

   m_templateType      = pCreateInfo->templateType;
}

} // namespace bvk
