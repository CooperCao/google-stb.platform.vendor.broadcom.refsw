/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "libs/core/v3d/v3d_gen.h"
#include "libs/khrn/glsl/glsl_backend_uniforms.h"
#include "libs/khrn/glsl/glsl_binary_program.h"
#include "libs/khrn/glsl/glsl_backend_cfg.h"
#include "libs/core/v3d/v3d_vpm.h"
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

struct AttributeRecord
{
   uint8_t idx;
   uint8_t cScalarsUsed;
   uint8_t vScalarsUsed;
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

   enum eFlags
   {
      POINT_SIZE_SHADED_VERTEX_DATA       = 1 << 0,
      VS_READS_VERTEX_ID                  = 1 << 2,
      VS_READS_VERTEX_ID_BIN              = 1 << 2,
      VS_READS_VERTEX_ID_RENDER           = 1 << 3,
      VS_READS_INSTANCE_ID                = 1 << 4,
      VS_READS_INSTANCE_ID_BIN            = 1 << 4,
      VS_READS_INSTANCE_ID_RENDER         = 1 << 5,
      VS_READS_BASE_INSTANCE              = 1 << 6,
      VS_READS_BASE_INSTANCE_BIN          = 1 << 6,
      VS_READS_BASE_INSTANCE_RENDER       = 1 << 7,
      VS_SEPARATE_I_O_VPM_BLOCKS          = 1 << 8,
      VS_SEPARATE_I_O_VPM_BLOCKS_BIN      = 1 << 8,
      VS_SEPARATE_I_O_VPM_BLOCKS_RENDER   = 1 << 9,
      FS_WRITES_Z                         = 1 << 10,
      FS_EARLY_Z_DISABLE                  = 1 << 11,
      FS_NEEDS_W                          = 1 << 12,
      TLB_WAIT_FIRST_THRSW                = 1 << 13,
      PER_SAMPLE                          = 1 << 14,
      TCS_BARRIERS                        = 1 << 15,
      PRIM_ID_USED                        = 1 << 16,
      PRIM_ID_TO_FS                       = 1 << 17,
      DISABLE_IMPLICIT_VARYS              = 1 << 18
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

   bool HasFlag(eFlags flag) const { return (m_flags & flag) != 0; }

   void LinkShaders(CompiledShaderHandle shaders[SHADER_FLAVOUR_COUNT],
                    std::function<GLSL_BACKEND_CFG_T(const GLSL_PROGRAM_T *)> calcBackendKey,
                    const std::bitset<V3D_MAX_ATTR_ARRAYS> &attribRBSwaps);

   static ShaderFlavour ConvertShaderFlavour(unsigned stage);

   DescriptorTables *GetDescriptorTables(ShaderFlavour flavour) { return &m_descriptorTables[flavour]; }

private:
   void Build(const IR_PROGRAM_T *irProg, const BINARY_PROGRAM_T *binary);
   void SetInitialFlags(const IR_PROGRAM_T &ir, const BINARY_PROGRAM_T &prog);
   void SetVertexShaderData(const BINARY_PROGRAM_T &prog);
   void SetTnGData(const IR_PROGRAM_T &ir, const BINARY_PROGRAM_T &prog);

   bool ProgHasVStage(const BINARY_PROGRAM_T *p, ShaderFlavour f)
   {
      assert((p->vstages[f][MODE_RENDER] && p->vstages[f][MODE_BIN]) ||
            (!p->vstages[f][MODE_RENDER] && !p->vstages[f][MODE_BIN]));
      return p->vstages[f][MODE_RENDER] != NULL;
   }

public:
   DevMemHeap       *m_devMemHeap;
   ShaderData        m_vps[SHADER_VPS_COUNT][MODE_COUNT];
   ShaderData        m_fs;

   uint32_t          m_numVarys;
   uint8_t           m_varyMap[V3D_MAX_VARYING_COMPONENTS] = {};
   uint32_t          m_numBinQpuInstructions;

   uint32_t          m_attrCount;
   AttributeRecord   m_attr[V3D_MAX_ATTR_ARRAYS];

   uint32_t          m_flags;
   uint8_t           m_vsInputWords[MODE_COUNT]  = {};
   uint8_t           m_vsOutputWords[MODE_COUNT] = {};

   uint32_t          m_varyingCentroid[V3D_MAX_VARY_FLAG_WORDS]      = {};
   uint32_t          m_varyingFlat[V3D_MAX_VARY_FLAG_WORDS]          = {};
   uint32_t          m_varyingNoPerspective[V3D_MAX_VARY_FLAG_WORDS] = {};

   DescriptorTables  m_descriptorTables[SHADER_FLAVOUR_COUNT];

   ////////////////////////////////////////////////////
   // Everything from here down is specific only to T&G
   ////////////////////////////////////////////////////

   // set to 0 if TCS doesn't write vertex data, glxx_hw will create 1 invocation for the patch.
   uint8_t                    m_tcsOutputVerticesPerPatch;
   uint8_t                    m_tcsOutputWordsPerPatch[MODE_COUNT];
   uint8_t                    m_tcsOutputWords[MODE_COUNT];
   uint8_t                    m_tesOutputWords[MODE_COUNT];
   uint16_t                   m_gsOutputWords[MODE_COUNT];

   uint8_t                    m_geomInvocations;
   v3d_cl_geom_prim_type_t    m_geomPrimType;

   v3d_cl_tess_type_t         m_tessType;
   v3d_cl_tess_edge_spacing_t m_tessEdgeSpacing;
   bool                       m_tessPointMode;
   bool                       m_tessClockwise;

   bool                       m_hasTess = false;
   bool                       m_hasGeom = false;

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
