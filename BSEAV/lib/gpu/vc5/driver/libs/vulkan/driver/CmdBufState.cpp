/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "CmdBufState.h"
#include "Device.h"
#include "PhysicalDevice.h"
#include "Common.h"
#include "Pipeline.h"
#include "CommandBuffer.h"
#include "Framebuffer.h"
#include "Command.h"
#include "Viewport.h"
#include <cassert>
#include <algorithm>

namespace bvk {

#define CHECK_ENUMS(a, b) \
   static_assert(static_cast<uint32_t>(a) == static_cast<uint32_t>(b), "Enum constants don't match");

CHECK_ENUMS(VK_BLEND_FACTOR_ZERO,                     V3D_BLEND_MUL_ZERO);
CHECK_ENUMS(VK_BLEND_FACTOR_ONE,                      V3D_BLEND_MUL_ONE);
CHECK_ENUMS(VK_BLEND_FACTOR_SRC_COLOR,                V3D_BLEND_MUL_SRC);
CHECK_ENUMS(VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR,      V3D_BLEND_MUL_OM_SRC)
CHECK_ENUMS(VK_BLEND_FACTOR_DST_COLOR,                V3D_BLEND_MUL_DST);
CHECK_ENUMS(VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR,      V3D_BLEND_MUL_OM_DST);
CHECK_ENUMS(VK_BLEND_FACTOR_SRC_ALPHA,                V3D_BLEND_MUL_SRC_ALPHA);
CHECK_ENUMS(VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,      V3D_BLEND_MUL_OM_SRC_ALPHA);
CHECK_ENUMS(VK_BLEND_FACTOR_DST_ALPHA,                V3D_BLEND_MUL_DST_ALPHA);
CHECK_ENUMS(VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA,      V3D_BLEND_MUL_OM_DST_ALPHA);
CHECK_ENUMS(VK_BLEND_FACTOR_CONSTANT_COLOR,           V3D_BLEND_MUL_CONST);
CHECK_ENUMS(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR, V3D_BLEND_MUL_OM_CONST);
CHECK_ENUMS(VK_BLEND_FACTOR_CONSTANT_ALPHA,           V3D_BLEND_MUL_CONST_ALPHA);
CHECK_ENUMS(VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA, V3D_BLEND_MUL_OM_CONST_ALPHA);
CHECK_ENUMS(VK_BLEND_FACTOR_SRC_ALPHA_SATURATE,       V3D_BLEND_MUL_SRC_ALPHA_SAT);

CHECK_ENUMS(VK_BLEND_OP_ADD,                          V3D_BLEND_EQN_ADD);
CHECK_ENUMS(VK_BLEND_OP_SUBTRACT,                     V3D_BLEND_EQN_SUB);
CHECK_ENUMS(VK_BLEND_OP_REVERSE_SUBTRACT,             V3D_BLEND_EQN_RSUB);
CHECK_ENUMS(VK_BLEND_OP_MIN,                          V3D_BLEND_EQN_MIN);
CHECK_ENUMS(VK_BLEND_OP_MAX,                          V3D_BLEND_EQN_MAX);

CHECK_ENUMS(VK_COMPARE_OP_NEVER,                      V3D_COMPARE_FUNC_NEVER);
CHECK_ENUMS(VK_COMPARE_OP_LESS,                       V3D_COMPARE_FUNC_LESS);
CHECK_ENUMS(VK_COMPARE_OP_EQUAL,                      V3D_COMPARE_FUNC_EQUAL);
CHECK_ENUMS(VK_COMPARE_OP_LESS_OR_EQUAL,              V3D_COMPARE_FUNC_LEQUAL);
CHECK_ENUMS(VK_COMPARE_OP_GREATER,                    V3D_COMPARE_FUNC_GREATER);
CHECK_ENUMS(VK_COMPARE_OP_NOT_EQUAL,                  V3D_COMPARE_FUNC_NOTEQUAL);
CHECK_ENUMS(VK_COMPARE_OP_GREATER_OR_EQUAL,           V3D_COMPARE_FUNC_GEQUAL);
CHECK_ENUMS(VK_COMPARE_OP_ALWAYS,                     V3D_COMPARE_FUNC_ALWAYS);

v3d_addr_t CmdBufState::IndexBufferBinding::CalculateBufferOffsetAddr(VkDeviceSize offset) const
{
   // Include our own internal offset in the address calculation
   return m_buffer->CalculateBufferOffsetAddr(m_offset + offset);
}

VkDeviceSize CmdBufState::IndexBufferBinding::GetBufferSize() const
{
   // Returns the size of the buffer after its starting offset
   return m_buffer->Size() - m_offset;
}

v3d_addr_t CmdBufState::VertexBufferBinding::CalculateBufferOffsetAddr(VkDeviceSize offset) const
{
   // Include our own internal offset in the address calculation
   return m_buffer->CalculateBufferOffsetAddr(m_offset + offset);
}

VkDeviceSize CmdBufState::VertexBufferBinding::GetBufferSize() const
{
   // Returns the size of the buffer after its starting offset
   return m_buffer->Size() - m_offset;
}

bool CmdBufState::VertexBufferBinding::CalcMaxIndex(uint32_t &maxIndex,
            const VkVertexInputAttributeDescription &attDesc,
            uint32_t stride) const
{
   uint32_t offset = static_cast<uint32_t>(m_offset + attDesc.offset);
   uint32_t attribSize = gfx_lfmt_bytes_per_block(Formats::GetLFMT(attDesc.format));

   // not enough data for one vertex
   if (attribSize > m_buffer->Size() || offset > (m_buffer->Size() - attribSize))
      return false;

   // Any index is valid with a stride of zero
   if (stride == 0)
      maxIndex = std::numeric_limits<uint32_t>::max();
   else
      maxIndex = static_cast<uint32_t>((m_buffer->Size() - offset - attribSize) / stride);
   return true;
}


CmdBufState::CmdBufState(Device *device)
{
   assert(countof(m_curVertexBufferBindings) >=
          device->GetPhysicalDevice()->Limits().maxVertexInputBindings);

   m_dirtyBits.reset();
}

void CmdBufState::BindGraphicsPipeline(GraphicsPipeline *pipe)
{
   m_curBoundPipelines[VK_PIPELINE_BIND_POINT_GRAPHICS] = pipe;
   m_dirtyBits.set(eGraphicsPipeline);

   // Overwrite all the non-dynamic state with the current pipeline values
   if (!pipe->IsDynamic(VK_DYNAMIC_STATE_VIEWPORT))
      SetViewport(pipe->GetViewport());

   if (!pipe->IsDynamic(VK_DYNAMIC_STATE_SCISSOR))
      SetScissorRect(pipe->GetScissorRect());

   if (!pipe->IsDynamic(VK_DYNAMIC_STATE_STENCIL_COMPARE_MASK))
   {
      SetStencilCompareMask(VK_STENCIL_FACE_FRONT_BIT, pipe->GetDepthStencilState().front.compareMask);
      SetStencilCompareMask(VK_STENCIL_FACE_BACK_BIT, pipe->GetDepthStencilState().back.compareMask);
   }

   if (!pipe->IsDynamic(VK_DYNAMIC_STATE_STENCIL_WRITE_MASK))
   {
      SetStencilWriteMask(VK_STENCIL_FACE_FRONT_BIT, pipe->GetDepthStencilState().front.writeMask);
      SetStencilWriteMask(VK_STENCIL_FACE_BACK_BIT, pipe->GetDepthStencilState().back.writeMask);
   }

   if (!pipe->IsDynamic(VK_DYNAMIC_STATE_STENCIL_REFERENCE))
   {
      SetStencilReference(VK_STENCIL_FACE_FRONT_BIT, pipe->GetDepthStencilState().front.reference);
      SetStencilReference(VK_STENCIL_FACE_BACK_BIT, pipe->GetDepthStencilState().back.reference);
   }
}

void CmdBufState::BindComputePipeline(ComputePipeline *pipe)
{
   m_curBoundPipelines[VK_PIPELINE_BIND_POINT_COMPUTE] = pipe;
}

void CmdBufState::BindDescriptorSet(VkPipelineBindPoint bindPoint, uint32_t set, VkDescriptorSet ds,
                                    uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets,
                                    uint32_t startDynamicOffset)
{
   DescriptorSetBinding &dsb = m_curDescriptorSetBindings[bindPoint][set];

   dsb.descriptorSet = fromHandle<DescriptorSet>(ds);

   // Deal with dynamic offsets
   if (pDynamicOffsets != nullptr && dynamicOffsetCount > 0)
   {
      uint32_t numOffsetsForSet = dsb.descriptorSet->GetNumDynamicOffsetsNeeded();
      if (startDynamicOffset + numOffsetsForSet <= dynamicOffsetCount)
      {
         dsb.dynamicOffsets.resize(numOffsetsForSet);
         memcpy(dsb.dynamicOffsets.data(), pDynamicOffsets + startDynamicOffset,
                numOffsetsForSet * sizeof(uint32_t));
      }
   }
   else
      dsb.dynamicOffsets.clear();
}

void CmdBufState::SetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask)
{
   if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
      m_frontStencilState.writeMask = writeMask;

   if (faceMask & VK_STENCIL_FACE_BACK_BIT)
      m_backStencilState.writeMask = writeMask;

   SetDirty(CmdBufState::eStencil);
}

void CmdBufState::SetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask)
{
   if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
      m_frontStencilState.compareMask = compareMask;

   if (faceMask & VK_STENCIL_FACE_BACK_BIT)
      m_backStencilState.compareMask = compareMask;

   SetDirty(CmdBufState::eStencil);
}

void CmdBufState::SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference)
{
   if (faceMask & VK_STENCIL_FACE_FRONT_BIT)
      m_frontStencilState.reference = reference;

   if (faceMask & VK_STENCIL_FACE_BACK_BIT)
      m_backStencilState.reference = reference;

   SetDirty(CmdBufState::eStencil);
}

void CmdBufState::BuildScissorCL(CommandBuffer *cb, const VkRect2D &scissorRect, const Viewport &vp)
{
   int scx = scissorRect.offset.x;
   int scy = scissorRect.offset.y;
   int scw = scissorRect.extent.width;
   int sch = scissorRect.extent.height;

   int x = std::max({ 0, vp.x, scx });
   int y = std::max({ 0, vp.y, scy });

   int xmax = std::min({ (int)V3D_MAX_CLIP_WIDTH, vp.x + vp.w, scx + scw });
   int ymax = std::min({ (int)V3D_MAX_CLIP_WIDTH, vp.y + vp.h, scy + sch });

   if (x >= xmax || y >= ymax)
      v3d_cl_clip(cb->CLPtr(), 0, 0, 0, 0);
   else
      v3d_cl_clip(cb->CLPtr(), x, y, xmax - x, ymax - y);
}

void CmdBufState::BuildViewportCL(CommandBuffer *cb, const Viewport &vp)
{
   v3d_cl_clipper_xy(cb->CLPtr(),
      vp.internalScale[0],
      vp.internalScale[1]);

   v3d_cl_viewport_offset_from_rect(cb->CLPtr(), vp.x, vp.y, vp.w, vp.h);

   // Note : NDC space is 0->1 for Z in Vulkan
   v3d_cl_clipper_z(cb->CLPtr(), vp.depthDiff, vp.depthNear);

   v3d_cl_clipz(cb->CLPtr(),
      std::min(vp.depthNear, vp.depthFar),
      std::max(vp.depthNear, vp.depthFar));
}

void CmdBufState::BuildStencilCL(
   CommandBuffer           *cb,
   const VkStencilOpState  &vkState,
   const StencilState      &state,
   bool                     front)
{
   v3d_cl_stencil_cfg(cb->CLPtr(),
      state.reference & 0xff,
      state.compareMask & 0xff,
      TranslateCompareFunc(vkState.compareOp),
      TranslateStencilOp(vkState.failOp),
      TranslateStencilOp(vkState.depthFailOp),
      TranslateStencilOp(vkState.passOp),
      front, !front,
      state.writeMask & 0xff);
}

void CmdBufState::InitEarlyZState(bool enable)
{
   early_z_init(&m_ezState, enable);
}

void CmdBufState::DisableEarlyZ()
{
   early_z_disable(&m_ezState);
}

// This is called as a preamble to all draw calls being added to a command buffer
// to update any dynamically changing state.
void CmdBufState::BuildStateUpdateCL(CommandBuffer *cb)
{
   GraphicsPipeline *pipe = dynamic_cast<GraphicsPipeline*>(
                            m_curBoundPipelines[VK_PIPELINE_BIND_POINT_GRAPHICS]);
   assert(pipe != nullptr);

   if (m_dirtyBits.test(eGraphicsPipeline))
   {
      // Update m_ezState based on the new pipeline state
      pipe->UpdateEarlyZState(&m_ezState);

      // The pipeline has changed, so write the config bits.
      // We have to do this here, rather than as part of the pipeline construction since
      // the early z flags can't be calculated up-front as an earlier non-ez pipeline must
      // block ez for the entire render pass.
      V3D_CL_CFG_BITS_T cfgBits = pipe->GetConfigBits();
      cfgBits.ez        = m_ezState.cfg_bits_ez;
      cfgBits.ez_update = m_ezState.cfg_bits_ez_update;
      v3d_cl_cfg_bits_indirect(cb->CLPtr(), &cfgBits);

      // Now write the static setup control list from the pipeline
      const uint8_t *setupCL;
      size_t size;
      pipe->GetSetupCL(&setupCL, &size);
      uint8_t **clPtr = cb->CLPtr(size);
      memcpy(*clPtr, setupCL, size);
      *clPtr += size;
   }

   if (m_dirtyBits.test(eViewport) || m_dirtyBits.test(eScissor))
      BuildScissorCL(cb, m_scissorRect, m_viewport);

   if (m_dirtyBits.test(eViewport))
      BuildViewportCL(cb, m_viewport);

   // Line width
   if (m_dirtyBits.test(eLineWidth))
      v3d_cl_line_width(cb->CLPtr(), m_lineWidth);

   // Depth bias
   if (pipe->GetDepthBiasEnable() && m_dirtyBits.test(eDepthBias))
   {
      v3d_cl_set_depth_offset(cb->CLPtr(), m_depthBiasSlopeFactor, m_depthBiasConstantFactor,
                                           m_depthBiasClamp,
                                           pipe->GetDepthBits());
   }

   // Blend constants
   if (m_dirtyBits.test(eBlendConsts))
      v3d_cl_blend_ccolor(cb->CLPtr(), m_blendConstants[0], m_blendConstants[1],
                                       m_blendConstants[2], m_blendConstants[3]);

   // Stencil state
   if (m_dirtyBits.test(eStencil))
   {
      BuildStencilCL(cb, pipe->GetDepthStencilState().front, m_frontStencilState, true);
      BuildStencilCL(cb, pipe->GetDepthStencilState().back, m_backStencilState, false);
   }

   if (m_dirtyBits.test(eOcclusionCounter) && cb->m_occlusionAllowed)
      v3d_cl_occlusion_query_counter_enable(cb->CLPtr(), m_occlusionCounterAddr);

   // Clear all the dirty bits we've dealt with
   m_dirtyBits.reset();
}

} // namespace bvk
