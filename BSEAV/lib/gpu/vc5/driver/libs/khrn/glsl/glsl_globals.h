/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdbool.h>
#include "glsl_ir_program.h"

extern int g_ShaderVersion;

// Whether we're compiling a fragment shader, vertex shader, etc.
extern ShaderFlavour g_ShaderFlavour;

// The line number that Lex has reached.
extern int g_LineNumber;
#define LINE_NUMBER_UNDEFINED -1
extern int g_FileNumber;

// Whether the parser is currently in global scope.
extern bool g_InGlobalScope;
