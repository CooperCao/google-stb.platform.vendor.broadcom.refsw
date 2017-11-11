/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_basic_block.h"
#include "glsl_ir_shader.h"

struct cd_set {
   int n;
   struct cd_dep {
      int node;
      bool cond;
   } *dep;
};

struct abstract_cfg;

struct abstract_cfg *glsl_alloc_abstract_cfg(const BasicBlockList *l);
struct abstract_cfg *glsl_alloc_abstract_cfg_ssa(const SSABlock *blocks, int n_blocks);
struct abstract_cfg *glsl_alloc_abstract_cfg_cfg(const CFGBlock *blocks, int n_blocks);
struct abstract_cfg *glsl_alloc_abstract_cfg_reverse(const struct abstract_cfg *fwd, int **node_map);
void   glsl_free_abstract_cfg(struct abstract_cfg *cfg);

int   *glsl_alloc_idoms(const struct abstract_cfg *cfg);
bool **glsl_dom_frontiers_alloc(const struct abstract_cfg *cfg, const int *idoms);
void   glsl_dom_frontiers_free (bool **df, int n_blocks);

struct cd_set *glsl_cd_sets_alloc(const struct abstract_cfg *cfg);
void           glsl_cd_sets_free (struct cd_set *cd, int n_blocks);

bool *glsl_alloc_uncond_blocks(const struct abstract_cfg *cfg);

struct dflow_state {
   bool *gen;     /* Values that are generated in each block */
   bool *kill;    /* Values that are killed in each block */
};

/* Solve the dataflow equation defined by dflow_state, the result is returned in 'in'
 * and 'out'. Each of these arrays must be size cfg->n. 'size' is the number of variables
 * in the problem and each member of 'in' and 'out' and each array gen/kill must be of
 * size 'size' */
void glsl_solve_dataflow_equation(const struct abstract_cfg *cfg, unsigned size,
                                  const struct dflow_state *b, bool **in, bool **out);

/* TODO: This doesn't belong here. Mark dominance utils and users better */
bool glsl_ssa_block_flatten(const BasicBlockList *l, int *head, int *exit);
