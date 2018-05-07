/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "linkres.h"
#include "../glsl/glsl_binary_program.h"

static inline bool prog_has_vstage(const BINARY_PROGRAM_T *p, ShaderFlavour f) {
   assert(( p->vstages[f][MODE_RENDER] &&  p->vstages[f][MODE_BIN]) ||
          (!p->vstages[f][MODE_RENDER] && !p->vstages[f][MODE_BIN]));
   return p->vstages[f][MODE_RENDER] != NULL;
}

static void fshader_flags(const BINARY_PROGRAM_T *p, linkres_t *out) {
   if (p->fshader != NULL)
   {
      /* Fill in the LINK_RESULT_DATA based on the fshader compiler output */
      if (p->fshader->u.fragment.writes_z)
         out->flags |= LINKRES_FLAG_FS_WRITES_Z;

      if (p->fshader->u.fragment.ez_disable)
         out->flags |= LINKRES_FLAG_FS_EARLY_Z_DISABLE;

      if (p->fshader->u.fragment.needs_w)
         out->flags |= LINKRES_FLAG_FS_NEEDS_W;

      if (p->fshader->u.fragment.tlb_wait_first_thrsw)
         out->flags |= LINKRES_FLAG_TLB_WAIT_FIRST_THRSW;

      if (p->fshader->u.fragment.per_sample)
         out->flags |= LINKRES_FLAG_PER_SAMPLE;

      if (p->fshader->u.fragment.reads_prim_id && !prog_has_vstage(p, SHADER_GEOMETRY))
         out->flags |= (LINKRES_FLAG_PRIM_ID_USED | LINKRES_FLAG_PRIM_ID_TO_FS);
   }

#if V3D_VER_AT_LEAST(4,1,34,0)
   if (!p->fshader || !p->fshader->u.fragment.reads_implicit_varys)
      out->flags |= LINKRES_FLAG_DISABLE_IMPLICIT_VARYS;
#endif
}

static void write_vertex_shader_data(const BINARY_PROGRAM_T *p, linkres_t *out)
{
   for (unsigned m = 0; m != MODE_COUNT; ++m)
   {
      unsigned inputs  = p->vstages[SHADER_VERTEX][m]->u.vertex.input_words;
      unsigned outputs = p->vstages[SHADER_VERTEX][m]->u.vertex.output_words;

      if (p->vstages[SHADER_VERTEX][m]->u.vertex.combined_seg_ok) {
         out->vs_output_words[m] = gfx_umax(inputs, outputs);
         out->vs_input_words[m]  = 0;
      } else {
         if (inputs > 0) out->flags |= LINKRES_FLAG_VS_SEPARATE_VPM_BLOCKS << m;

         out->vs_output_words[m] = outputs;
         out->vs_input_words[m]  = inputs;
      }

      if (p->vstages[SHADER_VERTEX][m]->u.vertex.attribs.vertexid_used)
         out->flags |= LINKRES_FLAG_VS_VERTEX_ID << m;
      if (p->vstages[SHADER_VERTEX][m]->u.vertex.attribs.instanceid_used)
         out->flags |= LINKRES_FLAG_VS_INSTANCE_ID << m;
#if V3D_VER_AT_LEAST(4,1,34,0)
      if (p->vstages[SHADER_VERTEX][m]->u.vertex.attribs.baseinstance_used)
         out->flags |= LINKRES_FLAG_VS_BASE_INSTANCE << m;
#endif
   }
}

static void fill_vary_data(const BINARY_PROGRAM_T *p, const IR_PROGRAM_T *ir, linkres_vary_data_t *out) {
   assert(p->vary_map.n <= V3D_MAX_VARYING_COMPONENTS);
   out->count = p->vary_map.n;
#if !V3D_USE_CSD
   for (unsigned i = 0; i != p->vary_map.n; ++i)
      out->map[i] = p->vary_map.entries[i];
#endif

   bool all_centroid = true;
   bool centroids    = true;
#if !V3D_VER_AT_LEAST(4,2,14,0)
   assert(p->fshader);     /* null_fs was not available pre-v4.2 */
   centroids = glsl_ir_program_has_stage(ir, SHADER_FRAGMENT) && !p->fshader->u.fragment.per_sample;
#endif

   for (unsigned i = 0; i < out->count; i++) {
      const VARYING_INFO_T *vary = &ir->varying[p->vary_map.entries[i]];
      uint32_t idx = i / V3D_VARY_FLAGS_PER_WORD;
      uint32_t flag = 1 << (i % V3D_VARY_FLAGS_PER_WORD);

      if (vary->flat)                  out->flat[idx]          |= flag;
      if (vary->centroid && centroids) out->centroid[idx]      |= flag;
      else                             all_centroid = false;
#if V3D_VER_AT_LEAST(4,1,34,0)
      if (vary->noperspective)         out->noperspective[idx] |= flag;
#else
      assert(!vary->noperspective);
#endif
   }
   if (out->count && all_centroid)
      // HW can avoid calculating non-centroid w and passing to frag shader,
      // but will only do so if *all* centroid flags are set. This includes
      // unused ones...
      for (unsigned i = out->count; i < V3D_MAX_VARYING_COMPONENTS; i++)
         out->centroid[i / V3D_VARY_FLAGS_PER_WORD] |= 1 << (i % V3D_VARY_FLAGS_PER_WORD);
}

void linkres_fill_data(const BINARY_PROGRAM_T *p, const IR_PROGRAM_T *ir, linkres_t *out)
{
   if (glsl_ir_program_has_stage(ir, SHADER_FRAGMENT)) fshader_flags(p, out);

   bool point_size_included = false;

   if (prog_has_vstage(p, SHADER_VERTEX))
   {
      if (!prog_has_vstage(p, SHADER_TESS_EVALUATION) && !prog_has_vstage(p, SHADER_GEOMETRY))
         point_size_included = p->vstages[SHADER_VERTEX][MODE_RENDER]->u.vertex.has_point_size;

      write_vertex_shader_data(p, out);

      out->attr_count = 0;
      for (int i = 0; i < V3D_MAX_ATTR_ARRAYS; i++) {
         if (p->vstages[SHADER_VERTEX][MODE_BIN]->u.vertex.attribs.scalars_used[i]    > 0 ||
             p->vstages[SHADER_VERTEX][MODE_RENDER]->u.vertex.attribs.scalars_used[i] > 0   )
         {
            int a = out->attr_count++;
            out->attr[a].idx = i;
            out->attr[a].c_scalars_used = p->vstages[SHADER_VERTEX][MODE_BIN]->u.vertex.attribs.scalars_used[i];
            out->attr[a].v_scalars_used = p->vstages[SHADER_VERTEX][MODE_RENDER]->u.vertex.attribs.scalars_used[i];
         }
      }

      /* Data for bin load balancing, todo T+G */
      out->num_bin_qpu_instructions = p->vstages[SHADER_VERTEX][MODE_BIN]->code_size/8;
   }

   fill_vary_data(p, ir, &out->vary);

#if V3D_VER_AT_LEAST(4,1,34,0)
   out->has_geom = prog_has_vstage(p, SHADER_GEOMETRY);
   out->has_tess = prog_has_vstage(p, SHADER_TESS_CONTROL);
   uint8_t has_tesse = prog_has_vstage(p, SHADER_TESS_EVALUATION);
   assert((out->has_tess ^ has_tesse) == 0);

   if (out->has_tess)
   {
      if (!prog_has_vstage(p, SHADER_GEOMETRY))
         point_size_included = p->vstages[SHADER_TESS_EVALUATION][MODE_RENDER]->u.tess_e.has_point_size;

      if (p->vstages[SHADER_TESS_CONTROL   ][MODE_RENDER]->u.tess_c.prim_id_used ||
          p->vstages[SHADER_TESS_EVALUATION][MODE_RENDER]->u.tess_e.prim_id_used )
      {
         out->flags |= LINKRES_FLAG_PRIM_ID_USED;
      }

      if (p->vstages[SHADER_TESS_CONTROL][MODE_BIN]->u.tess_c.barrier)
         out->flags |= LINKRES_FLAG_TCS_BARRIERS;

      out->tcs_output_vertices_per_patch = ir->tess_vertices;

      out->tess_type         = ir->tess_mode;
      out->tess_edge_spacing = ir->tess_spacing;
      out->tess_point_mode   = ir->tess_point_mode;
      out->tess_clockwise    = ir->tess_cw;

      for (unsigned m = 0; m != MODE_COUNT; ++m) {
         out->tcs_output_words_per_patch[m] = p->vstages[SHADER_TESS_CONTROL][m]->u.tess_c.output_words_patch;
         out->tcs_output_words[m] = p->vstages[SHADER_TESS_CONTROL][m]->u.tess_c.output_words;
         out->tes_output_words[m] = p->vstages[SHADER_TESS_EVALUATION][m]->u.tess_e.output_words;
      }
   }

   if (out->has_geom)
   {
      point_size_included = p->vstages[SHADER_GEOMETRY][MODE_RENDER]->u.geometry.has_point_size;

      if (p->vstages[SHADER_GEOMETRY][MODE_RENDER]->u.geometry.prim_id_used)
         out->flags |= LINKRES_FLAG_PRIM_ID_USED;

      out->geom_prim_type   = ir->gs_out;
      out->geom_invocations = ir->gs_n_invocations;

      for (unsigned m = 0; m != MODE_COUNT; ++m)
         out->gs_output_words[m] = p->vstages[SHADER_GEOMETRY][m]->u.geometry.output_words;
   }
#endif

   if(point_size_included)
      out->flags |= LINKRES_FLAG_POINT_SIZE;
}

#if V3D_VER_AT_LEAST(4,1,34,0)
void linkres_compute_tng_vpm_cfg(
   v3d_vpm_cfg_v cfg_v[2],
   uint32_t shadrec_tg_packed[],
   const linkres_t *link_data,
   unsigned num_patch_vertices, uint32_t vpm_size)
{
   V3D_VPM_CFG_TG_T cfg_tg[2];
   bool ok = v3d_vpm_compute_cfg_tg(
      cfg_v, cfg_tg,
      link_data->has_tess,
      link_data->has_geom,
      vpm_size / 512,
      link_data->vs_input_words,
      link_data->vs_output_words,
      num_patch_vertices,
      link_data->tcs_output_words_per_patch,
      link_data->tcs_output_words,
      !!(link_data->flags & LINKRES_FLAG_TCS_BARRIERS),
      link_data->tcs_output_vertices_per_patch,
      link_data->tes_output_words,
      link_data->tess_type,
      6u, // maximum number of vertices per GS primitive
      link_data->gs_output_words);
   assert(ok); // GL requirements mean this should always succeed.

   V3D_SHADREC_GL_TESS_OR_GEOM_T shadrec_tg =
   {
      .tess_type           = link_data->tess_type,
      .tess_point_mode     = link_data->tess_point_mode,
      .tess_edge_spacing   = link_data->tess_edge_spacing,
      .tess_clockwise      = link_data->tess_clockwise,
      //.tcs_bypass        = todo_not_implemented,
      //.tcs_bypass_render = todo_not_implemented,
      .tes_no_inp_verts    = true, // todo_not_implemented
      .num_tcs_invocations = gfx_umax(link_data->tcs_output_vertices_per_patch, 1u),
      .geom_output         = link_data->geom_prim_type,
      .geom_num_instances  = gfx_umax(link_data->geom_invocations, 1u),
      .bin                 = cfg_tg[0],
      .render              = cfg_tg[1],
   };
   v3d_pack_shadrec_gl_tess_or_geom(shadrec_tg_packed, &shadrec_tg);
}
#endif
