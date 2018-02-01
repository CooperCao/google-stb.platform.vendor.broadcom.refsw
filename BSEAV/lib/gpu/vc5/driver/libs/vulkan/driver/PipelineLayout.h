/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class PipelineLayout: public NonCopyable, public Allocating
{
public:
   PipelineLayout(
      const VkAllocationCallbacks      *pCallbacks,
      bvk::Device                      *pDevice,
      const VkPipelineLayoutCreateInfo *pCreateInfo);

   ~PipelineLayout() noexcept;

   // Implementation specific from this point on
   uint32_t GetPushConstantBytesRequired() const { return m_pcTotalBytes; }

private:
   uint32_t                            m_pcTotalBytes;   // Total push constant bytes required
};

} // namespace bvk
