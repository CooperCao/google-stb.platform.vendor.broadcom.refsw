/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_dataflow.h"
#include "glsl_basic_block.h"
#include "glsl_primitive_types.auto.h"

GLenum glsl_fmt_qualifier_to_gl_enum(FormatQualifier fmt_qual);
void glsl_calculate_dataflow_image_atomic(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr);
void glsl_calculate_dataflow_image_size(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr);
/* translate input coord into an index in array + s coord */
void glsl_imgbuffer_translate_coord(Dataflow *sampler, Dataflow *coord,
      Dataflow **x, Dataflow **elem_no);
