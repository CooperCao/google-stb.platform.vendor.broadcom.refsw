/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_ast.h"
#include "glsl_intrinsic_types.h"
#include "glsl_symbols.h"

const char *glsl_intrinsic_name          (glsl_intrinsic_index_t intrinsic);
Expr       *glsl_intrinsic_construct_expr(int line_num, glsl_intrinsic_index_t intrinsic, ExprChain* args);
