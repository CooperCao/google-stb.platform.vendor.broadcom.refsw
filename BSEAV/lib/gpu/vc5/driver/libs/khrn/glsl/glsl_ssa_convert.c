/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_map.h"
#include "glsl_basic_block.h"
#include "glsl_ir_shader.h"
#include "glsl_symbols.h"
#include "glsl_shader_interfaces.h"
#include "glsl_dataflow_visitor.h"
#include "glsl_dominators.h"

/* TODO: This is duplicated here. Refactor so this makes more sense */
static Dataflow *dprev_apply_opt_map(Dataflow *d, void *data) {
   Map *opt_map = data;

   for (int i=0; i<d->dependencies_count; i++) {
      if (d->d.dependencies[i] != NULL) {
         Dataflow *new = glsl_map_get(opt_map, d->d.dependencies[i]);
         if (new != NULL)
            d->d.dependencies[i] = new;
      }
   }

   return d;
}


static bool symbol_really_written(BasicBlock *b, const Symbol *s) {
   Dataflow **written = glsl_map_get(b->scalar_values, s);
   Dataflow **initial = glsl_map_get(b->loads, s);
   if (!initial) return true;

   for (unsigned int i=0; i<s->type->scalar_count; i++) {
      if (written[i] != initial[i]) return true;
   }
   return false;
}

static void prune_fake_writes(BasicBlock **blocks, int n_blocks) {
   /* Scalar values is the set of all symbols touched in a block. Drop those
    * symbols that are only read from the map by replacing it                 */
   for (int i=0; i<n_blocks; i++) {
      Map *new_scv = glsl_map_new();
      for (MapNode *mn = blocks[i]->scalar_values->head; mn != NULL; mn = mn->next) {
         if (symbol_really_written(blocks[i], mn->k)) {
            glsl_map_put(new_scv, mn->k, mn->v);
         }
      }
      blocks[i]->scalar_values = new_scv;
   }
}

static Map *map_block_ids(const BasicBlockList *bl) {
   Map *block_ids = glsl_map_new();
   int block_id = 0;
   for (const BasicBlockList *b = bl; b != NULL; b=b->next) {
      int *id = malloc_fast(sizeof(int));
      *id = block_id++;
      glsl_map_put(block_ids, b->v, id);
   }
   return block_ids;
}

static int get_block_id(const Map *ids, const BasicBlock *b) {
   int *id_ptr = glsl_map_get(ids, b);
   return id_ptr[0];
}

static void get_block_info_outputs(BasicBlock **blocks, int n_blocks, Map *block_ids, SSABlock *ssa_blocks, Map **output_maps)
{
   /* Go through the blocks sorting out outputs */
   for (int i=0; i<n_blocks; i++) {
      BasicBlock *b     = blocks[i];
      SSABlock   *b_out = &ssa_blocks[i];
      b_out->id = i;

      output_maps[b_out->id] = glsl_map_new();
      Map *output_map = output_maps[b_out->id];

      int count = 0;
      for(MapNode *out_node = b->scalar_values->head; out_node != NULL; out_node = out_node->next) {
         const Symbol *s = out_node->k;
         count += s->type->scalar_count;
      }
      if (b->memory_head != NULL) count++;
      if (b->branch_cond != NULL) count++;

      b_out->n_outputs = count;
      b_out->outputs = malloc_fast(count * sizeof(Dataflow *));

      int output_idx = 0;
      for (MapNode *out_node = b->scalar_values->head; out_node != NULL; out_node = out_node->next) {
         const Symbol *s = out_node->k;
         Dataflow **df = out_node->v;

         IROutput *o = malloc_fast(sizeof(IROutput) * s->type->scalar_count);
         for (unsigned i=0; i<s->type->scalar_count; i++) {
            b_out->outputs[output_idx] = df[i];

            /* Add the outputs to the output map */
            o[i].block = b_out->id;
            o[i].output = output_idx++;
         }
         glsl_map_put(output_map, s, o);
      }
      if (b->memory_head != NULL) b_out->outputs[output_idx++] = b->memory_head;
      if (b->branch_cond != NULL) b_out->outputs[output_idx++] = b->branch_cond;
      assert(output_idx == count);

      b_out->successor_condition = (b->branch_cond != NULL) ? output_idx - 1 : -1;
      b_out->next_true  = -1;
      b_out->next_false = -1;

      /* Remove loop condition from the end if it matches an earlier entry */
      if (b->branch_cond) {
         for (int i=0; i<b_out->n_outputs-1; i++) {
            if (b_out->outputs[i] == b_out->outputs[b_out->successor_condition]) {
               b_out->successor_condition = i;
               b_out->n_outputs--;
               break;
            }
         }
      }

      if (b->branch_target != NULL)
         b_out->next_true = get_block_id(block_ids, b->branch_target);
      if (b->fallthrough_target != NULL)
         b_out->next_false = get_block_id(block_ids, b->fallthrough_target);

      b_out->barrier = b->barrier;
   }
}

static void normalise_ir_format(BasicBlock **blocks, int n_blocks, Map *block_ids, SSABlock *ssa_blocks, Map **output_maps, Map *symbol_ids, Map **phi_args)
{
   for (int id=0; id<n_blocks; id++) {
      BasicBlock *b = blocks[id];

      Map *load_map = glsl_map_new();

      /* Create phis and external symbols for the loads */
      for (MapNode *load_node = b->loads->head; load_node; load_node = load_node->next) {
         const Symbol *s = load_node->k;
         Dataflow   **df = load_node->v;

         int *ids = glsl_map_get(symbol_ids, s);
         if (ids != NULL && (s->flavour == SYMBOL_INTERFACE_BLOCK || (s->flavour == SYMBOL_VAR_INSTANCE && s->u.var_instance.storage_qual == STORAGE_UNIFORM)))
         {
            Dataflow **v;
            if (s->flavour == SYMBOL_VAR_INSTANCE || s->u.interface_block.sq == STORAGE_UNIFORM)
               v = glsl_shader_interface_create_uniform_dataflow(s, ids);
            else
               v = glsl_shader_interface_create_buffer_dataflow(s, ids);

            for (unsigned i=0; i<s->type->scalar_count; i++) glsl_map_put(load_map, df[i], v[i]);
            continue;
         }

         for (unsigned i=0; i<s->type->scalar_count; i++) {
            /* phi_args contains, in principle, the blocks that have reaching
             * definitions of this symbol                                     */
            int incoming_id = -1;
            Dataflow *ext_res = NULL;

            BasicBlockList **phi_arg_val = glsl_map_get(phi_args[id], s);
            for (BasicBlockList *r_b = phi_arg_val[0], *i_b = phi_arg_val[1]; r_b; r_b=r_b->next, i_b = i_b->next)
            {
               Dataflow **reaching_def_values = glsl_map_get(r_b->v->scalar_values, s);
               int block_id = get_block_id(block_ids, r_b->v);
               Map *output_map = output_maps[block_id];
               IROutput *o = glsl_map_get(output_map, s);
               assert(o != NULL && o[i].block != -1 && o[i].output != -1);

               Dataflow *external = glsl_dataflow_construct_external(reaching_def_values[i]->type);
               external->u.external.block = o[i].block;
               external->u.external.output = o[i].output;

               int incoming_block_id = get_block_id(block_ids, i_b->v);

               if (ext_res == NULL) {
                  ext_res = external;
                  incoming_id = incoming_block_id;
               } else {
                  assert(ext_res->flavour == DATAFLOW_EXTERNAL || ext_res->flavour == DATAFLOW_PHI);
                  if (ext_res->flavour == DATAFLOW_EXTERNAL) {
                     /* TODO: We screwed up the creation of the phi_args so that every symbol that is ever
                      * read where there are multiple entry paths gets a phi, just sometimes with all the same value.
                      * This is obviously appaling. Just don't create a phi to work around the issue. */
                     if (o[i].block != ext_res->u.external.block || o[i].output != ext_res->u.external.output)
                        ext_res = glsl_dataflow_construct_phi(ext_res, incoming_id, external, incoming_block_id);
                  } else {
                     ext_res = glsl_dataflow_construct_phi(external, incoming_block_id, ext_res, -1);
                  }
               }
            }
            /* TODO: This is not the right way to solve this problem. We spuriously create
             * loads in the entry block which need to be ignored so silently drop them here.
             * The presence of DATAFLOW_LOAD in the graph later will cause enough problems. */
            if(ext_res != NULL)
               glsl_map_put(load_map, df[i], ext_res);
         }
      }

      SSABlock *bbc = &ssa_blocks[id];

      /* Substitute proper dataflow for the ridiculous LOAD constructs */
      glsl_dataflow_visit_array(bbc->outputs, 0, bbc->n_outputs, load_map, dprev_apply_opt_map, NULL);
      for (int i=0; i<bbc->n_outputs; i++) {
         if (bbc->outputs[i]->flavour == DATAFLOW_LOAD)
            bbc->outputs[i] = glsl_map_get(load_map, bbc->outputs[i]);
      }
   }
}

static void fill_ir_outputs(const SymbolList *outs, Map *symbol_final_values, Map *block_ids, Map **output_maps, Map *symbol_ids, IROutput **ir_outs, int *n_outputs) {
   int max_id = 0;
   for (SymbolListNode *n = outs->head; n != NULL; n=n->next) {
      int *ids = glsl_map_get(symbol_ids, n->s);
      if (ids == NULL) continue;

      for (unsigned i=0; i<n->s->type->scalar_count; i++) {
         if (max_id < ids[i]) max_id = ids[i];
      }
   }

   IROutput *ir_outputs = glsl_safemem_malloc( (max_id + 1) * sizeof(IROutput));
   for (int i=0; i<=max_id; i++) {
      /* Initially all outputs are uninitialised */
      ir_outputs[i].block  = -1;
      ir_outputs[i].output = -1;
   }

   for (SymbolListNode *n = outs->head; n != NULL; n=n->next) {
      int *ids = glsl_map_get(symbol_ids, n->s);
      BasicBlock *b = glsl_map_get(symbol_final_values, n->s);
      if (b == NULL) {
         /* Mark this as uninitialised in the symbol map. TODO: Possibly not needed? */
         for (unsigned i=0; i<n->s->type->scalar_count; i++) ids[i] = -1;
         continue;
      }

      int block_id = get_block_id(block_ids, b);
      Map *output_map = output_maps[block_id];
      IROutput *o = glsl_map_get(output_map, n->s);
      for (unsigned i=0; i<n->s->type->scalar_count; i++) {
         ir_outputs[ids[i]] = o[i];
      }
   }

   *ir_outs = ir_outputs;
   *n_outputs = max_id + 1;
}

/* Place phis in the program according to the dominance frontiers.          *
 * This uses the algorithm from figure 11 of Cytron et al, "Efficiently     *
 * computing static single assignment form and the control dependence graph */
static void place_phis(BasicBlock **blocks, int n_blocks, const Map *symbols, bool **df)
{
   int *work_list = glsl_safemem_malloc(n_blocks * sizeof(int));
   int work_depth = 0;
   bool *work        = glsl_safemem_malloc(n_blocks * sizeof(bool));
   bool *has_already = glsl_safemem_malloc(n_blocks * sizeof(bool));

   /* For each variable V assigned in the program: */
   for (MapNode *n = symbols->head; n != NULL; n=n->next) {
      memset(has_already, 0, n_blocks * sizeof(bool));
      memset(work,        0, n_blocks * sizeof(bool));

      /* Add all blocks that assign to V to the worklist */
      for (int i=0; i<n_blocks; i++) {
         const BasicBlock *b = blocks[i];
         if (glsl_map_get(b->scalar_values, n->k) != NULL) {
            /* This block assigns to this variable. Add to the work list */
            work_list[work_depth++] = i;
         }
      }

      while (work_depth > 0) {
         /* Pop the last item off the work list */
         int work_item = work_list[work_depth-1];
         work_depth--;

         for (int j=0; j<n_blocks; j++) {
            if (df[work_item][j] && !has_already[j]) {
               /* Create LOADs here so that the phi will be created properly */
               glsl_basic_block_get_scalar_values(blocks[j], n->k);

               has_already[j] = true;
               if (!work[j]) {
                  work[j] = true;
                  work_list[work_depth++] = j;
               }
            }
         }
      }
   }

   glsl_safemem_free(work_list);
   glsl_safemem_free(work);
   glsl_safemem_free(has_already);
}

static void update_phi_args(Map *phi_args, BasicBlock *from, const Map *symbol_stacks) {
   for (MapNode *n = phi_args->head; n != NULL; n=n->next) {
      BasicBlockList **stack = glsl_map_get(symbol_stacks, n->k);
      BasicBlockList **args = n->v;
      glsl_basic_block_list_add(&args[0], stack[0]->v);
      glsl_basic_block_list_add(&args[1], from);
   }
}

static int hacky_get_block_id(BasicBlock **blocks, BasicBlock *b) {
   for (int i=0; ; i++) {
      if (blocks[i] == b) return i;
   }
   unreachable();
}

void setup_phi_args(int block, BasicBlock **blocks, int n_blocks, Map *symbol_stacks, const int *idom, Map *final_values, Map **phi_args)
{
   BasicBlock *b = blocks[block];
   /* For each symbol written in this block add the definition to the stack */
   for (MapNode *n = b->scalar_values->head; n != NULL; n=n->next) {
      BasicBlockList **stack = glsl_map_get(symbol_stacks, n->k);
      glsl_basic_block_list_add(stack, b);
   }

   /* In all successors, update the phi nodes with the definitions in the stacks */
   if (b->branch_target != NULL)      update_phi_args(phi_args[hacky_get_block_id(blocks, b->branch_target)],      b, symbol_stacks);
   if (b->fallthrough_target != NULL) update_phi_args(phi_args[hacky_get_block_id(blocks, b->fallthrough_target)], b, symbol_stacks);

   /* As a special addition, if this is the exit block, record the final definitions of symbols */
   /* TODO: Is this needed or could it be worked out simply afterwards? */
   if (b->branch_target == NULL && b->fallthrough_target == NULL) {
      for (MapNode *n = symbol_stacks->head; n != NULL; n=n->next) {
         BasicBlockList **stack = n->v;
         glsl_map_put(final_values, n->k, stack[0]->v);
      }
   }

   /* Recurse by visiting all the children in the dominance tree */
   /* (Start at 1 because we know 0 is the root) */
   for (int i=1; i<n_blocks; i++) {
      if (idom[i] == block)
         setup_phi_args(i, blocks, n_blocks, symbol_stacks, idom, final_values, phi_args);
   }

   /* Pop all this block's definitions off the stacks */
   for (MapNode *n = b->scalar_values->head; n != NULL; n=n->next) {
      BasicBlockList **stack = glsl_map_get(symbol_stacks, n->k);
      glsl_basic_block_list_pop(stack);
   }
}

static Map *setup_ssa_information(BasicBlockList *l, BasicBlock **blocks, int n_blocks, Map **phi_args)
{
   struct abstract_cfg *cfg = glsl_alloc_abstract_cfg(l);

   int *doms = glsl_alloc_idoms(cfg);
   bool **df = glsl_dom_frontiers_alloc(cfg, doms);

   /* Gather the complete set of variables written by the program */
   /* TODO: Use of glsl_map here is unnecessary (we really want a set) */
   Map *symbols = glsl_map_new();
   for (int i=0; i<n_blocks; i++) {
      for (MapNode *mn = blocks[i]->scalar_values->head; mn != NULL; mn = mn->next) {
         const Dataflow **scv = glsl_map_get(symbols, mn->k);
         if (scv == NULL) glsl_map_put(symbols, mn->k, mn->v);
      }
   }

   place_phis(blocks, n_blocks, symbols, df);

   for (int i=0; i<n_blocks; i++) {
      phi_args[i] = glsl_map_new();
      for (MapNode *n = blocks[i]->loads->head; n != NULL; n=n->next) {
         BasicBlockList **args = malloc_fast(2*sizeof(BasicBlockList *));
         args[0] = NULL;
         args[1] = NULL;
         glsl_map_put(phi_args[i], n->k, args);
      }
   }

   /* Initialise a definition stack per symbol, initially empty */
   Map *symbol_stacks = glsl_map_new();
   BasicBlockList **stack = glsl_safemem_malloc(symbols->count * sizeof(BasicBlockList *));
   int s_id = 0;
   for (MapNode *n = symbols->head; n != NULL; n=n->next) {
      stack[s_id] = NULL;
      glsl_map_put(symbol_stacks, n->k, &stack[s_id]);
      s_id++;
   }

   Map *final_values = glsl_map_new();

   setup_phi_args(0, blocks, n_blocks, symbol_stacks, doms, final_values, phi_args);

   glsl_safemem_free(stack);

   glsl_dom_frontiers_free(df, n_blocks);
   glsl_safemem_free(doms);
   glsl_free_abstract_cfg(cfg);

   return final_values;
}

void glsl_ssa_convert(SSAShader *sh, BasicBlock *entry_block, const SymbolList *outs, Map *symbol_ids)
{
   BasicBlockList *bl = glsl_basic_block_get_reverse_postorder_list(entry_block);

   sh->n_blocks = glsl_basic_block_list_count(bl);
   sh->blocks   = glsl_safemem_malloc(sh->n_blocks * sizeof(SSABlock));

   Map **output_maps = glsl_safemem_malloc(sh->n_blocks * sizeof(Map *));
   Map **phi_args    = glsl_safemem_malloc(sh->n_blocks * sizeof(Map *));

   /* Create a mapping from block_id to block pointer */
   BasicBlock **blocks = glsl_safemem_malloc(sh->n_blocks * sizeof(BasicBlock *));
   int id = 0;
   for (BasicBlockList *b = bl; b != NULL; b=b->next) {
      blocks[id] = b->v;
      id++;
   }

   prune_fake_writes(blocks, sh->n_blocks);

   /* TODO: Passing the list in here as well is really a bit odd */
   Map *symbol_final_values = setup_ssa_information(bl, blocks, sh->n_blocks, phi_args);

   Map *block_ids = map_block_ids(bl);
   get_block_info_outputs(blocks, sh->n_blocks, block_ids, sh->blocks, output_maps);
   normalise_ir_format(blocks, sh->n_blocks, block_ids, sh->blocks, output_maps, symbol_ids, phi_args);

   fill_ir_outputs(outs, symbol_final_values, block_ids, output_maps, symbol_ids, &sh->outputs, &sh->n_outputs);

   glsl_safemem_free(blocks);
   glsl_safemem_free(phi_args);
   glsl_safemem_free(output_maps);
}
