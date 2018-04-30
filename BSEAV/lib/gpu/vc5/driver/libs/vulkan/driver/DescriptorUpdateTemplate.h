/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class DescriptorUpdateTemplate: public NonCopyable, public Allocating
{
public:
   DescriptorUpdateTemplate(
      const VkAllocationCallbacks                *pCallbacks,
      bvk::Device                                *pDevice,
      const VkDescriptorUpdateTemplateCreateInfo *pCreateInfo);

   // Implementation specific from this point on
   VkDescriptorUpdateTemplateType               GetTemplateType() const  { return m_templateType;  }
   bvk::vector<VkDescriptorUpdateTemplateEntry> GetUpdateEntries() const { return m_updateEntries; }

private:
   bvk::vector<VkDescriptorUpdateTemplateEntry> m_updateEntries;
   VkDescriptorUpdateTemplateType               m_templateType;
};

} // namespace bvk
