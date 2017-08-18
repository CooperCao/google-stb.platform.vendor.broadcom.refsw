/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_ir_shader.h"

// Merge duplicate dataflow trees. Modifications are done in-place.
void glsl_dataflow_cse(SSABlock *block, int n_blocks, bool assume_read_only);
