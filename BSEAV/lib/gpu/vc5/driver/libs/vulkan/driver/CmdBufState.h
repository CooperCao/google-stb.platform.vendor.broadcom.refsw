/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "Common.h"
#include "DriverLimits.h"
#include "Pipeline.h"
#include "DescriptorSet.h"
#include "Viewport.h"
#include "libs/core/v3d/v3d_gen.h"

#include <memory.h>
#include <array>

namespace bvk {

v3d_compare_func_t TranslateCompareFunc(VkCompareOp op);

class CmdBufState
{
public:
   // These dirty bits only correspond to things that will be updated during BuildStateUpdateCL()
   enum DirtyBitType
   {
      eViewport = 0,
      eScissor,
      eLineWidth,
      eDepthBias,
      eBlendConsts,
      eStencil,
      eGraphicsPipeline,
      eComputePipeline,
      eOcclusionCounter,
      eNumDirtyBits        // Must be last entry
   };

   using DirtyBits = Bitfield<DirtyBitType, eNumDirtyBits, uint32_t>;

   class IndexBufferBinding
   {
   public:
      void Set(Buffer *buffer, VkDeviceSize offset, VkIndexType indexType)
      {
         m_buffer = buffer;
         m_offset = offset;
         m_indexType = indexType;
      }

      v3d_addr_t   CalculateBufferOffsetAddr(VkDeviceSize offset) const;
      VkDeviceSize GetBufferSize() const;
      VkIndexType  GetIndexType() const { return m_indexType; }

   private:
      Buffer      *m_buffer;
      VkDeviceSize m_offset;
      VkIndexType  m_indexType;
   };

   class VertexBufferBinding
   {
   public:
      void Set(const Buffer *buffer, VkDeviceSize offset)
      {
         m_buffer = buffer;
         m_offset = offset;
      }

      v3d_addr_t   CalculateBufferOffsetAddr(VkDeviceSize offset) const;
      VkDeviceSize GetBufferSize() const;
      // return false if there is not enough data for one index in the attrib
      bool CalcMaxIndex(uint32_t &maxIndex,
            const VkVertexInputAttributeDescription &attDesc,
            uint32_t stride) const;

   private:
      const Buffer   *m_buffer{};
      VkDeviceSize    m_offset{};
   };

   struct DescriptorSetBinding
   {
      DescriptorSet          *descriptorSet;
      std::vector<uint32_t>   dynamicOffsets; // Transient - std::vector for now

      uint32_t GetDynamicOffset(uint32_t binding, uint32_t element) const
      {
         if (!dynamicOffsets.empty() && descriptorSet->HasDynamicOffset(binding))
         {
            uint32_t dynamicIndex = descriptorSet->DynamicOffsetIndexFor(binding) + element;
            assert(dynamicIndex < dynamicOffsets.size());
            return dynamicOffsets[dynamicIndex];
         }
         return 0;
      }
   };

   using DescriptorSetBindings = std::array<DescriptorSetBinding, DriverLimits::eMaxBoundDescriptorSets>;

   struct StencilState
   {
      uint32_t compareMask;
      uint32_t writeMask;
      uint32_t reference;
   };

public:
   CmdBufState(Device *device);

   void BindGraphicsPipeline(GraphicsPipeline *pipe);
   void BindComputePipeline(ComputePipeline *pipe);

   const GraphicsPipeline *BoundGraphicsPipeline() const
   {
      return dynamic_cast<GraphicsPipeline*>(
         m_curBoundPipelines[VK_PIPELINE_BIND_POINT_GRAPHICS]);
   }

   const ComputePipeline *BoundComputePipeline() const
   {
      return dynamic_cast<ComputePipeline*>(
         m_curBoundPipelines[VK_PIPELINE_BIND_POINT_COMPUTE]);
   }

   void BindIndexBuffer(Buffer *buffer, VkDeviceSize offset, VkIndexType indexType)
   {
      m_curIndexBufferBinding.Set(buffer, offset, indexType);
   }

   const IndexBufferBinding &BoundIndexBuffer() const
   {
      return m_curIndexBufferBinding;
   }

   void BindVertexBuffer(uint32_t slot, Buffer *buffer, VkDeviceSize offset)
   {
      m_curVertexBufferBindings[slot].Set(buffer, offset);
   }

   const VertexBufferBinding &BoundVertexBuffer(uint32_t i) const
   {
      return m_curVertexBufferBindings[i];
   }

   void BindDescriptorSet(VkPipelineBindPoint bindPoint, uint32_t set, VkDescriptorSet ds,
                          uint32_t dynamicOffsetCount, const uint32_t *pDynamicOffsets,
                          uint32_t startDynamicOffset);

   const DescriptorSetBindings &BoundDescriptorSetArray(VkPipelineBindPoint bindPoint) const
   {
      return m_curDescriptorSetBindings[bindPoint];
   }

   const DescriptorSetBinding &BoundDescriptorSet(VkPipelineBindPoint bindPoint, uint32_t set) const
   {
      return m_curDescriptorSetBindings[bindPoint][set];
   }

   void SetStencilWriteMask(VkStencilFaceFlags faceMask, uint32_t writeMask);
   void SetStencilCompareMask(VkStencilFaceFlags faceMask, uint32_t compareMask);
   void SetStencilReference(VkStencilFaceFlags faceMask, uint32_t reference);

   const Viewport &GetViewport() const { return m_viewport; }
   void SetViewport(const Viewport &val)
   {
      m_viewport = val;
      SetDirty(CmdBufState::eViewport);
   }

   void SetScissorRect(const VkRect2D &val)
   {
      m_scissorRect = val;
      SetDirty(CmdBufState::eScissor);
   }

   void SetLineWidth(float val)
   {
      m_lineWidth = val;
      SetDirty(CmdBufState::eLineWidth);
   }

   void DepthBias(float *dbConstFactor, float *dbClamp, float *dbSlopeFactor) const
   {
      *dbConstFactor = m_depthBiasConstantFactor;
      *dbClamp = m_depthBiasClamp;
      *dbSlopeFactor = m_depthBiasSlopeFactor;
   }

   void SetDepthBias(float dbConstFactor, float dbClamp, float dbSlopeFactor)
   {
      m_depthBiasConstantFactor = dbConstFactor;
      m_depthBiasClamp = dbClamp;
      m_depthBiasSlopeFactor = dbSlopeFactor;
      SetDirty(CmdBufState::eDepthBias);
   }

   void SetBlendConstants(const float val[4])
   {
      memcpy(m_blendConstants, val, 4 * sizeof(float));
      SetDirty(CmdBufState::eBlendConsts);
   }

   void SetOcclusionCounter(v3d_addr_t addr)
   {
      if (addr != m_occlusionCounterAddr)
      {
         m_occlusionCounterAddr = addr;
         SetDirty(CmdBufState::eOcclusionCounter);
      }
   }

   DirtyBits GetDirtyBits()               { return m_dirtyBits; }
   const DirtyBits &GetDirtyBits() const  { return m_dirtyBits; }
   bool IsDirty(DirtyBitType type) const  { return m_dirtyBits.IsSet(type); }

   void SetDirty(DirtyBitType type)       { m_dirtyBits.Set(type);   }
   void SetAllDirty()                     { m_dirtyBits.SetAll();    }
   void SetDirty(std::initializer_list<DirtyBitType> types) { m_dirtyBits.Set(types); }

   void ClearDirty(DirtyBitType type)     { m_dirtyBits.Clear(type); }
   void ClearAllDirty()                   { m_dirtyBits.ClearAll();  }
   void ClearDirty(std::initializer_list<DirtyBitType> types) { m_dirtyBits.Clear(types); }

   void BuildStateUpdateCL(CommandBuffer *cb);

private:
   void BuildScissorCL (CommandBuffer *cb, const VkRect2D &scissorRect, const Viewport &vp);
   void BuildViewportCL(CommandBuffer *cb, const Viewport &vp);

   void BuildStencilCL(
      CommandBuffer           *cb,
      const VkStencilOpState  &vkState,
      const StencilState      &state,
      bool                    front);

private:
   Pipeline             *m_curBoundPipelines[VK_PIPELINE_BIND_POINT_RANGE_SIZE] = {};
   IndexBufferBinding    m_curIndexBufferBinding = {};
   VertexBufferBinding   m_curVertexBufferBindings[V3D_MAX_ATTR_ARRAYS] = {};
   DescriptorSetBindings m_curDescriptorSetBindings[VK_PIPELINE_BIND_POINT_RANGE_SIZE]{};

   // These dynamic state items will override the settings in the
   // bound Pipeline if they are defined as dynamic in the Pipeline.
   // We will always use these values (initialised from the Pipeline)
   // as the master.
   Viewport             m_viewport;
   VkRect2D             m_scissorRect;
   float                m_lineWidth;
   float                m_depthBiasConstantFactor;
   float                m_depthBiasClamp;
   float                m_depthBiasSlopeFactor;
   float                m_blendConstants[4] = {};
   StencilState         m_frontStencilState;
   StencilState         m_backStencilState;
   v3d_addr_t           m_occlusionCounterAddr = 0;

   // A bit-mask of dirty bits (dirty with respect to the control list being built)
   DirtyBits            m_dirtyBits;
};

} // namespace bvk
