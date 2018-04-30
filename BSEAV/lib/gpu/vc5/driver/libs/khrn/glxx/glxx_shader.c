/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../glsl/glsl_binary_program.h"
#include "../glsl/glsl_backend_uniforms.h"
#include "glxx_shader.h"
#include "glxx_server.h"
#include "libs/khrn/common/khrn_mem.h"
#include "libs/core/v3d/v3d_shadrec.h"
#include "libs/util/gfx_util/gfx_util.h"

typedef struct glxx_shader_ubo_load_batch_tmp
{
   uint32_t index;      // UBO index.
   uint32_t load_end;   // End of loads from this UBO.
   uint16_t range_start;
   uint16_t range_end;
} glxx_shader_ubo_load_batch_tmp;

static inline size_t compute_array_offset_fn(size_t* cur_offset, size_t alignof_type, size_t sizeof_type, size_t count)
{
   size_t offset = gfx_zround_up_p2(*cur_offset, alignof_type);
   *cur_offset = offset + sizeof_type*count;
   return offset;
}

#define compute_array_offset(cur_offset, type, count)\
   compute_array_offset_fn(cur_offset, align_of(type), sizeof(type), count)

static GLXX_UNIFORM_MAP_T *format_uniform_map(umap_entry *uniform_map,
                                              size_t      uniform_count)
{
   GLXX_UNIFORM_MAP_T* ret = NULL;
   unsigned num_generals;
   unsigned num_batches = 0;
   glxx_shader_ubo_load_batch_tmp batches[GLXX_SHADER_MAX_UNIFORM_BUFFERS];
   uint8_t buffer_map[GLXX_SHADER_MAX_UNIFORM_BUFFERS];
   memset(buffer_map, 0xff, sizeof(buffer_map));

   for (unsigned pass2 = 0; pass2 <= 1; ++pass2)
   {
      unsigned general_seek = 0;
      unsigned num_loads = 0;
      num_generals = 0;

      for (unsigned i = 0; i != uniform_count; ++i)
      {
         const BackendUniformFlavour type  = uniform_map[i].type;
         const uint32_t              value = uniform_map[i].value;

         uint32_t binding = ~0u;
         uint32_t offset = 0;
         switch (type)
         {
         case BACKEND_UNIFORM_UBO_LOAD:
            binding = value & gfx_mask(5);
            offset = value >> 5;
            assert(binding < GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS);
            assert(offset <= GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE-sizeof(uint32_t));
            assert(!(offset & 3)); // 4 byte alignment
            break;
         case BACKEND_UNIFORM_SPECIAL:
            if (value >= BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_X && value <= BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_Z)
            {
               binding = GLXX_SHADER_COMPUTE_INDIRECT_BUFFER;
               offset = sizeof(uint32_t)*(value - BACKEND_SPECIAL_UNIFORM_NUMWORKGROUPS_X);
               break;
            }
            // fallthrough
         default:
            break;
         }

         if (binding != ~0u)
         {
            // Find local buffer index.
            if (buffer_map[binding] == 0xff)
            {
               // Create new batch for this buffer if not found.
               assert(!pass2);
               buffer_map[binding] = num_batches;
               batches[num_batches].index = binding;
               batches[num_batches].load_end = 0;
               batches[num_batches].range_start = 0xffff;
               batches[num_batches].range_end = 0;
               num_batches += 1;
            }

            // Create load entry.
            glxx_shader_ubo_load_batch_tmp* batch = &batches[buffer_map[binding]];
            if (!pass2)
            {
               batch->range_start = gfx_umin(batch->range_start, offset);
               batch->range_end = gfx_umax(batch->range_end, offset + sizeof(uint32_t));
            }
            else
            {
               uint32_t word_offset = gfx_udiv_exactly(offset - batch->range_start, sizeof(uint32_t));
               ret->ubo_loads[batch->load_end].dst20_src12 = gfx_bits(i, 20) << 12
                                                             | gfx_bits(word_offset, 12);
            }
            batch->load_end += 1;
            general_seek += 1;
            num_loads += 1;
         }
         else
         {
            // Repurpose BACKEND_UNIFORM_UBO_LOAD entry to seek forward the write pointer.
            if (general_seek)
            {
               if (pass2)
               {
                  ret->general_uniforms[num_generals].type = BACKEND_UNIFORM_UBO_LOAD;
                  ret->general_uniforms[num_generals].value = general_seek;
               }
               num_generals += 1;
               general_seek = 0;
            }

            // Write the general uniform.
            if (pass2)
            {
               ret->general_uniforms[num_generals].type = type;
               ret->general_uniforms[num_generals].value = value;
            }
            num_generals += 1;
         }
      }

      // Finished at pass 2.
      if (pass2)
         break;

      // Compute size of uniform map, and offsets of all arrays.
      size_t size = sizeof(GLXX_UNIFORM_MAP_T);
      size_t offsetof_generals = compute_array_offset(&size, glxx_shader_general_uniform, num_generals);
      size_t offsetof_batches  = compute_array_offset(&size, glxx_shader_ubo_load_batch, num_batches);
      size_t offsetof_loads    = compute_array_offset(&size, glxx_shader_ubo_load, num_loads);

      // Allocate memory and initialise pointers and constants.
      char* ret_ptr = (char*)khrn_mem_alloc(size, "GLXX_UNIFORM_MAP_T");
      if (!ret_ptr)
         return NULL;
      ret = (GLXX_UNIFORM_MAP_T*)ret_ptr;
      ret->general_uniforms = (glxx_shader_general_uniform*)(ret_ptr + offsetof_generals);
      ret->ubo_load_batches = (glxx_shader_ubo_load_batch*)(ret_ptr + offsetof_batches);
      ret->ubo_loads = (glxx_shader_ubo_load*)(ret_ptr + offsetof_loads);

      // After pass1 load_end is the number of loads in this batch.
      // The load_end written to ret->batches is the cumulative number of loads up to including this batch.
      // The load_end written to batches is the cumulative number of loads up to excluding this batch.
      unsigned load_end = 0;
      for (unsigned b = 0; b != num_batches; ++b)
      {
         load_end += batches[b].load_end;
         ret->ubo_load_batches[b].index = batches[b].index;
         ret->ubo_load_batches[b].load_end = load_end;
         ret->ubo_load_batches[b].range_start = batches[b].range_start;
         ret->ubo_load_batches[b].range_size = batches[b].range_end - batches[b].range_start;
         batches[b].load_end = load_end - batches[b].load_end;
      }

      // Fill out constants.
      ret->num_uniforms = uniform_count;
      ret->num_general_uniforms = num_generals;
      ret->num_ubo_load_batches = num_batches;
   }

   return ret;
}

static bool write_common_data(GLXX_SHADER_DATA_T    *data,
                              void *code, v3d_size_t *code_offset,
                              const BINARY_SHADER_T *bin)
{
   if (bin == NULL)
   {
      data->code_offset = ~(v3d_size_t)0;
      return true;
   }

   data->code_offset = *code_offset;
   memcpy((char *)code + *code_offset, bin->code, bin->code_size);
   *code_offset += bin->code_size;

   data->uniform_map = format_uniform_map(bin->unif, bin->unif_count);
   data->threading   = v3d_translate_threading(bin->n_threads);
#if V3D_VER_AT_LEAST(4,1,34,0)
   data->single_seg  = bin->single_seg;
#endif

   return data->uniform_map != NULL;
}

static bool write_link_result_data(GLXX_LINK_RESULT_DATA_T  *data,
                                   const IR_PROGRAM_T       *ir,
                                   const BINARY_PROGRAM_T   *prog)
{
   memset(data, 0, sizeof(*data));

   // Map from vertex pipe stages to GLSL shader flavours.
   const ShaderFlavour flavours[GLXX_SHADER_VPS_COUNT] =
   {
      SHADER_VERTEX,             // GLXX_SHADER_VPS_VS
     #if V3D_VER_AT_LEAST(4,1,34,0)
      SHADER_GEOMETRY,           // GLXX_SHADER_VPS_GS
      SHADER_TESS_CONTROL,       // GLXX_SHADER_VPS_TCS
      SHADER_TESS_EVALUATION,    // GLXX_SHADER_VPS_TES
     #endif
   };

   // Separate shaders for ease of debugging.
#if KHRN_DEBUG
   const v3d_size_t code_pad_size = V3D_MAX_QPU_INSTRS_READAHEAD + 8;
#else
   const v3d_size_t code_pad_size = 0;
#endif

   v3d_size_t total_code_size = 0;
   for (unsigned m = 0; m != MODE_COUNT; ++m)
      for (unsigned s = 0; s != GLXX_SHADER_VPS_COUNT; ++s)
         if (prog->vstages[flavours[s]][m])
            total_code_size += prog->vstages[flavours[s]][m]->code_size + code_pad_size;
   if (prog->fshader != NULL)
      total_code_size += prog->fshader->code_size;

   data->res = khrn_resource_create(
      total_code_size + V3D_MAX_QPU_INSTRS_READAHEAD, 8, GMEM_USAGE_V3D_READ,
      "Shader");
   if (!data->res)
      return false;

   void *code = khrn_resource_begin_access(
      &data->res,
      0,
      total_code_size,
      KHRN_ACCESS_WRITE | KHRN_ACCESS_INVALIDATE_BUFFER,
      KHRN_RESOURCE_PARTS_ALL);
   if (!code)
      return false;
   v3d_size_t code_offset = 0;

   /* Convert all the common data to the format expected by the driver */
   for (unsigned m = 0; m != MODE_COUNT; ++m)
   {
      for (unsigned s = 0; s != GLXX_SHADER_VPS_COUNT; ++s)
      {
         BINARY_SHADER_T *bs = prog->vstages[flavours[s]][m];
         if (!write_common_data(&data->vps[s][m], code, &code_offset, bs))
            return false;
         if (bs != NULL)
            code_offset += code_pad_size;
      }
   }

   if (!write_common_data(&data->fs, code, &code_offset, prog->fshader))
      return false;

   assert(code_offset == total_code_size);
   khrn_resource_end_access(data->res, 0, total_code_size, KHRN_ACCESS_WRITE);

#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool uses_control_flow[MODE_COUNT] = { false,  };
   for (unsigned m = 0; m < MODE_COUNT; m++) {
      for (unsigned s = 0; s < SHADER_FRAGMENT; s++) {
         if (prog->vstages[s][m])
            uses_control_flow[m] |= prog->vstages[s][m]->uses_control_flow;
      }
   }
   if (prog->fshader != NULL)
      uses_control_flow[MODE_RENDER] |= prog->fshader->uses_control_flow;

   data->bin_uses_control_flow = uses_control_flow[MODE_BIN];
   data->render_uses_control_flow = uses_control_flow[MODE_RENDER];
#endif

   linkres_fill_data(prog, ir, &data->data);

 #if V3D_VER_AT_LEAST(3,3,0,0)
   if (glsl_ir_program_has_stage(ir, SHADER_COMPUTE))
   {
      const V3D_IDENT_T* ident = v3d_scheduler_get_identity();
      const compute_params params = {
         .num_qpus = ident->num_qpus_per_slice * ident->num_slices,
         .shared_mem_per_core = v3d_scheduler_get_compute_shared_mem_size_per_core(),
         .wg_size = ir->cs_wg_size[0] * ir->cs_wg_size[1] * ir->cs_wg_size[2],
         .shared_block_size = ir->cs_shared_block_size,
         .num_threads = prog->fshader->n_threads,
         .has_barrier = prog->fshader->u.compute.barrier,
         .has_subgroup_ops  = prog->fshader->u.compute.sg_ops,
         .has_quad_ops      = prog->fshader->u.compute.q_ops,
      };

      compute_sg_config cfg = compute_choose_sg_config(&params);
      data->cs.allow_concurrent_jobs = compute_allow_concurrent_jobs(&params, cfg.wgs_per_sg);

      data->cs.wgs_per_sg = cfg.wgs_per_sg;
    #if V3D_USE_CSD
      data->cs.max_sg_id = cfg.max_wgs / cfg.wgs_per_sg - 1;
    #else
      data->cs.max_wgs = cfg.max_wgs;
      data->cs.has_barrier = params.has_barrier;
    #endif
   }
 #endif

   return true;
}

/* Create shaders for a program's dataflow */
bool glxx_hw_emit_shaders(GLXX_LINK_RESULT_DATA_T  *data,
                          const GLSL_BACKEND_CFG_T *key,
                          IR_PROGRAM_T             *ir)
{
   /* Hackily randomise the centroid-ness of the varyings */
   for (int i=0; i<V3D_MAX_VARYING_COMPONENTS; i++)
      if (!ir->varying[i].flat && khrn_options_make_centroid()) ir->varying[i].centroid = true;
#if V3D_VER_AT_LEAST(4,1,34,0)
   for (int i=0; i<V3D_MAX_VARYING_COMPONENTS; i++)
      if (!ir->varying[i].flat && khrn_options_make_noperspective()) ir->varying[i].noperspective = true;
#endif

   BINARY_PROGRAM_T *prog = glsl_binary_program_from_dataflow(ir, key);
   if(!prog) return false;

   bool success = false;
   if(!write_link_result_data(data, ir, prog))
      glxx_hw_cleanup_shaders(data);
   else
      success = true;

   /* Everything in the binary gets copied away into internal driver datastructures,
      and the binary itself can be freed whether we succeed or not */
   glsl_binary_program_free(prog);

   return success;
}

static void clear_shader(GLXX_SHADER_DATA_T *data)
{
   khrn_mem_release(data->uniform_map);
   data->uniform_map = NULL;
}

/* Clean up shaders previously emitted by glxx_hw_emit_shaders */
void glxx_hw_cleanup_shaders(GLXX_LINK_RESULT_DATA_T *data)
{
   khrn_resource_refdec(data->res);
   data->res = NULL;

   for (unsigned m = 0; m != MODE_COUNT; ++m)
      for (unsigned s = 0; s != GLXX_SHADER_VPS_COUNT; ++s)
         clear_shader(&data->vps[s][m]);
   clear_shader(&data->fs);
}

void glxx_shader_process_ubo_loads(
   uint32_t* dst,
   uint32_t const* src,
   glxx_shader_ubo_load const* loads,
   unsigned num_loads)
{
   assert(num_loads);
   unsigned i = 0;
   do
   {
      dst[loads->dst20_src12 >> 12] = src[loads->dst20_src12 & gfx_mask(12)];
      loads += 1;
   }
   while (++i != num_loads);
}
