/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_AST_PRINT_H
#define GLSL_AST_PRINT_H

// Debug only.
#ifndef NDEBUG

#include <stdio.h>
#include "middleware/khronos/glsl/glsl_ast.h"


// Pretty print a compile time constant.
void glsl_print_compile_time_value(FILE* f, SymbolType* type, void* compile_time_value);

// Pretty prints the Expr as source code to the given file.
// No newlines are added.
// If fully_evaluated is set, will replace expressions by their values.
void glsl_print_expr(FILE* f, Expr* expr, bool fully_evaluated);

// Pretty prints the Statement as source code to the given file.
// The string will be prepended by indent_depth tabs.
// Newlines may be added in the string but it will not start or end with a newline.
// If fully_evaluated is set, will replace expressions by their values.
void glsl_print_statement(FILE* f, Statement* statement, bool fully_evaluated, int indent_depth, bool suppress_semicolon);


#endif // _DEBUG
#endif // AST_PRINT_H