/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "glsl_dominators.h"
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
static void dom_tree(int *doms, const NodeSet *pred, int n) {
   /* Mark all Doms as uninitialised */
   for (int i=0; i<n; i++) doms[i] = -1;
   /* Entry block has id n-1 and dominates itself */
   doms[0] = 0;

   bool changed;
   do {
      changed = false;
      /* Process the loop in reverse postorder, skipping the start node */
      for (int rpo_id = 1; rpo_id < n; rpo_id++) {
         const NodeSet *p = &pred[rpo_id];

         assert(p->n > 0);
         assert(p->id[0] != -1);

         int i = 0;
         /* Try the first processed predecessor as the new idom */
         while (doms[p->id[i]] == -1) i++;
         int new_idom = p->id[i];

         /* Starting from the next predecessor intersect the sets */
         for (i++; i<p->n; i++) {
            if (doms[p->id[i]] != -1) new_idom = intersect(doms, p->id[i], new_idom);
         }

         if (new_idom != doms[rpo_id]) {
            doms[rpo_id] = new_idom;
            changed = true;
         }
      }
   } while (changed);
}

/* Using the doms form of the dominance tree, calculate dominance frontiers. *
 * Store as a flag, so df[i][j] is true iff j is in the dominance frontier   *
 * of i. Assumes that the array is initialised to false.                     */
static void dominance_frontiers(bool **df, const int *doms, const NodeSet *pred, int n) {
   for (int i=0; i<n; i++) {
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

/* Return whether b dominates a */
static bool dominates(const int *idoms, int a, int b) {
   assert(idoms[0] == 0);
   if      (a == 0) return false;
   else if (a == b) return true;
   else             return dominates(idoms, idoms[a], b);
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

static int get_reverse_postorder_number(const BasicBlockList *rplist, BasicBlock *b) {
   int n = 0;
   while (rplist->v != b) {
      n++;
      rplist = rplist->next;
   }
   return n;
}

/******************************************************************************/
/* Higher level functions for working with abstract_cfg handles               */

struct abstract_cfg {
   int      n;
   NodeSet *pred;
   NodeSet *succ;
};

static void nodeset_add(NodeSet *s, int n) {
   s->id[s->n++] = n;
}

struct abstract_cfg *glsl_alloc_abstract_cfg(const BasicBlockList *l) {
   struct abstract_cfg *cfg = glsl_safemem_malloc(sizeof(struct abstract_cfg));
   cfg->n    = glsl_basic_block_list_count(l);
   cfg->pred = alloc_nodeset(cfg->n);
   cfg->succ = alloc_nodeset(cfg->n);

   int reverse_postorder_id = 0;
   for (const BasicBlockList *n = l; n != NULL; n = n->next) {
      int f_rpo = n->v->fallthrough_target ? get_reverse_postorder_number(l, n->v->fallthrough_target) : -1;
      int t_rpo = n->v->branch_target ? get_reverse_postorder_number(l, n->v->branch_target) : -1;

      if (f_rpo != -1) {
         nodeset_add(&cfg->succ[reverse_postorder_id], f_rpo);
         nodeset_add(&cfg->pred[f_rpo], reverse_postorder_id);
      }
      if (t_rpo != -1) {
         nodeset_add(&cfg->succ[reverse_postorder_id], t_rpo);
         nodeset_add(&cfg->pred[t_rpo], reverse_postorder_id);
      }
      reverse_postorder_id++;
   }
   return cfg;
}

struct abstract_cfg *glsl_alloc_abstract_cfg_ssa(const SSABlock *blocks, int n_blocks) {
   struct abstract_cfg *cfg = glsl_safemem_malloc(sizeof(struct abstract_cfg));
   cfg->n    = n_blocks;
   cfg->pred = alloc_nodeset(n_blocks);
   cfg->succ = alloc_nodeset(n_blocks);

   for (int i=0; i<n_blocks; i++) {
      if (blocks[i].next_false != -1) {
         nodeset_add(&cfg->succ[i], blocks[i].next_false);
         nodeset_add(&cfg->pred[blocks[i].next_false], i);
      }
      if (blocks[i].next_true != -1) {
         nodeset_add(&cfg->succ[i], blocks[i].next_true);
         nodeset_add(&cfg->pred[blocks[i].next_true], i);
      }
   }
   return cfg;
}

struct abstract_cfg *glsl_alloc_abstract_cfg_cfg(const CFGBlock *blocks, int n_blocks) {
   struct abstract_cfg *cfg = glsl_safemem_malloc(sizeof(struct abstract_cfg));
   cfg->n    = n_blocks;
   cfg->pred = alloc_nodeset(n_blocks);
   cfg->succ = alloc_nodeset(n_blocks);

   for (int i=0; i<n_blocks; i++) {
      if (blocks[i].next_if_true != -1) {
         nodeset_add(&cfg->succ[i], blocks[i].next_if_true);
         nodeset_add(&cfg->pred[blocks[i].next_if_true], i);
      }
      if (blocks[i].next_if_false != -1) {
         nodeset_add(&cfg->succ[i], blocks[i].next_if_false);
         nodeset_add(&cfg->pred[blocks[i].next_if_false], i);
      }
   }
   return cfg;
}

/* Helpers which are used for reversing a CFG */

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

/* TODO: This is hard because the RPO is baked into the CFG. Make it separate? */
struct abstract_cfg *glsl_alloc_abstract_cfg_reverse(const struct abstract_cfg *fwd, int **node_map) {
   int *rpo_id = alloc_reverse_postorder_numbering(fwd->pred, fwd->n);

   struct abstract_cfg *rev = glsl_safemem_malloc(sizeof(struct abstract_cfg));
   rev->n = fwd->n;
   rev->pred = renumber_nodeset(fwd->succ, fwd->n, rpo_id);
   rev->succ = renumber_nodeset(fwd->pred, fwd->n, rpo_id);

   *node_map = rpo_id;
   return rev;
}

void glsl_free_abstract_cfg(struct abstract_cfg *cfg) {
   if (!cfg) return;

   free_nodeset(cfg->pred, cfg->n);
   free_nodeset(cfg->succ, cfg->n);
   glsl_safemem_free(cfg);
}

int *glsl_alloc_idoms(const struct abstract_cfg *cfg) {
   int *idom = glsl_safemem_malloc(cfg->n * sizeof(int));
   dom_tree(idom, cfg->pred, cfg->n);
   return idom;
}

bool **glsl_dom_frontiers_alloc(const struct abstract_cfg *cfg, const int *idoms) {
   /* Store the dominance frontiers using a simple boolean grid. n^2 memory *
    * usage but it probably doesn't matter. Always time to optimise later.  */
   bool **df = glsl_safemem_malloc(cfg->n * sizeof(bool *));
   for (int i=0; i<cfg->n; i++) {
      df[i] = glsl_safemem_calloc(cfg->n, sizeof(bool));
   }

   dominance_frontiers(df, idoms, cfg->pred, cfg->n);
   return df;
}

void glsl_dom_frontiers_free(bool **df, int n_blocks) {
   if (df == NULL) return;
   for (int i=0; i<n_blocks; i++) glsl_safemem_free(df[i]);
   glsl_safemem_free(df);
}


struct cd_set *glsl_cd_sets_alloc(const struct abstract_cfg *cfg) {
   struct cd_set *cd = glsl_safemem_malloc(cfg->n * sizeof(struct cd_set));

   int *rpo_id;
   struct abstract_cfg *rcfg = glsl_alloc_abstract_cfg_reverse(cfg, &rpo_id);

   int   *pdoms = glsl_alloc_idoms(rcfg);
   bool **df    = glsl_dom_frontiers_alloc(rcfg, pdoms);

   glsl_free_abstract_cfg(rcfg);

   int *invert_rpo_id = glsl_safemem_malloc(cfg->n * sizeof(int));
   for (int i=0; i<cfg->n; i++) invert_rpo_id[rpo_id[i]] = i;

   for (int i=0; i<cfg->n; i++) {
      int count = 0;
      int *cd_nodes = glsl_safemem_malloc(cfg->n * sizeof(int));
      bool *cond = glsl_safemem_malloc(cfg->n * sizeof(bool));
      for (int j=0; j<cfg->n; j++) {
         if (df[i][j]) {
            assert(cfg->succ[invert_rpo_id[j]].n == 2);
            int false_succ = rpo_id[cfg->succ[invert_rpo_id[j]].id[0]];
            cd_nodes[count] = invert_rpo_id[j];
            cond[count++] = !dominates(pdoms, false_succ, i);
         }
      }

      int id = invert_rpo_id[i];
      cd[id].n = count;
      cd[id].dep = glsl_safemem_malloc(count * sizeof(struct cd_dep));
      for (int j=0; j<count; j++) {
         cd[id].dep[j].node = cd_nodes[j];
         cd[id].dep[j].cond = cond[j];
      }
      glsl_safemem_free(cd_nodes);
      glsl_safemem_free(cond);
   }

   glsl_safemem_free(invert_rpo_id);
   glsl_safemem_free(rpo_id);

   glsl_dom_frontiers_free(df, cfg->n);
   glsl_safemem_free(pdoms);

   return cd;
}

void glsl_cd_sets_free(struct cd_set *cd, int n_blocks) {
   if (!cd) return;
   for (int i=0; i<n_blocks; i++) glsl_safemem_free(cd[i].dep);
   glsl_safemem_free(cd);
}

bool *glsl_alloc_uncond_blocks(const struct abstract_cfg *cfg) {
   struct cd_set *cd  = glsl_cd_sets_alloc(cfg);
   bool *uncond = glsl_safemem_malloc(cfg->n * sizeof(bool));
   for (int i=0; i<cfg->n; i++) uncond[i] = (cd[i].n == 0);

   glsl_cd_sets_free(cd, cfg->n);
   return uncond;
}

void glsl_solve_dataflow_equation(const struct abstract_cfg *cfg, unsigned size,
                                  const struct dflow_state *b, bool **in, bool **out)
{
   for (unsigned i=0; i<size; i++) out[0][i] = b[0].gen[i];

   bool changed;
   do {
      changed = false;

      for (int rpo_id = 1; rpo_id < cfg->n; rpo_id++) {
         const struct dflow_state *bb = &b[rpo_id];
         NodeSet *p = &cfg->pred[rpo_id];

         /* Update 'in' from the predecessor information */
         for (int pred_id = 0; pred_id<p->n; pred_id++) {
            const bool *pred_out = out[p->id[pred_id]];

            for (unsigned c_id = 0; c_id < size; c_id++) {
               if (pred_out[c_id] && !in[rpo_id][c_id]) {
                  changed = true;
                  in[rpo_id][c_id] = true;
               }
            }
         }

         /* Use the new 'in' and current block information to update 'out' */
         for (unsigned c_id = 0; c_id < size; c_id++)
            out[rpo_id][c_id] = (in[rpo_id][c_id] && !bb->kill[c_id]) || bb->gen[c_id];
      }
   } while (changed);
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
   if (l->next == NULL) return false;

   struct abstract_cfg *cfg = glsl_alloc_abstract_cfg(l);

   int *doms = glsl_alloc_idoms(cfg);

   /* Construct a version of the dominator tree that's good for walking */
   NodeSet *child = alloc_nodeset(cfg->n);
   for (int i=1; i<cfg->n; i++) {    /* Loop, skipping the start block */
      NodeSet *ch = &child[doms[i]];
      ch->id[ch->n++] = i;
   }

   int *candidates = glsl_safemem_malloc(cfg->n * sizeof(int));
   int n_candidates = 0;

   bool valid = false;
   append_merge_candidates(child, 0, candidates, &n_candidates);
   for (int c=0; c<n_candidates; c++) {
      *head = candidates[c];
      *exit = identify_merge_exit(child, *head, cfg->succ);

      valid = merge_valid_entry(*head, &child[*head], cfg->pred)       &&
              merge_valid_exit(*head, *exit, &child[*head], cfg->succ) &&
              merge_valid_backedges(*head, &child[*head], cfg->succ);

      if (valid) break;
   }

   glsl_safemem_free(candidates);
   free_nodeset(child, cfg->n);
   glsl_safemem_free(doms);

   glsl_free_abstract_cfg(cfg);

   return valid;
}
