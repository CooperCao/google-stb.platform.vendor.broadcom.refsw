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
typedef struct _SymbolList        SymbolList;
typedef struct _BasicBlock        BasicBlock;
typedef union  iface_data         IFaceData;

#include "glsl_common.h"

EXTERN_C_BEGIN

struct symbol_usage {
   const Symbol *symbol;
   bool used;
};

struct if_usage {
   unsigned n;
   struct symbol_usage *v;
};

struct sh_usage {
   struct if_usage in;
   struct if_usage out;
   struct if_usage uniform;
   struct if_usage buffer;
};

CompiledShader *glsl_compile_shader(ShaderFlavour flavour, const GLSL_SHADER_SOURCE_T *source, bool multicore, bool clamp_shared_idx, uint32_t shared_mem_per_core);

CompiledShader *glsl_compiled_shader_create(ShaderFlavour f, int version);
void glsl_compiled_shader_free(CompiledShader *sh);

Symbol *glsl_construct_shared_block(const SymbolList *members);
void glsl_generate_compute_variables(const Symbol *s_l_idx, const Symbol *s_l_id,
                                     const Symbol *s_wg_id, const Symbol *s_g_id,
                                     const Symbol *s_n_sgs, const Symbol *s_sg_id,
                                     BasicBlock *entry_block, const unsigned *wg_size, const Symbol *shared_block,
                                     bool multicore, bool clamp_shared_idx, uint32_t shared_mem_per_core);

void glsl_predicate_dataflow(Dataflow *d, Dataflow *g);

CompiledShader *glsl_compile_common(ShaderFlavour flavour, int version, BasicBlock *entry_block,
                                    const IFaceData *iface_data, const struct sh_usage *symbs,
                                    Map *symbol_ids, bool loop_unroll, bool activity_supported);

GLSL_PROGRAM_T *glsl_link_compute_program(CompiledShader *c);
GLSL_PROGRAM_T *glsl_link_program(CompiledShader **stages, const GLSL_PROGRAM_SOURCE_T *source, bool separable, bool validate);

EXTERN_C_END
