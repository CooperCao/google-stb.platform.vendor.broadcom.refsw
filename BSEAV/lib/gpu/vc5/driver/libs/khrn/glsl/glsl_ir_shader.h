/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_IR_SHADER_H_INCLUDED
#define GLSL_IR_SHADER_H_INCLUDED

#include "glsl_dataflow.h"

VCOS_EXTERN_C_BEGIN

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


/* IR format used in relocatable programs: (Some elements are shared from above) */

typedef struct {
   int successor_condition;
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

IRShader *glsl_ir_shader_create();
void      glsl_ir_shader_free  (IRShader *shader);

IRShader *glsl_ir_shader_from_blocks(CFGBlock *blocks, int num_blocks, IROutput *outputs, int num_outputs);

IRShader *glsl_ir_shader_from_file(const char *fname);
void      glsl_ir_shader_to_file  (const IRShader *sh, const char *fname);

VCOS_EXTERN_C_END

#endif
