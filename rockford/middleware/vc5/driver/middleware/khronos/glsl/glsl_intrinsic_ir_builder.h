/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_INTRINSIC_IR_BUILDER_H_INCLUDED
#define GLSL_INTRINSIC_IR_BUILDER_H_INCLUDED

#include "glsl_ast.h"
#include "glsl_dataflow.h"
#include "glsl_dataflow_builder.h"
#include "glsl_intrinsic_types.h"

void glsl_intrinsic_ir_calculate_dataflow(BasicBlock* ctx, Dataflow **scalar_values, Expr *expr);

#endif
