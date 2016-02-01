/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_PROGRAM_H_INCLUDED
#define GLSL_PROGRAM_H_INCLUDED

#include "interface/khronos/glxx/gl_public_api.h"
#include "interface/khronos/glxx/glxx_int_config.h"

#include "glsl_ir_program.h"

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

//
// Attributes.
//

// Note that attribute variables can appear in the source as:
// - float scalars/vectors/matrices
// - arrays of these

#define SLANG_MAX_NUM_ATTRIBUTES       64
#define GL_MAXVERTEXATTRIBS            GLXX_CONFIG_MAX_VERTEX_ATTRIBS


typedef struct {
   int    location;
   GLenum type;
   bool   is_32bit;
   bool   in_vshader;
} GLSL_SAMPLER_T;

typedef struct {
   GLenum    type;
   char     *name;
   unsigned  offset;
   unsigned  array_length;
   unsigned  array_stride;
   unsigned  matrix_stride;
   unsigned  top_level_size;
   unsigned  top_level_stride;
   int       atomic_idx;
   bool      column_major;
   bool      used_in_vs;
   bool      used_in_fs;
   bool      used_in_cs;
} GLSL_BLOCK_MEMBER_T;

typedef struct {
   /* Things that only exist once */
   int       index;  /* First index */
   int       array_length;
   bool      is_array;
   unsigned  size;
   char     *name;

   unsigned             num_members;
   GLSL_BLOCK_MEMBER_T *members;

   /* Things that are once per binding */
   bool used_in_vs;
   bool used_in_fs;
   bool used_in_cs;
} GLSL_BLOCK_T;

typedef struct {
   int       index;
   char     *name;
   GLenum    type;
   bool      is_array;
   unsigned  array_size;
} GLSL_INOUT_T;

typedef struct {
   char  *name;
   GLenum type;
   int    array_length;
} GLSL_TF_CAPTURE_T;

typedef struct {
   int      binding;
   unsigned size;
   bool     used_in_vs;
   bool     used_in_fs;
   bool     used_in_cs;
} GLSL_ATOMIC_BUF_T;

typedef struct {
   int id;
   int binding;
} GLSL_LAYOUT_BINDING_T;

typedef struct {
   /* Outputs from compile_and_link */
   unsigned               num_samplers;
   GLSL_SAMPLER_T        *samplers;
   unsigned               num_uniform_blocks;
   GLSL_BLOCK_T          *uniform_blocks;
   GLSL_BLOCK_T           default_uniforms;
   unsigned               num_buffer_blocks;
   GLSL_BLOCK_T          *buffer_blocks;
   unsigned               num_attributes;
   GLSL_INOUT_T          *attributes;
   unsigned               num_frag_outputs;
   GLSL_INOUT_T          *frag_out;
   unsigned               num_tf_captures;
   GLSL_TF_CAPTURE_T     *tf_capture;
   unsigned               num_atomic_buffers;
   GLSL_ATOMIC_BUF_T     *atomic_buffers;
   IR_PROGRAM_T          *ir;
   unsigned int           num_sampler_bindings;
   GLSL_LAYOUT_BINDING_T *sampler_binding;
   unsigned int           num_ubo_bindings;
   GLSL_LAYOUT_BINDING_T *ubo_binding;
   unsigned int           num_ssbo_bindings;
   GLSL_LAYOUT_BINDING_T *ssbo_binding;

   unsigned               wg_size[3];
   unsigned               shared_block_size;
} GLSL_PROGRAM_T;

GLSL_PROGRAM_T *glsl_program_create();
void            glsl_program_free  (GLSL_PROGRAM_T *program);

/* Shrink should be called after all modifications to the program are complete,
   as it will make the allocated array sizes match the stated number of elements */
void            glsl_program_shrink(GLSL_PROGRAM_T *program);


#endif
