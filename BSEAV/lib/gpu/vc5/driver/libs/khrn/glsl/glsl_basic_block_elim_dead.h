/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_basic_block.h"

EXTERN_C_BEGIN

void glsl_basic_block_elim_dead(BasicBlock *entry);

EXTERN_C_END
