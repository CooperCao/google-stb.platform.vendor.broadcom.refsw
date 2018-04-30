/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_dataflow.h"

EXTERN_C_BEGIN

/* IR format used inside the compiler: */
typedef struct _IROutput {
   int block;
   int output;
} IROutput;

typedef struct _SSABlock {
   int id;
   int n_outputs;
   Dataflow **outputs;
   int successor_condition;
   int next_true;
   int next_false;
   bool barrier;
} SSABlock;

typedef struct _SSAShader{
   SSABlock *blocks;
   int       n_blocks;
   IROutput *outputs;
   int       n_outputs;
} SSAShader;

void glsl_ssa_shader_term(SSAShader *sh);


/* IR format used in relocatable programs: (Some elements are shared from above) */

typedef struct {
   int successor_condition;      /* If -1 (no condition) then always false */
   int next_if_true;
   int next_if_false;
   bool barrier;

   Dataflow *dataflow;
   int num_dataflow;
   int *outputs;
   int num_outputs;
} CFGBlock;

/* A struct containing the relocatable dataflow for a shader to pass
   outside the compiler */
typedef struct {
   CFGBlock *blocks;
   int       num_cfg_blocks;

   IROutput *outputs;
   int       num_outputs;
} IRShader;

typedef struct {
   int *ins;
   int num_ins;
   int *outs;
   int num_outs;
   int *uniforms;
   int num_uniforms;
   int *buffers;
   int num_buffers;
} LinkMap;

bool glsl_ir_copy_block(CFGBlock *b, Dataflow **dataflow_in, int count);

void glsl_ir_shader_init(IRShader *sh);
void glsl_ir_shader_term(IRShader *sh);

IRShader *glsl_ir_shader_create();
void      glsl_ir_shader_free  (IRShader *sh);

IRShader *glsl_ir_shader_copy(const IRShader *in);

IRShader *glsl_ir_shader_from_file(const char *fname);
void      glsl_ir_shader_to_file  (const IRShader *sh, const char *fname);

EXTERN_C_END
