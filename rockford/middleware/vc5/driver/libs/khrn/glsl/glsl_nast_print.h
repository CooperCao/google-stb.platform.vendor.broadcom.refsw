/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_NAST_PRINT_H_INCLUDED
#define GLSL_NAST_PRINT_H_INCLUDED

#include "glsl_nast.h"

/* These functions are only defined if NDEBUG is not defined */

void glsl_nast_print_statement(FILE* f, int indent, const NStmt* stmt);
void glsl_nast_print_statements(FILE* f, int indent, const NStmtList* statements);

#endif
