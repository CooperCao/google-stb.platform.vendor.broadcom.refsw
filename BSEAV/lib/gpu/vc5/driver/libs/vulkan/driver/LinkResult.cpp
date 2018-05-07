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

#include "glsl_binary_program.h"
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

   linkres_fill_data(prog, ir, &m_data);

   if (glsl_ir_program_has_stage(ir, SHADER_COMPUTE))
   {
      const V3D_IDENT_T* ident = v3d_scheduler_get_identity();
      compute_params params{};
      params.num_qpus = ident->num_qpus_per_slice * ident->num_slices;
      params.shared_mem_per_core = v3d_scheduler_get_compute_shared_mem_size_per_core();
      params.wg_size = ir->cs_wg_size[0] * ir->cs_wg_size[1] * ir->cs_wg_size[2];
      params.shared_block_size = ir->cs_shared_block_size;
      params.num_threads = prog->fshader->n_threads;
      params.has_barrier = prog->fshader->u.compute.barrier;
      params.has_subgroup_ops  = prog->fshader->u.compute.sg_ops;
      params.has_quad_ops      = prog->fshader->u.compute.q_ops;
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
      m_cs.program.scb_wait_on_first_thrsw = true;
      m_cs.program.threading = v3d_translate_threading(prog->fshader->n_threads);

      assert((unsigned)prog->vary_map.n <= countof(m_cs.program.vary_map));
      for (unsigned i = 0; i != (unsigned)prog->vary_map.n; ++i)
         m_cs.program.vary_map[i] = prog->vary_map.entries[i];

      m_cs.allowConcurrentJobs = compute_allow_concurrent_jobs(&params, cfg.wgs_per_sg);
#endif
   };
}

} // namespace bvk
