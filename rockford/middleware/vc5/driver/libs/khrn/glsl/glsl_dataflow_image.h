/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/
#include "glsl_dataflow.h"
#include "glsl_basic_block.h"
#include "glsl_primitive_types.auto.h"

GLenum glsl_fmt_qualifier_to_gl_enum(FormatQualifier fmt_qual);
void glsl_calculate_dataflow_image_store(BasicBlock *ctx, Expr *expr);
void glsl_calculate_dataflow_image_size(BasicBlock *ctx, Dataflow **scalar_values, Expr *expr);
