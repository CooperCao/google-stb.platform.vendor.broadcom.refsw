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
   uint32_t GetNumElems() const { return m_nElems;   }

   void WriteTextureStateRecord(uint8_t *ptr) const
   {
      memcpy(ptr, m_texState.data(), m_texState.size());
   }

private:
   void CreateTSR(v3d_addr_t addr, GFX_LFMT_T fmt, uint32_t size);

private:
   uint32_t                                           m_nElems;
   std::array<uint8_t, V3D_TMU_TEX_STATE_PACKED_SIZE> m_texState;
};

} // namespace bvk
