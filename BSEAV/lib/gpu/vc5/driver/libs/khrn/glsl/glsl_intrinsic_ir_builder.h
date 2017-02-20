/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_ast.h"
#include "glsl_dataflow.h"
#include "glsl_dataflow_builder.h"
#include "glsl_intrinsic_types.h"

void glsl_intrinsic_ir_calculate_dataflow(BasicBlock* ctx, Dataflow **scalar_values, Expr *expr);
