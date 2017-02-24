/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdbool.h>

#include "glsl_ir_program.h"
#include "glsl_globals.h"

int g_ShaderVersion;

ShaderFlavour g_ShaderFlavour;

int g_LineNumber;
int g_FileNumber;
bool g_InGlobalScope;
