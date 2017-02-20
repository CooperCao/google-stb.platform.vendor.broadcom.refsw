/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_ast.h"
#include "glsl_nast.h"

NStmtList *glsl_nast_build(Statement *ast);
