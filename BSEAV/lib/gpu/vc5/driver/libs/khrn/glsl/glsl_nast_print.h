/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_nast.h"

/* These functions are only defined if NDEBUG is not defined */

void glsl_nast_print_statement(FILE* f, int indent, const NStmt* stmt);
void glsl_nast_print_statements(FILE* f, int indent, const NStmtList* statements);
