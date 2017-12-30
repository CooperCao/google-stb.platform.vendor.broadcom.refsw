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

   m_format = pCreateInfo->format;

   m_size = 0;
   if (buffer->Size() > buffer->GetBoundMemoryOffset() + pCreateInfo->offset)
      m_size = static_cast<uint32_t>(buffer->Size() - buffer->GetBoundMemoryOffset() - pCreateInfo->offset);

   if (pCreateInfo->range != VK_WHOLE_SIZE)
      m_size = std::min(m_size, static_cast<uint32_t>(pCreateInfo->range));

   m_physAddr = buffer->CalculateBufferOffsetAddr(pCreateInfo->offset);

   CreateTSR(pCreateInfo);
}

BufferView::~BufferView() noexcept
{
}

void BufferView::CreateTSR(const VkBufferViewCreateInfo *info)
{
   V3D_TMU_TEX_STATE_T tsr = {};

   tsr.l0_addr = m_physAddr;

#if V3D_VER_AT_LEAST(4,1,34,0)
   v3d_tmu_get_wh_for_1d_tex_state(&tsr.width, &tsr.height, m_size);
#else
   // If we have to fully support older hardware, we will need to use a 1d-array texture
   // and manipulate texture coords in the qpu.
   assert(info->range <= 4096); // Best we can do on older h/w without using an array
   tsr.width  = info->range;
   tsr.height = 1;
#endif
   tsr.depth = 1;

   // TODO : modify fmt based on our special cases where we don't use exact format
   GFX_LFMT_T fmt = Formats::GetLFMT(info->format, Formats::HW);

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
