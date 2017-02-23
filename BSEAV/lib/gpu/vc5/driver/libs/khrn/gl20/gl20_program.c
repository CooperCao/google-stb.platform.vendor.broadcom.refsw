/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of OpenGL ES 2.0 program structure.
=============================================================================*/

#include <string.h>

#include "../common/khrn_int_common.h"

#include "../glsl/glsl_compiler.h"
#include "../glsl/glsl_errors.h"

#include "../glxx/glxx_server_program_interface.h"
#include "../glxx/glxx_server_pipeline.h"

#include "gl20_program.h"
#include "gl20_shader.h"

extern uint32_t glxx_get_element_count(GLenum type);

static GLSL_BINDING_T *copy_binding_array  (const GLSL_BINDING_T *src_ary, unsigned src_len);
static GLSL_BINDING_T *append_binding_array(const GLSL_BINDING_T *ary, unsigned len,
                                            const GLSL_BINDING_T *appended, unsigned appended_len);
static void            free_binding_array  (GLSL_BINDING_T *ary, unsigned len);

static void write_tf_specs(GLXX_PROGRAM_TFF_POST_LINK_T *tf_post,
                           const GLSL_PROGRAM_T *prog,
                           GLenum buffer_mode);

void gl20_program_common_init(GL20_PROGRAM_COMMON_T *common)
{
   memset(common, 0, sizeof(GL20_PROGRAM_COMMON_T));
}

void gl20_program_common_term(GL20_PROGRAM_COMMON_T *common)
{
   if (common == NULL)
      return;

   glsl_program_free(common->linked_glsl_program);
   glxx_binary_cache_invalidate(&common->cache);
   free(common->uniform_data);
   free(common->ubo_binding_point);
   free(common->ssbo_binding_point);
}

void gl20_program_init(GL20_PROGRAM_T *program, int32_t name)
{
   /* we never re-init a program structure, so all these should be new */
   assert(program->vertex   == NULL);
   assert(program->fragment == NULL);

   assert(program->mh_info == KHRN_MEM_HANDLE_INVALID);
   assert(program->bindings == NULL);

   program->sig                 = SIG_PROGRAM;
   program->refs                = 0;
   program->name                = name;

   program->deleted   = false;
   program->validated = false;
   program->separable = false;

   program->transform_feedback.buffer_mode    = GL_INTERLEAVED_ATTRIBS;
   program->transform_feedback.varying_count  = 0;
   for (int i = 0; i < GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS; ++i) {
      program->transform_feedback.name [i] = NULL;
   }

   gl20_program_common_init(&program->common);

   KHRN_MEM_ASSIGN(program->mh_info, KHRN_MEM_HANDLE_EMPTY_STRING);

   program->debug_label = NULL;
}

void gl20_program_term(void *v, size_t size)
{
   GL20_PROGRAM_T *program = v;

   vcos_unused(size);

   free_binding_array(program->bindings, program->num_bindings);

   gl20_program_common_term(&program->common);

   for (int i = 0; i < GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS; ++i)
      free(program->transform_feedback.name[i]);

   KHRN_MEM_ASSIGN(program->mh_info, KHRN_MEM_HANDLE_INVALID);

   free(program->debug_label);
}

static void free_binding_data(GLSL_BINDING_T *ary, unsigned count) {
   while(count--) {
      free(ary[count].name);
   }
}

static bool copy_binding_data(GLSL_BINDING_T *dest, const GLSL_BINDING_T *src, unsigned count) {
   unsigned i;
   for(i=0;i<count;++i) {
      dest[i].index = src[i].index;
      dest[i].name  = strdup(src[i].name);
      if(!dest[i].name) {
         free_binding_data(dest, i);
         return false;
      }
   }
   return true;
}

static GLSL_BINDING_T *copy_binding_array(const GLSL_BINDING_T *src_ary, unsigned len) {
   GLSL_BINDING_T *ret;
   ret = malloc(len * sizeof(*ret));
   if(!ret) {
      return NULL;
   }
   if(!copy_binding_data(ret, src_ary, len)) {
      free(ret);
      return NULL;
   }
   return ret;
}

static GLSL_BINDING_T *append_binding_array(const GLSL_BINDING_T *ary,
                                            unsigned len,
                                            const GLSL_BINDING_T *appended,
                                            unsigned appended_len) {
   GLSL_BINDING_T *ret;
   ret = malloc((len + appended_len) * sizeof(*ret));
   if(!ret) {
      return NULL;
   }
   if(!copy_binding_data(ret, ary, len)) {
      free(ret);
      return NULL;
   }
   if(!copy_binding_data(ret+len, appended, appended_len)) {
      free_binding_data(ret, len);
      free(ret);
      return NULL;
   }
   return ret;
}

static void free_binding_array(GLSL_BINDING_T *ary, unsigned len) {
   free_binding_data(ary, len);
   free(ary);
}

bool gl20_program_bind_attrib(GL20_PROGRAM_T *program, uint32_t index, const char *name)
{
   /* try to replace an existing binding */
   for (unsigned i = 0; i < program->num_bindings; i++) {
      if (!strcmp(name, program->bindings[i].name)) {
         program->bindings[i].index = index;
         return true;
      }
   }

   /* failing that, make a new binding and add it to the head of the list */
   GLSL_BINDING_T *new_bindings;
   GLSL_BINDING_T new_entry;
   new_entry.index = index;
   new_entry.name  = strdup(name);
   new_bindings    = append_binding_array(program->bindings, program->num_bindings, &new_entry, 1);
   if (new_entry.name == NULL || new_bindings == NULL) {
      free(new_entry.name);
      free_binding_array(new_bindings, program->num_bindings + 1);
      return false; /* Return leaving old bindings intact */
   }

   /* Install the new binding table */
   free_binding_array(program->bindings, program->num_bindings);
   program->bindings     = new_bindings;
   program->num_bindings = program->num_bindings + 1;

   return true;
}

void gl20_program_acquire(GL20_PROGRAM_T *program)
{
   assert(program);
   assert(program->refs >= 0);

   program->refs++;

   assert(program->refs >= 0);
}

void gl20_program_release(GL20_PROGRAM_T *program)
{
   assert(program);
   assert(program->refs > 0);

   program->refs--;
}

void gl20_program_save_error(GL20_PROGRAM_T *program, const char *error) {
   KHRN_MEM_HANDLE_T handle = khrn_mem_strdup(error);

   if (handle != KHRN_MEM_HANDLE_INVALID) {
      KHRN_MEM_ASSIGN(program->mh_info, handle);
      khrn_mem_release(handle);
   }
}

/* Return how many locations are taken up in the default block. Can be padded */
static unsigned default_block_locations(const GLenum gl_type) {
   switch(gl_type) {
   case GL_BOOL:              return 1;
   case GL_BOOL_VEC2:         return 2;
   case GL_BOOL_VEC3:         return 4;
   case GL_BOOL_VEC4:         return 4;
   case GL_FLOAT:             return 1;
   case GL_FLOAT_MAT2:        return 4;
   case GL_FLOAT_MAT2x3:      return 8;
   case GL_FLOAT_MAT2x4:      return 8;
   case GL_FLOAT_MAT3:        return 12;
   case GL_FLOAT_MAT3x2:      return 6;
   case GL_FLOAT_MAT3x4:      return 12;
   case GL_FLOAT_MAT4:        return 16;
   case GL_FLOAT_MAT4x2:      return 8;
   case GL_FLOAT_MAT4x3:      return 16;
   case GL_FLOAT_VEC2:        return 2;
   case GL_FLOAT_VEC3:        return 4;
   case GL_FLOAT_VEC4:        return 4;
   case GL_INT:               return 1;
   case GL_INT_VEC2:          return 2;
   case GL_INT_VEC3:          return 4;
   case GL_INT_VEC4:          return 4;
   case GL_UNSIGNED_INT:      return 1;
   case GL_UNSIGNED_INT_VEC2: return 2;
   case GL_UNSIGNED_INT_VEC3: return 4;
   case GL_UNSIGNED_INT_VEC4: return 4;
   case GL_INT_SAMPLER_1D_BRCM:
   case GL_INT_SAMPLER_1D_ARRAY_BRCM:
   case GL_INT_SAMPLER_2D:
   case GL_INT_SAMPLER_2D_MULTISAMPLE:
   case GL_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
   case GL_INT_SAMPLER_2D_ARRAY:
   case GL_INT_SAMPLER_3D:
   case GL_INT_SAMPLER_CUBE:
   case GL_INT_SAMPLER_CUBE_MAP_ARRAY:
   case GL_INT_SAMPLER_BUFFER:
   case GL_SAMPLER_1D_BRCM:
   case GL_SAMPLER_1D_ARRAY_BRCM:
   case GL_SAMPLER_2D:
   case GL_SAMPLER_2D_MULTISAMPLE:
   case GL_SAMPLER_2D_MULTISAMPLE_ARRAY:
   case GL_SAMPLER_2D_ARRAY:
   case GL_SAMPLER_2D_ARRAY_SHADOW:
   case GL_SAMPLER_2D_SHADOW:
   case GL_SAMPLER_3D:
   case GL_SAMPLER_CUBE:
   case GL_SAMPLER_CUBE_SHADOW:
   case GL_SAMPLER_CUBE_MAP_ARRAY:
   case GL_SAMPLER_CUBE_MAP_ARRAY_SHADOW:
   case GL_SAMPLER_BUFFER:
   case GL_UNSIGNED_INT_SAMPLER_1D_BRCM:
   case GL_UNSIGNED_INT_SAMPLER_1D_ARRAY_BRCM:
   case GL_UNSIGNED_INT_SAMPLER_2D:
   case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE:
   case GL_UNSIGNED_INT_SAMPLER_2D_MULTISAMPLE_ARRAY:
   case GL_UNSIGNED_INT_SAMPLER_2D_ARRAY:
   case GL_UNSIGNED_INT_SAMPLER_3D:
   case GL_UNSIGNED_INT_SAMPLER_CUBE:
   case GL_UNSIGNED_INT_SAMPLER_CUBE_MAP_ARRAY:
   case GL_UNSIGNED_INT_SAMPLER_BUFFER:
   case GL_SAMPLER_EXTERNAL_OES:
   case GL_IMAGE_2D:
   case GL_IMAGE_3D:
   case GL_IMAGE_CUBE:
   case GL_IMAGE_CUBE_MAP_ARRAY:
   case GL_IMAGE_2D_ARRAY:
   case GL_IMAGE_BUFFER:
   case GL_INT_IMAGE_2D:
   case GL_INT_IMAGE_3D:
   case GL_INT_IMAGE_CUBE:
   case GL_INT_IMAGE_CUBE_MAP_ARRAY:
   case GL_INT_IMAGE_2D_ARRAY:
   case GL_INT_IMAGE_BUFFER:
   case GL_UNSIGNED_INT_IMAGE_2D:
   case GL_UNSIGNED_INT_IMAGE_3D:
   case GL_UNSIGNED_INT_IMAGE_CUBE:
   case GL_UNSIGNED_INT_IMAGE_CUBE_MAP_ARRAY:
   case GL_UNSIGNED_INT_IMAGE_2D_ARRAY:
   case GL_UNSIGNED_INT_IMAGE_BUFFER:
      return 1;
   case GL_UNSIGNED_INT_ATOMIC_COUNTER:
      return 0;
   default:
      unreachable();
      return 0;
   }
}

static unsigned get_default_uniform_size(const GLSL_PROGRAM_T *program_ir) {
   if (program_ir->default_uniforms.num_members == 0) return 0;

   unsigned ret = 0;

   /* Scan all the locations and take the max of the offset */
   for (unsigned i=0; i<program_ir->default_uniforms.num_members; i++) {
      const GLSL_BLOCK_MEMBER_T *unif = &program_ir->default_uniforms.members[i];
      if (unif->atomic_idx != -1) continue;     /* Atomic counters don't go in the block */

      unsigned base_offset = program_ir->uniform_offsets[unif->offset];
      unsigned size = unif->array_length * default_block_locations(unif->type);

      ret = GFX_MAX(ret, base_offset + size);
   }
   return ret;
}

static bool save_linked_program_data(GL20_PROGRAM_T *program, GLSL_PROGRAM_T *program_ir)
{
   unsigned  num_scalar_uniforms = get_default_uniform_size(program_ir);
   uint32_t *uniform_data        = calloc(num_scalar_uniforms,                            sizeof(uint32_t));
   unsigned *ubo_binding_point   = calloc(GLXX_CONFIG_MAX_UNIFORM_BUFFER_BINDINGS,        sizeof(unsigned));
   unsigned *ssbo_binding_point  = calloc(GLXX_CONFIG_MAX_SHADER_STORAGE_BUFFER_BINDINGS, sizeof(unsigned));
   if(!uniform_data || !ubo_binding_point || !ssbo_binding_point) {
      free(uniform_data);
      free(ubo_binding_point);
      free(ssbo_binding_point);
      return false;
   }

   GL20_PROGRAM_COMMON_T *common = &program->common;

   free(common->uniform_data);
   free(common->ubo_binding_point);
   free(common->ssbo_binding_point);
   glsl_program_free(common->linked_glsl_program);

   common->linked_glsl_program = program_ir;
   common->uniform_data        = uniform_data;
   common->num_scalar_uniforms = num_scalar_uniforms;
   common->ubo_binding_point   = ubo_binding_point;
   common->ssbo_binding_point  = ssbo_binding_point;

   for (unsigned int i = 0; i < program_ir->num_ubo_bindings; i++) {
      unsigned block_index = program_ir->ubo_binding[i].id;
      common->ubo_binding_point[block_index] = program_ir->ubo_binding[i].binding;
   }

   for (unsigned int i = 0; i < program_ir->num_ssbo_bindings; i++) {
      unsigned block_index = program_ir->ssbo_binding[i].id;
      common->ssbo_binding_point[block_index] = program_ir->ssbo_binding[i].binding;
   }

   for (unsigned int i = 0; i < program_ir->num_sampler_bindings; i++) {
      unsigned loc = program_ir->sampler_binding[i].id;
      common->uniform_data[loc] = program_ir->sampler_binding[i].binding;
   }

   for (unsigned int i = 0; i < program_ir->num_image_bindings; i++) {
      unsigned loc = program_ir->image_binding[i].id;
      common->uniform_data[loc] = program_ir->image_binding[i].binding;
   }

   // Transform feedback
   write_tf_specs(&common->transform_feedback, program_ir, program->transform_feedback.buffer_mode);
   return true;
}

static GLSL_PROGRAM_T *link_compute(GL20_PROGRAM_T *program) {
   /* Assume this is a compute program */
   if (program->vertex != NULL || program->fragment != NULL) {
      gl20_program_save_error(program, "Link Error: Graphics shader cannot be linked with compute shader");
      return NULL;
   }

   if (program->compute->binary == NULL) {
      gl20_program_save_error(program, "Link Error: Compute shader not compiled");
      return NULL;
   }

   return glsl_link_compute_program(program->compute->binary);
}

static GLSL_PROGRAM_T *link_graphics(GL20_PROGRAM_T *program)
{
   assert(program->compute == NULL);

   if (program->vertex && program->vertex->binary == NULL) {
      gl20_program_save_error(program, "Link Error: Vertex shader not compiled");
      return NULL;
   }

#if GLXX_HAS_TNG
   if (program->tess_control && program->tess_control->binary == NULL) {
      gl20_program_save_error(program, "Link Error: Tessellation control shader not compiled");
      return NULL;
   }

   if (program->tess_evaluation && program->tess_evaluation->binary == NULL) {
      gl20_program_save_error(program, "Link Error: Tessellation evaluation shader not compiled");
      return NULL;
   }

   if (program->geometry && program->geometry->binary == NULL) {
      gl20_program_save_error(program, "Link Error: Geometry shader not compiled");
      return NULL;
   }
#endif

   if (program->fragment && program->fragment->binary == NULL) {
      gl20_program_save_error(program, "Link Error: Fragment shader not compiled");
      return NULL;
   }

   if ((program->transform_feedback.buffer_mode == GL_SEPARATE_ATTRIBS  &&
            program->transform_feedback.varying_count > GLXX_CONFIG_MAX_TF_SEPARATE_COMPONENTS) ||
        (program->transform_feedback.buffer_mode == GL_INTERLEAVED_ATTRIBS &&
            program->transform_feedback.varying_count > GLXX_CONFIG_MAX_TF_INTERLEAVED_COMPONENTS))
   {
      gl20_program_save_error(program, "Link Error: Total number of components to capture exceeds allowed limit");
      return NULL;
   }

   GLSL_PROGRAM_T *ret = NULL;

   GLSL_PROGRAM_SOURCE_T source;
   source.name            = program->name;
   source.bindings        = copy_binding_array(program->bindings, program->num_bindings);
   source.num_bindings    = source.bindings ? program->num_bindings : 0;
   source.num_tf_varyings = program->transform_feedback.varying_count;
   source.tf_varyings     = malloc(source.num_tf_varyings * sizeof(const char *));

   if (!source.bindings || !source.tf_varyings) {
      gl20_program_save_error(program, "Link Error: Out of memory");
      goto end;
   }

   for (unsigned i = 0; i < source.num_tf_varyings; i++)
      source.tf_varyings[i] = program->transform_feedback.name[i];

#ifdef KHRN_SHADER_DUMP_SOURCE
   glsl_program_source_dump(&source, program->vertex ? program->vertex->name : 0, program->fragment ? program->fragment->name : 0);
#endif

   CompiledShader *stages[SHADER_FLAVOUR_COUNT] = { 0, };
   if (program->vertex)
      stages[SHADER_VERTEX] = program->vertex->binary;
   if (program->fragment)
      stages[SHADER_FRAGMENT] = program->fragment->binary;
#if GLXX_HAS_TNG
   if (program->tess_control)
      stages[SHADER_TESS_CONTROL] = program->tess_control->binary;
   if (program->tess_evaluation)
      stages[SHADER_TESS_EVALUATION] = program->tess_evaluation->binary;
   if (program->geometry)
      stages[SHADER_GEOMETRY] = program->geometry->binary;
#endif

   ret = glsl_link_program(stages, &source, program->separable);
   if (!ret) {
      /* TODO: This communication by globals is unnecessary */
      gl20_program_save_error(program, glsl_compile_error_get());
      goto end;
   }

end:
   free(source.bindings);
   free((void *)source.tf_varyings);   /* MSVC won't do const ** -> void * without a cast */
   return ret;
}

void gl20_program_link(GL20_PROGRAM_T *program)
{
   KHRN_MEM_ASSIGN(program->mh_info, KHRN_MEM_HANDLE_EMPTY_STRING);

   GLSL_PROGRAM_T *program_ir;
   if (program->compute != NULL) program_ir = link_compute(program);
   else                          program_ir = link_graphics(program);

   program->common.link_status = program_ir != NULL;

   if (!program->common.link_status)
      return;

   program->common.separable = program->separable;

   if (!save_linked_program_data(program, program_ir)) {
      glsl_program_free(program_ir);
      gl20_program_save_error(program, "Link Error: Out of memory");
   }
}

int gl20_is_program(GL20_PROGRAM_T *program)
{
   return program->sig == SIG_PROGRAM;
}

bool gl20_validate_program(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_COMMON_T *program)
{
   vcos_unused(state);

   /*TODO */
   return program->link_status;
}

/**************** TRANSFORM FEEDBACK ************/
/* TODO: Can we make this another file? */

static void write_tf_specs(GLXX_PROGRAM_TFF_POST_LINK_T *tf_post,
                           const GLSL_PROGRAM_T *prog,
                           GLenum buffer_mode)
{
   // In this code, we always assume point size is used, so 0 for gl_Position
   // and 7 for first user varying. gl_PointSize is copied into the user block.
   // During draw calls, 7 and above is decremented by 1 if point size is not used
   // (and 6 is not allowed)

   uint32_t offset = 7; // next tf varying index in VPM shaded vertex format

   tf_post->varying_count = prog->num_tf_captures;
   tf_post->buffer_mode   = buffer_mode;
   tf_post->spec_count    = 0;
   tf_post->addr_count    = 0;
   for (unsigned i = 0; i < V3D_MAX_TF_SPECS; ++i)
   {
      tf_post->spec[i].buffer = 0;
      tf_post->spec[i].count = 0;
      tf_post->spec[i].first = 0;
   }

   for (unsigned i = 0; i < prog->num_tf_captures; ++i)
   {
      /* We don't need to check PointSize because the compiler copies that
       * into the standard varyings block.
       */
      char    *name = prog->tf_capture[i].name;
      bool     builtin_gl_Position = strcmp("gl_Position", name) == 0;
      uint32_t first = builtin_gl_Position ? 0 : offset;
      uint32_t count = glxx_get_element_count(prog->tf_capture[i].type);
      uint32_t buffer;

      count *= prog->tf_capture[i].array_length;

      if (!builtin_gl_Position)
         offset += count;

      if (tf_post->buffer_mode == GL_SEPARATE_ATTRIBS)
         buffer = i;
      else
         buffer = 0;

      tf_post->addr_count = buffer + 1;

      /* XXX Hackily allow continuing where we left off ... */
      int spec_i = tf_post->spec_count;
      if (spec_i > 0 &&
          tf_post->spec[spec_i-1].buffer == buffer &&
          tf_post->spec[spec_i-1].first +
          tf_post->spec[spec_i-1].count   == first )
      {
         first = tf_post->spec[spec_i-1].first;
         count = tf_post->spec[spec_i-1].count + count;
         tf_post->spec_count--;
      }

      while (count > 0) {
         int this_count = count > 16 ? 16 : count;
         V3D_TF_SPEC_T *spec = &tf_post->spec[tf_post->spec_count];

         spec->buffer = buffer;
         spec->first = first;
         spec->count = this_count;
#if V3D_VER_AT_LEAST(4,0,2,0)
         spec->stream = 0;
#endif

         tf_post->spec_count++;

         count -= this_count;
         first += this_count;
      }

      assert(tf_post->spec_count < V3D_MAX_TF_SPECS);
   }
}

GL_API void GL_APIENTRY glGetTransformFeedbackVarying(GLuint program, GLuint index, GLsizei bufSize, GLsizei* length, GLsizei* size, GLenum* type, GLchar* name)
{
   GLXX_SERVER_STATE_T  *state = glxx_lock_server_state_unchanged(OPENGL_ES_3X);
   if (!state)
      return;

   // Section 11.1 of the spec says this is equivalent to:
   const GLenum props[] = { GL_ARRAY_SIZE, GL_TYPE };
   glxx_get_program_resource_name(state, program, GL_TRANSFORM_FEEDBACK_VARYING, index, bufSize, length, name);
   glxx_get_program_resourceiv(state, program, GL_TRANSFORM_FEEDBACK_VARYING, index, 1, &props[0], 1, NULL, size);
   glxx_get_program_resourceiv(state, program, GL_TRANSFORM_FEEDBACK_VARYING, index, 1, &props[1], 1, NULL, (int*)type);

   glxx_unlock_server_state();
}

GL20_PROGRAM_COMMON_T *gl20_program_common_get(const GLXX_SERVER_STATE_T *state)
{
   GL20_PROGRAM_COMMON_T *common = NULL;

   if (state->current_program != NULL) {
      common = &state->current_program->common;
   } else if (state->pipelines.bound != NULL) {
      if (state->pipelines.bound->common_is_compute)
         common = &state->pipelines.bound->stage[COMPUTE_STAGE_COMPUTE].program->common;
      else
         common = &state->pipelines.bound->common;
   }

   assert(common);
   return common;
}

GL20_PROGRAM_T *gl20_get_tf_program(const GLXX_SERVER_STATE_T *state)
{
   if (state->current_program != NULL)
      return state->current_program;

   if (state->pipelines.bound != NULL)
   {
      GLuint prog_name = glxx_pipeline_get_program_name(state, GRAPHICS_STAGE_VERTEX);

      return glxx_shared_get_pobject(state->shared, prog_name);
   }

   assert(0);
   return NULL;
}

static GL20_SHADER_T **get_shader(GL20_PROGRAM_T *program, GL20_SHADER_T *shader)
{
   switch (shader->type) {
   case GL_VERTEX_SHADER:
      return &program->vertex;
   case GL_FRAGMENT_SHADER:
      return &program->fragment;
#if GLXX_HAS_TNG
   case GL_TESS_CONTROL_SHADER:
      return &program->tess_control;
   case GL_TESS_EVALUATION_SHADER:
      return &program->tess_evaluation;
   case GL_GEOMETRY_SHADER:
      return &program->geometry;
#endif
   case GL_COMPUTE_SHADER:
      return &program->compute;
   default:
      unreachable();
   }
}

bool gl20_program_attach_shader(GL20_PROGRAM_T *program, GL20_SHADER_T *shader)
{
   GL20_SHADER_T **pshader = get_shader(program, shader);

   if (*pshader != NULL)
      return false;

   gl20_shader_acquire(shader);
   *pshader = shader;

   return true;
}

bool gl20_program_detach_shader(GL20_PROGRAM_T *program, GL20_SHADER_T *shader)
{
   GL20_SHADER_T **pshader = get_shader(program, shader);

   if (*pshader != shader)
      return false;

   gl20_shader_release(shader);
   *pshader = NULL;

   return true;
}
