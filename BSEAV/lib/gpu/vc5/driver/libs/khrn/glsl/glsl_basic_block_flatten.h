/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_basic_block.h"

EXTERN_C_BEGIN

bool glsl_basic_block_flatten_a_bit(BasicBlock *entry, Map *block_age_offsets);

// converts complex control flow graph into as few basic basics as possible
// using loop unrolling and conditional execution (i.e. guards).
BasicBlock *glsl_basic_block_flatten(BasicBlock *entry, Map *block_age_offsets);

EXTERN_C_END
