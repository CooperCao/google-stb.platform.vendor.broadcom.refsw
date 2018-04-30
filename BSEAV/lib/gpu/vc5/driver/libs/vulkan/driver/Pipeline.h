/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include <vulkan.h>

#include "ForwardDecl.h"
#include "NonCopyable.h"
#include "Allocating.h"
#include "DevMemDataBlock.h"
#include "LinkResult.h"
#include "Compiler.h"
#include "PipelineLayout.h"
#include "Device.h"
#include "Viewport.h"

#include "libs/compute/compute.h"
#include "libs/util/early_z/early_z.h"

#include <mutex>

namespace bvk {

/////////////////////////////////////////////////////////////////////////////////////////
// LinkResultHandle -- manages the lifetime of a link result
/////////////////////////////////////////////////////////////////////////////////////////

using LinkResultHandle = UniqueHandle<LinkResult, VK_SYSTEM_ALLOCATION_SCOPE_OBJECT>;

/////////////////////////////////////////////////////////////////////////////////////////
// Abstract base class for Pipelines
/////////////////////////////////////////////////////////////////////////////////////////
class Pipeline : public NonCopyable, public Allocating
{
public:
   Pipeline(DevMemHeap *device, const VkAllocationCallbacks *pCallbacks, VkPipelineBindPoint bindPoint);

   virtual ~Pipeline() noexcept {}

   union Uniform
   {
      uint32_t u = 0;
      int32_t i;
      float f;
   };

   struct UniformPatchNode
   {
      UniformPatchNode(BackendUniformFlavour uniformFlavour, uint32_t offset, uint32_t value) :
         uniformFlavour(uniformFlavour), offset(offset), value(value) {}

      BackendUniformFlavour         uniformFlavour;
      uint32_t                      offset;
      uint32_t                      value;
   };

   using Uniforms = bvk::vector<Uniform>;
   using UniformPatches = bvk::vector<UniformPatchNode>;

   struct UniformData
   {
      UniformData(Allocating &allocating)
         :  defaults(allocating.GetObjScopeAllocator<Uniform>()),
            patches(allocating.GetObjScopeAllocator<UniformPatchNode>()) {}

      Uniforms       defaults;
      UniformPatches patches;
   };

   VkPipelineBindPoint   GetBindPoint() const      { return m_bindPoint; }
   const PipelineLayout *GetPipelineLayout() const { return fromHandle<PipelineLayout>(m_layout); }
   const LinkResult     &GetLinkResult() const     { return *m_linkResult.GetPtr(); }

   bool IsBroken() const { return m_broken; }

protected:
   void CompileAndLinkShaders(
      const VkPipelineShaderStageCreateInfo *stages,
      size_t numStages,
      const std::bitset<V3D_MAX_ATTR_ARRAYS> &attribRBSwaps,
      std::function<GLSL_BACKEND_CFG_T(GLSL_PROGRAM_T const*)> calcBackendKey,
      const Compiler::Controls &controls);

   void BuildUniforms(UniformData &uniforms, ShaderFlavour flavour, const ShaderData  &shaderData);
   virtual void BuildUniformSpecial(UniformData &uniforms, uint32_t offset, BackendSpecialUniformFlavour special) = 0;

protected:
   DevMemHeap             *m_devMemHeap;
   VkPipelineBindPoint     m_bindPoint;
   VkPipelineLayout        m_layout;

   // The linker results
   LinkResultHandle        m_linkResult;

   // One giant mutex to protect the glsl bits of the compiler
   static std::mutex       m_compLinkMutex;

   // True if the pipeline is in a broken state due to shader compile/link problems for example
   bool                    m_broken = false;

private:
   void CreateDefaultFragmentShader(CompiledShaderHandle compiledShaders[SHADER_FLAVOUR_COUNT],
                                    const Compiler::Controls &controls);
};

/////////////////////////////////////////////////////////////////////////////////////////
// Graphics pipeline
/////////////////////////////////////////////////////////////////////////////////////////

// Notes:
// We don't expose support for optional logicOp
// We don't have h/w support for optional depth bounds test.

class GraphicsPipeline : public Pipeline
{
public:
   using DynamicStateBits = std::bitset<VK_DYNAMIC_STATE_RANGE_SIZE>;

   struct ShaderPatchingData
   {
      bool hasT;
      bool hasG;
      uint32_t offsetGLGeom[LinkResult::SHADER_VPS_COUNT]{};
      uint32_t offsetGLMain{};
      uint32_t offsetGLAttr{};
   };

   static const uint32_t UNIFORM_PATCHTAG = 0x9A7C47A9;

   static const size_t MAX_SETUP_CL_SIZE = V3D_CL_CFG_BITS_SIZE      +
                                           V3D_CL_POINT_SIZE_SIZE    +     // Default for T+G
                                           V3D_CL_BLEND_ENABLES_SIZE +
                                           V3D_CL_BLEND_CFG_SIZE * V3D_MAX_RENDER_TARGETS +
                                           V3D_CL_COLOR_WMASKS_SIZE  +
                                           V3D_CL_SAMPLE_STATE_SIZE  +
                                           V3D_CL_BLEND_CCOLOR_SIZE  +
                                           V3D_CL_LINE_WIDTH_SIZE    +
                                           V3D_CL_DEPTH_OFFSET_SIZE  +
                                           V3D_CL_VCM_CACHE_SIZE     +
                                           V3D_CL_VARY_FLAGS_SIZE * V3D_MAX_VARY_FLAG_WORDS * 3;

public:
   GraphicsPipeline(
      const VkAllocationCallbacks         *pCallbacks,
      Device                              *device,
      const VkGraphicsPipelineCreateInfo  *createInfo);

   virtual ~GraphicsPipeline();

   bool IsDynamic(VkDynamicState test) const { return m_dynamicStateBits.test(test); }

   const Viewport  &GetViewport()    const { return m_viewport; }
   const VkRect2D  &GetScissorRect() const { return m_scissorRect; }

   const VkPipelineDepthStencilStateCreateInfo  &GetDepthStencilState() const { return m_depthStencilState; }

   uint32_t                      GetDepthBits()           const { return m_depthBits; }
   const v3d_prim_mode_t        &GetDrawPrimMode()        const { return m_drawPrimMode; }
   bool                          PrimitiveRestartEnable() const { return m_primitiveRestartEnable; }
   bool                          GetDepthBiasEnable()     const { return m_depthBiasEnable; }
   const bvk::vector<uint32_t>  &GetShaderRecord()        const { return m_shaderRecord; }
   const ShaderPatchingData     &GetShaderPatchingData()  const { return m_shaderPatchingData; }


   const UniformData &GetFSUniforms() const { return m_fsUniforms; }
   const UniformData &GetVPSUniforms(LinkResult::eVertexPipeStage s, shader_mode m) const
   {
      return m_vpsUniforms[s*MODE_COUNT + m];
   }

   void GetSetupCL(const uint8_t **cl, size_t *size) const
   {
      *cl = m_setupCL.data();
      *size = m_setupCLSize;
   }

   const VkVertexInputBindingDescription &GetVertexBindingDesc(uint32_t binding) const
   {
      assert(binding < V3D_MAX_ATTR_ARRAYS);
      return m_vertexBindingDescriptions[binding];
   }

   const VkVertexInputAttributeDescription &GetVertexAttributeDesc(uint32_t attr) const
   {
      assert(attr < V3D_MAX_ATTR_ARRAYS);
      return m_vertexAttributeDescriptions[attr];
   }

   bool UpdateEarlyZState(EARLY_Z_STATE_T *state);

   const V3D_CL_CFG_BITS_T &GetConfigBits() const { return m_cfgBits; }

protected:
   void BuildUniformSpecial(UniformData &uniforms, uint32_t offset, BackendSpecialUniformFlavour special) override;

private:
   static DynamicStateBits CalcDynamicStateBits(
      uint32_t dynamicStateCount,
      const VkDynamicState *dynamicStates);

   void CreateShaderRecord(bool rasterizerDiscard, bool sampleShading,
                           v3d_vpm_cfg_v vpmV[2], uint32_t vpmSize, uint32_t patchControlPoints);
   void CreateShaderRecordDummyAttribute(uint32_t *dstPtr);
   static uint32_t CalcBackendKey(const VkGraphicsPipelineCreateInfo *ci);
   uint32_t CalcBlendState(uint32_t *blendEnables, V3D_CL_BLEND_CFG_T *blendCfg,
                           const VkPipelineColorBlendStateCreateInfo *ci) const;
   void CalcCfgBits(V3D_CL_CFG_BITS_T *cfg_bits,
                    const VkPipelineRasterizationStateCreateInfo *rastCi,
                    bool ms, bool blendEnable,
                    bool rpEarlyZCompatible, bool usesFlatShading);
   void CalcSetupCL(const VkGraphicsPipelineCreateInfo *ci,
                    bool rpEarlyZCompatible, const v3d_vpm_cfg_v vpmV[2]);

private:
   ///////////////////////////////
   // State from pipeline creation
   ///////////////////////////////
   Viewport                               m_viewport = {};
   VkRect2D                               m_scissorRect = {};
   VkPipelineDepthStencilStateCreateInfo  m_depthStencilState = {};
   DynamicStateBits                       m_dynamicStateBits;

   VkVertexInputBindingDescription        m_vertexBindingDescriptions[V3D_MAX_ATTR_ARRAYS] = {};
   VkVertexInputAttributeDescription      m_vertexAttributeDescriptions[V3D_MAX_ATTR_ARRAYS] = {};

   v3d_prim_mode_t                        m_drawPrimMode;   // Primitive mode to pass to draw calls
   bool                                   m_primitiveRestartEnable;
   bool                                   m_hasDepthStencil;
   V3D_CL_CFG_BITS_T                      m_cfgBits;

   ////////////////
   // Internal data
   ////////////////

   std::array<uint8_t, MAX_SETUP_CL_SIZE> m_setupCL;
   uint8_t                                m_setupCLSize;

   // So that we know whether to issue dynamic depth bias commands
   bool                                   m_depthBiasEnable;
   uint32_t                               m_depthBits;

#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
   DevMemRange                            m_attribDefaultsMem;
#endif
   DevMemRange                            m_dummyAttribMem;

   // System memory (incomplete) shader record - needs patching on use
   bvk::vector<uint32_t>                  m_shaderRecord;

   std::array<UniformData, LinkResult::SHADER_VPS_COUNT * MODE_COUNT>
                                          m_vpsUniforms;
   UniformData                            m_fsUniforms;

   // Data needed to patch the shader record when we have all the data
   ShaderPatchingData                     m_shaderPatchingData = {};
};

/////////////////////////////////////////////////////////////////////////////////////////
// Compute pipeline
/////////////////////////////////////////////////////////////////////////////////////////
class ComputePipeline : public Pipeline
{
public:
   struct NumWorkGroupsUniform
   {
      uint16_t offset;
      uint16_t index;
   };

   struct PreprocessPatchInfo
   {
       bvk::vector<NumWorkGroupsUniform> numWorkGroups;
       uint16_t rangeOffset = 0xffff;  // min uniform index during construction.
       uint16_t rangeSize   = 0;       // max uniform index during construction.
   };

   ComputePipeline(
      const VkAllocationCallbacks         *pCallbacks,
      Device                              *device,
      const VkComputePipelineCreateInfo   *createInfo);

   const UniformData &GetUniforms() const { return m_uniforms; }
   const PreprocessPatchInfo &GetPreprocessPatchInfo() const { return m_preprocessPatchInfo; }

protected:
   void BuildUniformSpecial(UniformData &uniforms, uint32_t offset,
                            BackendSpecialUniformFlavour special) override;

private:
   UniformData         m_uniforms;
   PreprocessPatchInfo m_preprocessPatchInfo;
};
} // namespace bvk
