/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdio.h>
#include "middleware/khronos/glsl/glsl_ast.h"

// Pretty prints the Statement as source code to the given file.
// The string will be prepended by indent_depth tabs.
// Newlines may be added in the string but it will not start or end with a newline.
// If fully_evaluated is set, will replace expressions by their values.
void glsl_print_statement(FILE* f, Statement* statement, bool fully_evaluated, int indent_depth, bool suppress_semicolon);