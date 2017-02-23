/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
common GL ES 1.1 and 2.0 code for
shaders as dataflow graphs and passing them to the compiler backend.
=============================================================================*/

#include "../glsl/glsl_binary_program.h"
#include "glxx_shader.h"
#include "glxx_server.h"
#include "libs/core/v3d/v3d_shadrec.h"

static GLXX_UNIFORM_MAP_T *format_uniform_map(uint32_t *uniform_map,
                                              size_t    uniform_count)
{
   const unsigned entries_size = sizeof(*uniform_map) * (2*uniform_count);
   GLXX_UNIFORM_MAP_T *our_uniform_map;

   our_uniform_map = malloc(offsetof(GLXX_UNIFORM_MAP_T, entry) + entries_size);
   if (our_uniform_map == NULL) return NULL;

   our_uniform_map->count = uniform_count;
   memcpy(our_uniform_map->entry, uniform_map, entries_size);

   return our_uniform_map;
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
   data->threading   = v3d_translate_threading(bin->n_threads);

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
bool glxx_hw_emit_shaders(GLXX_LINK_RESULT_DATA_T      *data,
                          const GLXX_LINK_RESULT_KEY_T *key,
                          IR_PROGRAM_T                 *ir)
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
