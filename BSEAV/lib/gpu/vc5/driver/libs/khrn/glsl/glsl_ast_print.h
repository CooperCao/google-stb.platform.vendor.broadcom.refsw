/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdio.h>
#include "glsl_ast.h"
#include "glsl_common.h"

const char *glsl_storage_qual_string(StorageQualifier sq);
const char *glsl_type_qual_string(TypeQualifier tq);
const char *glsl_param_qual_string(ParamQualifier pq);

/* The following functions are only defined if NDEBUG is not defined */

// Pretty print a compile time constant.
void glsl_print_compile_time_value(FILE *f, SymbolType *type, const_value *compile_time_value);

void glsl_print_qualifiers(FILE* f, Symbol* symbol);

// Pretty prints the Expr as source code to the given file.
// No newlines are added.
// If fully_evaluated is set, will replace expressions by their values.
void glsl_print_expr(FILE *f, Expr *expr, bool fully_evaluated);

// Pretty prints the Statement as source code to the given file.
// The string will be prepended by indent_depth tabs.
// Newlines may be added in the string but it will not start or end with a newline.
// If fully_evaluated is set, will replace expressions by their values.
void glsl_print_statement(FILE *f, Statement *statement, bool fully_evaluated, unsigned int indent_depth, bool suppress_semicolon);
