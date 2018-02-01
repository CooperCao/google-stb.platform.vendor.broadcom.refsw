/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"

#include <algorithm>

#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/core/v3d/v3d_tmu.h"

namespace bvk {

BufferView::BufferView(
   const VkAllocationCallbacks   *pCallbacks,
   bvk::Device                   *pDevice,
   const VkBufferViewCreateInfo  *pCreateInfo) :
      Allocating(pCallbacks)
{
   Buffer *buffer = fromHandle<Buffer>(pCreateInfo->buffer);

   v3d_addr_t physAddr = buffer->CalculateBufferOffsetAddr(pCreateInfo->offset);
   GFX_LFMT_T format   = Formats::GetLFMT(pCreateInfo->format);

   uint32_t size = 0;
   if (buffer->Size() > buffer->GetBoundMemoryOffset() + pCreateInfo->offset)
      size = static_cast<uint32_t>(buffer->Size() - buffer->GetBoundMemoryOffset() - pCreateInfo->offset);

   if (pCreateInfo->range != VK_WHOLE_SIZE)
      size = std::min(size, static_cast<uint32_t>(pCreateInfo->range));

   m_nElems = size / gfx_lfmt_bytes_per_block(format);

   CreateTSR(physAddr, format, m_nElems);
}

BufferView::~BufferView() noexcept
{
}

void BufferView::CreateTSR(v3d_addr_t addr, GFX_LFMT_T fmt, uint32_t size)
{
   V3D_TMU_TEX_STATE_T tsr = {};

   tsr.l0_addr = addr;

   v3d_tmu_get_wh_for_1d_tex_state(&tsr.width, &tsr.height, size);
   tsr.depth = 1;

   GFX_LFMT_TMU_TRANSLATION_T t;
   gfx_lfmt_translate_tmu(&t, gfx_lfmt_ds_to_red(fmt)
#if !V3D_HAS_TMU_R32F_R16_SHAD
      , gfx_lfmt_has_depth(fmt)
#endif
      );

   tsr.type = t.type;
   tsr.srgb = t.srgb;

   tsr.swizzles[0] = t.swizzles[0];
   tsr.swizzles[1] = t.swizzles[1];
   tsr.swizzles[2] = t.swizzles[2];
   tsr.swizzles[3] = t.swizzles[3];

   v3d_pack_tmu_tex_state(m_texState.data(), &tsr);
}

} // namespace bvk
