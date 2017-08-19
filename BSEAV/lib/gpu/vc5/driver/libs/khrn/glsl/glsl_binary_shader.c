/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>

#ifdef KHRN_SHADER_DUMP
#include "libs/tools/dqasmc/dqasmc.h"
#endif

#include "glsl_backend.h"
#include "glsl_backend_uniforms.h"
#include "glsl_backflow.h"
#include "glsl_errors.h"
#include "glsl_binary_shader.h"
#include "glsl_dataflow_visitor.h"
#include "glsl_backflow_combine.h"
#include "glsl_dominators.h"

#include "glsl_sched_node_helpers.h"
#include "glsl_backend_cfg.h"
#include "glsl_qbe_vertex.h"
#include "glsl_qbe_fragment.h"
#include "glsl_inline_asm.h"

#include "libs/core/v3d/v3d_gen.h"
#include "libs/util/gfx_util/gfx_util.h"

BINARY_SHADER_T *glsl_binary_shader_create(ShaderFlavour flavour) {
   BINARY_SHADER_T *ret = malloc(sizeof(*ret));
   if(!ret) return NULL;

   ret->flavour    = flavour;
   ret->code       = NULL;
   ret->code_size  = 0;
   ret->unif       = NULL;
   ret->unif_count = 0;
   ret->n_threads  = 0;
#if V3D_HAS_RELAXED_THRSW
   ret->single_seg  = true;
#endif

   switch(flavour) {
   case SHADER_FRAGMENT:
      ret->u.fragment.writes_z             = false;
      ret->u.fragment.ez_disable           = false;
      ret->u.fragment.tlb_wait_first_thrsw = false;
      ret->u.fragment.per_sample           = false;
      ret->u.fragment.reads_prim_id        = false;
#if V3D_VER_AT_LEAST(4,0,2,0)
      ret->u.fragment.reads_implicit_varys = false;
#endif
      ret->u.fragment.barrier = false;
      break;
   case SHADER_GEOMETRY:
      ret->u.geometry.prim_id_used   = false;
      ret->u.geometry.has_point_size = false;
      ret->u.geometry.output_words   = 0;
      break;
   case SHADER_TESS_EVALUATION:
      ret->u.tess_e.prim_id_used   = false;
      ret->u.tess_e.has_point_size = false;
      ret->u.tess_e.output_words   = 0;
      break;
   case SHADER_TESS_CONTROL:
      ret->u.tess_c.prim_id_used = false;
      ret->u.tess_c.barrier      = false;
      ret->u.tess_c.output_words = 0;
      ret->u.tess_c.output_words_patch = 0;
      break;
   case SHADER_VERTEX:
      for (int i=0; i<V3D_MAX_ATTR_ARRAYS; i++) ret->u.vertex.attribs.scalars_used[i] = 0;
      ret->u.vertex.attribs.vertexid_used     = false;
      ret->u.vertex.attribs.instanceid_used   = false;
      ret->u.vertex.attribs.baseinstance_used = false;
      ret->u.vertex.input_words     = 0;
      ret->u.vertex.output_words    = 0;
      ret->u.vertex.has_point_size  = false;
      ret->u.vertex.combined_seg_ok = true;
      break;
   default:
      unreachable();
      return NULL;
   }
   return ret;
}

struct find_attribs {
   ATTRIBS_USED_T *attr;
   const LinkMap *link;
};

static void dpostv_find_live_attribs(Dataflow *dataflow, void *data)
{
   struct find_attribs *d = data;
   ATTRIBS_USED_T *attr = d->attr;

   if (dataflow->flavour == DATAFLOW_IN) {
      int id = dataflow->u.linkable_value.row;
      assert(id < d->link->num_ins);
      int row = d->link->ins[id];
      assert(row < 4*V3D_MAX_ATTR_ARRAYS);

      /* This is attribute i, scalar j. We are using at least j scalars of i */
      int i = row / 4;
      int j = row % 4;
      attr->scalars_used[i] = gfx_umax(attr->scalars_used[i], j+1);
   } else if (dataflow->flavour == DATAFLOW_GET_VERTEX_ID)
      attr->vertexid_used = true;
   else if (dataflow->flavour == DATAFLOW_GET_INSTANCE_ID)
      attr->instanceid_used = true;
#if V3D_VER_AT_LEAST(4,0,2,0)
   else if (dataflow->flavour == DATAFLOW_GET_BASE_INSTANCE)
      attr->baseinstance_used = true;
#endif
}

static void reloc_visit_block(const CFGBlock *b, const bool *output_active, void *data, DataflowPostVisitor fn) {
   DataflowRelocVisitor pass;

   bool *temp = glsl_safemem_malloc(sizeof(*temp) * b->num_dataflow);
   glsl_dataflow_reloc_visitor_begin(&pass, b->dataflow, b->num_dataflow, temp);

   for (int i=0; i < b->num_outputs; i++) {
      if (output_active[i])
         glsl_dataflow_reloc_post_visitor(&pass, b->outputs[i], data, fn);
   }

   glsl_dataflow_reloc_visitor_end(&pass);
   glsl_safemem_free(temp);
}

static void get_attrib_info(const CFGBlock *b, ATTRIBS_USED_T *attr, const LinkMap *link, const bool *output_active)
{
   /* Calculate which attributes/vertex_id/instance_id are live */
   struct find_attribs data = { .attr = attr,
                                .link = link };

   attr->vertexid_used = false;
   attr->instanceid_used = false;
   attr->baseinstance_used = false;
   for (int i = 0; i < V3D_MAX_ATTR_ARRAYS; i++)
      attr->scalars_used[i] = 0;

   reloc_visit_block(b, output_active, &data, dpostv_find_live_attribs);
}

#if V3D_VER_AT_LEAST(4,0,2,0)
static void dpostv_find_implicit_vary(Dataflow *dataflow, void *data)
{
   bool *found = data;
   if (dataflow->flavour == DATAFLOW_GET_POINT_COORD_X ||
       dataflow->flavour == DATAFLOW_GET_POINT_COORD_Y ||
       dataflow->flavour == DATAFLOW_GET_LINE_COORD       )
   {
      *found = true;
   }
}

static bool reads_implicit_varys(const CFGBlock *b, const bool *output_active) {
   bool ret = false;
   reloc_visit_block(b, output_active, &ret, dpostv_find_implicit_vary);
   return ret;
}
#endif

static void fragment_backend_init(FragmentBackendState *s,
                                  uint32_t backend, bool early_fragment_tests
#if !V3D_HAS_RELAXED_THRSW
                                  , bool requires_sbwait
#endif
                                  )
{
   s->ms                       = !!(backend & GLSL_SAMPLE_MS);
   s->sample_alpha_to_coverage = s->ms && !!(backend & GLSL_SAMPLE_ALPHA);
#if !V3D_HAS_FEP_SAMPLE_MASK
   s->sample_mask              = s->ms && !!(backend & GLSL_SAMPLE_MASK);
#endif
   s->fez_safe_with_discard    = !!(backend & GLSL_FEZ_SAFE_WITH_DISCARD);
   s->early_fragment_tests     = early_fragment_tests;
#if !V3D_HAS_RELAXED_THRSW
   s->requires_sbwait          = requires_sbwait;
#endif

   for (int i=0; i<V3D_MAX_RENDER_TARGETS; i++) {
      uint32_t fb_gadget = glsl_unpack_fb_gadget(backend, i);
      /* For now, just say that int targets are never 16. TODO: Improve this in the backend */
      s->rt[i].is_16      = (fb_gadget & GLSL_FB_16) && !(fb_gadget & GLSL_FB_INT);
      s->rt[i].is_int     = (fb_gadget & GLSL_FB_INT);
      s->rt[i].is_present = (fb_gadget & GLSL_FB_PRESENT);
#if !V3D_VER_AT_LEAST(4,0,2,0)
      s->rt[i].alpha_16_workaround = (fb_gadget & GLSL_FB_ALPHA_16_WORKAROUND);
#endif
   }

   s->adv_blend = (backend & GLSL_ADV_BLEND_M) >> GLSL_ADV_BLEND_S;
}

static void vertex_backend_init(VertexBackendState *s, uint32_t backend, bool bin_mode, bool has_point_size) {
   s->bin_mode        = bin_mode;
   s->emit_point_size = ((backend & GLSL_PRIM_M) == GLSL_PRIM_POINT) && has_point_size;
   s->z_only_active   = !!(backend & GLSL_Z_ONLY_WRITE) && bin_mode;
}

static RegList *reg_list_prepend(RegList *p, Backflow *node, uint32_t reg) {
   RegList *new = malloc_fast(sizeof(RegList));
   new->node = node;
   new->reg = reg;
   new->next = p;
   return new;
}

struct active_finding {
   bool **output_translate;
   bool **output_reg;
   bool *next_blocks;
};

static void dfpv_outputs_active(Dataflow *d, void *data) {
   if (d->flavour != DATAFLOW_EXTERNAL) return;

   struct active_finding *c = data;
   if (!c->output_translate[d->u.external.block][d->u.external.output])
      c->next_blocks[d->u.external.block] = true;

   c->output_translate[d->u.external.block][d->u.external.output] = true;
   c->output_reg[d->u.external.block][d->u.external.output] = true;
}

static void block_transfer_output_active(CFGBlock *b, bool **output_translate, bool **output_reg, const IRShader *sh) {
   bool *next_blocks = glsl_safemem_calloc(sh->num_cfg_blocks, sizeof(bool));

   int block_id = b - sh->blocks;
   if (b->successor_condition != -1)
      output_translate[block_id][b->successor_condition] = true;

   bool *temp = glsl_safemem_malloc(b->num_dataflow * sizeof(bool));
   DataflowRelocVisitor dfv;
   glsl_dataflow_reloc_visitor_begin(&dfv, b->dataflow, b->num_dataflow, temp);
   struct active_finding data = { .output_translate = output_translate, .output_reg = output_reg, .next_blocks = next_blocks };

   for (int i=0; i<b->num_outputs; i++) {
      if (output_translate[block_id][i])
         glsl_dataflow_reloc_post_visitor(&dfv, b->outputs[i], &data, dfpv_outputs_active);
   }
   glsl_dataflow_reloc_visitor_end(&dfv);

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      if (next_blocks[i])
         block_transfer_output_active(&sh->blocks[i], output_translate, output_reg, sh);
   }

   glsl_safemem_free(temp);

   glsl_safemem_free(next_blocks);
}

static void get_outputs_active(const IRShader *sh, bool **output_translate, bool **output_reg, bool *shader_outputs_used) {
   for (int i=0; i<sh->num_outputs; i++) {
      if (sh->outputs[i].block != -1 && shader_outputs_used[i]) {
         output_translate[sh->outputs[i].block][sh->outputs[i].output] = true;
         output_reg[sh->outputs[i].block][sh->outputs[i].output] = true;
      }
   }
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      for (int j=0; j<sh->blocks[i].num_outputs; j++) {
         int out_idx = sh->blocks[i].outputs[j];
         if (glsl_dataflow_affects_memory(sh->blocks[i].dataflow[out_idx].flavour)) output_translate[i][j] = true;
      }
      block_transfer_output_active(&sh->blocks[i], output_translate, output_reg, sh);
   }
}

static inline void mark_used(bool *nodes_used, const LinkMap *l, int node) {
   if (l->outs[node] == -1) return;    /* Uninitialised anyway. No point in including */
   nodes_used[l->outs[node]] = true;
}

/* Return whether node_index is a xyzw/pointsize thing (hence always used) *
 * or if it's a varying return whether it's used in vary_map. Ensures we   *
 * only visit subtrees that we need.                                       */
static void get_nodes_used(const LinkMap *l, const GLSL_VARY_MAP_T *vary_map,
                           bool points, bool *nodes_used)
{
   /* x, y, z and w */
   for (int i=0; i<DF_VNODE_POINT_SIZE; i++) mark_used(nodes_used, l, i);
   if (points) mark_used(nodes_used, l, DF_VNODE_POINT_SIZE);
   /* varyings */
   if (vary_map != NULL)
      for (int i=0; i<vary_map->n; i++) mark_used(nodes_used, l, DF_VNODE_VARY(vary_map->entries[i]));
   else
      for (int i=0; i<V3D_MAX_VARYING_COMPONENTS; i++) mark_used(nodes_used, l, DF_VNODE_VARY(i));
}

enum postamble_type {
   POSTAMBLE_NONE                      = 0,
   POSTAMBLE_SIMPLE                    = 1,
#if !V3D_VER_AT_LEAST(3,3,0,0)
   POSTAMBLE_GFXH1181_NO_EXTRA_UNIFS   = 2,
   POSTAMBLE_GFXH1181_1_EXTRA_UNIF     = 3,
   POSTAMBLE_GFXH1181_2_EXTRA_UNIFS    = 4
#endif
};

struct postamble_info {
   int unif_count;
   int unif_patch_loc;
   bool patch_two_unifs;
   int unif_branch_from;

   int instr_count;
   int code_patch_loc;
   uint64_t code[9];
};

static const struct postamble_info postamble_infos[] = {
   /* POSTAMBLE_NONE */
   {
      .unif_count = 0,
      .instr_count = 0
   },

   /* POSTAMBLE_SIMPLE */
   {
      .unif_count = 1,
      .unif_patch_loc = 0,
      .patch_two_unifs = false,
      .unif_branch_from = 1,

      .instr_count = 4,
      .code_patch_loc = 0,
      .code = {
         0x020000060020d000ull,    // bu.anyap  0, r:unif
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull }   // nop
   },

#if !V3D_VER_AT_LEAST(3,3,0,0)
   /* GFXH-1181 workaround postambles.
    * First branch is a dummy branch -- rel instr/unif addr is 0, so it doesn't
    * branch anywhere.
    * Second branch is the real branch. */

   /* POSTAMBLE_GFXH1181_NO_EXTRA_UNIFS */
   {
      .unif_count = 3,
      .unif_patch_loc = 1,
      .patch_two_unifs = true,
      .unif_branch_from = 2,

      .instr_count = 9,
      .code_patch_loc = 4,
      .code = {
         0x020000060020d000ull,    // bu.anyap  0, r:unif
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x020000060020d000ull,    // bu.anyap  0, r:unif
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x3c403186bb800000ull }   // nop   ; ldunif
   },

   /* POSTAMBLE_GFXH1181_1_EXTRA_UNIF */
   {
      .unif_count = 4,
      .unif_patch_loc = 2,
      .patch_two_unifs = true,
      .unif_branch_from = 3,

      .instr_count = 9,
      .code_patch_loc = 4,
      .code = {
         0x020000060020d000ull,    // bu.anyap  0, r:unif
         0x3c403186bb800000ull,    // nop   ; ldunif
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x020000060020d000ull,    // bu.anyap  0, r:unif
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x3c403186bb800000ull }   // nop   ; ldunif
   },

   /* POSTAMBLE_GFXH1181_2_EXTRA_UNIFS */
   {
      .unif_count = 5,
      .unif_patch_loc = 3,
      .patch_two_unifs = true,
      .unif_branch_from = 4,

      .instr_count = 9,
      .code_patch_loc = 4,
      .code = {
         0x020000060020d000ull,    // bu.anyap  0, r:unif
         0x3c403186bb800000ull,    // nop   ; ldunif
         0x3c403186bb800000ull,    // nop   ; ldunif
         0x3c003186bb800000ull,    // nop
         0x020000060020d000ull,    // bu.anyap  0, r:unif
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x3c003186bb800000ull,    // nop
         0x3c403186bb800000ull }   // nop   ; ldunif
   }
#endif
};

typedef struct iu_sizes {
   size_t code;
   size_t unif;
} iu_sizes;

static iu_sizes copy_preamble(uint64_t *code, umap_entry *unif, ShaderFlavour flavour, bool bin_mode, unsigned threads,
                              bool tsy_barrier, bool barrier_first, bool has_compute_padding)
{
   iu_sizes s = { 0, 0 };

   if (tsy_barrier) {
      assert(flavour == SHADER_TESS_CONTROL || flavour == SHADER_FRAGMENT);
      assert(flavour == SHADER_TESS_CONTROL || !bin_mode);

      const inline_qasm* qasm;
      const inline_umap* umap;
      switch (flavour == SHADER_FRAGMENT ? 2 : (bin_mode ? 0 : 1))
      {
      case 0: qasm = &tcs_barrier_preamble_bin; umap = &tcs_barrier_preamble_bin_unif; break;
      case 1: qasm = &tcs_barrier_preamble_rdr; umap = &tcs_barrier_preamble_rdr_unif; break;
      case 2: qasm = &cs_barrier_preamble;      umap = &cs_barrier_preamble_unif; break;
      default: unreachable();
      }
      s.code = copy_inline_qasm_if(code, 0, qasm);
      s.unif = copy_inline_umap_if(unif, 0, umap);
   }

 #if V3D_USE_CSD
   assert(!barrier_first && !has_compute_padding);
 #else
   if (has_compute_padding) {
      assert(flavour == SHADER_FRAGMENT);
      if (tsy_barrier) {
         s.code -= cs_pad_setmsf_with_barriers_code_truncate;
         s.unif -= cs_pad_setmsf_with_barriers_unif_truncate;
         s.code = copy_inline_qasm_if(code, s.code, &cs_pad_setmsf_after_barrier_preamble);
         s.unif = copy_inline_umap_if(unif, s.unif, &cs_pad_setmsf_after_barrier_preamble_unif);
      } else {
         bool one_thread = !V3D_HAS_RELAXED_THRSW && threads == 1;
         s.code = copy_inline_qasm_if(code, s.code, one_thread ? &cs_pad_setmsf_unthreaded : &cs_pad_setmsf_threaded);
         s.unif = copy_inline_umap_if(unif, s.unif, &cs_pad_setmsf_unif);
      }
   }

   if (barrier_first) {
      assert(tsy_barrier);
      s.code = copy_inline_qasm_if(code, s.code, &cs_barrier);
      s.unif = copy_inline_umap_if(unif, s.unif, &cs_barrier_unif);
   }
#endif

   return s;
}

static iu_sizes copy_barrier(uint64_t *code, umap_entry *unif, ShaderFlavour flavour, bool bin_mode, bool barrier_mem_only, bool is_lthrsw_block)
{
   assert(flavour == SHADER_FRAGMENT || flavour == SHADER_TESS_CONTROL);
   assert(flavour == SHADER_TESS_CONTROL || !bin_mode);

   const inline_qasm* qasm = NULL;
   const inline_umap* umap = NULL;
   if (barrier_mem_only) {
      if (flavour == SHADER_TESS_CONTROL)
         qasm = &tcs_barrier_mem_only;
   } else {
      if (flavour == SHADER_FRAGMENT) {
         qasm = is_lthrsw_block ? &cs_barrier_lthrsw : &cs_barrier;
         umap = &cs_barrier_unif;
      }
      else {
         switch ((is_lthrsw_block ? 2 : 0) | (bin_mode ? 0 : 1))
         {
         case 0: qasm = &tcs_barrier_bin; umap = &tcs_barrier_bin_unif; break;
         case 1: qasm = &tcs_barrier_rdr; umap = &tcs_barrier_rdr_unif; break;
         case 2: qasm = &tcs_barrier_lthrsw_bin; umap = &tcs_barrier_lthrsw_bin_unif; break;
         case 3: qasm = &tcs_barrier_lthrsw_rdr; umap = &tcs_barrier_lthrsw_rdr_unif; break;
         default: unreachable();
         }
      }
   }

   iu_sizes s = {
      .code = copy_inline_qasm_if(code, 0, qasm),
      .unif = copy_inline_umap_if(unif, 0, umap),
   };
   return s;
}

static bool translate_per_quad(const Dataflow *d) {
   if (d->flavour == DATAFLOW_FDX || d->flavour == DATAFLOW_FDY)
      return true;
   else if (d->flavour == DATAFLOW_TEXTURE)
      return !glsl_dataflow_tex_cfg_implies_bslod(d->u.texture.bits);
   else
      return false;
}

struct pq {
   const Dataflow *arr;
   bool *per_quad;
   Dataflow **externals;
   unsigned external_count;
};

static void mark_deps_as_per_quad(uint32_t d_id, const Dataflow *arr, bool *per_quad) {
   if (d_id == -1) return;

   const Dataflow *d = &arr[d_id];

   if (per_quad[d->id]) return;
   per_quad[d->id] = true;

   for (int i=0; i<d->dependencies_count; i++) {
      if (d->d.reloc_deps[i] != -1)
         mark_deps_as_per_quad(d->d.reloc_deps[i], arr, per_quad);
   }
}

static void resolve_per_quadness(Dataflow *d, void *dat) {
   struct pq *data = dat;
   if (translate_per_quad(d) || data->per_quad[d->id]) {
      for (int i=0; i<d->dependencies_count; i++) {
         mark_deps_as_per_quad(d->d.reloc_deps[i], data->arr, data->per_quad);
      }
   }

   if (d->flavour == DATAFLOW_EXTERNAL) data->externals[data->external_count++] = d;
}

static void set_shader_per_quadness(const IRShader *sh, bool **per_quad, /* const */ bool **output_active) {
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      for (int j=0; j<sh->blocks[i].num_dataflow; j++) per_quad[i][j] = false;
   }

   bool *visit = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(bool));
   for (int i=0; i<sh->num_cfg_blocks; i++) visit[i] = true;
   bool again = true;

   while (again) {
      again = false;
      for (int i=0; i<sh->num_cfg_blocks; i++) {
         if (!visit[i]) continue;
         visit[i] = false;

         Dataflow **externals = glsl_safemem_malloc(sh->blocks[i].num_dataflow * sizeof(Dataflow *));
         struct pq df = { .arr = sh->blocks[i].dataflow, .per_quad = per_quad[i], .externals = externals, .external_count = 0 };

         reloc_visit_block(&sh->blocks[i], output_active[i], &df, resolve_per_quadness);

         for (unsigned j=0; j<df.external_count; j++) {
            if (per_quad[i][externals[j]->id]) {
               int block_id = externals[j]->u.external.block;
               const CFGBlock *b = &sh->blocks[block_id];
               const Dataflow *o = &b->dataflow[b->outputs[externals[j]->u.external.output]];
               if (!per_quad[block_id][o->id]) {
                  visit[block_id] = true;
                  again = true;
                  per_quad[block_id][o->id] = true;
               }

               /* If we need any output from a block per_quad then we need the branch condition
                * to be per_quad as well. */
               if (b->successor_condition != -1) {
                  const Dataflow *bc = &b->dataflow[b->outputs[b->successor_condition]];
                  if (!per_quad[block_id][bc->id]) {
                     per_quad[block_id][bc->id] = true;
                     visit[block_id] = true;
                     again = true;
                  }
               }
            }
         }

         glsl_safemem_free(externals);
      }
   }

   glsl_safemem_free(visit);
}

BINARY_SHADER_T *glsl_binary_shader_from_dataflow(ShaderFlavour            flavour,
                                                  bool                     bin_mode,
                                                  GLSL_VARY_MAP_T          *vary_map,
                                                  IR_PROGRAM_T             *ir,
                                                  const GLSL_BACKEND_CFG_T *key)
{
   BINARY_SHADER_T *binary = glsl_binary_shader_create(flavour);
   if(!binary) goto failed;

   /* If returning from a compile error, exit here */
   if (setjmp(g_ErrorHandlerEnv) != 0) {
      goto failed;
   }

   glsl_fastmem_init();

   const IRShader *sh = ir->stage[flavour].ir;
   const LinkMap  *l  = ir->stage[flavour].link_map;

   bool *shader_outputs_used = glsl_safemem_calloc(sh->num_outputs, sizeof(bool));
   switch (flavour) {
      case SHADER_FRAGMENT:         /* TODO: Could disable outputs for unwritten targets */
      case SHADER_TESS_CONTROL:     /* TODO: Currently assume all tessellation outputs are needed */
      case SHADER_GEOMETRY:         /* TODO: This doesn't work yet */
         for (int i=0; i<sh->num_outputs; i++) shader_outputs_used[i] = true;
         break;
      case SHADER_TESS_EVALUATION:
      case SHADER_VERTEX:
         get_nodes_used(l, vary_map, ((key->backend & GLSL_PRIM_M) == GLSL_PRIM_POINT), shader_outputs_used);
         break;
      default: unreachable();
   }

   bool **output_translate = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(bool *));
   bool **output_reg       = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(bool *));
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      output_translate[i] = glsl_safemem_calloc(sh->blocks[i].num_outputs, sizeof(bool));
      output_reg[i]       = glsl_safemem_calloc(sh->blocks[i].num_outputs, sizeof(bool));
   }

   get_outputs_active(sh, output_translate, output_reg, shader_outputs_used);

   const bool is_compute_shader = flavour == SHADER_FRAGMENT && ir->cs_shared_block_size != ~0u;
   const bool has_compute_padding = key->backend & GLSL_COMPUTE_PADDING;

   bool tsy_barrier = false;
   bool barrier_mem_only = (flavour == SHADER_TESS_CONTROL && ir->tess_vertices <= V3D_VPAR)
                        || (is_compute_shader && !glsl_wg_size_requires_barriers(ir->cs_wg_size));

   if (!barrier_mem_only) {
      for (int i=0; i<sh->num_cfg_blocks; i++) {
         if (sh->blocks[i].barrier) {
            tsy_barrier = true;
            break;
         }
      }
   }

   bool barrier_first = false;
#if !V3D_USE_CSD
   if (is_compute_shader) {
      if (ir->cs_shared_block_size != 0 && glsl_wg_size_requires_barriers(ir->cs_wg_size)) {
         tsy_barrier = true;
         barrier_first = true;
      }
      if (tsy_barrier || has_compute_padding)
         binary->u.fragment.tlb_wait_first_thrsw = true;
   }
#endif

   if (tsy_barrier) switch (flavour) {
      case SHADER_FRAGMENT:      binary->u.fragment.barrier = true; break;
      case SHADER_TESS_CONTROL:  binary->u.tess_c.barrier = true; break;
      default: unreachable();
   }

   SchedShaderInputs ins;
   switch (flavour) {
      case SHADER_FRAGMENT:
      {
         uint32_t prim_varys = key->backend & GLSL_PRIM_M;
#if V3D_VER_AT_LEAST(4,0,2,0)
         binary->u.fragment.reads_implicit_varys = prim_varys != 0 && reads_implicit_varys(&sh->blocks[0], output_translate[0]);
         if (!binary->u.fragment.reads_implicit_varys) prim_varys = 0;
#else
         if (tsy_barrier || has_compute_padding) prim_varys = 0; // varying consumed by barrier preamble
#endif
         fragment_shader_inputs(&ins, prim_varys, ir->varying);
         break;
      }
      case SHADER_VERTEX:
      {
         /* TODO: We assume that all attributes are loaded in the first block.
          * This should be true, for now. */
         ATTRIBS_USED_T *attr = &binary->u.vertex.attribs;
         get_attrib_info(&sh->blocks[0], attr, l, output_translate[0]);

         binary->u.vertex.input_words = vertex_shader_inputs(&ins, attr);
         binary->u.vertex.has_point_size = (((key->backend & GLSL_PRIM_M) == GLSL_PRIM_POINT) && l->outs[DF_VNODE_POINT_SIZE] != -1);
         break;
      }
      case SHADER_TESS_CONTROL:
         break;

      case SHADER_TESS_EVALUATION:
         binary->u.tess_e.has_point_size = (((key->backend & GLSL_PRIM_M) == GLSL_PRIM_POINT) && l->outs[DF_VNODE_POINT_SIZE] != -1);
         break;

      case SHADER_GEOMETRY:
         binary->u.geometry.has_point_size = false;
         break;

      default: unreachable();
   }

#ifdef KHRN_SHADER_DUMP_IR
   static int ir_file_no = 0;
   char ir_fname[1024];
   snprintf(ir_fname, 1024, "%s%d.ir", glsl_shader_flavour_name(flavour), ir_file_no++);
   glsl_ir_shader_to_file(sh, ir_fname);
#endif

   bool **per_quad = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(bool *));
   for (int i=0; i<sh->num_cfg_blocks; i++) per_quad[i] = glsl_safemem_malloc(sh->blocks[i].num_dataflow * sizeof(bool));

   set_shader_per_quadness(sh, per_quad, output_translate);

   SchedBlock **tblocks = malloc_fast(sh->num_cfg_blocks * sizeof(SchedBlock *));
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      tblocks[i] = translate_block(&sh->blocks[i], l, output_translate[i], (i==0) ? &ins : NULL, key, per_quad[i]);

      for (int j=0; j<sh->blocks[i].num_outputs; j++)
         if (!output_reg[i][j]) tblocks[i]->outputs[j] = NULL;
   }

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      glsl_safemem_free(output_translate[i]);
      glsl_safemem_free(output_reg[i]);
      glsl_safemem_free(per_quad[i]);
   }
   glsl_safemem_free(output_translate);
   glsl_safemem_free(output_reg);
   glsl_safemem_free(per_quad);

#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (flavour == SHADER_VERTEX) tblocks[0]->last_vpm_read = ins.read_dep;

   /* Only the final block may be per-sample. Blocks reading per-sample from  *
    * other blocks is not permitted.                                          */
   for (int i=0; i<sh->num_cfg_blocks; i++)
      assert(!tblocks[i]->per_sample || i == sh->num_cfg_blocks-1);
#endif

   bool *does_thrsw = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(bool));
   int max_threading = 4;
   bool has_thrsw = tsy_barrier;
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      int block_max_threads;
      does_thrsw[i] = get_max_threadability(tblocks[i]->tmu_lookups, &block_max_threads);

      if (sh->blocks[i].barrier && !barrier_mem_only)
         does_thrsw[i] = true;

      max_threading = gfx_smin(max_threading, block_max_threads);
      has_thrsw = has_thrsw || does_thrsw[i];
   }

   int lthrsw_block = -1;
   if (has_thrsw) lthrsw_block = glsl_find_lthrsw_block(sh->blocks, sh->num_cfg_blocks, does_thrsw);
   glsl_safemem_free(does_thrsw);

#if V3D_HAS_RELAXED_THRSW
   bool no_single_seg = is_compute_shader ? !V3D_USE_CSD : flavour == SHADER_FRAGMENT;
   if (!has_thrsw && no_single_seg) {
      lthrsw_block = 0;
      has_thrsw = true;
   }
#else
   if (!has_thrsw) max_threading = 1;
#endif

   int final_block_id = sh->num_cfg_blocks-1;

   bool shader_needs_tmuwt = false;
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      order_texture_lookups(tblocks[i]);

      Backflow *last_read = NULL, *last_write = NULL;
      for (struct tmu_lookup_s *l = tblocks[i]->tmu_lookups; l; l=l->next) {
         if (l->read_count == 0) {
            shader_needs_tmuwt = true;
            last_write = l->last_write;
         } else {
            last_read = l->last_read;
         }
      }

      if (i == final_block_id && shader_needs_tmuwt) {
         Backflow *tmuwt = glsl_backflow_tmuwt();
         glsl_iodep(tmuwt, last_write);
         last_write = tmuwt;
      }

      if (last_read != NULL)
         glsl_backflow_chain_push_back(&tblocks[i]->iodeps, last_read);
      if (last_write != NULL)
         glsl_backflow_chain_push_back(&tblocks[i]->iodeps, last_write);

      if (tblocks[i]->first_tlb_read) binary->u.fragment.tlb_wait_first_thrsw = true;
   }

   SchedBlock *final_block = tblocks[final_block_id];
   if (flavour == SHADER_FRAGMENT) {
      FragmentBackendState s;
#if V3D_HAS_RELAXED_THRSW
      fragment_backend_init(&s, key->backend, ir->early_fragment_tests);
#else
      bool sbwaited = tsy_barrier || has_compute_padding || (final_block_id != 0 && tblocks[0]->first_tlb_read);
      fragment_backend_init(&s, key->backend, ir->early_fragment_tests, !sbwaited);
#endif
      for (int i=0; i<sh->num_cfg_blocks; i++) binary->u.fragment.per_sample = binary->u.fragment.per_sample ||
                                                                               (s.ms && tblocks[i]->per_sample);
      glsl_fragment_backend(final_block, final_block_id, sh, l, &s, shader_outputs_used,
                            &binary->u.fragment.writes_z,
                            &binary->u.fragment.ez_disable);
   } else if (flavour == SHADER_VERTEX || flavour == SHADER_TESS_EVALUATION) {
      VertexBackendState s;
      vertex_backend_init(&s, key->backend, bin_mode, l->outs[DF_VNODE_POINT_SIZE] != -1);
      SchedShaderInputs *inputs = final_block_id == 0 ? &ins : NULL;
      glsl_vertex_backend(final_block, final_block_id, sh, l, inputs, &s, shader_outputs_used, vary_map);

      unsigned output_words = (s.bin_mode ? 6 : 4) + (s.emit_point_size ? 1 : 0) +
                              (s.z_only_active ? (s.emit_point_size ? 1 : 2) : 0) + vary_map->n;
      if (flavour == SHADER_VERTEX) binary->u.vertex.output_words = output_words;
      else                          binary->u.tess_e.output_words = output_words;
   }
   final_block->num_outputs = 0;
   final_block->outputs = NULL;

   glsl_safemem_free(shader_outputs_used);

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      glsl_combine_sched_nodes(tblocks[i]);
   }

   /* Sort out global register assignment */
   int **register_class = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(int *));
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      register_class[i] = glsl_safemem_malloc(sh->blocks[i].num_outputs * sizeof(int));
      for (int j=0; j<sh->blocks[i].num_outputs; j++) {
         register_class[i][j] = -1;
      }
   }

   /* TODO: Do we want to require that the exit is the last block and is unconditional? */
   assert(sh->blocks[final_block_id].next_if_true  == -1 &&
          sh->blocks[final_block_id].next_if_false == -1   );

   unsigned next_class = 0;
   /* Skip the last block because we don't need to keep those outputs */
   for (int i=0; i<sh->num_cfg_blocks-1; i++) {
      for (int j=0; j<sh->blocks[i].num_outputs; j++) {
         /* We're going to find a register class for block i, output j */
         /* Search all the phi-lists: */
         for (int k=0; k<sh->num_cfg_blocks; k++) {
            for (PhiList *p = tblocks[k]->phi_list; p; p=p->next) {
               /* If we can find a previous register use that register class */
               /* TODO: Here we cheat by assuming children are EXTERNALs, they could be PHIs */
               const Dataflow *phi = p->phi;
               const Dataflow *l = &sh->blocks[k].dataflow[phi->d.reloc_deps[0]];
               const Dataflow *r = &sh->blocks[k].dataflow[phi->d.reloc_deps[1]];
               assert(l->flavour == DATAFLOW_EXTERNAL && r->flavour == DATAFLOW_EXTERNAL);
               if (l->u.external.block == i && l->u.external.output == j) {
                  /* This is a phi with something. Is it earlier? */
                  if (r->u.external.block < i || (r->u.external.block == i && r->u.external.output < j)) {
                     /* Success! Take this register class */
                     int class = register_class[r->u.external.block][r->u.external.output];
                     if (register_class[i][j] != -1 ) {
                        int old_class = register_class[i][j];
                        for (int q = 0; q<sh->num_cfg_blocks; q++) {
                           for (int r = 0; r<sh->blocks[q].num_outputs; r++) {
                              if (register_class[q][r] == old_class) register_class[q][r] = class;
                           }
                        }
                     } else
                        register_class[i][j] = class;
                  }
               }
               if (r->u.external.block == i && r->u.external.output == j) {
                  /* This is a phi with something. Is it earlier? */
                  if (l->u.external.block < i || (l->u.external.block == i && l->u.external.output < j)) {
                     /* Success! Take this register class */
                     int class = register_class[l->u.external.block][l->u.external.output];
                     if (register_class[i][j] != -1 ) {
                        int old_class = register_class[i][j];
                        for (int q = 0; q<sh->num_cfg_blocks; q++) {
                           for (int r = 0; r<sh->blocks[q].num_outputs; r++) {
                              if (register_class[q][r] == old_class) register_class[q][r] = class;
                           }
                        }
                     } else
                        register_class[i][j] = class;
                  }
               }
            }
         }
         if (register_class[i][j] == -1) register_class[i][j] = next_class++;
      }
   }

   /* Compact the register set by removing the holes created above */
   int *register_mapping = glsl_safemem_malloc(next_class * sizeof(int));
   unsigned reg = 0;
   for (unsigned i=0; i<next_class; i++) {
      register_mapping[i] = -1;
   }
   for (int i=0; i<sh->num_cfg_blocks-1; i++) {
      for (int j=0; j<sh->blocks[i].num_outputs; j++) {
         if (tblocks[i]->outputs[j] == NULL) continue;
         if (register_mapping[register_class[i][j]] == -1) register_mapping[register_class[i][j]] = reg++;
      }
   }

   for (int i=0; i<sh->num_cfg_blocks-1; i++) {
      for (int j=0; j<sh->blocks[i].num_outputs; j++) {
         if (tblocks[i]->outputs[j] != NULL) {
            register_class[i][j] = register_mapping[register_class[i][j]];
            assert(register_class[i][j] != -1);
         } else
            register_class[i][j] = -1;
      }
   }
   next_class = reg;
   glsl_safemem_free(register_mapping);

   GENERATED_SHADER_T **backend_result = glsl_safemem_malloc(sh->num_cfg_blocks*sizeof(GENERATED_SHADER_T));
   int threads;
#if V3D_HAS_RELAXED_THRSW
   int min_threads = 2;
#else
   int min_threads = tsy_barrier ? 2 : 1;
#endif
   for (threads = max_threading; threads >= min_threads; threads /= 2) {
      bool good = true;

      if (next_class > get_max_regfile(threads)) continue;  /* Too many global registers */

      for (int i=0; i<sh->num_cfg_blocks; i++) {
         /* Put together a prescheduled list for this block */
         RegList *in  = NULL;
         RegList *out = NULL;
         for (ExternalList *el = tblocks[i]->external_list; el; el=el->next)
            in = reg_list_prepend(in, el->node, REG_RF(register_class[el->block][el->output]));

         /* The prescheduled values are only valid in the first block */
         if (i == 0 && flavour == SHADER_FRAGMENT) {
            if (ins.rf0) in = reg_list_prepend(in, ins.rf0, REG_RF(0));
            if (ins.rf1) in = reg_list_prepend(in, ins.rf1, REG_RF(1));
            if (ins.rf2) in = reg_list_prepend(in, ins.rf2, REG_RF(2));
         }

         for (int j=0; j<tblocks[i]->num_outputs; j++) {
            /* Blocks can have NULL outputs if they are not required TODO: This is a bit stupid */
            if (tblocks[i]->outputs[j] != NULL)
               out = reg_list_prepend(out, tblocks[i]->outputs[j], REG_RF(register_class[i][j]));
         }

         /* This is conservative but checking CFG paths is expensive and this catches common cases */
         bool sbwaited = tsy_barrier || has_compute_padding || (i != 0 && tblocks[0]->first_tlb_read);
         backend_result[i] = glsl_backend_schedule(tblocks[i], in, out, i != 0 ? next_class : 0,
                                                   flavour, bin_mode, threads, (i == final_block_id),
                                                   (i == lthrsw_block && !sh->blocks[i].barrier), sbwaited);
         if(!backend_result[i]) good = false;
      }

      if (good) break;

      for (int i=0; i<sh->num_cfg_blocks; i++) {
         if (backend_result[i]) {
            glsl_safemem_free(backend_result[i]->instructions);
            glsl_safemem_free(backend_result[i]->unifs);
            glsl_safemem_free(backend_result[i]);
         }
      }
   }
   if (threads < min_threads) goto failed;

   for (int i=0; i<sh->num_cfg_blocks; i++)
      glsl_safemem_free(register_class[i]);
   glsl_safemem_free(register_class);

   if(flavour == SHADER_FRAGMENT) {
      vary_map->n = backend_result[0]->varying_count;
      memcpy(vary_map->entries, backend_result[0]->varyings, backend_result[0]->varying_count*sizeof(uint32_t));
   }
   for (int i=1; i<sh->num_cfg_blocks; i++) assert(backend_result[i]->varying_count == 0);

   struct block_info {
      int unif_start;
      int code_start;
      bool invert_bcond;
      enum postamble_type postamble_type;
   } *bi = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(struct block_info));

   iu_sizes preamble_sizes = copy_preamble(NULL, NULL, flavour, bin_mode, threads, tsy_barrier, barrier_first, has_compute_padding);
   size_t instr_count = preamble_sizes.code;
   size_t unif_count  = preamble_sizes.unif;
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      bi[i].unif_start = unif_count;
      bi[i].code_start = instr_count;

      bi[i].invert_bcond = tblocks[i]->branch_invert ^ (sh->blocks[i].next_if_false != i+1);

      /* TODO GNL:
       * It is also possible that next_if_true and next_if_false are both != i+1. This
       * will only happen following CFG transformations. For now that is banned. */
      assert(sh->blocks[i].next_if_false == i+1 || sh->blocks[i].next_if_true == i+1 || sh->blocks[i].successor_condition == -1);

      if (sh->blocks[i].barrier) {
         iu_sizes barrier_sizes = copy_barrier(NULL, NULL, flavour, bin_mode, barrier_mem_only, i == lthrsw_block);
         instr_count += barrier_sizes.code;
         unif_count  += barrier_sizes.unif;
      }

      if (sh->blocks[i].successor_condition == -1 && (sh->blocks[i].next_if_false == i+1 || sh->blocks[i].next_if_false == -1))
         bi[i].postamble_type = POSTAMBLE_NONE;
      else {
#if V3D_VER_AT_LEAST(3,3,0,0)
         bi[i].postamble_type = POSTAMBLE_SIMPLE;
#else
         /* Must workaround GFXH-1181. May need extra unifs to ensure unif
          * stream address % 16 is 0 or 4 at the point the dummy branch
          * actually happens. */
         int unif_pos_after_dummy_branch_instr = bi[i].unif_start + backend_result[i]->unif_count + 1;
         if (unif_pos_after_dummy_branch_instr % 4 == 2)
            bi[i].postamble_type = POSTAMBLE_GFXH1181_2_EXTRA_UNIFS;
         else if (unif_pos_after_dummy_branch_instr % 4 == 3)
            bi[i].postamble_type = POSTAMBLE_GFXH1181_1_EXTRA_UNIF;
         else
            bi[i].postamble_type = POSTAMBLE_GFXH1181_NO_EXTRA_UNIFS;
#endif
      }

      unif_count  += backend_result[i]->unif_count + postamble_infos[bi[i].postamble_type].unif_count;
      instr_count += backend_result[i]->instruction_count + postamble_infos[bi[i].postamble_type].instr_count;
   }

   binary->unif = malloc(unif_count*sizeof(umap_entry));
   binary->code = malloc(instr_count*sizeof(uint64_t));
   if(binary->unif == NULL || binary->code == NULL)
      goto failed;

   binary->n_threads = threads;
#if V3D_HAS_RELAXED_THRSW
   binary->single_seg  = !has_thrsw;
   assert(!binary->single_seg || !no_single_seg);
#endif

#if !V3D_VER_AT_LEAST(3,3,0,0)
   binary->uses_control_flow = (sh->num_cfg_blocks != 1);
#endif

   if (preamble_sizes.code != 0)
      copy_preamble(binary->code, binary->unif, flavour, bin_mode, threads, tsy_barrier, barrier_first, has_compute_padding);

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      memcpy(binary->code + bi[i].code_start, backend_result[i]->instructions,
                  backend_result[i]->instruction_count*sizeof(uint64_t));

      memcpy(binary->unif + bi[i].unif_start, backend_result[i]->unifs,
                  backend_result[i]->unif_count*sizeof(umap_entry));

      int code_end = bi[i].code_start + backend_result[i]->instruction_count;
      int unif_end = bi[i].unif_start + backend_result[i]->unif_count;
      if (sh->blocks[i].barrier) {
         iu_sizes barrier_sizes = copy_barrier(binary->code + code_end,
                                               binary->unif + unif_end,
                                               flavour, bin_mode, barrier_mem_only, i == lthrsw_block);
         code_end += barrier_sizes.code;
         unif_end += barrier_sizes.unif;
      }

      if (bi[i].postamble_type != POSTAMBLE_NONE) {
         const struct postamble_info *p = &postamble_infos[bi[i].postamble_type];

         /* Location in each stream from which we will branch */
         int bu_loc = unif_end + p->unif_branch_from;
         int bi_loc = code_end + p->code_patch_loc + 4;
         /* Relative branch distance */
         int b_target = (sh->blocks[i].next_if_false != i+1) ? sh->blocks[i].next_if_false : sh->blocks[i].next_if_true;
         int bu_rel = -(bu_loc - bi[b_target].unif_start)*sizeof(uint32_t);
         int bi_rel = -(bi_loc - bi[b_target].code_start)*sizeof(uint64_t);

         /* Locations of the beginning of the postamble */
         umap_entry *unif_ptr = binary->unif + unif_end;
         uint64_t   *code_ptr = binary->code + code_end;
         for (int j=0; j<p->unif_count; j++) {
            unif_ptr[j].type  = BACKEND_UNIFORM_LITERAL;
            unif_ptr[j].value = 0;
         }
         unif_ptr[p->unif_patch_loc].value = bu_rel;
         if (p->patch_two_unifs)
            unif_ptr[p->unif_patch_loc+1].value = bu_rel;

         memcpy(code_ptr, p->code, p->instr_count * sizeof(uint64_t));
         struct v3d_qpu_instr in;
         v3d_qpu_instr_unpack(&in, code_ptr[p->code_patch_loc]);
         in.u.branch.cond   = sh->blocks[i].successor_condition == -1 ? V3D_QPU_BCOND_ALWAYS :
                                             (bi[i].invert_bcond ? V3D_QPU_BCOND_ALLNA : V3D_QPU_BCOND_ANYA);
         in.u.branch.i_addr = bi_rel;
         in.u.branch.msfign = tblocks[i]->branch_per_quad ? V3D_QPU_MSFIGN_QUAD : V3D_QPU_MSFIGN_PIXEL;
         v3d_qpu_instr_try_pack(&in, &code_ptr[p->code_patch_loc]);
      }
   }

   glsl_safemem_free(bi);

   binary->code_size  = instr_count * sizeof(uint64_t);
   binary->unif_count = unif_count;

#ifdef KHRN_SHADER_DUMP
   printf("%s%s:\n", glsl_shader_flavour_name(flavour), bin_mode ? " (bin)" : "");
   dqasmc_print_basic(binary->code, instr_count);
   printf("\n\n");
#endif

   glsl_fastmem_term();
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      glsl_safemem_free(backend_result[i]->instructions);
      glsl_safemem_free(backend_result[i]->unifs);
      glsl_safemem_free(backend_result[i]);
   }
   glsl_safemem_free(backend_result);
#ifndef NDEBUG
   glsl_safemem_verify();
#endif
   return binary;

 failed:
   glsl_binary_shader_free(binary);
   glsl_safemem_cleanup();
   return NULL;
}

void glsl_binary_shader_free(BINARY_SHADER_T *binary) {
   if(!binary) return;

   free(binary->code);
   free(binary->unif);
   free(binary);
}

const char *glsl_shader_flavour_name(ShaderFlavour f) {
   if      (f == SHADER_FRAGMENT)        return "fragment";
   else if (f == SHADER_VERTEX)          return "vertex";
   else if (f == SHADER_TESS_CONTROL)    return "tess_c";
   else if (f == SHADER_TESS_EVALUATION) return "tess_e";
   else if (f == SHADER_GEOMETRY)        return "geom";
   else /* SHADER_COMPUTE */             return "compute";
}
