/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"

namespace bvk {

class BufferView: public NonCopyable, public Allocating
{
public:
   BufferView(
      const VkAllocationCallbacks   *pCallbacks,
      bvk::Device                   *pDevice,
      const VkBufferViewCreateInfo  *pCreateInfo);

   ~BufferView() noexcept;

   // Implementation specific from this point on
   uint32_t GetSize() const   { return m_size;   }
   VkFormat GetFormat() const { return m_format; }

   v3d_addr_t CalculateBufferOffsetAddr() const { return m_physAddr; }

   void WriteTextureStateRecord(uint8_t *ptr) const
   {
      memcpy(ptr, m_texState.data(), m_texState.size());
   }

private:
   void CreateTSR(const VkBufferViewCreateInfo *info);

private:
   v3d_addr_t                                         m_physAddr;
   uint32_t                                           m_size;
   uint32_t                                           m_offset;
   VkFormat                                           m_format;
   std::array<uint8_t, V3D_TMU_TEX_STATE_PACKED_SIZE> m_texState;
   std::array<uint8_t, V3D_TMU_SAMPLER_PACKED_SIZE>   m_sampState;
};

} // namespace bvk
