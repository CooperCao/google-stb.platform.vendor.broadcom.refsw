/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_INTRINSIC_H_INCLUDED
#define GLSL_INTRINSIC_H_INCLUDED

#include "glsl_ast.h"
#include "glsl_intrinsic_types.h"
#include "glsl_symbols.h"

const glsl_intrinsic_data_t *glsl_intrinsic_lookup(const char *name, unsigned int len);

const char *glsl_intrinsic_name              (glsl_intrinsic_index_t intrinsic);
Expr       *glsl_intrinsic_construct_expr    (glsl_intrinsic_index_t intrinsic, ExprChain* args);

#endif
