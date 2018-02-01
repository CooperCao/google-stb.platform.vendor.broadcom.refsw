/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_dataflow.h"
#include "glsl_basic_block.h"
#include "glsl_primitive_types.auto.h"

EXTERN_C_BEGIN

GLenum glsl_fmt_qualifier_to_gl_enum(FormatQualifier fmt_qual);
void glsl_calculate_dataflow_image_atomic(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr);

EXTERN_C_END
