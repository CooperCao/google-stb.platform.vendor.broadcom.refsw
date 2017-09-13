/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdlib.h>

#include "libs/util/gfx_util/gfx_util.h"

#include "../glxx/glxx_int_config.h"

#include "glsl_program.h"
#include "glsl_ir_program.h"

GLSL_PROGRAM_T *glsl_program_create() {
   GLSL_PROGRAM_T *ret = malloc(sizeof(*ret));
   if(!ret) return NULL;

   ret->num_samplers        = 0;
   ret->samplers            = malloc(GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS * sizeof(GLSL_SAMPLER_T));
   ret->num_images          = 0;
   ret->images              = malloc(GLXX_CONFIG_MAX_IMAGE_UNITS * sizeof(GLSL_IMAGE_T));
   ret->num_uniform_blocks  = 0;
   ret->uniform_blocks      = malloc(GLXX_CONFIG_MAX_COMBINED_UNIFORM_BLOCKS * sizeof(GLSL_BLOCK_T));

   ret->default_uniforms.index        = -1;
   ret->default_uniforms.array_length =  1;
   ret->default_uniforms.size         =  0;
   ret->default_uniforms.name         =  NULL;
   ret->default_uniforms.num_members  =  0;
   ret->default_uniforms.members      =
      malloc(sizeof(*ret->default_uniforms.members) * GLXX_CONFIG_MAX_UNIFORM_VECTORS);

   ret->buffer_blocks     = malloc(GLXX_CONFIG_MAX_COMBINED_STORAGE_BLOCKS * sizeof(GLSL_BLOCK_T));
   ret->num_buffer_blocks = 0;

   static const int max_ins_outs = GLXX_CONFIG_MAX_TESS_PATCH_COMPONENTS + GLXX_CONFIG_MAX_TESS_CONTROL_OUTPUT_COMPONENTS;

   ret->num_inputs           = 0;
   ret->inputs               = malloc(sizeof(GLSL_INOUT_T) * max_ins_outs);
   ret->num_outputs          = 0;
   ret->outputs              = malloc(sizeof(GLSL_INOUT_T) * max_ins_outs);
   ret->num_tf_captures      = 0;
   ret->tf_capture           = malloc(sizeof(GLSL_TF_CAPTURE_T) * GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS);
   ret->num_atomic_buffers   = 0;
   ret->atomic_buffers       = malloc(sizeof(GLSL_ATOMIC_BUF_T) * GLXX_CONFIG_MAX_COMBINED_ATOMIC_COUNTER_BUFFERS);
   ret->num_sampler_bindings = 0;
   ret->sampler_binding      = malloc(sizeof(GLSL_LAYOUT_BINDING_T) * GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS);
   ret->num_image_bindings   = 0;
   ret->image_binding        = malloc(sizeof(GLSL_LAYOUT_BINDING_T) * GLXX_CONFIG_MAX_IMAGE_UNITS);
   ret->num_ubo_bindings     = 0;
   ret->ubo_binding          = malloc(sizeof(GLSL_LAYOUT_BINDING_T) * GLXX_CONFIG_MAX_COMBINED_UNIFORM_BLOCKS);
   ret->num_ssbo_bindings    = 0;
   ret->ssbo_binding         = malloc(sizeof(GLSL_LAYOUT_BINDING_T) * GLXX_CONFIG_MAX_COMBINED_STORAGE_BLOCKS);
   ret->num_uniform_offsets  = 0;
   ret->uniform_offsets      = malloc(sizeof(unsigned) * GLXX_CONFIG_MAX_UNIFORM_LOCATIONS);
   ret->ir                   = glsl_ir_program_create();

   if(ret->ir == NULL                       ||
      ret->uniform_blocks == NULL           ||
      ret->default_uniforms.members == NULL ||
      ret->buffer_blocks   == NULL          ||
      ret->inputs          == NULL          ||
      ret->outputs         == NULL          ||
      ret->tf_capture      == NULL          ||
      ret->atomic_buffers  == NULL          ||
      ret->sampler_binding == NULL          ||
      ret->image_binding   == NULL          ||
      ret->ubo_binding     == NULL          ||
      ret->ssbo_binding    == NULL          ||
      ret->uniform_offsets == NULL           )
   {
      glsl_program_free(ret);
      return NULL;
   }
   return ret;
}

static void *my_realloc(void *ptr, size_t new_size) {
   if(new_size == 0) {
      free(ptr);
      return NULL;
   }

   void *tmp = realloc(ptr, new_size);
   if(tmp) ptr = tmp;

   return ptr;
}

void glsl_program_shrink(GLSL_PROGRAM_T *program) {
   program->samplers =
      my_realloc(program->samplers,
                 sizeof(*program->samplers) * program->num_samplers);

   program->images =
      my_realloc(program->images,
                 sizeof(*program->images) * program->num_images);

   program->uniform_blocks =
      my_realloc(program->uniform_blocks,
                 sizeof(*program->uniform_blocks) * program->num_uniform_blocks);

   program->default_uniforms.members =
      my_realloc(program->default_uniforms.members,
                 sizeof(*program->default_uniforms.members) * program->default_uniforms.num_members);

   program->buffer_blocks =
      my_realloc(program->buffer_blocks,
                 sizeof(*program->buffer_blocks) * program->num_buffer_blocks);

   program->inputs = my_realloc(program->inputs,
                                sizeof(GLSL_INOUT_T) * program->num_inputs);

   program->outputs = my_realloc(program->outputs,
                                 sizeof(GLSL_INOUT_T) * program->num_outputs);

   program->tf_capture = my_realloc(program->tf_capture,
                                    program->num_tf_captures * sizeof(GLSL_TF_CAPTURE_T));

   program->atomic_buffers = my_realloc(program->atomic_buffers,
                                        program->num_atomic_buffers * sizeof(GLSL_ATOMIC_BUF_T));

   program->sampler_binding = my_realloc(program->sampler_binding,
                                         program->num_sampler_bindings * sizeof(GLSL_LAYOUT_BINDING_T));

   program->image_binding = my_realloc(program->image_binding,
                                         program->num_image_bindings * sizeof(GLSL_LAYOUT_BINDING_T));

   program->ubo_binding = my_realloc(program->ubo_binding,
                                     program->num_ubo_bindings * sizeof(GLSL_LAYOUT_BINDING_T));

   program->ssbo_binding = my_realloc(program->ssbo_binding,
                                      program->num_ssbo_bindings * sizeof(GLSL_LAYOUT_BINDING_T));

   program->uniform_offsets = my_realloc(program->uniform_offsets,
                                         program->num_uniform_offsets * sizeof(unsigned));
}

void glsl_program_free(GLSL_PROGRAM_T *program) {
   if(!program) return;

   free(program->samplers);
   free(program->images);

   for(unsigned i=0;i<program->num_uniform_blocks;++i) {
      for(unsigned j=0;j<program->uniform_blocks[i].num_members;++j) {
         free(program->uniform_blocks[i].members[j].name);
      }
      free(program->uniform_blocks[i].name);
      free(program->uniform_blocks[i].members);
   }
   free(program->uniform_blocks);

   for(unsigned i=0;i<program->default_uniforms.num_members;++i) {
      free(program->default_uniforms.members[i].name);
   }
   free(program->default_uniforms.members);

   for (unsigned i=0; i<program->num_buffer_blocks; i++) {
      for (unsigned j=0; j<program->buffer_blocks[i].num_members; j++) {
         free(program->buffer_blocks[i].members[j].name);
      }
      free(program->buffer_blocks[i].name);
      free(program->buffer_blocks[i].members);
   }
   free(program->buffer_blocks);

   for(unsigned i=0;i<program->num_inputs;++i) {
      free(program->inputs[i].name);
      free(program->inputs[i].struct_path);
   }
   free(program->inputs);

   for(unsigned i=0;i<program->num_outputs;++i) {
      free(program->outputs[i].name);
      free(program->outputs[i].struct_path);
   }
   free(program->outputs);

   for (unsigned i=0; i<program->num_tf_captures; i++) {
      free(program->tf_capture[i].name);
   }
   free(program->tf_capture);
   free(program->atomic_buffers);

   free(program->sampler_binding);
   free(program->image_binding);
   free(program->ubo_binding);
   free(program->ssbo_binding);
   free(program->uniform_offsets);

   glsl_ir_program_free(program->ir);
   free(program);
}
