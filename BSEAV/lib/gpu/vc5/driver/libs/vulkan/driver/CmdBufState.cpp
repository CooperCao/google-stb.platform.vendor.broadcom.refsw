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
#include "DerivedViewportState.h"
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

v3d_compare_func_t TranslateCompareFunc(VkCompareOp op)
{
   assert(op >= VK_COMPARE_OP_NEVER && op <= VK_COMPARE_OP_ALWAYS);
   return static_cast<v3d_compare_func_t>(op);
}

static v3d_stencil_op_t TranslateStencilOp(VkStencilOp op)
{
   switch (op)
   {
   case VK_STENCIL_OP_ZERO:                  return V3D_STENCIL_OP_ZERO;
   case VK_STENCIL_OP_KEEP:                  return V3D_STENCIL_OP_KEEP;
   case VK_STENCIL_OP_REPLACE:               return V3D_STENCIL_OP_REPLACE;
   case VK_STENCIL_OP_INCREMENT_AND_CLAMP:   return V3D_STENCIL_OP_INCR;
   case VK_STENCIL_OP_DECREMENT_AND_CLAMP:   return V3D_STENCIL_OP_DECR;
   case VK_STENCIL_OP_INVERT:                return V3D_STENCIL_OP_INVERT;
   case VK_STENCIL_OP_INCREMENT_AND_WRAP:    return V3D_STENCIL_OP_INCWRAP;
   case VK_STENCIL_OP_DECREMENT_AND_WRAP:    return V3D_STENCIL_OP_DECWRAP;
   default:
      unreachable();
      return V3D_STENCIL_OP_INVALID;
   }
}

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
            const VkVertexInputBindingDescription &bindDesc) const
{
   uint32_t offset = static_cast<uint32_t>(m_offset + attDesc.offset);
   uint32_t attribSize = Formats::NumBytes(attDesc.format);

   // not enough data for one vertex
   if (attribSize > m_buffer->Size() || offset > (m_buffer->Size() - attribSize))
      return false;

   // Any index is valid with a stride of zero
   if (bindDesc.stride == 0)
      maxIndex = std::numeric_limits<uint32_t>::max();
   else
      maxIndex = static_cast<uint32_t>((m_buffer->Size() - offset - attribSize) / bindDesc.stride);
   return true;
}


CmdBufState::CmdBufState(Device *device)
{
   assert(countof(m_curVertexBufferBindings) >=
          device->GetPhysicalDevice()->Limits().maxVertexInputBindings);

   m_dirtyBits.ClearAll();
}

void CmdBufState::BindGraphicsPipeline(GraphicsPipeline *pipe)
{
   m_curBoundPipelines[VK_PIPELINE_BIND_POINT_GRAPHICS] = pipe;
   m_dirtyBits.Set(eGraphicsPipeline);

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

void CmdBufState::BuildScissorAndViewportCL(
   CommandBuffer     *cb,
   const VkRect2D    *scissorRect,
   const VkViewport  *viewport)
{
   int scx = scissorRect->offset.x;
   int scy = scissorRect->offset.y;
   int scw = scissorRect->extent.width;
   int sch = scissorRect->extent.height;

   int vpx = std::lround(viewport->x);
   int vpy = std::lround(viewport->y);

   // The Vulkan spec says that the application should move the viewport origin to
   // the bottom left if using a -ve viewport height. We need to undo that to do the
   // correct thing.
   if (viewport->height < 0.0f)
      vpy = std::lround(viewport->y + viewport->height);

   int vpw = std::lround(viewport->width);
   int vph = abs(std::lround(viewport->height));   // Ensure +ve height is used here

   float vpNear = viewport->minDepth;
   float vpFar  = viewport->maxDepth;

   // Calculate intermediate values
   int x = std::max({ 0, vpx, scx });
   int y = std::max({ 0, vpy, scy });

   int xmax = std::min({ (int)V3D_MAX_CLIP_WIDTH, vpx + vpw, scx + scw });
   int ymax = std::min({ (int)V3D_MAX_CLIP_WIDTH, vpy + vph, scy + sch });

#if V3D_VER_AT_LEAST(4,1,34,0)
   if (x >= xmax || y >= ymax)
      v3d_cl_clip(cb->CLPtr(), 0, 0, 0, 0);
   else
      v3d_cl_clip(cb->CLPtr(), x, y, xmax - x, ymax - y);
#else
   if (cb->m_td->m_curRenderPassState.framebuffer == NULL)
   {
      // If we get here we are in a secondary command buffer where the framebuffer
      // has not been given in the inheritance info. We can't write the clip record
      // yet because we have to clamp to the framebuffer size.
      // We need to break the control list at this point and insert a command in
      // the command buffer to allow us to patch in the clip record later.
      cb->SplitSecondaryControlList();

      auto clipObj = cb->NewObject<CmdSecondaryDeferredClipObj>(x, y, xmax, ymax);

      // Add the deferred clip command into the command list. When recorded in a primary,
      // this command will be converted into a real clip record that takes account of the
      // framebuffer size.
      cb->m_commandList.push_back(clipObj);
   }
   else
   {
      cb->InsertClampedClipRecord(x, y, xmax, ymax);
   }
#endif

   DerivedViewportState derivedState(*viewport);

   v3d_cl_clipper_xy(cb->CLPtr(),
      derivedState.internalScale[0],
      derivedState.internalScale[1]);

   v3d_cl_viewport_offset_from_rect(cb->CLPtr(), vpx, vpy, vpw, vph);

   // Note : NDC space is 0->1 for Z in Vulkan
   v3d_cl_clipper_z(cb->CLPtr(),
      derivedState.internalDepthRangeDiff,
      derivedState.internalZOffset);

   v3d_cl_clipz(cb->CLPtr(),
      std::min(vpNear, vpFar),
      std::max(vpNear, vpFar));
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

void CmdBufState::BuildStateUpdateCL(CommandBuffer *cb)
{
   GraphicsPipeline *pipe = dynamic_cast<GraphicsPipeline*>(
                            m_curBoundPipelines[VK_PIPELINE_BIND_POINT_GRAPHICS]);
   assert(pipe != nullptr);

#if V3D_VER_AT_LEAST(4,1,34,0)
   if (m_dirtyBits.IsSet(eGraphicsPipeline))
#else
   if (m_dirtyBits.IsSet(eGraphicsPipeline) || m_dirtyBits.IsSet(eCullEverything))
#endif
   {
      const uint8_t *setupCL;
      size_t size;
#if V3D_VER_AT_LEAST(4,1,34,0)
      pipe->GetSetupCL(&setupCL, &size);
#else
      pipe->GetSetupCL(m_cullEverything, &setupCL, &size);
      m_dirtyBits.Clear(eCullEverything);
#endif
      uint8_t **clPtr = cb->CLPtr(size);
      memcpy(*clPtr, setupCL, size);
      *clPtr += size;

      m_dirtyBits.Clear(eGraphicsPipeline);
   }

   if (m_dirtyBits.IsSet(eViewport) || m_dirtyBits.IsSet(eScissor))
      BuildScissorAndViewportCL(cb, &m_scissorRect, &m_viewport);

   // Line width
   if (m_dirtyBits.IsSet(eLineWidth))
      v3d_cl_line_width(cb->CLPtr(), m_lineWidth);

   // Depth bias
   if (pipe->GetDepthBiasEnable() && m_dirtyBits.IsSet(eDepthBias))
   {
      v3d_cl_set_depth_offset(cb->CLPtr(), m_depthBiasSlopeFactor, m_depthBiasConstantFactor,
#if V3D_VER_AT_LEAST(4,1,34,0)
                                           m_depthBiasClamp,
#endif
                                           pipe->GetDepthBits());
   }

   // Blend constants
   if (m_dirtyBits.IsSet(eBlendConsts))
      v3d_cl_blend_ccolor(cb->CLPtr(), m_blendConstants[0], m_blendConstants[1],
                                       m_blendConstants[2], m_blendConstants[3]);

   // Stencil state
   if (m_dirtyBits.IsSet(eStencil))
   {
      BuildStencilCL(cb, pipe->GetDepthStencilState().front, m_frontStencilState, true);
      BuildStencilCL(cb, pipe->GetDepthStencilState().back, m_backStencilState, false);
   }

   if (m_dirtyBits.IsSet(eOcclusionCounter))
      v3d_cl_occlusion_query_counter_enable(cb->CLPtr(), m_occlusionCounterAddr);

   // Clear all the dirty bits we've dealt with
   m_dirtyBits.ClearAll();
}

} // namespace bvk
