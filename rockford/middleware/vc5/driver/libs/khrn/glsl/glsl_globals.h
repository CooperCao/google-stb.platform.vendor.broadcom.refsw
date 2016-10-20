/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_GLOBALS_H
#define GLSL_GLOBALS_H

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

#endif // GLOBALS_H
