/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <setjmp.h>
#include <stdlib.h>
#include <assert.h>

#ifdef KHRN_SHADER_DUMP
#include "tools/v3d/dqasmc/dqasmc.h"
#endif

#include "glsl_backend.h"
#include "glsl_backend_uniforms.h"
#include "glsl_backflow.h"
#include "glsl_errors.h"
#include "glsl_binary_shader.h"
#include "glsl_dataflow_visitor.h"
#include "glsl_backflow_combine.h"
#include "glsl_dominators.h"

BINARY_SHADER_T *glsl_binary_shader_create(glsl_binary_shader_flavour_t flavour) {
   BINARY_SHADER_T *ret = malloc(sizeof(*ret));
   if(!ret) return NULL;

   ret->flavour            = flavour;
   ret->code               = NULL;
   ret->code_size          = 0;
   ret->unif               = NULL;
   ret->unif_count         = 0;
   ret->n_threads          = 0;

   switch(flavour) {
   case BINARY_SHADER_FRAGMENT:
      ret->u.fragment.discard  = false;
      ret->u.fragment.z_change = false;
      break;
   case BINARY_SHADER_VERTEX:
   case BINARY_SHADER_COORDINATE:
      {
         ret->u.vertex.reads_total = 0;
         for (int i=0; i<V3D_MAX_ATTR_ARRAYS; i++) ret->u.vertex.attribs.scalars_used[i] = 0;
         ret->u.vertex.attribs.vertexid_used = false;
         ret->u.vertex.attribs.instanceid_used = false;
      }
      break;
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
      assert(row < 4*GLXX_CONFIG_MAX_VERTEX_ATTRIBS);

      /* This is attribute i, scalar j. We are using at least j scalars of i */
      int i = row / 4;
      int j = row % 4;
      attr->scalars_used[i] = MAX(attr->scalars_used[i], j+1);
   } else if (dataflow->flavour == DATAFLOW_GET_VERTEX_ID)
      attr->vertexid_used = true;
   else if (dataflow->flavour == DATAFLOW_GET_INSTANCE_ID)
      attr->instanceid_used = true;
}

static bool get_attrib_info(const CFGBlock *b, ATTRIBS_USED_T *attr, const LinkMap *link, const bool *output_active)
{
   /* Calculate which attributes/vertex_id/instance_id are live */
   DataflowRelocVisitor pass;
   struct find_attribs data;

   bool *temp = glsl_safemem_malloc(sizeof(*temp) * b->num_dataflow);
   if(!temp) return false;

   data.attr = attr;
   data.link = link;

   attr->vertexid_used = false;
   attr->instanceid_used = false;
   for (int i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
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
   for (int i=0; i<sh->num_cfg_blocks; i++)
      block_transfer_output_active(&sh->blocks[i], output_active, sh);
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
   for (int i=0; i<vary_map->n; i++) mark_used(nodes_used, l, DF_VNODE_VARY(0) + vary_map->entries[i]);
}

enum postamble_type {
   POSTAMBLE_NONE                      = 0,
   POSTAMBLE_SIMPLE                    = 1,
   POSTAMBLE_GFXH1181_NO_EXTRA_UNIFS   = 2,
   POSTAMBLE_GFXH1181_1_EXTRA_UNIF     = 3,
   POSTAMBLE_GFXH1181_2_EXTRA_UNIFS    = 4
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
};

BINARY_SHADER_T *glsl_binary_shader_from_dataflow(glsl_binary_shader_flavour_t  flavour,
                                                  GLSL_VARY_MAP_T              *vary_map,
                                                  IR_PROGRAM_T                 *ir,
                                                  const GLXX_LINK_RESULT_KEY_T *key,
                                                  int                           v3d_version)
{
   SchedShaderInputs ins;

   BINARY_SHADER_T *binary = glsl_binary_shader_create(flavour);
   if(!binary) goto failed;

   /* If returning from a compile error, exit here */
   if (setjmp(g_ErrorHandlerEnv) != 0) {
      goto failed;
   }

   glsl_fastmem_init();

   const IRShader *sh = (flavour == BINARY_SHADER_FRAGMENT) ? ir->fshader : ir->vshader;
   const LinkMap  *l  = (flavour == BINARY_SHADER_FRAGMENT) ? ir->flink_map : ir->vlink_map;

   bool *shader_outputs_used = glsl_safemem_calloc(sh->num_outputs, sizeof(bool));
   if (flavour == BINARY_SHADER_FRAGMENT) {
      /* TODO: Could disable outputs for unwritten targets */
      for (int i=0; i<sh->num_outputs; i++) shader_outputs_used[i] = true;
   } else {
      get_nodes_used(l, vary_map, ((key->backend & GLXX_PRIM_M) == GLXX_PRIM_POINT), shader_outputs_used);
   }

   bool **output_active = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(bool *));
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      output_active[i] = glsl_safemem_calloc(sh->blocks[i].num_outputs, sizeof(bool));
   }

   get_outputs_active(sh, output_active, shader_outputs_used);

   if (flavour == BINARY_SHADER_FRAGMENT) {
      fragment_shader_inputs(&ins, key->backend & GLXX_PRIM_M, ir->varying);
   } else {
      /* TODO: We assume that all attributes are loaded in the first block.
       * This should be true, for now. */
      ATTRIBS_USED_T *attr = &binary->u.vertex.attribs;
      if (!get_attrib_info(&ir->vshader->blocks[0], attr, ir->vlink_map, output_active[0]))
         goto failed;

      vertex_shader_inputs(&ins, attr, &binary->u.vertex.reads_total);
   }

#ifdef KHRN_SHADER_DUMP_IR
   static int ir_file_no = 0;
   char ir_fname[1024];
   snprintf(ir_fname, 1024, "%c%d.ir", flavour == BINARY_SHADER_FRAGMENT ? 'f' : 'v', ir_file_no++);
   glsl_ir_shader_to_file(sh, ir_fname);
#endif

   SchedBlock **tblocks = malloc_fast(sh->num_cfg_blocks * sizeof(SchedBlock *));
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      tblocks[i] = translate_block(&sh->blocks[i], l, output_active[i], (i==0) ? &ins : NULL, key, v3d_version);
   }

   glsl_safemem_free(shader_outputs_used);
   for (int i=0; i<sh->num_cfg_blocks; i++) glsl_safemem_free(output_active[i]);
   glsl_safemem_free(output_active);

   int final_block_id = sh->num_cfg_blocks-1;
   SchedBlock *final_block = tblocks[final_block_id];
   if (flavour == BINARY_SHADER_FRAGMENT) {
      glsl_fragment_translate(final_block, final_block_id, sh, l,
                              key, &binary->u.fragment.discard,
                              &binary->u.fragment.z_change);
   } else {
      SchedShaderInputs *inputs = final_block_id == 0 ? &ins : NULL;
      glsl_vertex_translate(final_block, final_block_id, sh, l,
                            inputs, key, flavour, vary_map);
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

   int next_class = 0;
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
   int reg = 0;
   for (int i=0; i<next_class; i++) {
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

   bool *does_thrsw = glsl_safemem_malloc(sh->num_cfg_blocks * sizeof(bool));
   int max_threading = 1;
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      int block_max_threads = get_max_threadability(tblocks[i]->tmu_lookups);
      does_thrsw[i] = (block_max_threads != 1);
      if (max_threading == 1 && block_max_threads != 1) max_threading = block_max_threads;
      if (max_threading != 1 && block_max_threads != 1) max_threading = MIN(max_threading, block_max_threads);
   }

   int lthrsw_block = -1;
   if (max_threading > 1) {
      lthrsw_block = glsl_find_lthrsw_block(sh->blocks, sh->num_cfg_blocks, does_thrsw);

      if (get_max_threadability(tblocks[lthrsw_block]->tmu_lookups) == 1) {
         /* The lthrsw block is not allowed to threadswitch so add a fake TMU request */
         glsl_backflow_fake_tmu(tblocks[lthrsw_block]);
      }
   }
   glsl_safemem_free(does_thrsw);

   /* Only the final block may be per-sample. Blocks reading per-sample from  *
    * other blocks is not permitted.                                          */
   for (int i=0; i<sh->num_cfg_blocks; i++)
      assert(!tblocks[i]->per_sample || i == final_block_id);

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
   }

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      glsl_combine_sched_nodes(tblocks[i]);
   }

   GENERATED_SHADER_T **backend_result = glsl_safemem_malloc(sh->num_cfg_blocks*sizeof(GENERATED_SHADER_T));
   int threads;
   for (threads = max_threading; threads >= 1; threads /= 2) {
      bool good = true;

      if (next_class > REGFILE_MAX / threads) continue;  /* Too many global registers */

      for (int i=0; i<sh->num_cfg_blocks; i++) {
         /* Put together a prescheduled list for this block */
         RegList *in  = NULL;
         RegList *out = NULL;
         for (ExternalList *el = tblocks[i]->external_list; el; el=el->next)
            in = reg_list_prepend(in, el->node, REG_RF(register_class[el->block][el->output]));

         /* The prescheduled values are only valid in the first block */
         if (i == 0) {
            if (ins.w)   in = reg_list_prepend(in, ins.w,   REG_RF(0));
            if (ins.w_c) in = reg_list_prepend(in, ins.w_c, REG_RF(1));
            if (ins.z)   in = reg_list_prepend(in, ins.z,   REG_RF(2));
         }

         for (int j=0; j<tblocks[i]->num_outputs; j++) {
            /* Blocks can have NULL outputs if they are not required TODO: This is a bit stupid */
            if (tblocks[i]->outputs[j] != NULL)
               out = reg_list_prepend(out, tblocks[i]->outputs[j], REG_RF(register_class[i][j]));
         }

         backend_result[i] = glsl_backend_schedule(tblocks[i], in, out, i != 0 ? next_class : 0, flavour, threads, (i == final_block_id), (i == lthrsw_block));
         if(!backend_result[i]) good = false;
      }

      if (good) break;

      for (int i=0; i<sh->num_cfg_blocks; i++) glsl_safemem_free(backend_result[i]);
   }
   if (threads == 0) goto failed;

   if(flavour == BINARY_SHADER_FRAGMENT) {
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
   for (int i=0; i<sh->num_cfg_blocks; i++) {
      bi[i].unif_start = unif_size;
      bi[i].code_start = code_size;

      assert(sh->blocks[i].next_if_false == i+1 ||
             sh->blocks[i].next_if_false == -1    );

      if (sh->blocks[i].successor_condition == -1)
         bi[i].postamble_type = POSTAMBLE_NONE;
      else if (v3d_version < V3D_MAKE_VER(3,3)) {
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
      } else
         bi[i].postamble_type = POSTAMBLE_SIMPLE;

      unif_size += 8*(backend_result[i]->unif_count + postamble_infos[bi[i].postamble_type].unif_count);
      code_size += 8*(backend_result[i]->instruction_count + postamble_infos[bi[i].postamble_type].instr_count);
   }

   binary->unif               = malloc(unif_size);
   binary->code               = malloc(code_size);
   binary->n_threads          = threads;
   if(binary->unif == NULL || binary->code == NULL)
      goto failed;

   binary->uses_control_flow = (sh->num_cfg_blocks != 1);

   for (int i=0; i<sh->num_cfg_blocks; i++) {
      memcpy(binary->code + bi[i].code_start/4, backend_result[i]->instructions,
            8*backend_result[i]->instruction_count);

      memcpy(binary->unif + bi[i].unif_start/4, backend_result[i]->unifs,
            8*backend_result[i]->unif_count);

      if (bi[i].postamble_type != POSTAMBLE_NONE) {
         const struct postamble_info *p = &postamble_infos[bi[i].postamble_type];

         /* Location in each stream from which we will branch */
         int bu_loc = bi[i].unif_start + 8*(backend_result[i]->unif_count + p->unif_branch_from);
         int bi_loc = bi[i].code_start + 8*(backend_result[i]->instruction_count + p->code_patch_loc + 4);
         /* Relative branch distance */
         int b_target = sh->blocks[i].next_if_true;
         int bu_rel = -(bu_loc - bi[b_target].unif_start)/2;
         int bi_rel = -(bi_loc - bi[b_target].code_start);

         /* Locations of the beginning of the postamble */
         uint32_t *unif_ptr = binary->unif + bi[i].unif_start/4 + 2*backend_result[i]->unif_count;
         uint32_t *code_ptr = binary->code + bi[i].code_start/4 + 2*backend_result[i]->instruction_count;
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
   printf("%s:\n", glsl_binary_shader_flavour_name(flavour));
   dqasmc_print(stdout, (const uint64_t *)binary->code, binary->code_size / 8,
                NULL, NULL, NULL);
   printf("\n\n");
#endif

   glsl_fastmem_term();
   for (int i=0; i<sh->num_cfg_blocks; i++)
      glsl_safemem_free(backend_result[i]);
   glsl_safemem_free(backend_result);
#ifdef NDEBUG
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

const char *glsl_binary_shader_flavour_name(glsl_binary_shader_flavour_t b) {
   switch(b) {
   case BINARY_SHADER_FRAGMENT:
      return "fragment";
   case BINARY_SHADER_VERTEX:
      return "vertex";
   case BINARY_SHADER_COORDINATE:
      return "coordinate";
   }
   UNREACHABLE();
   return "unknown";
}
