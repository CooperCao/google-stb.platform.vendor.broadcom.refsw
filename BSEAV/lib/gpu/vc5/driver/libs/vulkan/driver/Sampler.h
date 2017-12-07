/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class Sampler: public NonCopyable, public Allocating
{
public:
   Sampler(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkSamplerCreateInfo     *pCreateInfo);

   ~Sampler() noexcept;

   // Implementation specific from this point on
   bool UnnormalizedCoordinates() const { return m_unnormalized; }
   void WriteSamplerRecord(uint8_t *ptr) const { memcpy(ptr, m_sampState, V3D_TMU_SAMPLER_PACKED_SIZE); }

private:
   void CreateSamplerRecord(const VkSamplerCreateInfo *pci);

private:
   bool    m_unnormalized;
   uint8_t m_sampState[V3D_TMU_SAMPLER_PACKED_SIZE];
};

} // namespace bvk
