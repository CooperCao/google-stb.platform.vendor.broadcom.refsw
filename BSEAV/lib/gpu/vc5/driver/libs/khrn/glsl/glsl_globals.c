/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdbool.h>

#include "glsl_ir_program.h"
#include "glsl_globals.h"

int g_ShaderVersion;

ShaderFlavour g_ShaderFlavour;

int g_LineNumber;
int g_FileNumber;
bool g_InGlobalScope;
