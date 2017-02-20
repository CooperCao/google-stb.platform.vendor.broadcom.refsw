/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_map.h"
#include "glsl_ir_shader.h"
#include "glsl_symbols.h"
#include "glsl_basic_block.h"

EXTERN_C_BEGIN

void glsl_ssa_convert(SSAShader *sh, BasicBlock *entry_block, const SymbolList *outs, Map *symbol_ids);

EXTERN_C_END
