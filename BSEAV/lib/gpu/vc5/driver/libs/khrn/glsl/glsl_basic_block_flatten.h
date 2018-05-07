/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_basic_block.h"

EXTERN_C_BEGIN

// converts complex control flow graph into as few basic basics as possible
// using loop unrolling and conditional execution (i.e. guards).
BasicBlock *glsl_basic_block_flatten(BasicBlock *entry, bool unroll);

EXTERN_C_END
