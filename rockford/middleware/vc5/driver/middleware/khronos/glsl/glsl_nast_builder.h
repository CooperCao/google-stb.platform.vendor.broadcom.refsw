/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_NAST_BUILDER_H_INCLUDED
#define GLSL_NAST_BUILDER_H_INCLUDED

#include "glsl_ast.h"
#include "glsl_nast.h"

NStmtList *glsl_nast_build(Statement *ast);

#endif
