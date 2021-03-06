/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_GLOBALS_H
#define GLSL_GLOBALS_H

#include "glsl_compiler.h"
#include "glsl_shader_interfaces.h"

//
// Global variable declarations.
//

#define GLSL_SHADER_VERSION(major, minor, es) (((100 * (major)) + (minor))  + ((es) ? 1000000 : 0))
#define GLSL_SHADER_VERSION_NUMBER(version) ((version) >= 1000000 ? ((version) - 1000000) : (version))
#define GLSL_SHADER_VERSION_IS_ES(version) ((version) >= 1000000 ? 1 : 0)

extern int g_ShaderVersion;

// Whether we're compiling a fragment shader, vertex shader, etc.
extern ShaderFlavour g_ShaderFlavour;

// The line number that Lex has reached.
extern int g_LineNumber;
#define LINE_NUMBER_UNDEFINED -1
extern int g_FileNumber;

// The symbols in the symbol table that are sources for the dataflow graph.
extern ShaderInterfaces* g_ShaderInterfaces;

// Whether the parser is currently in global scope.
extern bool g_InGlobalScope;

#endif // GLOBALS_H
