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
#if V3D_HAS_RELAXED_THRSW
   ret->four_thread = true;
   ret->single_seg  = true;
#else
   ret->n_threads  = 0;
#endif

   switch(flavour) {
   case SHADER_FRAGMENT:
      ret->u.fragment.writes_z             = false;
      ret->u.fragment.ez_disable           = false;
      ret->u.fragment.tlb_wait_first_thrsw = false;
      ret->u.fragment.per_sample           = false;
      ret->u.fragment.reads_prim_id        = false;
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

static bool get_attrib_info(const CFGBlock *b, ATTRIBS_USED_T *attr, const LinkMap *link, const bool *output_active)
{
   /* Calculate which attributes/vertex_id/instance_id are live */
   DataflowRelocVisitor pass;

   bool *temp = glsl_safemem_malloc(sizeof(*temp) * b->num_dataflow);
   if(!temp) return false;

   struct find_attribs data = { .attr = attr,
                                .link = link };

   attr->vertexid_used = false;
   attr->instanceid_used = false;
   attr->baseinstance_used = false;
   for (int i = 0; i < V3D_MAX_ATTR_ARRAYS; i++)
      attr->scalars_used[i] = 0;

   glsl_dataflow_reloc_visitor_begin(&pass, b->dataflow, b->num_dataflow, temp);

   for (int i=0; i < b->num_outputs; i++) {
      if (output_active[i])
         glsl_dataflow_reloc_post_visitor(&pass, b->outputs[i],
                                          &data, dpostv_find_live_attribs);
   }

   glsl_dataflow_reloc_visitor_end(&pass);

   glsl_safemem_free(temp);

   return true;
}

static void fragment_backend_init(FragmentBackendState *s,
                                  uint32_t backend, bool early_fragment_tests
#if !V3D_HAS_RELAXED_THRSW
                                  , bool requires_sbwait
#endif
                                  )
{
   s->ms                       = !!(backend & GLSL_SAMPLE_MS);
   s->sample_alpha_to_coverage = s->ms && !!(backend & GLSL_SAMPLE_ALPHA);
   s->sample_mask              = s->ms && !!(backend & GLSL_SAMPLE_MASK);
   s->fez_safe_with_discard    = !!(backend & GLSL_FEZ_SAFE_WITH_DISCARD);
   s->early_fragment_tests     = early_fragment_tests;
#if !V3D_HAS_RELAXED_THRSW
   s->requires_sbwait          = requires_sbwait;
#endif

   for (int i=0; i<V3D_MAX_RENDER_TARGETS; i++) {
      int shift = GLSL_FB_GADGET_S + 3*i;
      uint32_t mask = GLSL_FB_GADGET_M << shift;
      int fb_gadget = (backend & mask) >> shift;
      s->rt[i].type = fb_gadget & 0x3;
      s->rt[i].alpha_16_workaround = (fb_gadget & GLSL_FB_ALPHA_16_WORKAROUND);
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
   bool **output_active;
   bool *next_blocks;
};

static void dfpv_outputs_active(Dataflow *d, void *data) {
   if (d->flavour != DATAFLOW_EXTERNAL) return;

   struct active_finding *c = data;
   if (!c->output_active[d->u.external.block][d->u.external.output]) {
      c->output_active[d->u.external.block][d->u.external.output] = true;
      c->next_blocks[d->u.external.block] = true;
   }
}

static void block_transfer_output_active(CFGBlock *b, bool **output_active, const IRShader *sh) {
   bool *next_blocks = glsl_safemem_calloc(sh->num_cfg_blocks, sizeof(bool));

   int block_id = b - sh->blocks;
   if (b->successor_condition != -1) output_active[block_id][b->successor_condition] = true;

   bool *temp = glsl_safemem_malloc(b->num_dataflow * sizeof(bool));
   DataflowRelocVisitor dfv;
   glsl_dataflow_reloc_visitor_begin(&dfv, b->dataflow, b->num_dataflow, temp);
   struct active_finding data;
   data.output_active = output_active;
   data.next_blocks = next_blocks;
   for (int i=0; i<b->num_outputs; i++) {
      if (output_active[block_id][i])
         glsl_dataflow_reloc_post_visitor(&dfv, b->outputs[i], &data, dfpv_outputs_active);
   }
   glsl_dataflow_reloc_visitor_end(&dfv);

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      if (next_blocks[i])
         block_transfer_output_active(&sh->blocks[i], output_active, sh);
   }

   glsl_safemem_free(temp);

   glsl_safemem_free(next_blocks);
}

static void get_outputs_active(const IRShader *sh, bool **output_active, bool *shader_outputs_used) {
   for (int i=0; i<sh->num_outputs; i++) {
      if (sh->outputs[i].block != -1 && shader_outputs_used[i])
         output_active[sh->outputs[i].block][sh->outputs[i].output] = true;
   }
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      for (int j=0; j<sh->blocks[i].num_outputs; j++) {
         int out_idx = sh->blocks[i].outputs[j];
         if (glsl_dataflow_affects_memory(sh->blocks[i].dataflow[out_idx].flavour)) output_active[i][j] = true;
      }
      block_transfer_output_active(&sh->blocks[i], output_active, sh);
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
   uint32_t code[18];
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
         0x0020d000, 0x02000006,    // bu.anyap  0, r:unif
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186 }   // nop
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
         0x0020d000, 0x02000006,    // bu.anyap  0, r:unif
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0x0020d000, 0x02000006,    // bu.anyap  0, r:unif
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c403186 }   // nop   ; ldunif
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
         0x0020d000, 0x02000006,    // bu.anyap  0, r:unif
         0xbb800000, 0x3c403186,    // nop   ; ldunif
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0x0020d000, 0x02000006,    // bu.anyap  0, r:unif
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c403186 }   // nop   ; ldunif
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
         0x0020d000, 0x02000006,    // bu.anyap  0, r:unif
         0xbb800000, 0x3c403186,    // nop   ; ldunif
         0xbb800000, 0x3c403186,    // nop   ; ldunif
         0xbb800000, 0x3c003186,    // nop
         0x0020d000, 0x02000006,    // bu.anyap  0, r:unif
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c003186,    // nop
         0xbb800000, 0x3c403186 }   // nop   ; ldunif
   }
#endif
};

/* Uniforms: quorum, op_set_quorum */
static const uint32_t barrier_preamble[] = {
#if V3D_HAS_RELAXED_THRSW
   0xbb80f000, 0x3c602183, // ycd rf3              ; nop          ; thrsw ; ldunif
   0xbbf42000, 0x3c005106, // eidx.pushz -         ; mov rf4, r5
   0x7d83e0c1, 0x3dea2184, // shr.ifna rf4, rf3, 1
   0xb6836100, 0x3c003191, // mov syncu, rf4
#else
   0xbb00f000, 0x08646003, // ycd rf3              ; sub.pushz r0, r0, r0 ; thrsw ; ldunif
   0xbbf42000, 0x3c005106, // eidx.pushz -         ; mov rf4, r5
   0x7de3e0c1, 0x3dfca304, // shr.ifna rf4, rf3, 1 ; mov.ifnb tmua, r0
   0xbb800000, 0x3c803186, // nop                                         ; ldtmu
   0xb6836100, 0x3c003191, // mov syncu, rf4
#endif
};

/* Uniform op_wait_inc_check */
static const uint32_t barrier_code[] = {
   0xbb80f000, 0x3c203180, // ycd r0 ; nop ; thrsw
   0x7d838001, 0x3de03191, // shr syncu, r0, 1
   0xbb800000, 0x3c003186, // nop
};

static const uint32_t barrier_code_lthrsw[] = {
   0xbb80f000, 0x3c203180, // ycd r0 ; nop ; thrsw
   0xbb800000, 0x3c203186, // nop    ; nop ; thrsw
   0x7d838001, 0x3de03191, // shr syncu, r0, 1
};

#if V3D_VER_AT_LEAST(4,0,2,0)
static inline uint32_t gfxh1370_tsy_op(v3d_tsy_op_t op) { return op; }
#else
/* Work around GFXH-1370 by setting the high 3 bits of TSY configs. */
static inline uint32_t gfxh1370_tsy_op(v3d_tsy_op_t op) { return 0xE0 | op; }
#endif

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

   bool **output_active = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(bool *));
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      output_active[i] = glsl_safemem_calloc(sh->blocks[i].num_outputs, sizeof(bool));
   }

   get_outputs_active(sh, output_active, shader_outputs_used);

   SchedShaderInputs ins;
   switch (flavour) {
      case SHADER_FRAGMENT:
         fragment_shader_inputs(&ins, key->backend & GLSL_PRIM_M, ir->varying);
         break;
      case SHADER_VERTEX:
      {
         /* TODO: We assume that all attributes are loaded in the first block.
          * This should be true, for now. */
         ATTRIBS_USED_T *attr = &binary->u.vertex.attribs;
         if (!get_attrib_info(&sh->blocks[0], attr, l, output_active[0]))
            goto failed;

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

   SchedBlock **tblocks = malloc_fast(sh->num_cfg_blocks * sizeof(SchedBlock *));
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      tblocks[i] = translate_block(&sh->blocks[i], l, output_active[i], (i==0) ? &ins : NULL, key);
   }

   glsl_safemem_free(shader_outputs_used);
   for (int i=0; i<sh->num_cfg_blocks; i++) glsl_safemem_free(output_active[i]);
   glsl_safemem_free(output_active);

#if !V3D_VER_AT_LEAST(4,0,2,0)
   if (flavour == SHADER_VERTEX) tblocks[0]->last_vpm_read = ins.read_dep;

   /* Only the final block may be per-sample. Blocks reading per-sample from  *
    * other blocks is not permitted.                                          */
   for (int i=0; i<sh->num_cfg_blocks; i++)
      assert(!tblocks[i]->per_sample || i == sh->num_cfg_blocks-1);
#endif

   bool *does_thrsw = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(bool));
   int max_threading = 4;
   bool has_thrsw = false;
   bool uses_barrier = false;
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      int block_max_threads;
      does_thrsw[i] = get_max_threadability(tblocks[i]->tmu_lookups, &block_max_threads);

      if (sh->blocks[i].barrier) {
         uses_barrier = true;
         does_thrsw[i] = true;
      }

      max_threading = gfx_smin(max_threading, block_max_threads);
      has_thrsw = has_thrsw || does_thrsw[i];
   }
   if (uses_barrier) {
      if (flavour == SHADER_FRAGMENT)     binary->u.fragment.tlb_wait_first_thrsw = true;
      if (flavour == SHADER_TESS_CONTROL) binary->u.tess_c.barrier = true;
   }

   int lthrsw_block = -1;
   if (has_thrsw) lthrsw_block = glsl_find_lthrsw_block(sh->blocks, sh->num_cfg_blocks, does_thrsw);
   glsl_safemem_free(does_thrsw);

#if V3D_HAS_RELAXED_THRSW
   if (!has_thrsw && flavour == SHADER_FRAGMENT) {
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
         glsl_backflow_chain_append(&tblocks[i]->iodeps, last_read);
      if (last_write != NULL)
         glsl_backflow_chain_append(&tblocks[i]->iodeps, last_write);

      if (tblocks[i]->first_tlb_read) binary->u.fragment.tlb_wait_first_thrsw = true;
   }

   SchedBlock *final_block = tblocks[final_block_id];
   if (flavour == SHADER_FRAGMENT) {
      FragmentBackendState s;
#if V3D_HAS_RELAXED_THRSW
      fragment_backend_init(&s, key->backend, ir->early_fragment_tests);
#else
      bool sbwaited = uses_barrier || (final_block_id != 0 && tblocks[0]->first_tlb_read);
      fragment_backend_init(&s, key->backend, ir->early_fragment_tests, !sbwaited);
#endif
      for (int i=0; i<sh->num_cfg_blocks; i++) binary->u.fragment.per_sample = binary->u.fragment.per_sample ||
                                                                               (s.ms && tblocks[i]->per_sample);
      glsl_fragment_backend(final_block, final_block_id, sh, l, &s,
                            &binary->u.fragment.writes_z,
                            &binary->u.fragment.ez_disable);
   } else if (flavour == SHADER_VERTEX || flavour == SHADER_TESS_EVALUATION) {
      VertexBackendState s;
      vertex_backend_init(&s, key->backend, bin_mode, l->outs[DF_VNODE_POINT_SIZE] != -1);
      SchedShaderInputs *inputs = final_block_id == 0 ? &ins : NULL;
      glsl_vertex_backend(final_block, final_block_id, sh, l, inputs, &s, vary_map);

      unsigned output_words = (s.bin_mode ? 6 : 4) + (s.emit_point_size ? 1 : 0) +
                              (s.z_only_active ? (s.emit_point_size ? 1 : 2) : 0) + vary_map->n;
      if (flavour == SHADER_VERTEX) binary->u.vertex.output_words = output_words;
      else                          binary->u.tess_e.output_words = output_words;
   }
   final_block->num_outputs = 0;
   final_block->outputs = NULL;

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      glsl_combine_sched_nodes(tblocks[i]);
   }

   /* Sort out global register assignment */
   int **register_class = malloc_fast(sh->num_cfg_blocks * sizeof(int *));
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      register_class[i] = malloc_fast(sh->blocks[i].num_outputs * sizeof(int));
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
   int *register_mapping = malloc_fast(next_class * sizeof(int));
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

   GENERATED_SHADER_T **backend_result = glsl_safemem_malloc(sh->num_cfg_blocks*sizeof(GENERATED_SHADER_T));
   int threads;
#if V3D_HAS_RELAXED_THRSW
   int min_threads = 2;
#else
   int min_threads = uses_barrier ? 2 : 1;
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
            if (ins.w)   in = reg_list_prepend(in, ins.w,   REG_RF(0));
            if (ins.w_c) in = reg_list_prepend(in, ins.w_c, REG_RF(1));
            if (ins.z)   in = reg_list_prepend(in, ins.z,   REG_RF(2));
         }

         for (int j=0; j<tblocks[i]->num_outputs; j++) {
            /* Blocks can have NULL outputs if they are not required TODO: This is a bit stupid */
            if (tblocks[i]->outputs[j] != NULL)
               out = reg_list_prepend(out, tblocks[i]->outputs[j], REG_RF(register_class[i][j]));
         }

         /* This is conservative but checking CFG paths is expensive and this catches common cases */
         bool sbwaited = uses_barrier || (i != 0 && tblocks[0]->first_tlb_read);
         backend_result[i] = glsl_backend_schedule(tblocks[i], in, out, i != 0 ? next_class : 0,
                                                   flavour, bin_mode, threads, (i == final_block_id),
                                                   (i == lthrsw_block && !sh->blocks[i].barrier), sbwaited);
         if(!backend_result[i]) good = false;
      }

      if (good) break;

      for (int i=0; i<sh->num_cfg_blocks; i++) glsl_safemem_free(backend_result[i]);
   }
   if (threads < min_threads) goto failed;

   if(flavour == SHADER_FRAGMENT) {
      vary_map->n = backend_result[0]->varying_count;
      memcpy(vary_map->entries, backend_result[0]->varyings, 4 * backend_result[0]->varying_count);
   }
   for (int i=1; i<sh->num_cfg_blocks; i++) assert(backend_result[i]->varying_count == 0);

   struct block_info {
      int unif_start;
      int code_start;
      enum postamble_type postamble_type;
   } *bi = malloc_fast(sh->num_cfg_blocks * sizeof(struct block_info));

   int unif_size = 0;
   int code_size = 0;
   if (uses_barrier) {
      code_size += sizeof(barrier_preamble);
      unif_size += 8*2;
   }

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      bi[i].unif_start = unif_size;
      bi[i].code_start = code_size;

      assert(sh->blocks[i].next_if_false == i+1 ||
             sh->blocks[i].next_if_false == -1    );

      if (sh->blocks[i].barrier) {
         /* TODO: It would be less code to get this right than to assert the hack is not wrong ... */
         assert(sizeof(barrier_code) == sizeof(barrier_code_lthrsw));
         code_size += sizeof(barrier_code);
         unif_size += 8;
      }

      if (sh->blocks[i].successor_condition == -1)
         bi[i].postamble_type = POSTAMBLE_NONE;
      else {
#if V3D_VER_AT_LEAST(3,3,0,0)
         bi[i].postamble_type = POSTAMBLE_SIMPLE;
#else
         /* Must workaround GFXH-1181. May need extra unifs to ensure unif
          * stream address % 16 is 0 or 4 at the point the dummy branch
          * actually happens. */
         int unif_pos_after_dummy_branch_instr = (bi[i].unif_start + 8*(backend_result[i]->unif_count + 1)) / 2;
         if (unif_pos_after_dummy_branch_instr % 16 == 8)
            bi[i].postamble_type = POSTAMBLE_GFXH1181_2_EXTRA_UNIFS;
         else if (unif_pos_after_dummy_branch_instr % 16 == 12)
            bi[i].postamble_type = POSTAMBLE_GFXH1181_1_EXTRA_UNIF;
         else
            bi[i].postamble_type = POSTAMBLE_GFXH1181_NO_EXTRA_UNIFS;
#endif
      }

      unif_size += 8*(backend_result[i]->unif_count + postamble_infos[bi[i].postamble_type].unif_count);
      code_size += 8*(backend_result[i]->instruction_count + postamble_infos[bi[i].postamble_type].instr_count);
   }

   binary->unif = malloc(unif_size);
   binary->code = malloc(code_size);
   if(binary->unif == NULL || binary->code == NULL)
      goto failed;

#if V3D_HAS_RELAXED_THRSW
   binary->four_thread = (threads == 4);
   binary->single_seg  = !has_thrsw;
   assert(!binary->single_seg || flavour != SHADER_FRAGMENT);
#else
   binary->n_threads = threads;
#endif

#if !V3D_VER_AT_LEAST(3,3,0,0)
   binary->uses_control_flow = (sh->num_cfg_blocks != 1);
#endif

   if (uses_barrier) {
      memcpy(binary->code, barrier_preamble, sizeof(barrier_preamble));
      binary->unif[0] = BACKEND_UNIFORM_SPECIAL;
      binary->unif[1] = BACKEND_SPECIAL_UNIFORM_QUORUM;
      binary->unif[2] = BACKEND_UNIFORM_LITERAL;
      binary->unif[3] = 0xFFFFFF00 | gfxh1370_tsy_op(V3D_TSY_OP_SET_QUORUM);
   }

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      memcpy(binary->code + bi[i].code_start/4, backend_result[i]->instructions,
            8*backend_result[i]->instruction_count);

      memcpy(binary->unif + bi[i].unif_start/4, backend_result[i]->unifs,
            8*backend_result[i]->unif_count);

      int code_end = bi[i].code_start + 8*backend_result[i]->instruction_count;
      int unif_end = bi[i].unif_start + 8*backend_result[i]->unif_count;
      if (sh->blocks[i].barrier) {
         memcpy(binary->code + code_end/4, i == lthrsw_block ? barrier_code_lthrsw : barrier_code, sizeof(barrier_code));
         code_end += sizeof(barrier_code);
         /* NOTE: GFXH-1370 workaround, see above */
         binary->unif[unif_end/4] = BACKEND_UNIFORM_LITERAL;
         binary->unif[unif_end/4+1] = 0xFFFFFF00 | gfxh1370_tsy_op(V3D_TSY_OP_WAIT_INC_CHECK);
         unif_end += 8;
      }

      if (bi[i].postamble_type != POSTAMBLE_NONE) {
         const struct postamble_info *p = &postamble_infos[bi[i].postamble_type];

         /* Location in each stream from which we will branch */
         int bu_loc = unif_end + 8*p->unif_branch_from;
         int bi_loc = code_end + 8*(p->code_patch_loc + 4);
         /* Relative branch distance */
         int b_target = sh->blocks[i].next_if_true;
         int bu_rel = -(bu_loc - bi[b_target].unif_start)/2;
         int bi_rel = -(bi_loc - bi[b_target].code_start);

         /* Locations of the beginning of the postamble */
         uint32_t *unif_ptr = binary->unif + unif_end/4;
         uint32_t *code_ptr = binary->code + code_end/4;
         for (int j=0; j<p->unif_count; j++) {
            unif_ptr[2*j] = BACKEND_UNIFORM_LITERAL;
            unif_ptr[2*j + 1] = 0;
         }
         unif_ptr[2*p->unif_patch_loc + 1] = bu_rel;
         if (p->patch_two_unifs)
            unif_ptr[2*p->unif_patch_loc + 3] = bu_rel;

         memcpy(code_ptr, p->code, 8*p->instr_count);
         code_ptr[2*p->code_patch_loc]     |= (bi_rel >> 24) << 24;
         code_ptr[2*p->code_patch_loc + 1] |= ((bi_rel >> 3) & 0x1FFFFF) << 3;
      }
   }

   binary->code_size  = code_size;
   binary->unif_count = unif_size / 8;

#ifdef KHRN_SHADER_DUMP
   printf("%s%s:\n", glsl_shader_flavour_name(flavour), bin_mode ? " (bin)" : "");
   dqasmc_print(stdout, (const uint64_t *)binary->code, binary->code_size / 8,
                NULL, NULL, NULL);
   printf("\n\n");
#endif

   glsl_fastmem_term();
   for (int i=0; i<sh->num_cfg_blocks; i++)
      glsl_safemem_free(backend_result[i]);
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
