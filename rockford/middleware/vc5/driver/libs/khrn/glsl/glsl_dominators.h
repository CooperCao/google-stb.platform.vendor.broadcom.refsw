/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_DOMINATORS_H_
#define GLSL_DOMINATORS_H_

#include "glsl_basic_block.h"
#include "glsl_ir_shader.h"

Map *glsl_construct_dominator_tree(BasicBlockList *l, Map **phi_args);
void glsl_find_unconditional_blocks(const BasicBlockList *l, bool *uncond);

/* TODO: These don't belong here. Mark dominance utils and users better */
bool glsl_ssa_block_flatten(const BasicBlockList *l, int *head, int *exit);
int glsl_find_lthrsw_block(const CFGBlock *b, int n_blocks, bool *does_thrsw);

#endif
