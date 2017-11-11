/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "CommandBuffer.h"
#include "Command.h"
#include "ColorAspectCommandBuilder.h"
#include "DSAspectCommandBuilder.h"

#include "libs/core/v3d/v3d_gen.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_shadrec.h"
#include "libs/core/v3d/v3d_clear_shader.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

namespace bvk {

using RectArray = std::vector<VkRect2D>;

static void NVVertex(uint32_t *addr, uint32_t x, uint32_t y, uint32_t z)
{
   // Clipping and 1/Wc come from defaults
   addr[0] = x << 8;
   addr[1] = y << 8;
   addr[2] = z;
}

// Allocate vertex data for DrawClearRect
void CommandBuffer::DrawRectVertexData(DevMemRange *devMem, uint32_t *dataMaxIndex,
                                       uint32_t rectCount, const VkRect2D *rects, uint32_t z)
{
   const size_t attComponents = 3; // x, y, z

   *dataMaxIndex = rectCount * 4 - 1;

   NewDevMemRange(devMem, attComponents * sizeof(uint32_t) * (*dataMaxIndex + 1), V3D_ATTR_ALIGN);

   uint32_t *addr = static_cast<uint32_t*>(devMem->Ptr());

   for (uint32_t r = 0; r < rectCount; r++)
   {
      const VkRect2D &rect = rects[r];

      uint32_t xmax = rect.offset.x + rect.extent.width;
      uint32_t ymax = rect.offset.y + rect.extent.height;

      NVVertex(addr, rect.offset.x, rect.offset.y, z);
      addr += attComponents;
      NVVertex(addr, xmax, rect.offset.y, z);
      addr += attComponents;
      NVVertex(addr, xmax, ymax, z);
      addr += attComponents;
      NVVertex(addr, rect.offset.x, ymax, z);
      addr += attComponents;
   }
}

void CommandBuffer::DrawSingleLayerClearRects(ControlListBuilder &cb, uint32_t rtIndex,
                                              const VkClearAttachment &ca,
                                              uint32_t rectCount, const VkRect2D *rects,
                                              GFX_LFMT_T format)
{
   uint32_t   *shaderCode;
   uint32_t    shaderSize;

   // Allocate devMem storage for uniforms
   DevMemRange uniformMem;
   NewDevMemRange(&uniformMem, V3D_CLEAR_SHADER_MAX_UNIFS * sizeof(uint32_t), V3D_QPU_UNIFS_ALIGN);
   uint32_t *unifPtr = static_cast<uint32_t*>(uniformMem.Ptr());

   if (gfx_lfmt_has_color(format))
   {
      assert(ca.aspectMask == VK_IMAGE_ASPECT_COLOR_BIT);

      const uint32_t *colVal = ca.clearValue.color.uint32;

      V3D_RT_FORMAT_T rt_format;
      gfx_lfmt_translate_rt_format(&rt_format, format);

      v3d_clear_shader_color(&shaderCode, &shaderSize, unifPtr, rt_format.type, rtIndex, colVal);
   }
   else
   {
      v3d_clear_shader_no_color(&shaderCode, &shaderSize, unifPtr);
   }

   uint32_t z = 0;
   if (gfx_lfmt_has_depth(format))
      z = gfx_float_to_bits(ca.clearValue.depthStencil.depth);

   // Use x, y, z to create vertex data to be used in the clear
   DevMemRange vdataMem;
   uint32_t    vdataMaxIndex;
   DrawRectVertexData(&vdataMem, &vdataMaxIndex, rectCount, rects, z);

   // Install the shader
   DevMemRange fshaderMem;
   NewDevMemRange(&fshaderMem, shaderSize, V3D_QPU_INSTR_ALIGN);
   memcpy(fshaderMem.Ptr(), shaderCode, shaderSize);

   // Find what sized allocations we need for the NV shader record
   V3D_NV_SHADER_RECORD_ALLOC_SIZES_T  sizes;
   v3d_get_nv_shader_record_alloc_sizes(&sizes);

   // Allocate the device memory for defaults
   DevMemRange defaultsMem;
   NewDevMemRange(&defaultsMem, sizes.defaults_size, sizes.defaults_align);

   // Allocate the device memory for the shader record
   DevMemRange shadRecMem;
   NewDevMemRange(&shadRecMem, sizes.packed_shader_rec_size, sizes.packed_shader_rec_align);

   // Fill the NV shader record
   v3d_create_nv_shader_record(
      static_cast<uint32_t*>(shadRecMem.Ptr()),  shadRecMem.Phys(),
      static_cast<uint32_t*>(defaultsMem.Ptr()), defaultsMem.Phys(),
      fshaderMem.Phys(), uniformMem.Phys(), vdataMem.Phys(), vdataMaxIndex,
      /*does_z_writes=*/false,
#if V3D_VER_AT_LEAST(4,1,34,0)
      V3D_THREADING_4
#else
      V3D_THREADING_1
#endif
      );

   // Calculate write mask
   // Note: since we are only ever writing to one renderTarget at a time currently
   // we have no need for color masking other than all-on or all-off
   const uint32_t rtWriteMask = !gfx_lfmt_has_color(format) ? gfx_mask(V3D_MAX_RENDER_TARGETS * 4) : 0;

   const bool depth   = gfx_lfmt_has_depth(format)   && (ca.aspectMask & VK_IMAGE_ASPECT_DEPTH_BIT);
   const bool stencil = gfx_lfmt_has_stencil(format) && (ca.aspectMask & VK_IMAGE_ASPECT_STENCIL_BIT);

   // We need the union of all the rects to use as the clip rect
   VkRect2D clipRect = rects[0];
   for (uint32_t r = 1; r < rectCount; r++)
      Union(&clipRect, clipRect, rects[r]);

   m_td->InsertNVShaderRenderState(clipRect, depth, stencil, rtWriteMask);

   if (gfx_lfmt_has_stencil(format))
   {
      v3d_cl_stencil_cfg(cb.CLPtr(),
         ca.clearValue.depthStencil.stencil,
         0xFFu,                              // test mask
         V3D_COMPARE_FUNC_ALWAYS,            // test function
         V3D_STENCIL_OP_ZERO,                // stencil fail op = don't care
         V3D_STENCIL_OP_ZERO,                // depth fail op = don't care
         V3D_STENCIL_OP_REPLACE,             // pass op = replace
         true,                               // back config
         true,                               // front config
         0xFFu);                             // stencil write mask
   }

   v3d_cl_nv_shader(cb.CLPtr(), 2, shadRecMem.Phys());

   for (uint32_t r = 0; r < rectCount; r++)
      v3d_cl_vertex_array_prims(cb.CLPtr(), V3D_PRIM_MODE_TRI_FAN, 4, r * 4);

   // Ensure dynamic state will get refreshed for next draw
   CurState().SetAllDirty();
}

void CommandBuffer::DrawMultiLayerClearRects(uint32_t rtIndex, const VkClearAttachment &ca,
                                             uint32_t rectCount, const VkClearRect *rects,
                                             GFX_LFMT_T format, const VkImageSubresourceRange &srr)
{
   // NOTE: the baseArrayLayer in the rects is treated as a relative offset from the baseArrayLayer in
   // the srr. However, because the TLBAttachmentInfo has already taken the srr baseArrayLayer into
   // account, we can ignore the srr baseArrayLayer for the most part.

   // Find the largest layer we will be writing
   uint32_t maxLayer = srr.layerCount;
   for (uint32_t r = 0; r < rectCount; r++)
      maxLayer = std::max(maxLayer, rects[r].baseArrayLayer + rects[r].layerCount);

   // Make rect lists for each layer to be cleared
   std::vector<RectArray> layers(maxLayer);

   for (uint32_t r = 0; r < rectCount; r++)
   {
      for (uint32_t l = rects[r].baseArrayLayer; l < rects[r].baseArrayLayer + rects[r].layerCount; l++)
      {
         if (l < srr.layerCount) // Only clear layers that exist in the srr
            layers[l].push_back(rects[r].rect);
      }
   }

   // Flush the currently recording control list (this has an implicit execution barrier)
   if (m_level == VK_COMMAND_BUFFER_LEVEL_PRIMARY)
      FlushPrimaryControlList();
   else
      FlushSecondaryControlList();

   for (uint32_t layer = 0; layer < layers.size(); layer++)
   {
      RectArray &rects = layers[layer];

      if (rects.size() > 0)
      {
         // Get tlb params for the attachment (this is already relative to the srr.baseArrayLayer)
         uint32_t attIndex = gfx_lfmt_has_color(format) ?
                             m_td->m_curSubpassGroup->m_colorRTAttachments[ca.colorAttachment] :
                             m_td->m_curSubpassGroup->m_dsAttachment;

         auto     tlbInfo  = m_td->GetTLBAttachmentInfo(attIndex);

         // Adjust for this layer
         tlbInfo.ldstParams.addr += tlbInfo.layerStride * layer;

         // We need the union of all the rects to use as the frame size
         VkRect2D clipRect = rects[0];
         for (uint32_t r = 1; r < rects.size(); r++)
            Union(&clipRect, clipRect, rects[r]);

         uint32_t regionW = clipRect.offset.x + clipRect.extent.width;
         uint32_t regionH = clipRect.offset.y + clipRect.extent.height;

         if (tlbInfo.isMultisampled)
         {
            regionW *= 2;
            regionH *= 2;
         }

         // Make a unique_ptr for the base-class ControlListBuilder
         std::unique_ptr<ControlListBuilder, std::function<void(ControlListBuilder*)>>
                                         clb(nullptr, [](ControlListBuilder *ptr) {::delete ptr;});

         ControlListBuilder::ControlList drawList;

         if (gfx_lfmt_has_color(format))
         {
            assert((ca.aspectMask  & VK_IMAGE_ASPECT_COLOR_BIT) != 0);
            assert((srr.aspectMask & VK_IMAGE_ASPECT_COLOR_BIT) != 0);

            ColorAspectCommandBuilder *cb = ::new ColorAspectCommandBuilder(GetCallbacks(), this);
            clb.reset(cb); // Set the unique_ptr

            cb->SetDstTLBParamsDirect(tlbInfo.ldstParams, tlbInfo.isMultisampled, regionW, regionH);
            cb->SetLoadDestination();
            cb->SetDrawList(&drawList);
         }
         else
         {
            VkImageAspectFlags img = (gfx_lfmt_has_depth(format)   ? VK_IMAGE_ASPECT_DEPTH_BIT   : 0) |
                                     (gfx_lfmt_has_stencil(format) ? VK_IMAGE_ASPECT_STENCIL_BIT : 0);
            VkImageAspectFlags aspect = img & ca.aspectMask & srr.aspectMask;

            assert((aspect & (VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT)) != 0);

            DSAspectCommandBuilder *cb = ::new DSAspectCommandBuilder(GetCallbacks(), this);
            clb.reset(cb); // Set the unique_ptr

            cb->SetDstTLBParamsDirect(tlbInfo.ldstParams, tlbInfo.isMultisampled, aspect, regionW, regionH);
            cb->SetLoadDestination();
            cb->SetDrawList(&drawList);
         }

         clb->SetCurrentControlList(&drawList);

         drawList.SetStart(*clb->m_curDeviceBlock);

         DrawSingleLayerClearRects(*clb, rtIndex, ca, rects.size(), rects.data(), format);

         v3d_cl_return(clb->CLPtr());

         drawList.SetEnd(*clb->m_curDeviceBlock);

         clb->SetCurrentControlList(nullptr);

         auto cmd = NewObject<CmdBinRenderJobObj>(m_device->GetPhysicalDevice());

         // Default control list builder sync flags are OK
         clb->CreateMasterControlLists(cmd);

         m_commandList.push_back(cmd);
      }
   }
   InsertExecutionBarrier();
}

void CommandBuffer::DrawClearRect(uint32_t rtIndex, const VkClearAttachment &ca,
                                  const VkRect2D &rect, GFX_LFMT_T format,
                                  const VkImageSubresourceRange &srr)
{
   // We can safely ignore srr.baseArrayLayer as that has already been taken care of by the
   // TLB setup pointing to the baseLayer.
   if (srr.layerCount == 1)
   {
      m_td->NeedControlList();
      DrawSingleLayerClearRects(*m_td, rtIndex, ca, 1, &rect, format);
   }
   else
   {
      VkClearRect cr;
      cr.baseArrayLayer = 0;  // This is relative to the srr baseArrayLayer, hence 0
      cr.layerCount     = srr.layerCount;
      cr.rect           = rect;

      // Always rt0 for multi-clearing
      DrawMultiLayerClearRects(/*rtIndex=*/0, ca, 1, &cr, format, srr);
   }
}

static bool AllRectsInFirstLayer(uint32_t rectCount, const VkClearRect *rects)
{
   for (uint32_t r = 0; r < rectCount; r++)
      if (rects[r].baseArrayLayer != 0 || rects[r].layerCount != 1)
         return false;

   return true;
}

void CommandBuffer::DrawClearRects(uint32_t rtIndex, const VkClearAttachment &ca,
                                   uint32_t rectCount, const VkClearRect *rects, GFX_LFMT_T format,
                                   const VkImageSubresourceRange &srr)
{
   // We can safely ignore srr.baseArrayLayer as that has already been taken care of by the
   // TLB setup pointing to the baseLayer.
   if (AllRectsInFirstLayer(rectCount, rects))
   {
      RectArray rectArr;
      for (uint32_t r = 0; r < rectCount; r++)
         rectArr.push_back(rects[r].rect);

      m_td->NeedControlList();
      DrawSingleLayerClearRects(*m_td, rtIndex, ca, rectArr.size(), rectArr.data(), format);
   }
   else
   {
      // Always rt0 for multi-clearing
      DrawMultiLayerClearRects(/*rtIndex=*/0, ca, rectCount, rects, format, srr);
   }
}

bool CommandBuffer::PrimaryClearRequired(uint32_t rectCount, const VkClearRect *rects)
{
   return !AllRectsInFirstLayer(rectCount, rects);
}

} // namespace bvk
