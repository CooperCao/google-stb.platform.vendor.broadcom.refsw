/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_COMPILER_H
#define GLSL_COMPILER_H

#include <stdlib.h>
#include <stdio.h>

#include "glsl_program.h"
#include "glsl_source.h"

typedef struct CompiledShader_s CompiledShader;

#include "glsl_common.h"
CompiledShader *glsl_compile_shader(ShaderFlavour flavour, const GLSL_SHADER_SOURCE_T *source);
void glsl_compiled_shader_free(CompiledShader *sh);

GLSL_PROGRAM_T *glsl_link_compute_program(CompiledShader *c);
GLSL_PROGRAM_T *glsl_link_program(CompiledShader **stages, const GLSL_PROGRAM_SOURCE_T *source, bool separable);

// This really does not belong here. However, declaring it in glsl_parser.h wouldn't work either,
// since Window's windef.h pollutes the namespace with typedefs like BOOL, FLOAT, ...
extern Statement *glsl_parse_ast(ShaderFlavour flavour, int version, int sourcec, const char * const *sourcev);

#endif // COMPILER_H
