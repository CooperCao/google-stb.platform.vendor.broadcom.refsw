/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_nast.h"
#include "glsl_basic_block.h"

BasicBlock *glsl_basic_block_build(NStmtList *nast);
