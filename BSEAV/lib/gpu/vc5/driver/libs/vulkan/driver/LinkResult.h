/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "libs/core/v3d/v3d_gen.h"
#include "libs/khrn/glsl/glsl_backend_uniforms.h"
#include "libs/khrn/glsl/glsl_binary_program.h"
#include "libs/khrn/glsl/glsl_backend_cfg.h"
#include "libs/core/v3d/v3d_vpm.h"
#include "libs/linkres/linkres.h"
#include "libs/compute/compute.h"
#include "libs/platform/v3d_scheduler.h"

#include "Allocating.h"
#include "DevMemDataBlock.h"
#include "DescriptorInfo.h"
#include "CompiledShaderHandle.h"
#include "DevMemHeap.h"

#include <array>
#include <bitset>

typedef struct GLSL_PROGRAM_T_   GLSL_PROGRAM_T;
typedef struct CompiledShader_s  CompiledShader;

namespace bvk {

struct UniformEntry
{
   BackendUniformFlavour   flavour;
   uint32_t                value;
};

struct ShaderData : public NonCopyable
{
   ShaderData() = default;
   ~ShaderData();

   void Set(DevMemHeap *devMemHeap, const BINARY_SHADER_T &bs);

   DevMemHeap                *devMemHeap = nullptr;
   DevMemRange                shaderMemory{};
   bvk::vector<UniformEntry>  uniformMap;
   v3d_threading_t            threading{};
   bool                       singleSeg{};
};

class LinkResult
{
public:
   enum eVertexPipeStage
   {
      SHADER_VPS_VS = 0,
      SHADER_VPS_GS,
      SHADER_VPS_TCS,
      SHADER_VPS_TES,
      SHADER_VPS_COUNT
   };

public:
   LinkResult(DevMemHeap *devMemHeap) :
      m_devMemHeap(devMemHeap)
   {
      static_assert(SHADER_VPS_GS  == (SHADER_VPS_VS + 1), "eVertexPipeStage enum values incorrect");
      static_assert(SHADER_VPS_TCS == (SHADER_VPS_VS + 2), "eVertexPipeStage enum values incorrect");
      static_assert(SHADER_VPS_TES == (SHADER_VPS_VS + 3), "eVertexPipeStage enum values incorrect");
   }

   virtual ~LinkResult()
   {
   }

   bool HasFlag(uint32_t flag) const { return (m_data.flags & flag) != 0; }

   void LinkShaders(CompiledShaderHandle shaders[SHADER_FLAVOUR_COUNT],
                    std::function<GLSL_BACKEND_CFG_T(const GLSL_PROGRAM_T *)> calcBackendKey,
                    const std::bitset<V3D_MAX_ATTR_ARRAYS> &attribRBSwaps);

   static ShaderFlavour ConvertShaderFlavour(unsigned stage);

   DescriptorTables *GetDescriptorTables(ShaderFlavour flavour) { return &m_descriptorTables[flavour]; }

private:
   void Build(const IR_PROGRAM_T *irProg, const BINARY_PROGRAM_T *binary);

public:
   DevMemHeap       *m_devMemHeap;
   ShaderData        m_vps[SHADER_VPS_COUNT][MODE_COUNT];
   ShaderData        m_fs;

   linkres_t         m_data = {};

   DescriptorTables  m_descriptorTables[SHADER_FLAVOUR_COUNT];

   struct CS
   {
#if V3D_USE_CSD
      // num_wgs and unif_addr are not used here.
      v3d_compute_subjob subjob{};
#else
      compute_program program{};
      bool allowConcurrentJobs{};
#endif
   } m_cs;
};

} // namespace bvk
