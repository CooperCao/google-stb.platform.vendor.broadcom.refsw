/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_COMPILER_H
#define GLSL_COMPILER_H

#include <stdlib.h>
#include <stdio.h>

#include "interface/khronos/glxx/gl_public_api.h"

#include "glsl_program.h"
#include "glsl_source.h"

//
// Varyings.
//

// Note that varying variables can appear in the source as:
// - float scalars/vectors/matrices
// - arrays of these

#define GL_MAXVARYINGVECTORS        GL20_CONFIG_MAX_VARYING_VECTORS

//
// Samplers.
// Note that vertex and fragment samplers are handled separately.
//
#define GL_MAXVERTEXTEXTUREIMAGEUNITS     GL20_CONFIG_MAX_VERTEX_TEXTURE_UNITS
#define GL_MAXCOMBINEDTEXTUREIMAGEUNITS   GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS
#define GL_MAXTEXTUREIMAGEUNITS           GL20_CONFIG_MAX_FRAGMENT_TEXTURE_UNITS

//
// New built-in constants in ES 30
//
#define GL_MINPROGRAMTEXELOFFSET    GLXX_CONFIG_MIN_TEXEL_OFFSET
#define GL_MAXPROGRAMTEXELOFFSET    GLXX_CONFIG_MAX_TEXEL_OFFSET
#define GL_MAXFRAGMENTINPUTVECTORS  GL20_CONFIG_MAX_VARYING_VECTORS
#define GL_MAXVERTEXOUTPUTVECTORS   GL20_CONFIG_MAX_VARYING_VECTORS

/* TODO: Probably none of this belongs here */
typedef enum {
   SHADER_VERTEX,
   SHADER_FRAGMENT,
   SHADER_COMPUTE,
   SHADER_FLAVOUR_COUNT
} ShaderFlavour;

typedef struct CompiledShader_s CompiledShader;

#include "glsl_common.h"
CompiledShader *glsl_compile_shader(ShaderFlavour flavour, const GLSL_SHADER_SOURCE_T *source);
void glsl_compiled_shader_free(CompiledShader *sh);

GLSL_PROGRAM_T *glsl_link_compute_program(CompiledShader *c);
GLSL_PROGRAM_T *glsl_link_program(CompiledShader *v, CompiledShader *f, const GLSL_PROGRAM_SOURCE_T *source);

// This really does not belong here. However, declaring it in glsl_parser.h wouldn't work either,
// since Window's windef.h pollutes the namespace with typedefs like BOOL, FLOAT, ...
extern Statement *glsl_parse_ast(ShaderFlavour flavour, int sourcec, const char * const *sourcev);

#endif // COMPILER_H
