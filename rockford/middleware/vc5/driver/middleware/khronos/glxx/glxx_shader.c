/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
common GL ES 1.1 and 2.0 code for
shaders as dataflow graphs and passing them to the compiler backend.
=============================================================================*/

#include "middleware/khronos/glsl/glsl_binary_program.h"
#include "middleware/khronos/glxx/glxx_shader.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "helpers/v3d/v3d_shadrec.h"

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

static uint32_t get_output_size(uint32_t inputs, uint32_t outputs) {
   uint32_t input_size  = (inputs  + 7) >> 3;
   uint32_t output_size = (outputs + 7) >> 3;
   assert(input_size <= 16);
   assert(output_size >= 1 && output_size <= 16);

   return MAX(input_size, output_size) & 0xff;
}

static void write_cv_shader_data(GLXX_LINK_RESULT_DATA_T  *data,
                                 int c_shader_varys,
                                 const BINARY_PROGRAM_T   *prog)
{
   const bool point_size_included = prog->has_point_size;

   if(point_size_included) {
      data->flags |= GLXX_SHADER_FLAGS_POINT_SIZE_SHADED_VERTEX_DATA;
   }

   data->vs_output_size = get_output_size(prog->vshader->u.vertex.reads_total,
                                          4 + point_size_included + data->num_varys);

   if (prog->vshader->u.vertex.attribs.vertexid_used)
      data->flags |= GLXX_SHADER_FLAGS_VS_READS_VERTEX_ID;
   if (prog->vshader->u.vertex.attribs.instanceid_used)
      data->flags |= GLXX_SHADER_FLAGS_VS_READS_INSTANCE_ID;

   data->cs_output_size = get_output_size(prog->cshader->u.vertex.reads_total,
                                          6 + point_size_included + c_shader_varys);

   if (prog->cshader->u.vertex.attribs.vertexid_used)
      data->flags |= GLXX_SHADER_FLAGS_CS_READS_VERTEX_ID;
   if (prog->cshader->u.vertex.attribs.instanceid_used)
      data->flags |= GLXX_SHADER_FLAGS_CS_READS_INSTANCE_ID;
}

static bool write_common_data(GLXX_SHADER_DATA_T    *data,
                              const BINARY_SHADER_T *bin)
{
   data->res_i       = khrn_res_interlock_from_data(bin->code, bin->code_size,
                                                    V3D_MAX_QPU_INSTRS_READAHEAD,
                                                    8, GMEM_USAGE_ALL, "Shader");
   data->uniform_map = format_uniform_map(bin->unif, bin->unif_count);
   data->threading   = v3d_translate_threading(bin->n_threads);

   return (data->res_i != NULL) && (data->uniform_map != NULL);
}

static bool write_link_result_data(GLXX_LINK_RESULT_DATA_T  *data,
                                   const IR_PROGRAM_T       *ir,
                                   const BINARY_PROGRAM_T   *prog,
                                   bool                      z_prepass)
{
   /* Convert all the common data to the format expected by the driver */
   {
      bool ok = true;
      ok = ok && write_common_data(&data->f, prog->fshader);
      ok = ok && (!prog->cshader || write_common_data(&data->c, prog->cshader));
      ok = ok && (!prog->vshader || write_common_data(&data->v, prog->vshader));
      if (!ok)
         return false;
   }

   assert(prog->vary_map.n <= GL20_CONFIG_MAX_VARYING_SCALARS);
   data->num_varys = prog->vary_map.n;

   /* Write out the LINK_RESULT_DATA based on the fshader compiler output */
   data->flags = GLXX_SHADER_FLAGS_ENABLE_CLIPPING; /* write_cv_shader_data may add more */

   if (prog->fshader->u.fragment.discard || prog->fshader->u.fragment.z_change) {
      /* Discard shaders do explicit z writes. Z-changing shaders obviously do too. */
      data->flags |= GLXX_SHADER_FLAGS_FS_WRITES_Z;
   }

   if (prog->fshader->u.fragment.z_change) {
      /* z-change shaders need early-z off, discard ones don't */
      data->flags |= GLXX_SHADER_FLAGS_FS_NOT_EARLY_Z_COMPATIBLE;
   }

   if (prog->cshader)
   {
      /* Now do the same for the coordinate and vertex shaders */
      write_cv_shader_data(data, ir->tf_vary_map.n, prog);

      data->attr_count = 0;
      for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++) {
         if (prog->cshader->u.vertex.attribs.scalars_used[i] > 0 ||
             prog->vshader->u.vertex.attribs.scalars_used[i] > 0   )
         {
            int a = data->attr_count++;
            data->attr[a].idx = i;
            data->attr[a].c_scalars_used = prog->cshader->u.vertex.attribs.scalars_used[i];
            data->attr[a].v_scalars_used = prog->vshader->u.vertex.attribs.scalars_used[i];
         }
      }

      data->bin_uses_control_flow = prog->cshader->uses_control_flow;
      data->render_uses_control_flow = prog->vshader->uses_control_flow ||
                                       prog->fshader->uses_control_flow;

      if (z_prepass) data->render_uses_control_flow = data->render_uses_control_flow ||
                                                      prog->cshader->uses_control_flow;

      /* Data for bin load balancing */
      data->num_bin_qpu_instructions = prog->cshader->code_size/8;
   }
   else
   {
      data->attr_count = 0;
      data->bin_uses_control_flow = false;
      data->render_uses_control_flow = prog->fshader->uses_control_flow;
      data->num_bin_qpu_instructions = 0;
   }

   memset(data->varying_centroid, 0, sizeof(data->varying_centroid));
   memset(data->varying_flat,     0, sizeof(data->varying_flat));

   for (unsigned i = 0; i < data->num_varys; i++) {
      const VARYING_INFO_T *vary = &ir->varying[prog->vary_map.entries[i]];
      if (vary->centroid) {
         data->varying_centroid[i/24] |= 1 << (i % 24);
      }
      if (vary->flat) {
         data->varying_flat[i/24] |= 1 << (i % 24);
      }
   }
   return true;
}

/* Create shaders for a program's dataflow */
bool glxx_hw_emit_shaders(GLXX_LINK_RESULT_DATA_T      *data,
                          const GLXX_LINK_RESULT_KEY_T *key,
                          IR_PROGRAM_T                 *ir)
{
   BINARY_PROGRAM_T *prog;
   bool success = false;

   /* Hackily randomise the centroid-ness of the varyings */
   for (int i=0; i<V3D_MAX_VARYING_COMPONENTS; i++) {
      if (!ir->varying[i].flat && khrn_options_make_centroid()) ir->varying[i].centroid = true;
   }

   prog = glsl_binary_program_from_dataflow(ir, key, khrn_get_v3d_version());
   if(!prog) {
      goto cleanup;
   }

   if(!write_link_result_data(data, ir, prog, (key->backend & GLXX_Z_ONLY_WRITE))) {
      glxx_hw_cleanup_shaders(data);
      goto cleanup;
   }

   success = true;

 cleanup:
   /* Everything in the binary gets copied away into internal driver datastructures,
      and the binary itself can be freed */
   glsl_binary_program_free(prog);

   return success;
}

static void clear_shader(GLXX_SHADER_DATA_T *data)
{
   khrn_res_interlock_refdec(data->res_i);
   free(data->uniform_map);

   data->res_i       = NULL;
   data->uniform_map = NULL;
}

/* Clean up shaders previously emitted by glxx_hw_emit_shaders */
void glxx_hw_cleanup_shaders(GLXX_LINK_RESULT_DATA_T *data)
{
   clear_shader(&data->c);
   clear_shader(&data->v);
   clear_shader(&data->f);
}
