/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Standalone GLSL compiler
=============================================================================*/
#ifndef GLSL_COMPILER_H
#define GLSL_COMPILER_H

#include <stdlib.h>
#include <stdio.h>

#include "middleware/khronos/common/khrn_mem.h"

#include "interface/khronos/include/GLES2/gl2.h"

#include "middleware/khronos/gl20/gl20_config.h"

//
// Uniforms.
//

// Note that uniform variables can appear in the source as:
// - float/int/bool scalars/vectors/matrices
// - samplers
// - arrays of these
// - structs of these
//
// Samplers are conceptually different to the other types and so they are handled separately (below).

// base <= row < limit
#define GL_MAXVERTEXUNIFORMVECTORS     GL20_CONFIG_MAX_UNIFORM_VECTORS
#define GL_MAXFRAGMENTUNIFORMVECTORS   GL20_CONFIG_MAX_UNIFORM_VECTORS

#define SLANG_UNIFORMS_BASE_ROW        0
#define SLANG_UNIFORMS_LIMIT_ROW       ( (SLANG_UNIFORMS_BASE_ROW + GL20_CONFIG_MAX_UNIFORM_VECTORS) * 4)

#define SLANG_GL_DEPTHRANGE_NEAR_ROW   1021
#define SLANG_GL_DEPTHRANGE_FAR_ROW    1022
#define SLANG_GL_DEPTHRANGE_DIFF_ROW   1023

//
// Attributes.
//

// Note that attribute variables can appear in the source as:
// - float scalars/vectors/matrices

#define SLANG_MAX_NUM_ATTRIBUTES       64
#define GL_MAXVERTEXATTRIBS            8

//
// Varyings.
//

// Note that varying variables can appear in the source as:
// - float scalars/vectors/matrices
// - arrays of these

// Varyings start at SLANG_VARYINGS_BASE_ROW and occupy slang_getNumVaryings() contiguous rows.
// base <= row < limit
#define GL_MAXVARYINGVECTORS GL20_CONFIG_MAX_VARYING_VECTORS
#define SLANG_VARYINGS_BASE_ROW     32
#define SLANG_VARYINGS_LIMIT_ROW    (SLANG_VARYINGS_BASE_ROW + (GL20_CONFIG_MAX_VARYING_VECTORS * 4))

//
// Samplers.
// Note that vertex and fragment samplers are handled separately.
//

#define GL_MAXVERTEXTEXTUREIMAGEUNITS     GL20_CONFIG_MAX_VERTEX_TEXTURE_UNITS
#define GL_MAXCOMBINEDTEXTUREIMAGEUNITS   GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS
#define GL_MAXTEXTUREIMAGEUNITS           GL20_CONFIG_MAX_FRAGMENT_TEXTURE_UNITS
#define GL_MAXDRAWBUFFERS                 1

//
// Structs and enums.
//

typedef struct slang_vshader_s
{
   int sourcec;
   const char** sourcev; // these are *copies* of the input and so can be safely mutated
} slang_vshader;

typedef struct slang_fshader_s
{
   int sourcec;
   const char** sourcev; // these are *copies* of the input and so can be safely mutated
} slang_fshader;

typedef struct slang_binding_s
{
   int index;
   const char *name;
} slang_binding;

typedef struct slang_sampler_s
{
   GLint uniform;
   GLint array_index;
   bool in_vshader;
} slang_sampler;

typedef struct slang_uniform_s
{
   GLenum type;
   union {
      const char* name;
      MEM_HANDLE_T handle;
   } u;
   unsigned int row;
   unsigned int array_length;
   bool is_array;
} slang_uniform;

typedef struct slang_attribute_s
{
   GLenum type;
   union {
      const char* name;
      MEM_HANDLE_T handle;
   } u;
   unsigned int row;
} slang_attribute;

typedef struct slang_program_s
{
   slang_vshader vshader;
   slang_fshader fshader;

   unsigned int num_bindings;
   slang_binding *bindings;

   unsigned int num_samplers;
   slang_sampler samplers[GL20_CONFIG_MAX_COMBINED_TEXTURE_UNITS];

   unsigned int num_uniforms;
   slang_uniform uniforms[SLANG_UNIFORMS_LIMIT_ROW - SLANG_UNIFORMS_BASE_ROW];

   unsigned int num_attributes;
   slang_attribute attributes[SLANG_MAX_NUM_ATTRIBUTES];

   unsigned int num_scalar_varyings;
   unsigned int num_scalar_uniforms;

   unsigned int live_attributes;

   void *result;  /* GL20_LINK_RESULT_T */

} slang_program;

typedef enum
{
   SHADER_VERTEX,
   SHADER_FRAGMENT,
   SHADER_FLAVOUR_COUNT
} ShaderFlavour;

//
// Functions.
//

#define MAX_ERROR_LENGTH 256

extern char error_buffer[MAX_ERROR_LENGTH];

// The top level compilation function.
// Expects that program has valid vshader and fshader,
// but all other program metadata will be set by this function.
extern bool glsl_compile_and_link(slang_program* program);

// Only used to verify compilation at a high level.  Just runs the parser
extern bool glsl_compile(ShaderFlavour flavour, int sourcec, const char** sourcev);

// Causes the compiler to exit early. Some errors may break the dataflow tree (ie. stack overflow).
// Pass flags back to the error handler to indicate this. (Don't use 0, it won't work)
#define GLSL_EXIT 1
#define GLSL_EXIT_WITH_BAD_TREE 2

void glsl_compiler_exit(bool bad_tree);

#endif // COMPILER_H