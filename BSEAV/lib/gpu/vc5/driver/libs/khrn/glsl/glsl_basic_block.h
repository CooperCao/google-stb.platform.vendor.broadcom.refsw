/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_dataflow.h"
#include "glsl_common.h"
#include "glsl_map.h"

EXTERN_C_BEGIN

typedef struct _BasicBlock {
   Map *loads;          // Symbol* -> Dataflow** - DATAFLOW_LOAD nodes
   Map *scalar_values;  // Symbol* -> Dataflow** - most recent value of all used symbols

   Dataflow *branch_cond;                    // branch condition (can be NULL)
   struct _BasicBlock *branch_target;        // branch taken
   struct _BasicBlock *fallthrough_target;   // branch not taken or no condition

   Dataflow *memory_head;
   bool barrier;
} BasicBlock;

BasicBlock *glsl_basic_block_construct();
void        glsl_basic_block_delete(BasicBlock *b);

// Returned map is a set of BasicBlock*s (k=v for all k,v pairs)
Map *glsl_basic_block_all_reachable(BasicBlock *entry);

void glsl_basic_block_delete_reachable(BasicBlock *entry);

typedef struct _BasicBlockList {
   BasicBlock *v;
   struct _BasicBlockList *next;
} BasicBlockList;

void glsl_basic_block_list_add(BasicBlockList **list, BasicBlock *value);
void glsl_basic_block_list_pop(BasicBlockList **list);
bool glsl_basic_block_list_contains(const BasicBlockList *list, BasicBlock *value);
int  glsl_basic_block_list_count(const BasicBlockList *list);

BasicBlockList *glsl_basic_block_get_reverse_postorder_list(BasicBlock *entry);

// Methods for getting/setting of scalar_values of a given symbol.
// If you read uninitialised symbol, DATAFLOW_LOAD nodes will be implicitely created.
Dataflow **glsl_basic_block_get_scalar_values(BasicBlock *basic_block, const Symbol *symbol);
Dataflow  *glsl_basic_block_get_scalar_value (BasicBlock *basic_block, const Symbol *symbol, unsigned index);
void       glsl_basic_block_set_scalar_values(BasicBlock *basic_block, const Symbol *symbol, Dataflow **values);
void       glsl_basic_block_set_scalar_value (BasicBlock *basic_block, const Symbol *symbol, unsigned index, Dataflow *value);

EXTERN_C_END
