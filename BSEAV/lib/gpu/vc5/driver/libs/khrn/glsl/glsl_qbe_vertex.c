/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "glsl_backflow.h"
#include "glsl_backflow_visitor.h"
#include "glsl_backend.h"

#include "glsl_sched_node_helpers.h"
#include "glsl_qbe_vertex.h"

#include "libs/util/gfx_util/gfx_util.h"

#include <assert.h>

static void collect_shader_outputs(SchedBlock *block, int block_id, const IRShader *sh,
                                   const LinkMap *link_map, const bool *shader_outputs_used, Backflow **nodes)
{
   for (int i=0; i<link_map->num_outs; i++) {
      int out_idx = link_map->outs[i];
      if (out_idx == -1) continue;
      if (!shader_outputs_used[out_idx]) continue;

      IROutput *o = &sh->outputs[out_idx];
      if (o->block == block_id) {
         nodes[i] = block->outputs[o->output];
      } else {
         nodes[i] = tr_external(o->block, o->output, &block->external_list);
      }
   }
}

/* Convert clip to window coordinates. i indexes through (x, y, z) */
static Backflow *get_win_coord(Backflow *clip, Backflow *recip_w, int i) {
   static const BackendSpecialUniformFlavour vp_scale[3] = { BACKEND_SPECIAL_UNIFORM_VP_SCALE_X,
                                                             BACKEND_SPECIAL_UNIFORM_VP_SCALE_Y,
                                                             BACKEND_SPECIAL_UNIFORM_VP_SCALE_Z };
   Backflow *win = mul(clip, tr_special_uniform(vp_scale[i]));
   if (!is_const(recip_w) || !(recip_w->unif == gfx_float_to_bits(1.0f)))
      win = mul(win, recip_w);

   Backflow *ret;
   if (i == 2) ret = add(win, tr_special_uniform(BACKEND_SPECIAL_UNIFORM_VP_OFFSET_Z));
   else        ret = tr_uop(V3D_QPU_OP_FTOIZ, tr_uop(V3D_QPU_OP_FFLOOR, win));
   ret->age = win->age = clip->age;
   return ret;
}

static Backflow *get_recip_w(Backflow *clip_w) {
   Backflow *ret;
   if (is_const(clip_w)) {
      float w_val = gfx_float_from_bits(clip_w->unif);
      ret = tr_cfloat(1.0f/w_val);
   } else
      ret = recip(clip_w);
   ret->age = clip_w->age;
   return ret;
}

#if V3D_VER_AT_LEAST(4,1,34,0)

static void vpmw(SchedBlock *b, uint32_t addr, Backflow *param, Backflow *dep, Backflow *final_vpmwt)
{
   Backflow *result = tr_binop_io(V3D_QPU_OP_STVPMV, tr_const(addr), param, dep);
   result->age = param->age;

#if V3D_HAS_GFXH1684_FIX
   assert(!final_vpmwt);
   glsl_backflow_chain_push_back(&b->iodeps, result);
#else
   glsl_iodep(final_vpmwt, result);
#endif
}

static void vertex_backend(const VertexBackendState *s,
                           SchedBlock *block, SchedShaderInputs *ins,
                           Backflow *clip_x, Backflow *clip_y,
                           Backflow *clip_z, Backflow *clip_w,
                           Backflow *point_size,
                           Backflow **vertex_vary)
{
   Backflow *fake_vpm_deps[MAX_VPM_DEPENDENCY] = { NULL, };
   Backflow **vpm_dep = ins ? ins->vpm_dep : fake_vpm_deps;

#if V3D_HAS_GFXH1684_FIX
   Backflow *final_vpmwt = NULL;
#else
   Backflow *final_vpmwt = glsl_backflow_vpmwt();
   glsl_backflow_chain_push_back(&block->iodeps, final_vpmwt);
#endif

   Backflow *recip_w = clip_w ? get_recip_w(clip_w) : NULL;
   Backflow *win_x   = (clip_x && recip_w) ? get_win_coord(clip_x, recip_w, 0) : NULL;
   Backflow *win_y   = (clip_y && recip_w) ? get_win_coord(clip_y, recip_w, 1) : NULL;
   Backflow *win_z   = (clip_z && recip_w) ? get_win_coord(clip_z, recip_w, 2) : NULL;

   if (s->bin_mode) {
      if (clip_x != NULL) vpmw(block, 0, clip_x, vpm_dep[0], final_vpmwt);
      if (clip_y != NULL) vpmw(block, 1, clip_y, vpm_dep[1], final_vpmwt);
      if (clip_z != NULL) vpmw(block, 2, clip_z, vpm_dep[2], final_vpmwt);
      if (clip_w != NULL) vpmw(block, 3, clip_w, vpm_dep[3], final_vpmwt);
      if (win_x  != NULL) vpmw(block, 4, win_x,  vpm_dep[4], final_vpmwt);
      if (win_y  != NULL) vpmw(block, 5, win_y,  vpm_dep[5], final_vpmwt);
   } else {
      if (win_x   != NULL) vpmw(block, 0, win_x,   vpm_dep[0], final_vpmwt);
      if (win_y   != NULL) vpmw(block, 1, win_y,   vpm_dep[1], final_vpmwt);
      if (win_z   != NULL) vpmw(block, 2, win_z,   vpm_dep[2], final_vpmwt);
      if (recip_w != NULL) vpmw(block, 3, recip_w, vpm_dep[3], final_vpmwt);
   }

   if (s->emit_point_size) {
      unsigned addr = s->bin_mode ? 6 : 4;
      vpmw(block, addr, point_size, vpm_dep[addr], final_vpmwt);
   }

   if (s->z_only_active) {
      /* For z-only mode we output z,w normally, just z for points */
      unsigned z_only_addr = (s->bin_mode ? 6 : 4) + (s->emit_point_size ? 1 : 0);
      if (win_z != NULL) vpmw(block, z_only_addr, win_z, vpm_dep[z_only_addr], final_vpmwt);
      if (!s->emit_point_size) {
         if (recip_w != NULL) vpmw(block, z_only_addr + 1, recip_w, vpm_dep[z_only_addr+1], final_vpmwt);
      }
   }

   if (s->vary_map != NULL) {
      unsigned vary_addr = (s->bin_mode ? 6 : 4) + (s->emit_point_size ? 1 : 0) +
                           (s->z_only_active ? (s->emit_point_size ? 1 : 2) : 0);
      for (int i = 0; i < s->vary_map->n; i++) {
         if (vertex_vary[s->vary_map->entries[i]] != NULL) {
            Backflow *read_dep = (vary_addr+i) < MAX_VPM_DEPENDENCY ? vpm_dep[vary_addr+i] : NULL;
            vpmw(block, vary_addr+i, vertex_vary[s->vary_map->entries[i]], read_dep, final_vpmwt);
         }
      }
   }
}

#else

static Backflow *vpm_write_setup(uint32_t addr)
{
   assert(addr <= 0x1fff);
   uint32_t value = 1<<24 | 1<<22 | 1<<15 | 2<<13 | (addr & 0x1fff);
   return tr_uop(V3D_QPU_OP_VPMSETUP, tr_const(value));
}

/* Write depends on both the previous write and the read from this location */
static Backflow *vpmw(Backflow *param, Backflow *write_dep, Backflow *read_dep)
{
   Backflow *result = tr_mov_to_reg(REG_MAGIC_VPM, param);
   glsl_iodep(result, write_dep);
   glsl_iodep(result, read_dep);
   result->age = param->age;
   if (result->age == 0) result->age = write_dep->age;
   return result;
}

static void vertex_backend(const VertexBackendState *s,
                           SchedBlock *block, SchedShaderInputs *ins,
                           Backflow *clip_x, Backflow *clip_y,
                           Backflow *clip_z, Backflow *clip_w,
                           Backflow *point_size,
                           Backflow **vertex_vary)
{
   Backflow *dep;
   Backflow *win_z, *recip_w;
   Backflow *fake_vpm_deps[MAX_VPM_DEPENDENCY] = { NULL, };
   Backflow **vpm_dep = ins ? ins->vpm_dep : fake_vpm_deps;
   unsigned write_index = 0;

   /* Unless the coordinates are valid then we skip writing them at all. The *
    * clip header, however, can be used for transform feedback, so if any    *
    * value is valid it must be output. Here we fill in the values to be     *
    * output, which has the side effect of making the coordinates valid, so  *
    * they get output below.                                                 */
   if (s->bin_mode && (clip_x != NULL || clip_y != NULL ||
                       clip_z != NULL || clip_w != NULL   ) )
   {
      if (clip_x == NULL) clip_x = tr_const(0);
      if (clip_y == NULL) clip_y = tr_const(0);
      if (clip_z == NULL) clip_z = tr_const(0);
      if (clip_w == NULL) clip_w = tr_const(1);
   }

   if (clip_x != NULL && clip_y != NULL && clip_z != NULL && clip_w != NULL) {
      dep = vpm_write_setup(0);
      block->first_vpm_write = dep;

      if (s->bin_mode) {
         dep = vpmw(clip_x, dep, vpm_dep[write_index++]);
         dep = vpmw(clip_y, dep, vpm_dep[write_index++]);
         dep = vpmw(clip_z, dep, vpm_dep[write_index++]);
         dep = vpmw(clip_w, dep, vpm_dep[write_index++]);
      }

      recip_w         = get_recip_w(clip_w);
      Backflow *win_x = get_win_coord(clip_x, recip_w, 0);
      Backflow *win_y = get_win_coord(clip_y, recip_w, 1);
      win_z           = get_win_coord(clip_z, recip_w, 2);

      dep = vpmw(win_x, dep, vpm_dep[write_index++]);
      dep = vpmw(win_y, dep, vpm_dep[write_index++]);
      if (!s->bin_mode) {
         dep = vpmw(win_z,   dep, vpm_dep[write_index++]);
         dep = vpmw(recip_w, dep, vpm_dep[write_index++]);
      }
   } else {
      write_index = s->bin_mode ? 6 : 4;
      dep = vpm_write_setup(write_index);
      block->first_vpm_write = dep;
      win_z = tr_const(0);
      recip_w = tr_const(1);
   }

   if (s->emit_point_size) dep = vpmw(point_size, dep, vpm_dep[write_index++]);

   if (s->z_only_active) {
      /* For z-only mode we output z,w normally, just z for points */
      dep = vpmw(win_z, dep, vpm_dep[write_index++]);
      if (!s->emit_point_size) dep = vpmw(recip_w, dep, vpm_dep[write_index++]);
   }

   assert(s->vary_map != NULL);
   for (int i = 0; i < s->vary_map->n; i++) {
      Backflow *read_dep = write_index < MAX_VPM_DEPENDENCY ? vpm_dep[write_index] : NULL;
      write_index++;
      if (vertex_vary[s->vary_map->entries[i]] != NULL)
         dep = vpmw(vertex_vary[s->vary_map->entries[i]], dep, read_dep);
      else
         dep = vpmw(tr_const(0), dep, read_dep);
   }

   glsl_backflow_chain_push_back(&block->iodeps, dep);
}

#endif

void glsl_vertex_backend(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   SchedShaderInputs *ins,
   const VertexBackendState *s,
   const bool *shader_outputs_used)
{
   Backflow *bnodes[DF_BLOB_VERTEX_COUNT] = { 0, };
   collect_shader_outputs(block, block_id, sh, link_map, shader_outputs_used, bnodes);

   vertex_backend(s, block, ins,
                  bnodes[DF_VNODE_X], bnodes[DF_VNODE_Y],
                  bnodes[DF_VNODE_Z], bnodes[DF_VNODE_W],
                  bnodes[DF_VNODE_POINT_SIZE],
                  bnodes+DF_VNODE_VARY(0));
}
