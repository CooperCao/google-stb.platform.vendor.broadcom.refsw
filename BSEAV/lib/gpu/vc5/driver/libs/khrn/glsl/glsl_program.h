/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_PROGRAM_H_INCLUDED
#define GLSL_PROGRAM_H_INCLUDED

#include "GLES3/gl32.h"

#include "glsl_ir_program.h"

VCOS_EXTERN_C_BEGIN

typedef struct {
   int    location;
   GLenum type;
   bool   is_32bit;
   bool   in_binning;
} GLSL_SAMPLER_T;

typedef struct {
   int    location;
   GLenum type;
   bool   is_32bit;
   bool   in_binning;
   GLenum internalformat;
} GLSL_IMAGE_T;

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
   bool      used_in_tcs;
   bool      used_in_tes;
   bool      used_in_gs;
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
   bool used_in_tcs;
   bool used_in_tes;
   bool used_in_gs;
   bool used_in_fs;
   bool used_in_cs;
} GLSL_BLOCK_T;

typedef struct {
   int       index;
   char     *name;
   GLenum    type;
   bool      is_array;
   unsigned  array_size;
   bool      used_in_vs;
   bool      used_in_tcs;
   bool      used_in_tes;
   bool      used_in_gs;
   bool      used_in_fs;
   bool      used_in_cs;
   uint32_t  precision;    /* TODO: Maybe some other SSO stuff for ease of comparison */
   bool      flat;
   bool      centroid;
   bool      is_per_patch;
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
   bool     used_in_tcs;
   bool     used_in_tes;
   bool     used_in_gs;
   bool     used_in_fs;
   bool     used_in_cs;
} GLSL_ATOMIC_BUF_T;

typedef struct {
   int id;
   int binding;
} GLSL_LAYOUT_BINDING_T;

typedef struct GLSL_PROGRAM_T_ {
   /* Outputs from compile_and_link */
   unsigned               num_samplers;
   GLSL_SAMPLER_T        *samplers;
   unsigned               num_images;
   GLSL_IMAGE_T          *images;
   unsigned               num_uniform_blocks;
   GLSL_BLOCK_T          *uniform_blocks;
   GLSL_BLOCK_T           default_uniforms;
   unsigned               num_buffer_blocks;
   GLSL_BLOCK_T          *buffer_blocks;
   unsigned               num_inputs;
   GLSL_INOUT_T          *inputs;
   unsigned               num_outputs;
   GLSL_INOUT_T          *outputs;
   unsigned               num_tf_captures;
   GLSL_TF_CAPTURE_T     *tf_capture;
   unsigned               num_atomic_buffers;
   GLSL_ATOMIC_BUF_T     *atomic_buffers;
   IR_PROGRAM_T          *ir;
   unsigned int           num_sampler_bindings;
   GLSL_LAYOUT_BINDING_T *sampler_binding;
   unsigned int           num_image_bindings;
   GLSL_LAYOUT_BINDING_T *image_binding;
   unsigned int           num_ubo_bindings;
   GLSL_LAYOUT_BINDING_T *ubo_binding;
   unsigned int           num_ssbo_bindings;
   GLSL_LAYOUT_BINDING_T *ssbo_binding;

   unsigned int           num_uniform_offsets;
   unsigned int          *uniform_offsets;

   unsigned               wg_size[3];
   unsigned               shared_block_size;
} GLSL_PROGRAM_T;

GLSL_PROGRAM_T *glsl_program_create();
void            glsl_program_free  (GLSL_PROGRAM_T *program);

/* Shrink should be called after all modifications to the program are complete,
   as it will make the allocated array sizes match the stated number of elements */
void            glsl_program_shrink(GLSL_PROGRAM_T *program);

static inline bool glsl_program_has_stage(GLSL_PROGRAM_T const* program, ShaderFlavour flavour)
{
   if (flavour == SHADER_COMPUTE)
      return program->shared_block_size != ~0u;
   else
      return program->ir->stage[flavour].ir != NULL;
}

VCOS_EXTERN_C_END

#endif
