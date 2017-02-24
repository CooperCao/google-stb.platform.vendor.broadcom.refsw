/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <stdlib.h>
#include <stdio.h>

#include "glsl_program.h"
#include "glsl_source.h"

typedef struct CompiledShader_s   CompiledShader;
typedef struct _Map               Map;
typedef struct _ShaderInterfaces  ShaderInterfaces;
typedef struct _ShaderInterface   ShaderInterface;
typedef struct _SSABlock          SSABlock;
typedef struct _SSAShader         SSAShader;

#include "glsl_common.h"

EXTERN_C_BEGIN

CompiledShader *glsl_compile_shader(ShaderFlavour flavour, const GLSL_SHADER_SOURCE_T *source);

CompiledShader *glsl_compiled_shader_create(ShaderFlavour f, int version);
void glsl_compiled_shader_free(CompiledShader *sh);

void glsl_mark_interface_actives(ShaderInterface *in, ShaderInterface *uniform, ShaderInterface *buffer,
                                 const SSABlock *block, int n_blocks);
bool glsl_copy_compiled_shader(CompiledShader *sh, ShaderInterfaces *ifaces, Map *symbol_map);
bool glsl_copy_shader_ir(CompiledShader *ret, const SSAShader *sh);

void glsl_ssa_shader_optimise(SSAShader *sh);

GLSL_PROGRAM_T *glsl_link_compute_program(CompiledShader *c);
GLSL_PROGRAM_T *glsl_link_program(CompiledShader **stages, const GLSL_PROGRAM_SOURCE_T *source, bool separable);

// This really does not belong here. However, declaring it in glsl_parser.h wouldn't work either,
// since Window's windef.h pollutes the namespace with typedefs like BOOL, FLOAT, ...
extern Statement *glsl_parse_ast(ShaderFlavour flavour, int version, int sourcec, const char * const *sourcev);

EXTERN_C_END
