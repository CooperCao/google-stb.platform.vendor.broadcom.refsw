/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "libs/core/v3d/v3d_ver.h"

#if V3D_VER_AT_LEAST(3,3,0,0)

#include "glxx_compute.h"
#include "../common/khrn_render_state.h"
#include "glxx_hw.h"
#include "glxx_server.h"
#include "glxx_server_pipeline.h"
#include "libs/compute/compute.h"
#include "libs/core/v3d/v3d_csd.h"

typedef struct compute_dispatch_cmd
{
   uint32_t num_groups[3];
} compute_dispatch_cmd;

typedef struct glxx_compute_num_work_groups
{
   const uint32_t* cpu_ptr;
   khrn_resource* indirect_res;
   v3d_size_t indirect_offset;
} glxx_compute_num_work_groups;

#if !V3D_USE_CSD
static uint8_t* write_cl(void* fmem, unsigned size)
{
   return khrn_fmem_cle((khrn_fmem*)fmem, size);
}

static uint8_t* write_cl_final(void* fmem, unsigned size)
{
   return khrn_fmem_cle_final((khrn_fmem*)fmem, size);
}

static compute_cl_mem_if const mem_if =
{
   .write_cl = write_cl,
   .write_cl_final = write_cl_final
};
#endif

static inline v3d_addr_t build_shader_uniforms(
   glxx_compute_render_state* rs,
   GLXX_SERVER_STATE_T* state,
   GLXX_LINK_RESULT_DATA_T const* link_data,
   glxx_hw_image_like_uniforms const* image_like_uniforms,
   const glxx_compute_num_work_groups* num_work_groups,
   unsigned shared_block_size)
{
   glxx_hw_compute_uniforms cu = {
      .num_work_groups = num_work_groups->cpu_ptr,
      .read_in_preprocess = num_work_groups->indirect_res != NULL
   };

   static_assrt(GLXX_CONFIG_MAX_COMPUTE_SHARED_MEM_SIZE <= V3D_SCHEDULER_COMPUTE_MIN_SHARED_MEM_PER_CORE);
   if (shared_block_size != 0)
   {
    #if V3D_USE_L2T_LOCAL_MEM
      cu.shared_ptr = v3d_scheduler_get_compute_shared_mem_addr();
    #else
      gmem_handle_t shared_mem = v3d_scheduler_get_compute_shared_mem(rs->fmem.br_info.details.secure, /*alloc=*/true);
      if (!shared_mem)
         return 0;
      cu.shared_ptr = khrn_fmem_sync_and_get_addr(
         &rs->fmem,
         shared_mem,
         0,
         V3D_BARRIER_TMU_DATA_READ | V3D_BARRIER_TMU_DATA_WRITE);
    #endif
   }

   GL20_HW_INDEXED_UNIFORM_T iu = { .valid = false };
   return glxx_hw_install_uniforms(
      &rs->base,
      state,
      MODE_RENDER,
      link_data->fs.uniform_map,
      &iu,
      image_like_uniforms,
      &cu);
}

static glxx_compute_render_state* create_compute_render_state(GLXX_SERVER_STATE_T* server_state)
{
   assert(server_state->compute_render_state == NULL);
   glxx_compute_render_state* rs = khrn_render_state_get_glxx_compute(
      khrn_render_state_new(KHRN_RENDER_STATE_TYPE_GLXX_COMPUTE)
      );

   if (!khrn_fmem_init(&rs->fmem, (khrn_render_state*)rs))
      goto error;

   #if !V3D_USE_CSD
   {
      khrn_render_state_disallow_flush((khrn_render_state*)rs);

      rs->clist_start = khrn_fmem_begin_clist(&rs->fmem);
      if (!rs->clist_start || !compute_cl_begin(&mem_if, &rs->fmem))
      {
         khrn_fmem_discard(&rs->fmem);
         goto error;
      }

      khrn_render_state_allow_flush((khrn_render_state*)rs);
   }
   #endif

   /* Record security status */
   rs->fmem.br_info.details.secure = egl_context_gl_secure(server_state->context);

   rs->server_state = server_state;
   server_state->compute_render_state = rs;

   khrn_mem_acquire(server_state->shared->gpu_aborted);
   rs->fmem.persist->gpu_aborted = server_state->shared->gpu_aborted;

   return rs;

error:
   khrn_render_state_delete((khrn_render_state*)rs);
   return NULL;
}

static bool dispatch_compute(GLXX_SERVER_STATE_T* state, glxx_compute_num_work_groups const* num_work_groups)
{
   glxx_compute_render_state* rs = state->compute_render_state;
   if (!rs)
   {
      rs = create_compute_render_state(state);
      if (!rs)
         return false;
   }

   bool success = false;
   khrn_render_state_disallow_flush((khrn_render_state*)rs);

   if (  !khrn_fmem_record_fence_to_signal(&rs->fmem, state->fences.fence)
      || !khrn_fmem_record_fence_to_depend_on(&rs->fmem, state->fences.fence_to_depend_on) )
   {
      goto end;
   }

   // For indirect dispatch, we'll need to read the buffer in preprocess.
   if (num_work_groups->indirect_res != NULL)
   {
      bool ok = khrn_fmem_record_preprocess_resource_read(
         &rs->fmem,
         num_work_groups->indirect_res,
         num_work_groups->indirect_offset,
         sizeof(compute_dispatch_cmd));
      if (!ok)
         goto end;
   }

   glxx_hw_image_like_uniforms image_like_uniforms;
   if (!glxx_compute_image_like_uniforms(state, &rs->base, &image_like_uniforms))
      goto end;

   IR_PROGRAM_T const* ir_program = gl20_program_common_get(state)->linked_glsl_program->ir;
   unsigned items_per_wg = ir_program->cs_wg_size[0] * ir_program->cs_wg_size[1] * ir_program->cs_wg_size[2];

   GLSL_BACKEND_CFG_T backend_cfg;
   backend_cfg.backend = 0;
 #if !V3D_USE_CSD
   backend_cfg.backend |= compute_backend_flags(items_per_wg);
 #endif
 #if KHRN_DEBUG
   if (khrn_options.no_ubo_to_unif)
      backend_cfg.backend |= GLSL_DISABLE_UBO_FETCH;
 #endif

   GLXX_LINK_RESULT_DATA_T* link_data = glxx_get_shaders(state, &backend_cfg);
   if (!link_data || !khrn_fmem_sync_res(&rs->fmem, link_data->res, 0, V3D_BARRIER_QPU_INSTR_READ))
      goto end;

   v3d_addr_t unifs_addr = build_shader_uniforms(rs, state, link_data, &image_like_uniforms, num_work_groups, ir_program->cs_shared_block_size);
   if (!unifs_addr)
      goto end;
   v3d_addr_t code_addr = khrn_resource_get_addr(link_data->res) + link_data->fs.code_offset;

   glxx_compute_dispatch* dispatch = khrn_vector_emplace_back(glxx_compute_dispatch, &rs->fmem.persist->compute_dispatches);
   if (!dispatch)
      goto end;

   #if V3D_USE_CSD
   {
      if (num_work_groups->indirect_res != NULL)
      {
         // Will access this pointer in preprocess.
         glxx_compute_indirect* indirect = khrn_vector_emplace_back(glxx_compute_indirect, &rs->fmem.persist->compute_indirect);
         if (!indirect)
         {
            khrn_vector_pop_back(&rs->fmem.persist->compute_dispatches);
            goto end;
         }

         indirect->num_wgs = num_work_groups->cpu_ptr;
         indirect->dispatch_index = rs->fmem.persist->compute_dispatches.size - 1;
      }
      else
      {
         const uint32_t* num_wgs_ptr = num_work_groups->cpu_ptr;
         dispatch->num_wgs_x = num_wgs_ptr[0];
         dispatch->num_wgs_y = num_wgs_ptr[1];
         dispatch->num_wgs_z = num_wgs_ptr[2];
      }

      dispatch->wg_size = items_per_wg;
      dispatch->wgs_per_sg = link_data->cs.wgs_per_sg;
      dispatch->max_sg_id = link_data->cs.max_sg_id;
      dispatch->threading = link_data->fs.threading;
      dispatch->single_seg = link_data->fs.single_seg;
      dispatch->propagate_nans = true;
      dispatch->shader_addr = code_addr;
      dispatch->unifs_addr = unifs_addr;
      dispatch->shared_block_size = ir_program->cs_shared_block_size;
      dispatch->no_overlap = !link_data->cs.allow_concurrent_jobs;
   }
   #else
   {
      uint8_t* dispatch_cl = compute_cl_add_dispatch(&mem_if, &rs->fmem);
      if (!dispatch_cl)
      {
         khrn_vector_pop_back(&rs->fmem.persist->compute_dispatches);
         goto end;
      }

      // Initialise compute binary program.
      compute_program* program = &dispatch->program;
      program->code_addr = code_addr;
      program->wg_size[0] = ir_program->cs_wg_size[0];
      program->wg_size[1] = ir_program->cs_wg_size[1];
      program->wg_size[2] = ir_program->cs_wg_size[2];
      program->items_per_wg = items_per_wg;
      program->wgs_per_sg = link_data->cs.wgs_per_sg;
      program->max_wgs = link_data->cs.max_wgs;
      program->has_barrier = link_data->cs.has_barrier;
      program->has_shared = ir_program->cs_shared_block_size != 0;
      program->num_varys = link_data->data.vary.count;
      program->scb_wait_on_first_thrsw = true;
      program->threading = link_data->fs.threading;

      assert(link_data->data.vary.count <= countof(program->vary_map));
      for (unsigned i = 0; i != link_data->data.vary.count; ++i)
         program->vary_map[i] = link_data->data.vary.map[i];

      dispatch->unifs_addr = unifs_addr;
      dispatch->dispatch_cl = dispatch_cl;

      if (num_work_groups->indirect_res != NULL)
      {
         // Will access this pointer in preprocess.
         dispatch->num_work_groups.indirect = num_work_groups->cpu_ptr;
         dispatch->is_indirect = true;
      }
      else
      {
         // Need to copy this pointer for later.
         dispatch->num_work_groups.immediate[0] = num_work_groups->cpu_ptr[0];
         dispatch->num_work_groups.immediate[1] = num_work_groups->cpu_ptr[1];
         dispatch->num_work_groups.immediate[2] = num_work_groups->cpu_ptr[2];
         dispatch->is_indirect = false;
      }

      if (!link_data->cs.allow_concurrent_jobs)
         rs->fmem.br_info.details.render_no_overlap = true;
   }
   #endif

   success = true;

end:

   khrn_render_state_allow_flush((khrn_render_state*)rs);

   // Check to see if we need to flush due to high client fmem use.
   // timh-todo: allow batching of smaller dispatches for SW compute runtime.
   // TODO: Batching currently breaks on multicore things such as Penrose.
   if (1 || !V3D_USE_CSD || !success || khrn_fmem_should_flush(&rs->fmem))
      glxx_compute_render_state_flush(rs);

   return success;
}

static bool check_state(GLXX_SERVER_STATE_T* state)
{
   if (state->current_program != NULL) {
      if (!gl20_validate_program(state, &state->current_program->common)) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
   } else if (state->pipelines.bound != NULL) {
      if (!glxx_pipeline_validate(state->pipelines.bound)) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
      if (!glxx_pipeline_create_compute_common(state->pipelines.bound)) {
         glxx_server_state_set_error(state, GL_INVALID_OPERATION);
         return false;
      }
   } else {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }

   GLSL_PROGRAM_T const* linked_program = gl20_program_common_get(state)->linked_glsl_program;
   if (!glsl_program_has_stage(linked_program, SHADER_COMPUTE))
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      return false;
   }

   unsigned const* wg_size = linked_program->ir->cs_wg_size;
   if (wg_size[0] == 0 || wg_size[1] == 0 || wg_size[2] == 0)
      return false;

   return true;
}

static bool check_num_work_groups(GLXX_SERVER_STATE_T* state, const uint32_t num_work_groups[3])
{
   // Give up (without error) if nothing to do.
   if (!num_work_groups[0] || !num_work_groups[1] || !num_work_groups[2])
      return false;

   static_assrt(GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT < (1 << 64/3));
   if (  num_work_groups[0] > GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT
      || num_work_groups[1] > GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT
      || num_work_groups[2] > GLXX_CONFIG_MAX_COMPUTE_GROUP_COUNT)
   {
      if (state)
         glxx_server_state_set_error(state, GL_INVALID_VALUE);
      return false;
   }

   if (((uint64_t)num_work_groups[0] * num_work_groups[1] * num_work_groups[2]) > UINT32_MAX)
   {
      if (state)
         glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      return false;
   }

   return true;
}

GL_APICALL void GL_APIENTRY glDispatchCompute(GLuint num_work_groups_x, GLuint num_work_groups_y, GLuint num_work_groups_z)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T* state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (!check_state(state))
      goto end;

   const uint32_t num_work_groups_vec[3] = { num_work_groups_x, num_work_groups_y, num_work_groups_z };
   glxx_compute_num_work_groups num_work_groups = {
      .cpu_ptr = num_work_groups_vec,
   };

   if (!check_num_work_groups(state, num_work_groups_vec))
      goto end;

   if (!dispatch_compute(state, &num_work_groups))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

end:
   glxx_unlock_server_state();
}

GL_APICALL void GL_APIENTRY glDispatchComputeIndirect(GLintptr indirect)
{
   PROFILE_FUNCTION_MT("GL");

   GLXX_SERVER_STATE_T* state = glxx_lock_server_state(OPENGL_ES_3X);
   if (!state)
      return;

   if (indirect < 0 || (indirect & 3))
   {
      glxx_server_state_set_error(state, GL_INVALID_VALUE);
      goto end;
   }

   GLXX_BUFFER_T* buffer = state->bound_buffer[GLXX_BUFTGT_DISPATCH_INDIRECT].obj;
   if (  buffer == NULL
      || buffer->size < sizeof(compute_dispatch_cmd)
      || buffer->size - sizeof(compute_dispatch_cmd) < (size_t)indirect)
   {
      glxx_server_state_set_error(state, GL_INVALID_OPERATION);
      goto end;
   }

   if (!check_state(state))
      goto end;

   glxx_compute_num_work_groups num_work_groups = { 0, };

   // Try and access the indirect buffer now if possible.
   bool read_now = true;
   num_work_groups.cpu_ptr = khrn_resource_try_read_now(
      buffer->resource,
      indirect,
      sizeof(compute_dispatch_cmd),
      &read_now);
   if (!num_work_groups.cpu_ptr)
   {
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);
      goto end;
   }

   if (read_now)
   {
      if (!check_num_work_groups(NULL, num_work_groups.cpu_ptr))
         goto end;
   }
   else
   {
      num_work_groups.indirect_res = buffer->resource;
      num_work_groups.indirect_offset = indirect;
   }

   if (!dispatch_compute(state, &num_work_groups))
      glxx_server_state_set_error(state, GL_OUT_OF_MEMORY);

end:
   glxx_unlock_server_state();
}

void glxx_compute_render_state_flush(glxx_compute_render_state* rs)
{
   if (rs->fmem.persist->compute_dispatches.size)
   {
      khrn_render_state_begin_flush((khrn_render_state*)rs);

      khrn_fmem* fmem = &rs->fmem;
    #if !V3D_USE_CSD
      compute_cl_end(&mem_if, fmem);
      fmem->render_rw_flags |= compute_mem_access_flags();
      fmem->br_info.num_layers = 1;
      fmem->br_info.render_subjobs.num_subjobs = 1;
      v3d_subjob subjobs;
      subjobs.start = rs->clist_start;
      subjobs.end = khrn_fmem_end_clist(fmem);
      fmem->br_info.render_subjobs.subjobs = &subjobs;

    #endif
      khrn_fmem_flush(fmem);
   }
   else
   {
      khrn_fmem_discard(&rs->fmem);
   }

   assert(rs == rs->server_state->compute_render_state);
   rs->server_state->compute_render_state = NULL;
   khrn_render_state_delete((khrn_render_state*)rs);
}

#if !V3D_USE_CSD
compute_job_mem* glxx_compute_process_dispatches(const khrn_vector* dispatches_vec)
{
   assert(dispatches_vec->size != 0);

   compute_job_mem* mem = compute_job_mem_new();
   if (!mem)
      return NULL;

   bool any_valid = false;
   for (unsigned i = 0; i != dispatches_vec->size; ++i)
   {
      glxx_compute_dispatch const* dispatch = khrn_vector_data(glxx_compute_dispatch, dispatches_vec) + i;

      const uint32_t* num_work_groups = dispatch->num_work_groups.immediate;
      if (dispatch->is_indirect)
      {
         // OK to use the indirect pointer as this was synced with khrn_fmem_record_preprocess_resource_read.
         num_work_groups = dispatch->num_work_groups.indirect;

         // Invalid number of work-groups, silently fail.
         if (!check_num_work_groups(NULL, num_work_groups))
            continue;
      }

      any_valid |= compute_build_dispatch(
         mem,
         dispatch->dispatch_cl,
         &dispatch->program,
         dispatch->unifs_addr,
         num_work_groups);

      // timh-todo: report out-of-memory failure somehow?
   }

   if (any_valid)
   {
      compute_job_mem_flush(mem);
   }
   else
   {
      compute_job_mem_delete(mem);
      mem = NULL;
   }
   return mem;
}
#endif

#if V3D_USE_CSD
void glxx_compute_process_indirect_dispatches(
   khrn_vector* dispatches_vec,
   const khrn_vector* indirect_vec)
{
   // Update while filtering invalid dispatches.
   unsigned i = 0;
   unsigned d = 0;
   for (unsigned s = 0; s != dispatches_vec->size; ++s)
   {
      glxx_compute_dispatch* src = khrn_vector_data(glxx_compute_dispatch, dispatches_vec) + s;
      if (i != indirect_vec->size)
      {
         const glxx_compute_indirect* indirect = khrn_vector_data(glxx_compute_indirect, indirect_vec) + i;
         if (indirect->dispatch_index == s)
         {
            i += 1;
            if (!check_num_work_groups(NULL, indirect->num_wgs))
               continue;
            src->num_wgs_x = indirect->num_wgs[0];
            src->num_wgs_y = indirect->num_wgs[1];
            src->num_wgs_z = indirect->num_wgs[2];
         }
      }

      if (s != d)
         khrn_vector_data(glxx_compute_dispatch, dispatches_vec)[d] = *src;
      d += 1;
   }
   assert(i == indirect_vec->size);
   dispatches_vec->size = d;
}
#endif

#endif
