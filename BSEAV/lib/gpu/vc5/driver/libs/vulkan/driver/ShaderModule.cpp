/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "Module.h"

namespace bvk {

ShaderModule::ShaderModule(
   const VkAllocationCallbacks      *pCallbacks,
   bvk::Device                      *pDevice,
   const VkShaderModuleCreateInfo   *pCreateInfo) :
      Allocating(pCallbacks),
      m_module(pCallbacks, pCreateInfo->pCode, pCreateInfo->codeSize)
{
}

ShaderModule::~ShaderModule() noexcept
{
}

} // namespace bvk
