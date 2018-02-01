/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "LinkResult.h"
#include "Pipeline.h"
#include "Linker.h"
#include "DevMemHeap.h"

#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_shadrec.h"
#include "libs/compute/compute.h"
#include "libs/platform/v3d_scheduler.h"

#include <string.h>
#include <functional>

#include "glsl_binary_shader.h"
#include "glsl_program.h"
#include "glsl_compiler.h"

namespace bvk {

void ShaderData::Set(DevMemHeap *dmHeap, const BINARY_SHADER_T &bs)
{
   devMemHeap = dmHeap;

   // Allocate device memory for the shader code
   devMemHeap->Allocate(&shaderMemory, bs.code_size, V3D_SHADREC_ALIGN);

   memcpy(shaderMemory.Ptr(), bs.code, bs.code_size);
   shaderMemory.SyncMemory();

   uniformMap.resize(bs.unif_count);
   for (uint32_t u = 0; u < bs.unif_count; u++)
   {
      uniformMap[u].flavour = bs.unif[u].type;
      uniformMap[u].value   = bs.unif[u].value;
   }

   threading  = v3d_translate_threading(bs.n_threads);
   singleSeg  = bs.single_seg;
}

ShaderData::~ShaderData()
{
   if (devMemHeap != nullptr)
      devMemHeap->Free(shaderMemory);
}

void LinkResult::LinkShaders(CompiledShaderHandle shaders[SHADER_FLAVOUR_COUNT],
                             std::function<GLSL_BACKEND_CFG_T(const GLSL_PROGRAM_T *)> calcBackendKey,
                             const std::bitset<V3D_MAX_ATTR_ARRAYS> &attribRBSwaps)
{
   using namespace std::placeholders;  // for _1, _2

   Linker::OutputFunc buildFn = std::bind(&LinkResult::Build, this, _1, _2);

   Linker::LinkShaders(shaders, calcBackendKey, buildFn, attribRBSwaps);
}

ShaderFlavour LinkResult::ConvertShaderFlavour(unsigned stage)
{
   // Map from vertex pipe stages to GLSL shader flavours.
   const ShaderFlavour flavours[SHADER_VPS_COUNT] =
   {
      SHADER_VERTEX,
      SHADER_GEOMETRY,
      SHADER_TESS_CONTROL,
      SHADER_TESS_EVALUATION
   };

   return flavours[stage];
}

// Build a LinkResult from the linker output
void LinkResult::Build(const IR_PROGRAM_T *ir, const BINARY_PROGRAM_T *prog)
{
   if (!prog)
      return;

   bool hasVertex  = ProgHasVStage(prog, SHADER_VERTEX);
   bool hasTessEv  = ProgHasVStage(prog, SHADER_TESS_EVALUATION);
   bool hasGeom    = ProgHasVStage(prog, SHADER_GEOMETRY);

   // Fill in the ShaderData structures for each shader stage.
   for (unsigned m = 0; m != MODE_COUNT; ++m)
   {
      for (unsigned s = 0; s != SHADER_VPS_COUNT; ++s)
      {
         const BINARY_SHADER_T *bs = prog->vstages[ConvertShaderFlavour(s)][m];

         if (bs != nullptr)
            m_vps[s][m].Set(m_devMemHeap, *bs);
      }
   }
   if (prog->fshader != nullptr)
      m_fs.Set(m_devMemHeap, *prog->fshader);

   // Copy the varyings map
   assert(prog->vary_map.n <= V3D_MAX_VARYING_COMPONENTS);
   m_numVarys = prog->vary_map.n;
   for (unsigned i = 0; i != m_numVarys; i++)
      m_varyMap[i] = prog->vary_map.entries[i];

   // Set the initial flags
   SetInitialFlags(*ir, *prog);

   // Fill out vertex shader and attribute data
   if (hasVertex)
   {
      if (!hasTessEv && !hasGeom && prog->vstages[SHADER_VERTEX][MODE_RENDER]->u.vertex.has_point_size)
         m_flags |= POINT_SIZE_SHADED_VERTEX_DATA;

      SetVertexShaderData(*prog);

      m_attrCount = 0;
      for (int i = 0; i < V3D_MAX_ATTR_ARRAYS; i++)
      {
         if (prog->vstages[SHADER_VERTEX][MODE_BIN]->u.vertex.attribs.scalars_used[i] > 0 ||
             prog->vstages[SHADER_VERTEX][MODE_RENDER]->u.vertex.attribs.scalars_used[i] > 0)
         {
            AttributeRecord &ar = m_attr[m_attrCount];
            ar.idx          = i;
            ar.cScalarsUsed = prog->vstages[SHADER_VERTEX][MODE_BIN]->u.vertex.attribs.scalars_used[i];
            ar.vScalarsUsed = prog->vstages[SHADER_VERTEX][MODE_RENDER]->u.vertex.attribs.scalars_used[i];
            m_attrCount++;
         }
      }

      // Data for bin load balancing, TODO T+G
      m_numBinQpuInstructions = prog->vstages[SHADER_VERTEX][MODE_BIN]->code_size / 8;
   }

   // Set the flat-shade and centroid data (per varying)
   bool centroids = true;
#if !V3D_HAS_SRS_CENTROID_FIX
   centroids = !prog->fshader->u.fragment.ignore_centroids;
#endif

   bool allCentroid = true;
   for (unsigned i = 0; i < m_numVarys; i++)
   {
      const VARYING_INFO_T &vary = ir->varying[prog->vary_map.entries[i]];
      uint32_t idx  = i / V3D_VARY_FLAGS_PER_WORD;
      uint32_t flag = 1 << (i % V3D_VARY_FLAGS_PER_WORD);

      if (vary.flat)                  m_varyingFlat[idx]          |= flag;
      if (vary.centroid && centroids) m_varyingCentroid[idx]      |= flag;
      else                            allCentroid = false;
      if (vary.noperspective)         m_varyingNoPerspective[idx] |= flag;
   }
   if (m_numVarys && allCentroid)
      // HW can avoid calculating non-centroid w and passing to frag shader,
      // but will only do so if *all* centroid flags are set. This includes
      // unused ones...
      for (unsigned i = m_numVarys; i < V3D_MAX_VARYING_COMPONENTS; i++)
         m_varyingCentroid[i / V3D_VARY_FLAGS_PER_WORD] |= 1 << (i % V3D_VARY_FLAGS_PER_WORD);

   // Fill out TnG data
   SetTnGData(*ir, *prog);

   if (ir->cs_shared_block_size != ~0u)
   {
      const V3D_IDENT_T* ident = v3d_scheduler_get_identity();
      compute_params params{};
      params.num_qpus = ident->num_qpus_per_slice * ident->num_slices;
      params.shared_mem_per_core = v3d_scheduler_get_compute_shared_mem_size_per_core();
      params.wg_size = ir->cs_wg_size[0] * ir->cs_wg_size[1] * ir->cs_wg_size[2];
      params.shared_block_size = ir->cs_shared_block_size;
      params.num_threads = prog->fshader->n_threads;
      params.has_barrier = prog->fshader->u.fragment.barrier;
      assert(!prog->fshader->u.fragment.writes_z);
      assert(!prog->fshader->u.fragment.ez_disable);
      assert(!prog->fshader->u.fragment.per_sample);
      assert(!prog->fshader->u.fragment.reads_implicit_varys);
      compute_sg_config cfg = compute_choose_sg_config(&params);

#if V3D_USE_CSD
      m_cs.subjob.wg_size = params.wg_size;
      m_cs.subjob.wgs_per_sg = cfg.wgs_per_sg;
      m_cs.subjob.max_sg_id = cfg.max_wgs / cfg.wgs_per_sg - 1;
      m_cs.subjob.threading = v3d_translate_threading(prog->fshader->n_threads);
      m_cs.subjob.single_seg = prog->fshader->single_seg;
      m_cs.subjob.propagate_nans = true;
      m_cs.subjob.shader_addr = m_fs.shaderMemory.Phys();
      m_cs.subjob.shared_block_size = ir->cs_shared_block_size;
      m_cs.subjob.no_overlap = !compute_allow_concurrent_jobs(&params, cfg.wgs_per_sg);
#else
      m_cs.program.code_addr = m_fs.shaderMemory.Phys();
      m_cs.program.wg_size[0] = ir->cs_wg_size[0];
      m_cs.program.wg_size[1] = ir->cs_wg_size[1];
      m_cs.program.wg_size[2] = ir->cs_wg_size[2];
      m_cs.program.items_per_wg = params.wg_size;
      m_cs.program.wgs_per_sg = cfg.wgs_per_sg;
      m_cs.program.max_wgs = cfg.max_wgs;
      m_cs.program.num_varys = prog->vary_map.n;
      m_cs.program.has_barrier = params.has_barrier;
      m_cs.program.has_shared = ir->cs_shared_block_size;
      m_cs.program.scb_wait_on_first_thrsw = prog->fshader->u.fragment.tlb_wait_first_thrsw;
      m_cs.program.threading = v3d_translate_threading(prog->fshader->n_threads);

      assert((unsigned)prog->vary_map.n <= countof(m_cs.program.vary_map));
      for (unsigned i = 0; i != (unsigned)prog->vary_map.n; ++i)
         m_cs.program.vary_map[i] = prog->vary_map.entries[i];

      m_cs.allowConcurrentJobs = compute_allow_concurrent_jobs(&params, cfg.wgs_per_sg);
#endif
   };
}

void LinkResult::SetInitialFlags(const IR_PROGRAM_T &ir, const BINARY_PROGRAM_T &prog)
{
   m_flags = 0;
   if (prog.fshader != nullptr)
   {
      if (prog.fshader->u.fragment.writes_z)
         m_flags |= FS_WRITES_Z;

      if (prog.fshader->u.fragment.ez_disable)
         m_flags |= FS_EARLY_Z_DISABLE;

      if (prog.fshader->u.fragment.needs_w)
         m_flags |= FS_NEEDS_W;

      if (prog.fshader->u.fragment.tlb_wait_first_thrsw)
         m_flags |= TLB_WAIT_FIRST_THRSW;

      if (ir.varyings_per_sample || prog.fshader->u.fragment.per_sample)
         m_flags |= PER_SAMPLE;

      if (prog.fshader->u.fragment.reads_prim_id && !ProgHasVStage(&prog, SHADER_GEOMETRY))
         m_flags |= (PRIM_ID_USED | PRIM_ID_TO_FS);
   }

   if (!prog.fshader || !prog.fshader->u.fragment.reads_implicit_varys)
      m_flags |= DISABLE_IMPLICIT_VARYS;
}

void LinkResult::SetVertexShaderData(const BINARY_PROGRAM_T &prog)
{
   for (unsigned m = 0; m != MODE_COUNT; m++)
   {
      unsigned inputs  = prog.vstages[SHADER_VERTEX][m]->u.vertex.input_words;
      unsigned outputs = prog.vstages[SHADER_VERTEX][m]->u.vertex.output_words;

      if (prog.vstages[SHADER_VERTEX][m]->u.vertex.combined_seg_ok)
      {
         m_vsOutputWords[m] = gfx_umax(inputs, outputs);
         m_vsInputWords[m]  = 0;
      }
      else
      {
         m_flags |= (VS_SEPARATE_I_O_VPM_BLOCKS << m);
         m_vsOutputWords[m] = outputs;
         m_vsInputWords[m]  = inputs;
      }

      if (prog.vstages[SHADER_VERTEX][m]->u.vertex.attribs.vertexid_used)
         m_flags |= (VS_READS_VERTEX_ID << m);
      if (prog.vstages[SHADER_VERTEX][m]->u.vertex.attribs.instanceid_used)
         m_flags |= (VS_READS_INSTANCE_ID << m);
      if (prog.vstages[SHADER_VERTEX][m]->u.vertex.attribs.baseinstance_used)
         m_flags |= (VS_READS_BASE_INSTANCE << m);
   }
}

void LinkResult::SetTnGData(const IR_PROGRAM_T &ir, const BINARY_PROGRAM_T &prog)
{
   bool hasTessEv = ProgHasVStage(&prog, SHADER_TESS_EVALUATION);
   m_hasGeom      = ProgHasVStage(&prog, SHADER_GEOMETRY);
   m_hasTess      = ProgHasVStage(&prog, SHADER_TESS_CONTROL);
   assert(m_hasTess == hasTessEv);

   if (m_hasTess)
   {
      if (!m_hasGeom && prog.vstages[SHADER_TESS_EVALUATION][MODE_RENDER]->u.tess_e.has_point_size)
         m_flags |= POINT_SIZE_SHADED_VERTEX_DATA;

      if (prog.vstages[SHADER_TESS_CONTROL][MODE_RENDER]->u.tess_c.prim_id_used ||
          prog.vstages[SHADER_TESS_EVALUATION][MODE_RENDER]->u.tess_e.prim_id_used)
         m_flags |= PRIM_ID_USED;

      if (prog.vstages[SHADER_TESS_CONTROL][MODE_BIN]->u.tess_c.barrier)
         m_flags |= TCS_BARRIERS;

      m_tcsOutputVerticesPerPatch = ir.tess_vertices;

      m_tessEdgeSpacing = ir.tess_spacing;
      m_tessType        = ir.tess_mode;
      m_tessPointMode   = ir.tess_point_mode;
      m_tessClockwise   = ir.tess_cw;

      for (unsigned m = 0; m != MODE_COUNT; m++)
      {
         m_tcsOutputWordsPerPatch[m] = prog.vstages[SHADER_TESS_CONTROL][m]->u.tess_c.output_words_patch;
         m_tcsOutputWords[m] = prog.vstages[SHADER_TESS_CONTROL][m]->u.tess_c.output_words;
         m_tesOutputWords[m] = prog.vstages[SHADER_TESS_EVALUATION][m]->u.tess_e.output_words;
      }
   }

   if (m_hasGeom)
   {
      if (prog.vstages[SHADER_GEOMETRY][MODE_RENDER]->u.geometry.has_point_size)
         m_flags |= POINT_SIZE_SHADED_VERTEX_DATA;

      if (prog.vstages[SHADER_GEOMETRY][MODE_RENDER]->u.geometry.prim_id_used)
         m_flags |= PRIM_ID_USED;

      m_geomPrimType    = ir.gs_out;
      m_geomInvocations = ir.gs_n_invocations;

      for (unsigned m = 0; m != MODE_COUNT; m++)
         m_gsOutputWords[m] = prog.vstages[SHADER_GEOMETRY][m]->u.geometry.output_words;
   }
}

} // namespace bvk
