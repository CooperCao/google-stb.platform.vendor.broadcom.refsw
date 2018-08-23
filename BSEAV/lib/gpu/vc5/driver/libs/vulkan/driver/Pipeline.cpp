/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "Extensions.h"

#include "Module.h"
#include "Specialization.h"
#include "Viewport.h"
#include "Options.h"

#include <cmath>
#include <algorithm>
#include <bitset>

#include "libs/core/v3d/v3d_shadrec.h"
#include "libs/core/v3d/v3d_tlb.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/lfmt/lfmt_translate_v3d.h"
#include "libs/platform/v3d_scheduler.h"
#include "libs/khrn/glsl/glsl_backend_cfg.h"
#include "glsl_tex_params.h"

namespace bvk {

static void CreateShaderRecordAttributes(
   const LinkResult *lr,
   const VkVertexInputAttributeDescription *attDescs,
   const VkVertexInputBindingDescription *bindDescs,
#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
   uint32_t *defaults,
#endif
   uint32_t *dstPtr);

static_assert((V3D_SHADREC_GL_GEOM_PACKED_SIZE & (sizeof(uint32_t) - 1)) == 0,
              "V3D_SHADREC_GL_GEOM_PACKED_SIZE must be a multiple of sizeof(uint32_t)");

static_assert((V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE & (sizeof(uint32_t) - 1)) == 0,
              "V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE must be a multiple of sizeof(uint32_t)");

static_assert((V3D_SHADREC_GL_MAIN_PACKED_SIZE & (sizeof(uint32_t) - 1)) == 0,
              "V3D_SHADREC_GL_MAIN_PACKED_SIZE must be a multiple of sizeof(uint32_t)");

std::mutex Pipeline::m_compLinkMutex;

static inline ShaderFlavour VkStageToShaderFlavour(VkShaderStageFlagBits stage)
{
   switch (stage)
   {
   case VK_SHADER_STAGE_VERTEX_BIT                  : return SHADER_VERTEX;
   case VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT    : return SHADER_TESS_CONTROL;
   case VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT : return SHADER_TESS_EVALUATION;
   case VK_SHADER_STAGE_GEOMETRY_BIT                : return SHADER_GEOMETRY;
   case VK_SHADER_STAGE_FRAGMENT_BIT                : return SHADER_FRAGMENT;
   case VK_SHADER_STAGE_COMPUTE_BIT                 : return SHADER_COMPUTE;
   default                                          : unreachable();
   }
}

////////////////////////////////////////////////////////////////////////////

Pipeline::Pipeline(DevMemHeap *devMemHeap, const VkAllocationCallbacks *pCallbacks,
                   VkPipelineBindPoint bindPoint) :
   Allocating(pCallbacks),
   m_devMemHeap(devMemHeap),
   m_bindPoint(bindPoint)
{
}

void Pipeline::CreateDefaultFragmentShader(CompiledShaderHandle compiledShaders[SHADER_FLAVOUR_COUNT],
                                           const Compiler::Controls &controls)
{
   // SPIRV for "void main() {}"
   static uint32_t defaultSPV[] =
   {
      0x07230203, 0x00010000, 0x00000000, 0x00000006, 0x00000000, // Header
      0x00020011, 0x00000001,                                     // OpCapability Shader
      0x0003000e, 0x00000000, 0x00000001,                         // OpMemoryModel Logical GLSL450
      0x0005000f, 0x00000004, 0x00000004, 0x6e69616d, 0x00000000, // OpEntryPoint Fragment %main "main"
      0x00030010, 0x00000004, 0x00000007,                         // OpExecutionMode %main OriginUpperLeft
      0x00020013, 0x00000002,                                     // %void   = OpTypeVoid
      0x00030021, 0x00000003, 0x00000002,                         // %voidfn = OpTypeFunction %void
      0x00050036, 0x00000002, 0x00000004, 0x00000000, 0x00000003, // %main   = OpFunction %void None %voidfn
      0x000200f8, 0x00000005,                                     // %start  = OpLabel
      0x000100fd,                                                 // OpReturn
      0x00010038                                                  // OpFunctionEnd
   };

   // If this is a graphics pipeline without a fragment shader
   // use a default shader that sets no output colour and serves just
   // to forward the depth.
   // TODO: V3D version >= 4.3.5 supports null shader record for fragment shader, so this
   // would not be necessary
   if (compiledShaders[SHADER_VERTEX] && !compiledShaders[SHADER_FRAGMENT])
   {
      const Module module(GetCallbacks(), defaultSPV, sizeof(defaultSPV));

      Compiler compiler(module, SHADER_FRAGMENT, "main", Specialization(nullptr), controls);
      DescriptorTables *descriptorTables = m_linkResult->GetDescriptorTables(SHADER_FRAGMENT);

      compiledShaders[SHADER_FRAGMENT] = compiler.Compile(descriptorTables, v3d_scheduler_get_compute_shared_mem_size_per_core());
   }
}

void Pipeline::CompileAndLinkShaders(
   const VkPipelineShaderStageCreateInfo *stages,
   size_t numStages,
   const std::bitset<V3D_MAX_ATTR_ARRAYS> &attribRBSwaps,
   std::function<GLSL_BACKEND_CFG_T(const GLSL_PROGRAM_T *)> calcBackendKey,
   const Compiler::Controls &controls)
{
   bool unroll    = true;
   bool succeeded = false;

   while (!succeeded)
   {
      try
      {
         // Take the giant mutex that prevents re-entrancy in the glsl backend.
         // TODO : we should make the glsl parts thread-safe
         std::lock_guard<std::mutex> lock(m_compLinkMutex);

         m_linkResult = LinkResultHandle(*this, m_devMemHeap);

         Compiler::Controls ctrls(controls);
         ctrls.SetUnroll(unroll);

         CompiledShaderHandle compiledShaders[SHADER_FLAVOUR_COUNT];

         // For each active stage
         for (unsigned i = 0; i != numStages; ++i)
         {
            const VkPipelineShaderStageCreateInfo &stage = stages[i];

            const ShaderModule *module  = fromHandle<ShaderModule>(stage.module);
            ShaderFlavour       flavour = VkStageToShaderFlavour(stage.stage);
            const char         *name    = stage.pName;

            Specialization  specialisation(stage.pSpecializationInfo); // todo: historically and currently
                                                                       // ignores allocator?

            // Initialize compiler memory pools and configure settings.
            Compiler compiler(*module, flavour, name, specialisation, ctrls);

            // Compile the shader for the given entry point
            DescriptorTables *descriptorTables = m_linkResult->GetDescriptorTables(flavour);

            compiledShaders[flavour] = compiler.Compile(descriptorTables, v3d_scheduler_get_compute_shared_mem_size_per_core());
         }

         // If there isn't a fragment shader, then create one
         CreateDefaultFragmentShader(compiledShaders, ctrls);

         // Do the linking
         m_linkResult->LinkShaders(compiledShaders, calcBackendKey, attribRBSwaps);
         succeeded = true;
      }
      catch (std::runtime_error &e)
      {
         // If we were unrolling loops then there is a chance we can recover the situation
         // so try again with loop unrolling disabled.
         if (unroll)
         {
            unroll = false;
         }
         else
         {
            // If we get here we've encountered an unrecoverable compile or link error.
            // Vulkan doesn't have a mechanism for handling failed pipeline creation (other than
            // out-of-memory) so we'll simply report the error here and rethrow which will flag the
            // pipeline as broken.
            fprintf(stderr, "******** Pipeline::CompileAndLinkShaders FAILED : %s\n", e.what());
            throw;
         }
      }
   }
}

void Pipeline::BuildUniforms(UniformData &uniforms, ShaderFlavour flavour, const ShaderData &shaderData)
{
   if (shaderData.uniformMap.empty())
      return;

   // Make enough room for all the uniforms
   uniforms.defaults.resize(shaderData.uniformMap.size());
   uint32_t offset = 0;

   for (const auto &entry : shaderData.uniformMap)
   {
      const BackendUniformFlavour &uType = entry.flavour;
      const uint32_t              &uValue = entry.value;

      switch (uType)
      {
      case BACKEND_UNIFORM_LITERAL:
         uniforms.defaults[offset].u = uValue;
         break;

      case BACKEND_UNIFORM_PLAIN:
         // PushConstant uValues are uint32_t offsets, so multiply back up to bytes
         uniforms.patches.emplace_back(uType, offset, uValue * sizeof(uint32_t));
         break;

      case BACKEND_UNIFORM_PLAIN_FPACK:
      case BACKEND_UNIFORM_UBO_LOAD:
         NOT_IMPLEMENTED_YET;
         break;

      case BACKEND_UNIFORM_UBO_ADDRESS:
      case BACKEND_UNIFORM_SSBO_ADDRESS:
      case BACKEND_UNIFORM_SSBO_SIZE:
      case BACKEND_UNIFORM_SSBO_ARRAY_LENGTH:
      case BACKEND_UNIFORM_UBO_SIZE:
      case BACKEND_UNIFORM_TEX_PARAM0:
      case BACKEND_UNIFORM_TEX_PARAM1:
      case BACKEND_UNIFORM_TEX_SIZE_X:
      case BACKEND_UNIFORM_TEX_SIZE_Y:
      case BACKEND_UNIFORM_TEX_SIZE_Z:
      case BACKEND_UNIFORM_TEX_LEVELS:
      case BACKEND_UNIFORM_IMG_PARAM0:
      case BACKEND_UNIFORM_IMG_SIZE_X:
      case BACKEND_UNIFORM_IMG_SIZE_Y:
      case BACKEND_UNIFORM_IMG_SIZE_Z:
         // These all just defer to the CommandBufferBuilder - they cannot be bound early
         uniforms.patches.emplace_back(uType, offset, uValue);
         break;
      case BACKEND_UNIFORM_SPECIAL:
         BuildUniformSpecial(uniforms, offset, (BackendSpecialUniformFlavour)uValue);
         break;

      case BACKEND_UNIFORM_ATOMIC_ADDRESS:
      case BACKEND_UNIFORM_ADDRESS:
      case BACKEND_UNIFORM_UNASSIGNED:
      default:
         unreachable();
      }

      offset += 1;
   }
}

////////////////////////////////////////////////////////////////////////////

ComputePipeline::ComputePipeline(
   const VkAllocationCallbacks       *pCallbacks,
   Device                            *device,
   const VkComputePipelineCreateInfo *ci) :
   Pipeline(device->GetDevMemHeap(), pCallbacks, VK_PIPELINE_BIND_POINT_COMPUTE),
   m_uniforms(*this)
{
   try
   {
      // Set stuff in the base Pipeline class
      m_layout = ci->layout;

      // Compile and link shaders
      auto calcBackendKey = [this](const GLSL_PROGRAM_T *program) -> GLSL_BACKEND_CFG_T
      {
#if !V3D_USE_CSD
         uint32_t compute_flags = compute_backend_flags(program->ir->cs_wg_size[0] * program->ir->cs_wg_size[1] * program->ir->cs_wg_size[2]);
#else
         uint32_t compute_flags = 0;
#endif
         return GLSL_BACKEND_CFG_T{ compute_flags | GLSL_DISABLE_UBO_FETCH };
      };

      Compiler::Controls controls;

      controls.SetRobustBufferAccess(device->GetRequestedFeatures().robustBufferAccess);

      CompileAndLinkShaders(&ci->stage, 1, std::bitset<V3D_MAX_ATTR_ARRAYS>(), calcBackendKey, controls);

      // Build shader uniforms
      BuildUniforms(m_uniforms, SHADER_FRAGMENT, m_linkResult->m_fs);

      m_preprocessPatchInfo.numWorkGroups.shrink_to_fit();

      // Convert from min/max in uniforms to offset/size in bytes.
      uint32_t min = m_preprocessPatchInfo.rangeOffset;
      uint32_t max = m_preprocessPatchInfo.rangeSize;
      if (min <= max)
      {
         m_preprocessPatchInfo.rangeOffset = min * sizeof(uint32_t);
         m_preprocessPatchInfo.rangeSize = uint32_t(max - min + 1u) * sizeof(uint32_t);
      }
      else
      {
         m_preprocessPatchInfo.rangeOffset = 0;
         m_preprocessPatchInfo.rangeSize = 0;
      }
   }
   catch (std::runtime_error &e)
   {
      // Something went wrong. Mark the pipeline as broken.
      m_broken = true;
   }
}

void ComputePipeline::BuildUniformSpecial(UniformData &uniforms, uint32_t offset, BackendSpecialUniformFlavour special)
{
   assert(offset <= UINT16_MAX);
   switch (special)
   {
   case BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_X:
   case BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Y:
   case BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Z:
      m_preprocessPatchInfo.numWorkGroups.emplace_back(
         NumWorkGroupsUniform{
            uint16_t(gfx_bits(offset, 16)),
            uint16_t(gfx_bits(special-BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_X, 16))
            });
      m_preprocessPatchInfo.rangeOffset = std::min(m_preprocessPatchInfo.rangeOffset, (uint16_t)offset);
      m_preprocessPatchInfo.rangeSize   = std::max(m_preprocessPatchInfo.rangeSize, (uint16_t)offset);
      break;

   case BACKEND_SPECIAL_UNIFORM_SHARED_PTR:
   {
    #if V3D_USE_L2T_LOCAL_MEM
      uniforms.defaults[offset].u = v3d_scheduler_get_compute_shared_mem_addr();
    #else
      gmem_handle_t handle = v3d_scheduler_get_compute_shared_mem(false /* TODO: secure_context */, /*alloc=*/true);
      if (!handle)
         throw bvk::bad_device_alloc();
      uniforms.defaults[offset].u = gmem_get_addr(handle);
    #endif
      break;
   }

   default:
      unreachable();
   }
}

////////////////////////////////////////////////////////////////////////////

GraphicsPipeline::DynamicStateBits GraphicsPipeline::CalcDynamicStateBits(
   uint32_t              dynamicStateCount,
   const VkDynamicState *dynamicStates)
{
   DynamicStateBits bits;
   for (uint32_t i = 0; i < dynamicStateCount; i++)
      bits.set(dynamicStates[i]);
   return bits;
}

static uint32_t ColorWriteMasks(const VkGraphicsPipelineCreateInfo *ci)
{
   static_assrt(VK_COLOR_COMPONENT_R_BIT == 1);
   static_assrt(VK_COLOR_COMPONENT_G_BIT == 2);
   static_assrt(VK_COLOR_COMPONENT_B_BIT == 4);
   static_assrt(VK_COLOR_COMPONENT_A_BIT == 8);

   assert(!ci->pRasterizationState->rasterizerDiscardEnable);

   const RenderPass* renderPass = fromHandle<RenderPass>(ci->renderPass);
   const RenderPass::SubpassGroup* group = renderPass->GroupForSubpass(ci->subpass);

   uint32_t enableMask = 0;
   for (uint32_t rt = 0; rt != group->m_numColorRenderTargets; ++rt)
      enableMask |= ci->pColorBlendState->pAttachments[rt].colorWriteMask << (rt*4);
   enableMask &= renderPass->ColorWriteMasks(ci->subpass);

   return enableMask;
}

static bool IsPointMode(VkPrimitiveTopology topology, VkPolygonMode fillMode)
{
   switch (topology)
   {
   case VK_PRIMITIVE_TOPOLOGY_POINT_LIST:
      return true;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST:
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP:
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN:
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY:
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY:
      return fillMode == VK_POLYGON_MODE_POINT;
   case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST:
      unreachable(); // not implemented.
   default:
      return false;
   }
}

static v3d_prim_mode_t CalculatePrimMode(VkPrimitiveTopology topology, uint32_t patchControlPoints)
{
   switch (topology)
   {
   case VK_PRIMITIVE_TOPOLOGY_POINT_LIST                    : return V3D_PRIM_MODE_POINTS;
   case VK_PRIMITIVE_TOPOLOGY_LINE_LIST                     : return V3D_PRIM_MODE_LINES;
   case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP                    : return V3D_PRIM_MODE_LINE_STRIP;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST                 : return V3D_PRIM_MODE_TRIS;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP                : return V3D_PRIM_MODE_TRI_STRIP;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN                  : return V3D_PRIM_MODE_TRI_FAN;
   case VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY      : return V3D_PRIM_MODE_LINES_ADJ;
   case VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY     : return V3D_PRIM_MODE_LINE_STRIP_ADJ;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY  : return V3D_PRIM_MODE_TRIS_ADJ;
   case VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY : return V3D_PRIM_MODE_TRI_STRIP_ADJ;
   case VK_PRIMITIVE_TOPOLOGY_PATCH_LIST                    :
      assert(patchControlPoints > 0 && patchControlPoints <= 32);
      return static_cast<v3d_prim_mode_t>(V3D_PRIM_MODE_PATCH1 - 1u + patchControlPoints);
   default:
      unreachable();
      return V3D_PRIM_MODE_POINTS;
   }
}

GraphicsPipeline::GraphicsPipeline(
      const VkAllocationCallbacks         *pCallbacks,
      Device                              *device,
      const VkGraphicsPipelineCreateInfo  *ci) :
   Pipeline(device->GetDevMemHeap(), pCallbacks, VK_PIPELINE_BIND_POINT_GRAPHICS),
   // This is horrible, but I'd like to compact all the shader uniforms into a single vector.
   m_vpsUniforms({ *this, *this, *this, *this, *this, *this, *this, *this }),
   m_fsUniforms( *this )
{
   try
   {
      // Set stuff in the base Pipeline class
      m_layout = ci->layout;

      // Copy optional parts
      if (ci->pDepthStencilState != nullptr) // todo: not valid to check for null.
         m_depthStencilState = *ci->pDepthStencilState;

      if (ci->pDynamicState != nullptr)
         m_dynamicStateBits = CalcDynamicStateBits(
                                 ci->pDynamicState->dynamicStateCount,
                                 ci->pDynamicState->pDynamicStates);

      if (!ci->pRasterizationState->rasterizerDiscardEnable)
      {
         // Viewport and scissor are only valid if not dynamic
         if (!m_dynamicStateBits.test(VK_DYNAMIC_STATE_VIEWPORT))
            m_viewport.Set(ci->pViewportState->pViewports[0]);

         if (!m_dynamicStateBits.test(VK_DYNAMIC_STATE_SCISSOR))
            m_scissorRect = ci->pViewportState->pScissors[0];
      }

      // Store which attributes require an RB swap in the shader (see SWVC5-801)
      std::bitset<V3D_MAX_ATTR_ARRAYS> attributeRBSwaps;

      // Populate m_vertexBindingDescriptions & m_vertexAttributeDescriptions
      uint32_t bindingCount = ci->pVertexInputState->vertexBindingDescriptionCount;
      uint32_t attrCount    = ci->pVertexInputState->vertexAttributeDescriptionCount;

      for (uint32_t i = 0; i < bindingCount; i++)
      {
         const VkVertexInputBindingDescription *d = &ci->pVertexInputState->pVertexBindingDescriptions[i];
         m_vertexBindingDescriptions[d->binding] = *d;
      }

      for (uint32_t i = 0; i < attrCount; i++)
      {
         const VkVertexInputAttributeDescription *d = &ci->pVertexInputState->pVertexAttributeDescriptions[i];
         m_vertexAttributeDescriptions[d->location] = *d;
         if (Formats::NeedsAttributeRBSwap(Formats::GetLFMT(d->format)))
            attributeRBSwaps.set(d->location);
      }

      m_primitiveRestartEnable = ci->pInputAssemblyState->primitiveRestartEnable ? true : false;

      // Build a bitmask of the shader types in this pipeline
      VkFlags shaderStages = 0;
      for (uint32_t i = 0; i < ci->stageCount; i++)
         shaderStages = shaderStages | ci->pStages[i].stage;

      uint32_t patchControlPoints = 0;
      // ci->pTessellationState is only valid if both tessellation shader types are present
      if ((shaderStages & VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT) &&
          (shaderStages & VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT))
      {
         patchControlPoints = ci->pTessellationState->patchControlPoints;

         // KHR_maintenance2 has an extension field for tessellation domain origin
         // TODO - use domain origin during tessellation
         //auto ext = Extensions::FindExtensionStruct<VkPipelineTessellationDomainOriginStateCreateInfoKHR>(
         //                       VK_STRUCTURE_TYPE_PIPELINE_TESSELLATION_DOMAIN_ORIGIN_STATE_CREATE_INFO_KHR,
         //                       ci->pTessellationState->pNext);
         //if (ext)
         //   m_tessDomainOrigin = ext->domainOrigin;
      }

      // Convert assembly and tess state into prim mode
      m_drawPrimMode = CalculatePrimMode(ci->pInputAssemblyState->topology, patchControlPoints);

      RenderPass *rp = fromHandle<RenderPass>(ci->renderPass);
      m_depthBits = rp->DepthBits();

      m_hasDepthStencil = rp->GroupForSubpass(ci->subpass)->m_dsAttachment != VK_ATTACHMENT_UNUSED;

      // Compile and link shaders
      bool multisampled = !ci->pRasterizationState->rasterizerDiscardEnable &&
                           ci->pMultisampleState->rasterizationSamples != VK_SAMPLE_COUNT_1_BIT;

      auto calcBackendKey = [ci](const GLSL_PROGRAM_T *) -> GLSL_BACKEND_CFG_T
      {
         return GLSL_BACKEND_CFG_T{ CalcBackendKey(ci) };
      };

      Compiler::Controls controls;

      controls.SetRobustBufferAccess(device->GetRequestedFeatures().robustBufferAccess)
              .SetDepthStencil(m_hasDepthStencil)
              .SetMultisampled(multisampled);

      CompileAndLinkShaders(ci->pStages, ci->stageCount, attributeRBSwaps, calcBackendKey,
                            controls);

      // Is sample rate shading required?
      bool sampleShading = (multisampled &&
                            ci->pMultisampleState->sampleShadingEnable &&
                            ci->pMultisampleState->minSampleShading >= 0.25f) ||
                            m_linkResult->HasFlag(LINKRES_FLAG_PER_SAMPLE);

      // Create as much of the shader record as we can
      v3d_vpm_cfg_v vpmV[2];
      CreateShaderRecord(ci->pRasterizationState->rasterizerDiscardEnable, sampleShading, vpmV,
                         device->GetPlatform().GetVPMSize(), patchControlPoints);

      CalcSetupCL(ci, rp->EarlyZCompatible(), vpmV);
   }
   catch (std::runtime_error &e)
   {
      // Something went wrong. Mark the pipeline as broken
      m_broken = true;
   }
}

static bool RequiresCColor(v3d_blend_mul_t src)
{
   return src == V3D_BLEND_MUL_CONST || src == V3D_BLEND_MUL_OM_CONST ||
          src == V3D_BLEND_MUL_CONST_ALPHA || src == V3D_BLEND_MUL_OM_CONST_ALPHA;
}

static bool RequiresCColor(const V3D_CL_BLEND_CFG_T &cfg)
{
   return RequiresCColor(cfg.a_src) || RequiresCColor(cfg.a_dst) ||
          RequiresCColor(cfg.c_src) || RequiresCColor(cfg.c_dst);
}

void GraphicsPipeline::CalcSetupCL(const VkGraphicsPipelineCreateInfo *ci,
                                   bool rpEarlyZCompatible, const v3d_vpm_cfg_v vpmV[2])
{
   uint8_t *clPtr = m_setupCL.data();

   bool ms = false;
   bool globalBlendEnable = false;

   if (!ci->pRasterizationState->rasterizerDiscardEnable)
   {
      if (!IsDynamic(VK_DYNAMIC_STATE_LINE_WIDTH))
         v3d_cl_line_width(&clPtr, ci->pRasterizationState->lineWidth);

      m_depthBiasEnable = ci->pRasterizationState->depthBiasEnable;
      if (!IsDynamic(VK_DYNAMIC_STATE_DEPTH_BIAS))
      {
         float factor = m_depthBiasEnable ? ci->pRasterizationState->depthBiasSlopeFactor : 0.0f;
         float units  = m_depthBiasEnable ? ci->pRasterizationState->depthBiasConstantFactor : 0.0f;
         float clamp  = m_depthBiasEnable ? ci->pRasterizationState->depthBiasClamp : 0.0f;
         v3d_cl_set_depth_offset(&clPtr, factor, units, clamp, m_depthBits);
      }

      uint32_t colorWriteMasks = ColorWriteMasks(ci);
      v3d_cl_color_wmasks(&clPtr, ~colorWriteMasks & gfx_mask(V3D_MAX_RENDER_TARGETS * 4));

      if (colorWriteMasks != 0)
      {
         uint32_t blendEnables;
         V3D_CL_BLEND_CFG_T blendCfg[V3D_MAX_RENDER_TARGETS];
         uint32_t blendCfgCount = CalcBlendState(&blendEnables, blendCfg, ci->pColorBlendState);
         assert(!blendEnables == !blendCfgCount);

         if (blendEnables)
         {
            assert(blendCfgCount != 0);
            v3d_cl_blend_enables(&clPtr, blendEnables);
            globalBlendEnable = true;

            bool requiresCColor = false;
            for (uint32_t i = 0; i < blendCfgCount; i++)
            {
               requiresCColor = requiresCColor || RequiresCColor(blendCfg[i]);
               v3d_cl_blend_cfg_indirect(&clPtr, &blendCfg[i]);
            }

            if (!IsDynamic(VK_DYNAMIC_STATE_BLEND_CONSTANTS) && requiresCColor)
            {
               v3d_cl_blend_ccolor(&clPtr, ci->pColorBlendState->blendConstants[0], ci->pColorBlendState->blendConstants[1],
                                           ci->pColorBlendState->blendConstants[2], ci->pColorBlendState->blendConstants[3]);
            }
         }
      }

      const VkPipelineMultisampleStateCreateInfo *ciM = ci->pMultisampleState;
      uint8_t sampleMask = 0xF;

      if (ciM->pSampleMask != nullptr)
      {
         if (ciM->rasterizationSamples != VK_SAMPLE_COUNT_1_BIT)
            sampleMask = ciM->pSampleMask[0] & 0xF;
         else
            sampleMask = (ciM->pSampleMask[0] & 0x1) ? 0xF : 0;
      }

      v3d_cl_sample_state(&clPtr, sampleMask, /* Coverage (unused) = */1.0f);

      ms = ciM->rasterizationSamples != VK_SAMPLE_COUNT_1_BIT;
   }

   bool usesFlatShading = false;
   for (uint32_t i = 0; i < V3D_MAX_VARY_FLAG_WORDS; i++)
      usesFlatShading = usesFlatShading || (m_linkResult->m_data.vary.flat[i] != 0);

   CalcCfgBits(&m_cfgBits, ci->pRasterizationState, ms, globalBlendEnable,
               rpEarlyZCompatible, usesFlatShading);

   v3d_cl_vcm_cache_size(&clPtr, vpmV[0].vcm_cache_size, vpmV[1].vcm_cache_size);

   v3d_cl_write_vary_flags(&clPtr, m_linkResult->m_data.vary.flat,
                           v3d_cl_flatshade_flags, v3d_cl_zero_all_flatshade_flags);

   v3d_cl_write_vary_flags(&clPtr, m_linkResult->m_data.vary.centroid,
                           v3d_cl_centroid_flags, v3d_cl_zero_all_centroid_flags);

   v3d_cl_write_vary_flags(&clPtr, m_linkResult->m_data.vary.noperspective,
                           v3d_cl_noperspective_flags, v3d_cl_zero_all_noperspective_flags);

   // When T+G is enabled, not writing a point size should have it default to 1.0
   if (m_linkResult->m_data.has_tess || m_linkResult->m_data.has_geom)
      v3d_cl_point_size(&clPtr, 1.0f);

   m_setupCLSize = clPtr - m_setupCL.data();
}

GraphicsPipeline::~GraphicsPipeline()
{
#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
   m_devMemHeap->Free(m_attribDefaultsMem);
#endif
   m_devMemHeap->Free(m_dummyAttribMem);
}

bool GraphicsPipeline::UpdateEarlyZState(EARLY_Z_STATE_T *state)
{
   const auto &ds = m_depthStencilState;

   v3d_compare_func_t depthFunc = (m_hasDepthStencil && ds.depthTestEnable) ?
                                  TranslateCompareFunc(ds.depthCompareOp) : V3D_COMPARE_FUNC_ALWAYS;

   bool depthUpdate = (ds.depthTestEnable && ds.depthWriteEnable);

   bool ret = early_z_update_cfg(state, depthFunc, depthUpdate,
      m_hasDepthStencil && ds.stencilTestEnable,
      TranslateStencilOp(ds.front.failOp), TranslateStencilOp(ds.front.depthFailOp),
      TranslateStencilOp(ds.back.failOp), TranslateStencilOp(ds.back.depthFailOp));

   early_z_log(state);

   return ret;
}

inline static v3d_blend_eqn_t TranslateBlendEqn(VkBlendOp eq)
{
   assert(eq >= VK_BLEND_OP_ADD && eq <= VK_BLEND_OP_MAX);
   return static_cast<v3d_blend_eqn_t>(eq);
}

inline static v3d_blend_mul_t TranslateBlendFactor(VkBlendFactor factor)
{
   assert(factor >= VK_BLEND_FACTOR_ZERO && factor <= VK_BLEND_FACTOR_SRC_ALPHA_SATURATE);
   return static_cast<v3d_blend_mul_t>(factor);
}

inline static bool BlendCfgsCompatible(const V3D_CL_BLEND_CFG_T &a, const V3D_CL_BLEND_CFG_T &b)
{
   return (a.a_eqn == b.a_eqn) && (a.a_src == b.a_src) && (a.a_dst == b.a_dst) &&
          (a.c_eqn == b.c_eqn) && (a.c_src == b.c_src) && (a.c_dst == b.c_dst) &&
          (a.vg_mode == b.vg_mode);
}

uint32_t GraphicsPipeline::CalcBlendState(uint32_t *blendEnable, V3D_CL_BLEND_CFG_T *blendCfg,
                                          const VkPipelineColorBlendStateCreateInfo *ci) const
{
   uint32_t enable = 0;
   uint32_t cfgCount = 0;

   for (uint32_t i = 0; i < ci->attachmentCount; i++)
   {
      const VkPipelineColorBlendAttachmentState &bAtt = ci->pAttachments[i];
      if (!bAtt.blendEnable)
         continue;

      enable |= 1u << i;
      blendCfg[cfgCount].a_eqn   = TranslateBlendEqn(bAtt.alphaBlendOp);
      blendCfg[cfgCount].a_src   = TranslateBlendFactor(bAtt.srcAlphaBlendFactor);
      blendCfg[cfgCount].a_dst   = TranslateBlendFactor(bAtt.dstAlphaBlendFactor);
      blendCfg[cfgCount].c_eqn   = TranslateBlendEqn(bAtt.colorBlendOp);
      blendCfg[cfgCount].c_src   = TranslateBlendFactor(bAtt.srcColorBlendFactor);
      blendCfg[cfgCount].c_dst   = TranslateBlendFactor(bAtt.dstColorBlendFactor);
      blendCfg[cfgCount].rt_mask = 1u << i;
      blendCfg[cfgCount].vg_mode = V3D_BLEND_VG_MODE_NORMAL;

      for (uint32_t j=0; j < cfgCount; j++)
      {
         // If this config is the same, add it to the previous one and don't move on
         if (BlendCfgsCompatible(blendCfg[j], blendCfg[cfgCount]))
         {
            blendCfg[j].rt_mask |= 1u << i;
            cfgCount--;
            break;
         }
      }
      cfgCount++;
   }

   // The blend configs for unused/disabled RTs don't really matter, but ensuring
   // they always match the config for the first RT can reduce the number
   // of binned instructions.
   if (cfgCount > 0)
      blendCfg[0].rt_mask |= ~enable & gfx_mask(V3D_MAX_RENDER_TARGETS);

   *blendEnable = enable;
   return cfgCount;
}

void GraphicsPipeline::CalcCfgBits(V3D_CL_CFG_BITS_T *cfg_bits,
                                   const VkPipelineRasterizationStateCreateInfo *rastCi,
                                   bool ms, bool blendEnable,
                                   bool rpEarlyZCompatible, bool usesFlatShading)
{
   const auto &ds = m_depthStencilState;
   v3d_compare_func_t depthFunc = (m_hasDepthStencil && ds.depthTestEnable) ? TranslateCompareFunc(ds.depthCompareOp) :
                                                       V3D_COMPARE_FUNC_ALWAYS;
   bool depthUpdate = (ds.depthTestEnable && ds.depthWriteEnable);

   cfg_bits->blend   = blendEnable;
   cfg_bits->stencil = m_hasDepthStencil && ds.stencilTestEnable;

   // The front and back_prims fields say whether we want them (i.e. !culled)
   cfg_bits->front_prims       = (rastCi->cullMode & VK_CULL_MODE_FRONT_BIT) ? false : true;
   cfg_bits->back_prims        = (rastCi->cullMode & VK_CULL_MODE_BACK_BIT) ? false : true;
   if (rastCi->rasterizerDiscardEnable)
      cfg_bits->front_prims = cfg_bits->back_prims = false;

   // Vulkan's origin is top-left, GL's is bottom-left. The 'clockwise' orientation
   // of triangles in the hardware assumes GL's view of the world. When the y-axis
   // is flipped (in Vulkan), the notion of 'clockwise' actually reverses, so we need
   // to tell hardware the opposite of what the API says.
   cfg_bits->cwise_is_front    = (rastCi->frontFace != VK_FRONT_FACE_CLOCKWISE);

   cfg_bits->depth_offset      = rastCi->depthBiasEnable ? true : false;
   cfg_bits->rast_oversample   = ms ? V3D_MS_4X : V3D_MS_1X;
   cfg_bits->depth_test        = depthFunc;
   cfg_bits->depth_update      = depthUpdate;
   cfg_bits->ez                = false;   // Updated by CmdBufState::BuildStateUpdateCL
   cfg_bits->ez_update         = false;   // Updated by CmdBufState::BuildStateUpdateCL
   cfg_bits->aa_lines          = true;
   cfg_bits->wireframe_tris    = (rastCi->polygonMode != VK_POLYGON_MODE_FILL);
   cfg_bits->wireframe_mode    = (rastCi->polygonMode == VK_POLYGON_MODE_POINT ?
                                 V3D_WIREFRAME_MODE_POINTS : V3D_WIREFRAME_MODE_LINES);
   cfg_bits->cov_pipe          = false;
   cfg_bits->cov_update        = V3D_COV_UPDATE_NONZERO;

   // Note : GFXH-1687 can be triggered in D3D-provoking-vertex-mode, so only enable when
   // really required.
   cfg_bits->d3d_prov_vtx      = usesFlatShading;
}

static void PackShaderArgs(V3D_SHADER_ARGS_T *args, const ShaderData &data)
{
   args->threading      = data.threading;
   args->single_seg     = data.singleSeg;
   args->addr           = data.shaderMemory.Phys();
   args->propagate_nans = true;
   args->unifs_addr     = GraphicsPipeline::UNIFORM_PATCHTAG;
}

void GraphicsPipeline::CreateShaderRecord(bool rasterizerDiscard, bool sampleShading,
                                          v3d_vpm_cfg_v vpmV[2], uint32_t vpmSize, uint32_t patchControlPoints)
{
   // How big is our shader record going to be?
   uint32_t numAttrs = std::max(1u, m_linkResult->m_data.attr_count);
   uint32_t shaderRecordBytes =
      + V3D_SHADREC_GL_GEOM_PACKED_SIZE
      + V3D_SHADREC_GL_TESS_PACKED_SIZE
      + V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE
      + V3D_SHADREC_GL_MAIN_PACKED_SIZE
      + V3D_SHADREC_GL_ATTR_PACKED_SIZE * numAttrs;

   assert((shaderRecordBytes & 3) == 0); // Must be multiple of 4

   // Find the shader code addresses
   v3d_addr_t vpShaderAddrs[LinkResult::SHADER_VPS_COUNT][MODE_COUNT];
   for (unsigned m = 0; m != MODE_COUNT; ++m)
      for (unsigned s = 0; s != LinkResult::SHADER_VPS_COUNT; ++s)
         vpShaderAddrs[s][m] = m_linkResult->m_vps[s][m].shaderMemory.Phys();

   // Bin/Render vertex pipeline uniforms
   for (unsigned m = 0; m != MODE_COUNT; ++m)
   {
      for (unsigned s = 0; s != LinkResult::SHADER_VPS_COUNT; ++s)
      {
         if (vpShaderAddrs[s][m] != 0)
            BuildUniforms(m_vpsUniforms[s*MODE_COUNT + m], LinkResult::ConvertShaderFlavour(s), m_linkResult->m_vps[s][m]);
      }
   }

   // Build fragment shader uniforms
   BuildUniforms(m_fsUniforms, SHADER_FRAGMENT, m_linkResult->m_fs);

   // Allocate system memory for the shader record
   m_shaderRecord.resize(shaderRecordBytes / sizeof(uint32_t));

   uint32_t *shadrecPtr = m_shaderRecord.data();
   uint32_t *baseShadrecPtr = shadrecPtr;

   m_shaderPatchingData.hasT = m_linkResult->m_data.has_tess;
   m_shaderPatchingData.hasG = m_linkResult->m_data.has_geom;

   // Record shader record parts for each T+G stage
   for (unsigned s = LinkResult::SHADER_VPS_VS + 1; s <= LinkResult::SHADER_VPS_TES; ++s)
   {
      if (vpShaderAddrs[s][MODE_RENDER] != 0)
      {
         V3D_SHADREC_GL_GEOM_T stageShadrec;
         PackShaderArgs(&stageShadrec.gs_bin,    m_linkResult->m_vps[s][MODE_BIN]);
         PackShaderArgs(&stageShadrec.gs_render, m_linkResult->m_vps[s][MODE_RENDER]);

         v3d_pack_shadrec_gl_geom(shadrecPtr, &stageShadrec);

         m_shaderPatchingData.offsetGLGeom[s] = shadrecPtr - baseShadrecPtr;

         shadrecPtr += V3D_SHADREC_GL_GEOM_PACKED_SIZE / sizeof(uint32_t);
      }
   }

   // Write out tess or geom part
   if (m_linkResult->m_data.has_tess || m_linkResult->m_data.has_geom)
   {
      linkres_compute_tng_vpm_cfg(vpmV, shadrecPtr, &m_linkResult->m_data, patchControlPoints, vpmSize);
      shadrecPtr += V3D_SHADREC_GL_TESS_OR_GEOM_PACKED_SIZE / sizeof(uint32_t);
   }
   else
   {
      // Calculate the VPM config
      bool zPrePass = false;   // TODO?
      v3d_vpm_compute_cfg(vpmV, vpmSize / 512, m_linkResult->m_data.vs_input_words,
                          m_linkResult->m_data.vs_output_words, zPrePass);
   }

#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
   // Allocate memory for the attribute defaults. Filled in as attributes are created.
   size_t defaultsSize = m_linkResult->m_data.attr_count * 4 * sizeof(uint32_t);
   m_devMemHeap->Allocate(&m_attribDefaultsMem, defaultsSize, V3D_ATTR_DEFAULTS_ALIGN);
#endif

   // Make the main shader record
   V3D_SHADREC_GL_MAIN_T srec{};

   srec.point_size_included      = m_linkResult->HasFlag(LINKRES_FLAG_POINT_SIZE);
   srec.cs_vertex_id             = m_linkResult->HasFlag(LINKRES_FLAG_VS_VERTEX_ID   << MODE_BIN);
   srec.cs_instance_id           = m_linkResult->HasFlag(LINKRES_FLAG_VS_INSTANCE_ID << MODE_BIN);
   srec.vs_vertex_id             = m_linkResult->HasFlag(LINKRES_FLAG_VS_VERTEX_ID   << MODE_RENDER);
   srec.vs_instance_id           = m_linkResult->HasFlag(LINKRES_FLAG_VS_INSTANCE_ID << MODE_RENDER);
   srec.z_write                  = m_linkResult->HasFlag(LINKRES_FLAG_FS_WRITES_Z);
   srec.no_ez                    = m_linkResult->HasFlag(LINKRES_FLAG_FS_EARLY_Z_DISABLE);
   srec.cs_separate_blocks       = m_linkResult->HasFlag(LINKRES_FLAG_VS_SEPARATE_VPM_BLOCKS << MODE_BIN);
   srec.vs_separate_blocks       = m_linkResult->HasFlag(LINKRES_FLAG_VS_SEPARATE_VPM_BLOCKS << MODE_RENDER);
   srec.fs_needs_w               = m_linkResult->HasFlag(LINKRES_FLAG_FS_NEEDS_W);
   srec.scb_wait_on_first_thrsw  = m_linkResult->HasFlag(LINKRES_FLAG_TLB_WAIT_FIRST_THRSW);

   // No need to clip if everything is thrown away
   srec.clipping                 = !rasterizerDiscard;

   srec.disable_implicit_varys = m_linkResult->HasFlag(LINKRES_FLAG_DISABLE_IMPLICIT_VARYS);
   srec.cs_baseinstance        = m_linkResult->HasFlag(LINKRES_FLAG_VS_BASE_INSTANCE << MODE_BIN);
   srec.vs_baseinstance        = m_linkResult->HasFlag(LINKRES_FLAG_VS_BASE_INSTANCE << MODE_RENDER);
   srec.prim_id_used           = m_linkResult->HasFlag(LINKRES_FLAG_PRIM_ID_USED);
   srec.prim_id_to_fs          = m_linkResult->HasFlag(LINKRES_FLAG_PRIM_ID_TO_FS);
   srec.sample_rate_shading    = sampleShading;
   srec.num_varys              = m_linkResult->m_data.vary.count;
   srec.cs_output_size         = vpmV[MODE_BIN].output_size;
   srec.cs_input_size          = vpmV[MODE_BIN].input_size;
   srec.vs_output_size         = vpmV[MODE_RENDER].output_size;
   srec.vs_input_size          = vpmV[MODE_RENDER].input_size;
#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
   srec.defaults               = m_attribDefaultsMem.Phys();
#endif

   PackShaderArgs(&srec.fs, m_linkResult->m_fs);
   PackShaderArgs(&srec.vs, m_linkResult->m_vps[LinkResult::SHADER_VPS_VS][MODE_RENDER]);
   PackShaderArgs(&srec.cs, m_linkResult->m_vps[LinkResult::SHADER_VPS_VS][MODE_BIN]);

   // Record the main shader record data
   v3d_pack_shadrec_gl_main(shadrecPtr, &srec);

   m_shaderPatchingData.offsetGLMain = shadrecPtr - baseShadrecPtr;

   shadrecPtr += V3D_SHADREC_GL_MAIN_PACKED_SIZE / sizeof(uint32_t);

   // Write the attributes into the remaining part of the shader record memory block
   if (m_linkResult->m_data.attr_count != 0)
   {
      CreateShaderRecordAttributes(
         m_linkResult.GetPtr(),
         m_vertexAttributeDescriptions,
         m_vertexBindingDescriptions,
#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
         (uint32_t*)m_attribDefaultsMem.Ptr(),
#endif
         shadrecPtr);
      m_shaderPatchingData.offsetGLAttr = shadrecPtr - baseShadrecPtr;

#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
      m_attribDefaultsMem.SyncMemory();
#endif
   }
   else
      CreateShaderRecordDummyAttribute(shadrecPtr);
}

void GraphicsPipeline::CreateShaderRecordDummyAttribute(uint32_t *dstPtr)
{
   // Workaround GFXH-930, must have at least 1 attribute:
   // Make a block a devMem with just a single dummy attribute value
   m_devMemHeap->Allocate(&m_dummyAttribMem, sizeof(uint32_t), V3D_ATTR_ALIGN);
   uint32_t *dummyData = static_cast<uint32_t*>(m_dummyAttribMem.Ptr());
   dummyData[0] = 0;
   m_dummyAttribMem.SyncMemory();

   V3D_SHADREC_GL_ATTR_T attr;
   attr.addr            = m_dummyAttribMem.Phys();
   attr.size            = 1;
   attr.type            = V3D_ATTR_TYPE_FLOAT;
   attr.signed_int      = false;
   attr.normalised_int  = false;
   attr.read_as_int     = false;
   attr.cs_num_reads    = 1;
   attr.vs_num_reads    = 1;
   attr.divisor         = 0;
   attr.stride          = 0;
   attr.max_index       = 0,

   v3d_pack_shadrec_gl_attr(dstPtr, &attr);
}

static void CreateShaderRecordAttributes(
   const LinkResult *lr,
   const VkVertexInputAttributeDescription *attDescs,
   const VkVertexInputBindingDescription *bindDescs,
#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
   uint32_t *defaults,
#endif
   uint32_t *dstPtr)
{
#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
   memset(defaults, 0, lr->m_data.attr_count * 4 * sizeof(uint32_t));
#endif

   uint32_t csTotalReads = 0;
   uint32_t vsTotalReads = 0;
   uint32_t recordCnt    = 0;

   for (uint32_t n = 0; n < lr->m_data.attr_count; n++)
   {
      csTotalReads += lr->m_data.attr[n].c_scalars_used;
      vsTotalReads += lr->m_data.attr[n].v_scalars_used;
   }

   for (uint32_t n = 0; n < lr->m_data.attr_count; n++)
   {
      uint32_t attrIndex  = lr->m_data.attr[n].idx;
      uint32_t csNumReads = lr->m_data.attr[n].c_scalars_used;
      uint32_t vsNumReads = lr->m_data.attr[n].v_scalars_used;

      assert(csNumReads > 0 || vsNumReads > 0); // attribute entry shouldn't exist if there are no reads

      // Workaround GFXH-930
      if (n == 0 && (csTotalReads == 0 || vsTotalReads == 0))
      {
         // We read all attributes either in CS or VS, bodge the first attribute
         // to be read by both. It must be the first because of GFXH-1602
         assert(csNumReads == 0 || vsNumReads == 0);
         csNumReads |= vsNumReads;
         vsNumReads = csNumReads;
      }

      const auto &attDesc = attDescs[attrIndex];
      const auto &bindDesc = bindDescs[attDesc.binding];

      GFX_LFMT_T lfmt   = Formats::GetLFMT(attDesc.format);
      bool       scaled = Formats::IsScaled(attDesc.format);

      V3D_SHADREC_GL_ATTR_T attr;
      attr.addr            = 0;
      attr.size            = gfx_lfmt_num_slots_from_channels(lfmt);
      attr.type            = gfx_lfmt_translate_attr_type(lfmt, scaled);
      attr.signed_int      = gfx_lfmt_contains_int_signed(lfmt) || gfx_lfmt_contains_snorm(lfmt);
      attr.normalised_int  = gfx_lfmt_contains_unorm(lfmt) || gfx_lfmt_contains_snorm(lfmt);
      attr.read_as_int     = gfx_lfmt_contains_int(lfmt) && !scaled;
      attr.cs_num_reads    = csNumReads;
      attr.vs_num_reads    = vsNumReads;
      attr.divisor         = bindDesc.inputRate == VK_VERTEX_INPUT_RATE_VERTEX ? 0 : 1;
      attr.stride          = bindDesc.stride;
      attr.max_index       = 0;

      uint32_t *ptr = dstPtr + (recordCnt * (V3D_SHADREC_GL_ATTR_PACKED_SIZE / sizeof(uint32_t)));

      v3d_pack_shadrec_gl_attr(ptr, &attr);

#if !V3D_HAS_IMPLICIT_ATTR_DEFAULTS
      defaults[4*recordCnt + 3] = attr.read_as_int ? 1 : 0x3f800000; // or 1.0f
#endif

      recordCnt++;
   }
}

void GraphicsPipeline::BuildUniformSpecial(UniformData &uniforms, uint32_t offset, BackendSpecialUniformFlavour special)
{
   Uniform uniform {};

   switch (special)
   {
   case BACKEND_SPECIAL_UNIFORM_VP_SCALE_X:
      uniform.f = m_viewport.internalScale[0];
      break;

   case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Y:
      uniform.f = m_viewport.internalScale[1];
      break;

   case BACKEND_SPECIAL_UNIFORM_VP_OFFSET_Z:
   case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_NEAR:
      uniform.f = m_viewport.depthNear;
      break;

   case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_FAR:
      uniform.f = m_viewport.depthFar;
      break;

   case BACKEND_SPECIAL_UNIFORM_VP_SCALE_Z:
   case BACKEND_SPECIAL_UNIFORM_DEPTHRANGE_DIFF:
      uniform.f = m_viewport.depthDiff;
      break;

   default:
      unreachable();
   }

   uniforms.defaults[offset] = uniform;

   if (m_dynamicStateBits.test(VK_DYNAMIC_STATE_VIEWPORT))
      uniforms.patches.emplace_back(BACKEND_UNIFORM_SPECIAL, offset, special);
}

uint32_t GraphicsPipeline::CalcBackendKey(const VkGraphicsPipelineCreateInfo *ci)
{
   uint32_t key = GLSL_DISABLE_UBO_FETCH;

   if (IsPointMode(ci->pInputAssemblyState->topology, ci->pRasterizationState->polygonMode))
      key |= GLSL_PRIM_POINT;

   // Early out here if rasterizer discard is enabled.
   if (ci->pRasterizationState->rasterizerDiscardEnable)
      return key;

   uint32_t enableMask = ColorWriteMasks(ci);

   // Fill in the framebuffer type for each render target
   const RenderPass *rp = fromHandle<RenderPass>(ci->renderPass);
   const RenderPass::SubpassGroup &subpassGroup = *rp->GroupForSubpass(ci->subpass);
   for (uint32_t rt = 0; rt != subpassGroup.m_numColorRenderTargets; ++rt)
   {
      // Disable writes to TLB if all components are disabled.
      if (((enableMask >> rt*4) & 0xf) == 0)
         continue;

      uint32_t attIndex = subpassGroup.m_colorRTAttachments[rt];
      if (attIndex == VK_ATTACHMENT_UNUSED)
         continue;

      const V3D_RT_FORMAT_T& v3dRtFormat = rp->Attachments()[attIndex].v3dRtFormat;
      uint32_t fbType = GLSL_FB_PRESENT;
      if (v3d_tlb_rt_type_use_rw_cfg_16 (v3dRtFormat.type)) fbType |= GLSL_FB_16;
      if (v3d_tlb_rt_type_is_int(v3dRtFormat.type))         fbType |= GLSL_FB_INT;
      glsl_pack_fb_gadget(&key, fbType, rt);
   }

   // Set MS state flags
   const VkPipelineMultisampleStateCreateInfo* multCi = ci->pMultisampleState;
   if (multCi->rasterizationSamples != VK_SAMPLE_COUNT_1_BIT)
      key |= GLSL_SAMPLE_MS;

   if (multCi->alphaToCoverageEnable)
      key |= GLSL_SAMPLE_ALPHA;

#if !V3D_VER_AT_LEAST(4,2,14,0)
   bool sampleShading = multCi != nullptr && multCi->sampleShadingEnable &&
                        multCi->rasterizationSamples != VK_SAMPLE_COUNT_1_BIT &&
                        multCi->minSampleShading >= 0.25f;
   if (sampleShading)
      key |= GLSL_SAMPLE_SHADING_ENABLED;
#endif

   // TODO : when should we set these
   //key |= GLSL_FEZ_SAFE_WITH_DISCARD;

   return key;
}

////////////////////////////////////////////////////////////////////////////

} // namespace bvk
