/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_common.h"
#include "glsl_globals.h"

int g_ShaderVersion;

ShaderFlavour g_ShaderFlavour;

int g_LineNumber;
int g_FileNumber;
bool g_InGlobalScope;

ShaderInterfaces *g_ShaderInterfaces;
