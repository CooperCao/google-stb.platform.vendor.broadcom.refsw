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
void   glsl_free_abstract_cfg(struct abstract_cfg *cfg);

int   *glsl_alloc_idoms(const struct abstract_cfg *cfg);
bool **glsl_dom_frontiers_alloc(const struct abstract_cfg *cfg, const int *idoms);
void   glsl_dom_frontiers_free (bool **df, int n_blocks);

struct cd_set *glsl_cd_sets_alloc(const struct abstract_cfg *cfg, const SSABlock *blocks);
void           glsl_cd_sets_free (struct cd_set *cd, int n_blocks);


/* TODO: These don't belong here. Mark dominance utils and users better */
void glsl_find_unconditional_blocks(const BasicBlockList *l, bool *uncond);

bool glsl_ssa_block_flatten(const BasicBlockList *l, int *head, int *exit);
int glsl_find_lthrsw_block(const CFGBlock *b, int n_blocks, bool *does_thrsw);
