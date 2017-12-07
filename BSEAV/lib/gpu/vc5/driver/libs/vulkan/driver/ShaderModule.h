/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

#include "Module.h"

namespace bvk {

class SPVCompiledShader;

class ShaderModule: public NonCopyable, public Allocating
{
public:
   ShaderModule(
      const VkAllocationCallbacks      *pCallbacks,
      bvk::Device                      *pDevice,
      const VkShaderModuleCreateInfo   *pCreateInfo);

   ~ShaderModule() noexcept;

   operator const Module &() const { return m_module; }
   const Module &GetModule() const { return m_module; }

private:
   Module m_module;
};

} // namespace bvk
