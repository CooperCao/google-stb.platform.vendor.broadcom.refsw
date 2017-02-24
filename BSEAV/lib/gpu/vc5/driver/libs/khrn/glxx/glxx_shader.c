/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../glsl/glsl_binary_program.h"
#include "../glsl/glsl_backend_uniforms.h"
#include "glxx_shader.h"
#include "glxx_server.h"
#include "libs/core/v3d/v3d_shadrec.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "vcos_types.h"

static inline size_t compute_array_offset_fn(size_t* cur_offset, size_t alignof_type, size_t sizeof_type, size_t count)
{
   size_t offset = gfx_zround_up_p2(*cur_offset, alignof_type);
   *cur_offset = offset + sizeof_type*count;
   return offset;
}

#define compute_array_offset(cur_offset, type, count)\
   compute_array_offset_fn(cur_offset, vcos_alignof(type), sizeof(type), count)

static GLXX_UNIFORM_MAP_T *format_uniform_map(uint32_t *uniform_map,
                                              size_t    uniform_count)
{
   GLXX_UNIFORM_MAP_T* ret = NULL;
   unsigned num_immediates;

   unsigned num_buffers = 0;
   glxx_ustream_buffer buffers[GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS];
   uint8_t buffer_map[GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS];
   memset(buffer_map, 0xff, sizeof(buffer_map));

   for (unsigned pass2 = 0; pass2 <= 1; ++pass2)
   {
      unsigned immediate_seek = 0;
      unsigned num_fetches = 0;
      num_immediates = 0;

      for (unsigned i = 0; i != uniform_count; ++i)
      {
         const BackendUniformFlavour type  = uniform_map[i*2 + 0];
         const uint32_t              value = uniform_map[i*2 + 1];

         if (type == BACKEND_UNIFORM_UBO_LOAD)
         {
            uint32_t binding = value & gfx_mask(5);
            uint32_t offset = value >> 5;
            assert(binding < GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS);
            assert(offset <= GLXX_CONFIG_MAX_UNIFORM_BLOCK_SIZE-sizeof(uint32_t));
            assert(!(offset & 3)); // 4 byte alignment

            // Find local buffer index.
            if (buffer_map[binding] == 0xff)
            {
               // Create buffer entry if not found.
               assert(!pass2);
               buffer_map[binding] = num_buffers;
               buffers[num_buffers].index = binding;
               buffers[num_buffers].fetch_end = 0;
               buffers[num_buffers].range_start = 0xffff;
               buffers[num_buffers].range_end = 0;
               num_buffers += 1;
            }

            // Create fetch entry.
            glxx_ustream_buffer* buffer = &buffers[buffer_map[binding]];
            if (!pass2)
            {
               buffer->range_start = gfx_umin(buffer->range_start, offset);
               buffer->range_end = gfx_umax(buffer->range_end, offset + sizeof(uint32_t));
            }
            else
            {
               uint32_t word_offset = gfx_udiv_exactly(offset - buffer->range_start, sizeof(uint32_t));
               ret->fetches[buffer->fetch_end].dst20_src12 = gfx_bits(i, 20) << 12
                                                           | gfx_bits(word_offset, 12);
            }
            buffer->fetch_end += 1;
            immediate_seek += 1;
            num_fetches += 1;
         }
         else
         {
            // Repurpose BACKEND_UNIFORM_UBO_LOAD entry to seek forward the write pointer.
            if (immediate_seek)
            {
               if (pass2)
               {
                  ret->immediates[num_immediates].type = BACKEND_UNIFORM_UBO_LOAD;
                  ret->immediates[num_immediates].value = immediate_seek;
               }
               num_immediates += 1;
               immediate_seek = 0;
            }

            // Write the immediate.
            if (pass2)
            {
               ret->immediates[num_immediates].type = type;
               ret->immediates[num_immediates].value = value;
            }
            num_immediates += 1;
         }
      }

      // Finished at pass 2.
      if (pass2)
         break;

      // Compute size of uniform map, and offsets of all arrays.
      size_t size = sizeof(GLXX_UNIFORM_MAP_T);
      size_t offsetof_immediates = compute_array_offset(&size, glxx_ustream_immediate, num_immediates);
      size_t offsetof_buffers    = compute_array_offset(&size, glxx_ustream_buffer, num_buffers);
      size_t offsetof_fetches    = compute_array_offset(&size, glxx_ustream_fetch, num_fetches);

      // Allocate memory and initialise pointers and constants.
      char* ret_ptr = (char*)malloc(size);
      if (!ret_ptr)
         return NULL;
      ret = (GLXX_UNIFORM_MAP_T*)ret_ptr;
      ret->immediates = (glxx_ustream_immediate*)(ret_ptr + offsetof_immediates);
      ret->buffers = (glxx_ustream_buffer*)(ret_ptr + offsetof_buffers);
      ret->fetches = (glxx_ustream_fetch*)(ret_ptr + offsetof_fetches);

      // After pass1 fetch_end is the number of fetches from this buffer.
      // The fetch_end written to ret->buffers is the cumulative number of fetches up to including this buffer.
      // The fetch_end written to buffers is the cumulative number of fetches up to excluding this buffer.
      unsigned fetch_end = 0;
      for (unsigned b = 0; b != num_buffers; ++b)
      {
         fetch_end += buffers[b].fetch_end;
         ret->buffers[b].index = buffers[b].index;
         ret->buffers[b].fetch_end = fetch_end;
         ret->buffers[b].range_start = buffers[b].range_start;
         ret->buffers[b].range_end = buffers[b].range_end;
         buffers[b].fetch_end = fetch_end - buffers[b].fetch_end;
      }

      // Fill out constants.
      ret->num_uniforms = uniform_count;
      ret->num_immediates = num_immediates;
      ret->num_buffers = num_buffers;
   }

   return ret;
}

static bool prog_has_vstage(const BINARY_PROGRAM_T *p, ShaderFlavour f) {
   assert( ( p->vstages[f][MODE_RENDER] &&  p->vstages[f][MODE_BIN]) ||
           (!p->vstages[f][MODE_RENDER] && !p->vstages[f][MODE_BIN])   );
   return p->vstages[f][MODE_RENDER] != NULL;
}

static void write_vertex_shader_data(GLXX_LINK_RESULT_DATA_T  *data,
                                     const BINARY_PROGRAM_T   *prog)
{
   for (unsigned m = 0; m != MODE_COUNT; ++m)
   {
      unsigned inputs  = prog->vstages[SHADER_VERTEX][m]->u.vertex.input_words;
      unsigned outputs = prog->vstages[SHADER_VERTEX][m]->u.vertex.output_words;

      if (prog->vstages[SHADER_VERTEX][m]->u.vertex.combined_seg_ok) {
         data->vs_output_words[m] = gfx_umax(inputs, outputs);
         data->vs_input_words[m] = 0;
      } else {
         if (inputs > 0)
            data->flags |= GLXX_SHADER_FLAGS_VS_SEPARATE_I_O_VPM_BLOCKS << m;
         data->vs_output_words[m] = outputs;
         data->vs_input_words[m] = inputs;
      }

      if (prog->vstages[SHADER_VERTEX][m]->u.vertex.attribs.vertexid_used)
         data->flags |= GLXX_SHADER_FLAGS_VS_READS_VERTEX_ID << m;
      if (prog->vstages[SHADER_VERTEX][m]->u.vertex.attribs.instanceid_used)
         data->flags |= GLXX_SHADER_FLAGS_VS_READS_INSTANCE_ID << m;
#if V3D_VER_AT_LEAST(4,0,2,0)
      if (prog->vstages[SHADER_VERTEX][m]->u.vertex.attribs.baseinstance_used)
         data->flags |= GLXX_SHADER_FLAGS_VS_READS_BASE_INSTANCE << m;
#endif
   }
}

static bool write_common_data(GLXX_SHADER_DATA_T    *data,
                              void *code, v3d_size_t *code_offset,
                              const BINARY_SHADER_T *bin)
{
   data->code_offset = *code_offset;
   memcpy((char *)code + *code_offset, bin->code, bin->code_size);
   *code_offset += bin->code_size;

   data->uniform_map = format_uniform_map(bin->unif, bin->unif_count);
#if V3D_HAS_RELAXED_THRSW
   data->four_thread = bin->four_thread;
   data->single_seg  = bin->single_seg;
#else
   data->threading   = v3d_translate_threading(bin->n_threads);
#endif

   return data->uniform_map != NULL;
}

static bool write_link_result_data(GLXX_LINK_RESULT_DATA_T  *data,
                                   const IR_PROGRAM_T       *ir,
                                   const BINARY_PROGRAM_T   *prog)
{
   memset(data, 0, sizeof(*data));

#if GLXX_HAS_TNG
   data->has_geom = prog_has_vstage(prog, SHADER_GEOMETRY);
   data->has_tess = prog_has_vstage(prog, SHADER_TESS_CONTROL);
   uint8_t has_tesse = prog_has_vstage(prog, SHADER_TESS_EVALUATION);
   assert((data->has_tess ^ has_tesse) == 0);
#endif

   // Map from vertex pipe stages to GLSL shader flavours.
   const ShaderFlavour flavours[GLXX_SHADER_VPS_COUNT] =
   {
      SHADER_VERTEX,             // GLXX_SHADER_VPS_VS
     #if GLXX_HAS_TNG
      SHADER_GEOMETRY,           // GLXX_SHADER_VPS_GS
      SHADER_TESS_CONTROL,       // GLXX_SHADER_VPS_TCS
      SHADER_TESS_EVALUATION,    // GLXX_SHADER_VPS_TES
     #endif
   };

   v3d_size_t total_code_size = 0;
   for (unsigned m = 0; m != MODE_COUNT; ++m)
      for (unsigned s = 0; s != GLXX_SHADER_VPS_COUNT; ++s)
         if (prog->vstages[flavours[s]][m])
            total_code_size += prog->vstages[flavours[s]][m]->code_size;
   total_code_size += prog->fshader->code_size;
   data->res_i = khrn_res_interlock_create(
      total_code_size + V3D_MAX_QPU_INSTRS_READAHEAD, 8, GMEM_USAGE_V3D_READ,
      "Shader");
   if (!data->res_i)
      return false;

   void *code = khrn_res_interlock_pre_cpu_access_now(&data->res_i,
      0, total_code_size, KHRN_MAP_WRITE_BIT | KHRN_MAP_INVALIDATE_BUFFER_BIT);
   if (!code)
      return false;
   v3d_size_t code_offset = 0;

   /* Convert all the common data to the format expected by the driver */
   for (unsigned m = 0; m != MODE_COUNT; ++m)
   {
      for (unsigned s = 0; s != GLXX_SHADER_VPS_COUNT; ++s)
      {
         BINARY_SHADER_T *bs = prog->vstages[flavours[s]][m];
         if (bs != NULL)
            if (!write_common_data(&data->vps[s][m], code, &code_offset, bs))
               return false;
      }
   }

   if (!write_common_data(&data->fs, code, &code_offset, prog->fshader))
      return false;

   assert(code_offset == total_code_size);
   khrn_res_interlock_post_cpu_access(data->res_i, 0, total_code_size, KHRN_MAP_WRITE_BIT);

#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool uses_control_flow[MODE_COUNT] = { false,  };
   for (unsigned m = 0; m < MODE_COUNT; m++) {
      for (unsigned s = 0; s < SHADER_FRAGMENT; s++) {
         if (prog->vstages[s][m])
            uses_control_flow[m] |= prog->vstages[s][m]->uses_control_flow;
      }
   }
   uses_control_flow[MODE_RENDER] |= prog->fshader->uses_control_flow;

   data->bin_uses_control_flow = uses_control_flow[MODE_BIN];
   data->render_uses_control_flow = uses_control_flow[MODE_RENDER];
#endif

   assert(prog->vary_map.n <= GLXX_CONFIG_MAX_VARYING_SCALARS);
   data->num_varys = prog->vary_map.n;
   for (unsigned i = 0; i != prog->vary_map.n; ++i)
      data->vary_map[i] = prog->vary_map.entries[i];

   /* Fill in the LINK_RESULT_DATA based on the fshader compiler output */
   if (prog->fshader->u.fragment.writes_z)
      data->flags |= GLXX_SHADER_FLAGS_FS_WRITES_Z;

   if (prog->fshader->u.fragment.ez_disable)
      data->flags |= GLXX_SHADER_FLAGS_FS_EARLY_Z_DISABLE;

   if (prog->fshader->u.fragment.tlb_wait_first_thrsw)
      data->flags |= GLXX_SHADER_FLAGS_TLB_WAIT_FIRST_THRSW;

   if (ir->varyings_per_sample || prog->fshader->u.fragment.per_sample)
      data->flags |= GLXX_SHADER_FLAGS_PER_SAMPLE;

   if (prog->fshader->u.fragment.reads_prim_id && !prog_has_vstage(prog, SHADER_GEOMETRY))
      data->flags |= (GLXX_SHADER_FLAGS_PRIM_ID_USED | GLXX_SHADER_FLAGS_PRIM_ID_TO_FS);


   /* Now fill in from any vertex pipeline stages that are present */
   bool point_size_included = false;

   if (prog_has_vstage(prog, SHADER_VERTEX))
   {
      if (!prog_has_vstage(prog, SHADER_TESS_EVALUATION) && !prog_has_vstage(prog, SHADER_GEOMETRY))
         point_size_included = prog->vstages[SHADER_VERTEX][MODE_RENDER]->u.vertex.has_point_size;

      write_vertex_shader_data(data, prog);

      data->attr_count = 0;
      for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++) {
         if (prog->vstages[SHADER_VERTEX][MODE_BIN]->u.vertex.attribs.scalars_used[i]    > 0 ||
             prog->vstages[SHADER_VERTEX][MODE_RENDER]->u.vertex.attribs.scalars_used[i] > 0   )
         {
            int a = data->attr_count++;
            data->attr[a].idx = i;
            data->attr[a].c_scalars_used = prog->vstages[SHADER_VERTEX][MODE_BIN]->u.vertex.attribs.scalars_used[i];
            data->attr[a].v_scalars_used = prog->vstages[SHADER_VERTEX][MODE_RENDER]->u.vertex.attribs.scalars_used[i];
         }
      }

      /* Data for bin load balancing, todo T+G */
      data->num_bin_qpu_instructions = prog->vstages[SHADER_VERTEX][MODE_BIN]->code_size/8;
   }

   for (unsigned i = 0; i < data->num_varys; i++) {
      const VARYING_INFO_T *vary = &ir->varying[prog->vary_map.entries[i]];
      if (vary->centroid) {
         data->varying_centroid[i/24] |= 1 << (i % 24);
      }
      if (vary->flat) {
         data->varying_flat[i/24] |= 1 << (i % 24);
      }
   }

#if GLXX_HAS_TNG
   if (prog_has_vstage(prog, SHADER_TESS_CONTROL))
   {
      assert(prog_has_vstage(prog, SHADER_TESS_EVALUATION));

      if (!prog_has_vstage(prog, SHADER_GEOMETRY))
         point_size_included = prog->vstages[SHADER_TESS_EVALUATION][MODE_RENDER]->u.tess_e.has_point_size;

      if (prog->vstages[SHADER_TESS_CONTROL   ][MODE_RENDER]->u.tess_c.prim_id_used ||
          prog->vstages[SHADER_TESS_EVALUATION][MODE_RENDER]->u.tess_e.prim_id_used )
      {
         data->flags |= GLXX_SHADER_FLAGS_PRIM_ID_USED;
      }

      if (prog->vstages[SHADER_TESS_CONTROL][MODE_BIN]->u.tess_c.barrier)
         data->flags |= GLXX_SHADER_FLAGS_TCS_BARRIERS;

      data->tcs_output_vertices_per_patch = ir->tess_vertices;

      v3d_cl_tess_type_t tess_type;
      switch (ir->tess_mode)
      {
      case TESS_ISOLINES:  tess_type = V3D_CL_TESS_TYPE_ISOLINES; break;
      case TESS_TRIANGLES: tess_type = V3D_CL_TESS_TYPE_TRIANGLE; break;
      case TESS_QUADS:     tess_type = V3D_CL_TESS_TYPE_QUAD;     break;
      default: unreachable();
      }

      v3d_cl_tess_edge_spacing_t tess_spacing;
      switch (ir->tess_spacing)
      {
      case TESS_SPACING_EQUAL:      tess_spacing = V3D_CL_TESS_EDGE_SPACING_EQUAL;           break;
      case TESS_SPACING_FRACT_EVEN: tess_spacing = V3D_CL_TESS_EDGE_SPACING_FRACTIONAL_EVEN; break;
      case TESS_SPACING_FRACT_ODD:  tess_spacing = V3D_CL_TESS_EDGE_SPACING_FRACTIONAL_ODD;  break;
      default: unreachable();
      }

      data->tess_type = tess_type;
      data->tess_edge_spacing = tess_spacing;
      data->tess_point_mode = ir->tess_point_mode;
      data->tess_clockwise = ir->tess_cw;

      for (unsigned m = 0; m != MODE_COUNT; ++m) {
         data->tcs_output_words_per_patch[m] = prog->vstages[SHADER_TESS_CONTROL][m]->u.tess_c.output_words_patch;
         data->tcs_output_words[m] = prog->vstages[SHADER_TESS_CONTROL][m]->u.tess_c.output_words;
         data->tes_output_words[m] = prog->vstages[SHADER_TESS_EVALUATION][m]->u.tess_e.output_words;
      }
   }

   if (prog_has_vstage(prog, SHADER_GEOMETRY))
   {
      point_size_included = prog->vstages[SHADER_GEOMETRY][MODE_RENDER]->u.geometry.has_point_size;

      if (prog->vstages[SHADER_GEOMETRY][MODE_RENDER]->u.geometry.prim_id_used)
         data->flags |= GLXX_SHADER_FLAGS_PRIM_ID_USED;

      switch (ir->gs_out) {
         case GS_OUT_POINTS:     data->geom_prim_type = V3D_CL_GEOM_PRIM_TYPE_POINTS;         break;
         case GS_OUT_LINE_STRIP: data->geom_prim_type = V3D_CL_GEOM_PRIM_TYPE_LINE_STRIP;     break;
         case GS_OUT_TRI_STRIP:  data->geom_prim_type = V3D_CL_GEOM_PRIM_TYPE_TRIANGLE_STRIP; break;
         default: unreachable();
      }

      data->geom_invocations = ir->gs_n_invocations;

      for (unsigned m = 0; m != MODE_COUNT; ++m)
         data->gs_output_words[m] = prog->vstages[SHADER_GEOMETRY][m]->u.geometry.output_words;
   }
#endif

   if(point_size_included)
      data->flags |= GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA;


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
   free(data->uniform_map);
   data->uniform_map = NULL;
}

/* Clean up shaders previously emitted by glxx_hw_emit_shaders */
void glxx_hw_cleanup_shaders(GLXX_LINK_RESULT_DATA_T *data)
{
   khrn_res_interlock_refdec(data->res_i);
   data->res_i = NULL;

   for (unsigned m = 0; m != MODE_COUNT; ++m)
      for (unsigned s = 0; s != GLXX_SHADER_VPS_COUNT; ++s)
         clear_shader(&data->vps[s][m]);
   clear_shader(&data->fs);
}

void glxx_shader_fill_ustream(
   uint32_t* dst,
   uint32_t const* src,
   glxx_ustream_fetch const* fetches,
   unsigned num_fetches)
{
   assert(num_fetches);
   unsigned f = 0;
   do
   {
      dst[fetches->dst20_src12 >> 12] = src[fetches->dst20_src12 & ((1 << 12) - 1)];
      fetches += 1;
   }
   while (++f != num_fetches);
}

typedef struct glxx_ustream_buffer_range
{
   gmem_handle_t mem;
   v3d_size_t start;
   v3d_size_t end;
} glxx_ustream_buffer_range;

static void flush_buffer_ranges(glxx_ustream_buffer_range const* buffers, unsigned num_buffers)
{
   for (unsigned b = 0; b != num_buffers; ++b)
   {
      gmem_invalidate_mapped_range(
         buffers[b].mem,
         buffers[b].start,
         buffers[b].end - buffers[b].start);
   }
}

void glxx_shader_process_ustream_jobs(glxx_ustream_job_block const* blocks, unsigned last_size)
{
   // Track unique buffer ranges.
   glxx_ustream_buffer_range buffers[512];
   unsigned num_buffers = 0;

   gmem_handle_t last_mem = NULL;
   unsigned last_index = ~0u;

   // Last block is partially filled, other blocks will use the maximum size.
   for (glxx_ustream_job_block const* block = blocks; block; block = block->next)
   {
      unsigned num_jobs = !block->next ? last_size : countof(block->jobs);
      for (unsigned i = 0; i != num_jobs; ++i)
      {
         glxx_ustream_job const* job = &block->jobs[i];

         if (job->src_mem == last_mem)
            goto existing;

         // Find buffer (in reverse).
         gmem_handle_t mem = job->src_mem;
         for (unsigned b = num_buffers; b-- != 0; )
         {
            if (buffers[b].mem == mem)
            {
               last_index = b;
               last_mem = mem;
               goto existing;
            }
         }

         // Full, so process buffers seen so far.
         if (num_buffers == countof(buffers))
         {
            flush_buffer_ranges(buffers, num_buffers);
            num_buffers = 0;
         }

         // Create new entry.
         last_index = num_buffers++;
         last_mem = mem;
         buffers[last_index].mem = last_mem;
         buffers[last_index].start = job->src_offset;
         buffers[last_index].end = job->src_offset + job->src_size;
         continue;

      existing:
         // Expand existing range.
         assert(last_mem == job->src_mem);
         buffers[last_index].start = gfx_umin(buffers[last_index].start, job->src_offset);
         buffers[last_index].end = gfx_umax(buffers[last_index].end, job->src_offset + job->src_size);
      }
   }
   flush_buffer_ranges(buffers, num_buffers);

   // Now that the UBOs are safe to read, loop over all the jobs and fill the uniform streams.
   for (glxx_ustream_job_block const* block = blocks; block; block = block->next)
   {
      unsigned num_jobs = !block->next ? last_size : countof(block->jobs);
      for (unsigned i = 0; i != num_jobs; ++i)
      {
         glxx_ustream_job const* job = &block->jobs[i];
         glxx_shader_fill_ustream(
            job->dst_ptr,
            (uint32_t const*)gmem_get_ptr(job->src_mem)
               + gfx_udiv_exactly(job->src_offset, sizeof(uint32_t)),
            job->fetches,
            job->num_fetches);
      }
   }
}
