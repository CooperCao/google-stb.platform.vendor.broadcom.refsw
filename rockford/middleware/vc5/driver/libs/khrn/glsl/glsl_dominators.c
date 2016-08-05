/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_basic_block.h"
#include "glsl_ir_shader.h"
#include "glsl_safemem.h"

#include <string.h>

typedef struct {
   int n;
   int *id;
} NodeSet;

/* A simple iterative algorithm for computing the dominance tree. See *
 * "A simple, fast dominance algorithm", Cooper et al.                */
static int intersect(const int *doms, int b1, int b2) {
   int f1 = b1;
   int f2 = b2;
   while (f1 != f2) {
      while (f1 > f2) f1 = doms[f1];
      while (f2 > f1) f2 = doms[f2];
   }
   return f1;
}

/* All nodes are identified by reverse-postorder number. pred is an array     *
 * which, when indexed by node ID gives it's predecessors. When done the doms *
 * array contains the idom for all nodes but the start node.                  */
static void dom_tree(int *doms, int n_blocks, const NodeSet *pred) {
   /* Mark all Doms as uninitialised */
   for (int i=0; i<n_blocks; i++) doms[i] = -1;
   /* Entry block has id n-1 and dominates itself */
   doms[0] = 0;

   bool changed = true;

   while (changed) {
      changed = false;
      /* Process the loop in reverse postorder, skipping the start node */
      for (int rpo_id = 1; rpo_id < n_blocks; rpo_id++) {
         const NodeSet *p = &pred[rpo_id];

         assert(p->n > 0);
         assert(p->id[0] != -1);

         int new_idom = p->id[0];
         for (int i=1; i<p->n; i++) {
            if (doms[p->id[i]] != -1) new_idom = intersect(doms, p->id[i], new_idom);
         }

         if (new_idom != doms[rpo_id]) {
            doms[rpo_id] = new_idom;
            changed = true;
         }
      }
   }
}

/* Using the doms form of the dominance tree, calculate dominance frontiers. *
 * Store as a flag, so df[i][j] is true iff j is in the dominance frontier   *
 * of i. Assumes that the array is initialised to false.                     */
static void dominance_frontiers(bool **df, int n_blocks, const int *doms, const NodeSet *pred) {
   for (int i=0; i<n_blocks; i++) {
      if (pred[i].n < 2) continue;  /* Nodes with 1 predecessor are never joins */

      for (int j=0; j<pred[i].n; j++) {
         int runner = pred[i].id[j];
         while (runner != doms[i]) {
            df[runner][i] = true;
            runner = doms[runner];
         }
      }
   }
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

static void update_phi_args(Map *phi_args, const Map *symbol_stacks) {
   for (MapNode *n = phi_args->head; n != NULL; n=n->next) {
      BasicBlockList **stack = glsl_map_get(symbol_stacks, n->k);
      glsl_basic_block_list_add(n->v, stack[0]->v);
   }
}

static void update_block_phi_args(BasicBlock **blocks, BasicBlock *b, Map **phi_args, const Map *symbol_stacks) {
   for (int i=0; ; i++) {
      if (blocks[i] == b) {
         update_phi_args(phi_args[i], symbol_stacks);
         break;
      }
   }
}

static void setup_phi_args(int block, BasicBlock **blocks, Map *symbol_stacks, const NodeSet *child, Map *final_values, Map **phi_args) {
   BasicBlock *b = blocks[block];
   /* For each symbol written in this block add the definition to the stack */
   for (MapNode *n = b->scalar_values->head; n != NULL; n=n->next) {
      BasicBlockList **stack = glsl_map_get(symbol_stacks, n->k);
      glsl_basic_block_list_add(stack, b);
   }

   /* In all successors, update the phi nodes with the definitions in the stacks */
   if (b->branch_target != NULL)      update_block_phi_args(blocks, b->branch_target,      phi_args, symbol_stacks);
   if (b->fallthrough_target != NULL) update_block_phi_args(blocks, b->fallthrough_target, phi_args, symbol_stacks);

   /* As a special addition, if this is the exit block, record the final definitions of symbols */
   /* TODO: Is this needed or could it be worked out simply afterwards? */
   if (b->branch_target == NULL && b->fallthrough_target == NULL) {
      for (MapNode *n = symbol_stacks->head; n != NULL; n=n->next) {
         BasicBlockList **stack = n->v;
         glsl_map_put(final_values, n->k, stack[0]->v);
      }
   }

   /* Recurse by visiting all the children in the dominance tree */
   for (int i=0; i<child[block].n; i++) {
      setup_phi_args(child[block].id[i], blocks, symbol_stacks, child, final_values, phi_args);
   }

   /* Pop all this block's definitions off the stacks */
   for (MapNode *n = b->scalar_values->head; n != NULL; n=n->next) {
      BasicBlockList **stack = glsl_map_get(symbol_stacks, n->k);
      glsl_basic_block_list_pop(stack);
   }
}

static NodeSet *alloc_nodeset(int n_blocks) {
   NodeSet *s = glsl_safemem_malloc(sizeof(NodeSet) * n_blocks);
   for (int i=0; i<n_blocks; i++) {
      s[i].n  = 0;
      s[i].id = glsl_safemem_malloc(sizeof(int) * n_blocks);
   }
   return s;
}

static void free_nodeset(NodeSet *s, int n_blocks) {
   for (int i=0; i<n_blocks; i++) glsl_safemem_free(s[i].id);
   glsl_safemem_free(s);
}

static Map *setup_ssa_information(BasicBlockList *l, int n_blocks, const int *doms, bool **df, Map **phi_args)
{
   /* Create a mapping from block_id to block pointer */
   BasicBlock **blocks = glsl_safemem_malloc(n_blocks * sizeof(BasicBlock *));
   int id = 0;
   for (BasicBlockList *b = l; b != NULL; b=b->next) {
      blocks[id] = b->v;
      id++;
   }

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
         BasicBlockList **args = malloc_fast(sizeof(BasicBlockList *));
         *args = NULL;
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

   /* Construct a version of the dominator tree that's good for walking */
   NodeSet *child = alloc_nodeset(n_blocks);
   for (int i=1; i<n_blocks; i++) {    /* Loop, skipping the start block */
      NodeSet *ch = &child[doms[i]];
      ch->id[ch->n++] = i;
   }

   setup_phi_args(0, blocks, symbol_stacks, child, final_values, phi_args);

   free_nodeset(child, n_blocks);
   glsl_safemem_free(stack);
   glsl_safemem_free(blocks);

   return final_values;
}

static int get_reverse_postorder_number(const BasicBlockList *rplist, BasicBlock *b) {
   int n = 0;
   while (rplist->v != b) {
      n++;
      rplist = rplist->next;
   }
   return n;
}

static void fill_pred_nodeset(const BasicBlockList *l, NodeSet *s) {
   int reverse_postorder_id = 0;
   for (const BasicBlockList *n = l; n != NULL; n = n->next) {
      if (n->v->branch_target != NULL) {
         NodeSet *p = &s[get_reverse_postorder_number(l, n->v->branch_target)];
         p->id[p->n++] = reverse_postorder_id;
      }
      if (n->v->fallthrough_target != NULL) {
         NodeSet *p = &s[get_reverse_postorder_number(l, n->v->fallthrough_target)];
         p->id[p->n++] = reverse_postorder_id;
      }
      reverse_postorder_id++;
   }
}

Map *glsl_construct_dominator_tree(BasicBlockList *l, Map **phi_args) {
   int n_blocks = glsl_basic_block_list_count(l);
   NodeSet *pred = alloc_nodeset(n_blocks);

   fill_pred_nodeset(l, pred);

   int *doms = glsl_safemem_malloc(n_blocks * sizeof(int));
   dom_tree(doms, n_blocks, pred);

   /* Store the dominance frontiers using a simple boolean grid. n^2 memory *
    * usage but it probably doesn't matter. Always time to optimise later.  */
   bool **df = glsl_safemem_malloc(n_blocks * sizeof(bool *));
   for (int i=0; i<n_blocks; i++) {
      df[i] = glsl_safemem_calloc(n_blocks, sizeof(bool));
   }

   dominance_frontiers(df, n_blocks, doms, pred);

   Map *symbol_final_values = setup_ssa_information(l, n_blocks, doms, df, phi_args);

   for (int i=0; i<n_blocks; i++) glsl_safemem_free(df[i]);
   glsl_safemem_free(df);
   glsl_safemem_free(doms);
   free_nodeset(pred, n_blocks);

   return symbol_final_values;
}

/* Serialise g in post-order into the node_ids array */
static void visit(int node, const NodeSet *g, int *node_ids, int *n_visited, bool *visited) {
   if (visited[node]) return;
   visited[node] = true;

   for (int i=0; i<g[node].n; i++)
      visit(g[node].id[i], g, node_ids, n_visited, visited);

   node_ids[(*n_visited)++] = node;
}

static int *alloc_postorder_numbering(const NodeSet *g, int n) {
   int *post_order_node_ids = glsl_safemem_malloc(n * sizeof(int));
   bool *visited = glsl_safemem_malloc(n * sizeof(bool));
   int n_visited = 0;
   for (int i=0; i<n; i++) visited[i] = false;
   visit(n-1, g, post_order_node_ids, &n_visited, visited);
   glsl_safemem_free(visited);
   return post_order_node_ids;
}

static int *alloc_reverse_postorder_numbering(const NodeSet *g, int n) {
   int *rpo_id = glsl_safemem_malloc(n * sizeof(int));
   int *po_ids = alloc_postorder_numbering(g, n);
   for (int i=0; i<n; i++) rpo_id[po_ids[i]] = n - 1 - i;
   glsl_safemem_free(po_ids);
   return rpo_id;
}

/* Return a new nodeset which is the same as 'g' where the ids have been  *
 * changed i --> id_mapping[i]                                            */
static NodeSet *renumber_nodeset(const NodeSet *g, int n, int *id_mapping) {
   NodeSet *new_g = alloc_nodeset(n);
   for (int i=0; i<n; i++) {
      int new_id = id_mapping[i];
      new_g[new_id].n = g[i].n;
      for (int j=0; j<g[i].n; j++) new_g[new_id].id[j] = id_mapping[g[i].id[j]];
   }
   return new_g;
}

/* From a graph whose nodes are in reverse postorder create the reverse graph */
static NodeSet *get_reverse_graph(const NodeSet *g, int n) {
   NodeSet *r_g = alloc_nodeset(n);
   for (int i=0; i<n; i++) {
      for (int j=0; j<g[i].n; j++) {
         NodeSet *p = &r_g[g[i].id[j]];
         /* This is guaranteed by the reverse postorder condition and is
          * required for the dominance algorithms */
         assert(p->n == 0 || p->id[p->n-1] < i);
         p->id[p->n++] = i;
      }
   }
   return r_g;
}

/* From the reverse CFG, work out which blocks are unconditional */
static void get_unconditional_blocks(const NodeSet *r_g_in, int n, bool *uncond) {
   /* The dominance algorithms require things to be in reverse postorder, so  *
    * renumber the input graph to meet that constraint                        */
   int *rpo_id = alloc_reverse_postorder_numbering(r_g_in, n);
   NodeSet *r_g = renumber_nodeset(r_g_in, n, rpo_id);

   /* Now reverse the nodeset so that each set tells us the predecessors of the node */
   NodeSet *pred = get_reverse_graph(r_g, n);
   free_nodeset(r_g, n);

   int *doms = glsl_safemem_malloc(n * sizeof(int));
   dom_tree(doms, n, pred);

   bool **df = glsl_safemem_malloc(n * sizeof(bool *));
   for (int i=0; i<n; i++) {
      df[i] = glsl_safemem_calloc(n, sizeof(bool));
   }
   dominance_frontiers(df, n, doms, pred);
   free_nodeset(pred, n);

   /* A node is unconditional if there are no nodes in its dominance frontier */
   /* Reorder the nodes back to the order the caller expects at the same time */
   for (int i=0; i<n; i++) {
      uncond[i] = true;
      for (int j=0; j<n; j++) {
         if (df[rpo_id[i]][j]) uncond[i] = false;
      }
   }

   glsl_safemem_free(rpo_id);
   for (int i=0; i<n; i++) glsl_safemem_free(df[i]);
   glsl_safemem_free(df);
   glsl_safemem_free(doms);
}

/* This requires the 'b' array to be indexed in reverse postorder */
int glsl_find_lthrsw_block(const CFGBlock *b, int n_blocks, bool *does_thrsw) {
   /* Build the reverse flow-graph by finding predecessors */
   NodeSet *r_g = alloc_nodeset(n_blocks);
   for (int i=0; i<n_blocks; i++) {
      if (b[i].next_if_true != -1) {
         NodeSet *p = &r_g[b[i].next_if_true];
         p->id[p->n++] = i;
      }
      if (b[i].next_if_false != -1) {
         NodeSet *p = &r_g[b[i].next_if_false];
         p->id[p->n++] = i;
      }
   }

   bool *uncond = glsl_safemem_malloc(n_blocks * sizeof(bool));
   get_unconditional_blocks(r_g, n_blocks, uncond);
   free_nodeset(r_g, n_blocks);

   /* Since the 'b' array is in reverse postorder we just need to find the first
    * unconditional block after the last block using a texture. The fact that we
    * require unconditional exit blocks means there must be one. */
   int i;
   for (i=n_blocks-1; i>=0; i--) {
      if (does_thrsw[i]) break;
   }
   for ( ; i < n_blocks; i++) {
      if (uncond[i]) break;
   }
   assert(i < n_blocks);
   glsl_safemem_free(uncond);
   return i;
}

void glsl_find_unconditional_blocks(const BasicBlockList *l, bool *uncond) {
   /* Build the reverse flow-graph by finding predecessors */
   int n_blocks = glsl_basic_block_list_count(l);
   NodeSet *r_g = alloc_nodeset(n_blocks);
   fill_pred_nodeset(l, r_g);

   get_unconditional_blocks(r_g, n_blocks, uncond);
   free_nodeset(r_g, n_blocks);
}

/* TODO: This is the most confusing of all possible naming schemes... */
static void append_merge_candidates(const NodeSet *dtree, int candidate, int *candidates, int *n_candidates) {
   for (int i=0; i<dtree[candidate].n; i++) {
      if (dtree[dtree[candidate].id[i]].n > 0)
         append_merge_candidates(dtree, dtree[candidate].id[i], candidates, n_candidates);
   }
   candidates[*n_candidates] = candidate;
   (*n_candidates)++;
}

static bool is_in_nodeset(int id, const NodeSet *set) {
   for (int i=0; i<set->n; i++)
      if (id == set->id[i]) return true;

   return false;
}

static bool branch_is_outward(int dest, int merge_head, const NodeSet *merge_children) {
   return !(dest == merge_head || is_in_nodeset(dest, merge_children));
}

static bool is_merge_exit(const NodeSet *s, int merge_head, const NodeSet *merge_children) {
   if (s->n == 0) return true;      /* This is the exit block */
   for (int i=0; i<s->n; i++)
      if (branch_is_outward(s->id[i], merge_head, merge_children)) return true;

   return false;
}

int identify_merge_exit(const NodeSet *child, int merge_head, const NodeSet *succ) {
   const NodeSet *merge_children = &child[merge_head];

   for (int i=0; i<merge_children->n; i++) {
      const NodeSet *s = &succ[merge_children->id[i]];
      if (is_merge_exit(s, merge_head, merge_children)) return merge_children->id[i];
   }

   /* If we get to here then the merge will fail. The exit must also come *
    * from the head node. Validation will later reject this               */
   assert(is_merge_exit(&succ[merge_head], merge_head, merge_children));
   return merge_head;
}

static void fill_succ_nodeset(const BasicBlockList *l, NodeSet *s) {
   int reverse_postorder_id = 0;
   for (const BasicBlockList *n = l; n != NULL; n = n->next) {
      NodeSet *p = &s[reverse_postorder_id];

      if (n->v->branch_target != NULL) {
         int id = get_reverse_postorder_number(l, n->v->branch_target);
         p->id[p->n++] = id;
      }
      if (n->v->fallthrough_target != NULL) {
         int id = get_reverse_postorder_number(l, n->v->fallthrough_target);
         p->id[p->n++] = id;
      }
      reverse_postorder_id++;
   }
}

static bool merge_valid_entry(int head, const NodeSet *merge, const NodeSet *pred) {
   for (int i=0; i<merge->n; i++) {
      int node = merge->id[i];
      /* If any predecessor is outside the merge then the entry is invalid */
      for (int j=0; j<pred[node].n; j++) {
         int p = pred[node].id[j];
         if (!is_in_nodeset(p, merge) && p != head) return false;
      }
   }
   return true;
}

static bool merge_valid_exit(int head, int exit, const NodeSet *merge, const NodeSet *succ) {
   if (is_merge_exit(&succ[head], head, merge)) return false;

   for (int i=0; i<merge->n; i++) {
      int node = merge->id[i];
      if (node != exit && is_merge_exit(&succ[node], head, merge)) return false;
   }

   return true;
}

static bool merge_valid_backedges(int head, const NodeSet *merge, const NodeSet *succ) {
   for (int i=0; i<merge->n; i++) {
      int node = merge->id[i];
      for (int j=0; j<succ[node].n; j++) {
         int succ_id = succ[node].id[j];
         /* Assume that the ids are in reverse-postorder, so a lower ID indicates a backedge */
         if (succ_id < node && (is_in_nodeset(succ_id, merge) || succ_id == head)) return false;
      }
   }
   return true;
}

bool glsl_ssa_block_flatten(const BasicBlockList *l, int *head, int *exit) {
   int n_blocks = glsl_basic_block_list_count(l);
   if (n_blocks == 1) return false;

   NodeSet *pred = alloc_nodeset(n_blocks);
   fill_pred_nodeset(l, pred);

   NodeSet *succ = alloc_nodeset(n_blocks);
   fill_succ_nodeset(l, succ);

   int *doms = glsl_safemem_malloc(n_blocks * sizeof(int));
   dom_tree(doms, n_blocks, pred);

   /* Construct a version of the dominator tree that's good for walking */
   NodeSet *child = alloc_nodeset(n_blocks);
   for (int i=1; i<n_blocks; i++) {    /* Loop, skipping the start block */
      NodeSet *ch = &child[doms[i]];
      ch->id[ch->n++] = i;
   }

   int *candidates = glsl_safemem_malloc(n_blocks * sizeof(int));
   int n_candidates = 0;

   bool valid = false;
   append_merge_candidates(child, 0, candidates, &n_candidates);
   for (int c=0; c<n_candidates; c++) {
      *head = candidates[c];
      *exit = identify_merge_exit(child, *head, succ);

      valid = merge_valid_entry(*head, &child[*head], pred)       &&
              merge_valid_exit(*head, *exit, &child[*head], succ) &&
              merge_valid_backedges(*head, &child[*head], succ);

      if (valid) break;
   }

   glsl_safemem_free(candidates);
   free_nodeset(child, n_blocks);
   glsl_safemem_free(doms);
   free_nodeset(succ, n_blocks);
   free_nodeset(pred, n_blocks);

   return valid;
}
