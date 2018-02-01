/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"

#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_shader.h"

#include "middleware/khronos/glsl/glsl_compiler.h"
#include "middleware/khronos/glsl/glsl_fastmem.h"

#include "middleware/khronos/common/khrn_mem.h"

#include <string.h>

void gl20_program_init(GL20_PROGRAM_T *program, int32_t name)
{
   assert(program);

   /*
      we never re-init a program structure, so all these
      should be shiny and new
   */

   assert(program->vertex == NULL);
   assert(program->fragment == NULL);
   assert(program->bindings == NULL);
   assert(program->samplers == NULL);
   assert(program->uniforms == NULL);
   assert(program->uniform_data == NULL);
   assert(program->attributes == NULL);
   assert(program->info_log == NULL);

   program->sig = SIG_PROGRAM;
   program->refs = 0;
   program->name = name;
   program->debug_save_count = 0;

   program->deleted = false;
   program->linked = false;
   program->validated = false;

   program->bindings = NULL;
   program->num_bindings = 0;

   program->attribs_live = 0;

   program->samplers = NULL;
   program->num_samplers = 0;

   program->uniforms = NULL;
   program->num_uniforms = 0;

   program->uniform_data = NULL;
   program->num_uniform_data = 0;

   program->attributes = NULL;
   program->num_attributes = 0;

   program->info_log = NULL;
}

static void free_bindings(GL20_PROGRAM_T *program)
{
   GL20_BINDING_T *base = program->bindings;
   for (unsigned i = 0; i < program->num_bindings; i++)
      free(base[i].name);
   free(program->bindings);
   program->num_bindings = 0;
}

static void free_uniforms(GL20_PROGRAM_T *program)
{
   GL20_UNIFORM_INFO_T *base = program->uniforms;
   for (unsigned i = 0; i < program->num_uniforms; i++)
      free(base[i].name);
   free(program->uniforms);
   program->num_uniforms = 0;
}

static void free_attributes(GL20_PROGRAM_T *program)
{
   GL20_ATTRIB_INFO_T *base = program->attributes;
   for (unsigned i = 0; i < program->num_attributes; i++)
      free(base[i].name);
   free(program->attributes);
   program->num_attributes = 0;
}

void gl20_program_term(void *p)
{
   GL20_PROGRAM_T *program = p;

   KHRN_MEM_ASSIGN(program->vertex, NULL);
   KHRN_MEM_ASSIGN(program->fragment, NULL);

   free_bindings(program);

   gl20_link_result_term(&program->result, 0);

   free(program->samplers);
   program->num_samplers = 0;

   free_uniforms(program);

   free(program->uniform_data);
   program->num_uniform_data = 0;

   free_attributes(program);

   free(program->info_log);
   program->info_log = NULL;
}

bool gl20_program_bind_attrib(GL20_PROGRAM_T *program, unsigned index, const char *name)
{
   GL20_BINDING_T *bindings = program->bindings;
   unsigned count = program->num_bindings;

   /*
      try to replace an existing binding
   */

   for (unsigned i = 0; i < count; i++) {
      int c = strcmp(name, bindings[i].name);
      if (!c) {
         bindings[i].index = index;
         return true;
      }
   }

   /*
      no existing binding, so allocate a new, larger binding table

      - copy the existing bindings across
      - stick the new binding on the end
   */

   GL20_BINDING_T *new_bindings = malloc((count + 1) * sizeof(GL20_BINDING_T));
   if (new_bindings == NULL)
      return false;

   for (unsigned i = 0; i < count; i++) {
      new_bindings[i].index = bindings[i].index;
      new_bindings[i].name = strdup(bindings[i].name);
   }

   new_bindings[count].index = index;
   new_bindings[count].name = strdup(name);

   /*
      install the new binding table
   */

   free_bindings(program);
   program->bindings = new_bindings;
   program->num_bindings = count + 1;

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

void gl20_program_link(GL20_PROGRAM_T *program)
{
   glsl_fastmem_init();

   program->linked = false;

   free(program->info_log);

   if ((program->vertex == NULL) ||
       (program->fragment == NULL)) goto end;

   GL20_SHADER_T *vertex = program->vertex;
   GL20_SHADER_T *fragment = program->fragment;

   if (!vertex->compiled || !fragment->compiled) {
      goto end;
   }

   slang_program *p = (slang_program *)glsl_fastmem_malloc(sizeof(slang_program), false);

   if (!p)
      goto end;

   p->vshader.sourcec = vertex->sourcec;
   p->vshader.sourcev = (const char **)vertex->sourcev;

   p->fshader.sourcec = fragment->sourcec;
   p->fshader.sourcev = (const char **)fragment->sourcev;

   p->num_bindings = program->num_bindings;
   p->bindings = (slang_binding *)program->bindings;
   p->result = &program->result;

   if (!p->vshader.sourcev || !p->fshader.sourcev)
      goto end;

   if (glsl_compile_and_link(p)) {

      GL20_SAMPLER_INFO_T *samplers = malloc(p->num_samplers * sizeof(GL20_SAMPLER_INFO_T));
      GL20_UNIFORM_INFO_T *uniforms = malloc(p->num_uniforms * sizeof(GL20_UNIFORM_INFO_T));
      void *uniform_data = calloc(p->num_scalar_uniforms, sizeof(uint32_t));
      GL20_ATTRIB_INFO_T *attributes = malloc(p->num_attributes * sizeof(GL20_ATTRIB_INFO_T));

      bool out_of_memory = samplers == NULL || uniforms == NULL || uniform_data == NULL || attributes == NULL;

      if (!out_of_memory) {
         /*
            copy the attribute liveness mask
         */

         program->attribs_live = p->live_attributes;

         /*
            copy the samplers
         */

         for (unsigned i = 0; i < p->num_samplers; i++) {
            samplers[i].uniform = p->samplers[i].uniform;
            samplers[i].index = p->samplers[i].array_index;
            samplers[i].in_vshader = p->samplers[i].in_vshader;
         }
         free(program->samplers);
         program->samplers = samplers;
         program->num_samplers = p->num_samplers;

         /*
            copy the uniforms
         */

         for (unsigned i = 0; i < p->num_uniforms; i++) {
            uniforms[i].offset = p->uniforms[i].row;
            uniforms[i].size = p->uniforms[i].array_length;
            uniforms[i].type = p->uniforms[i].type;
            uniforms[i].is_array = p->uniforms[i].is_array;
            uniforms[i].name = strdup(p->uniforms[i].name);
         }
         free_uniforms(program);
         program->uniforms = uniforms;
         program->num_uniforms = p->num_uniforms;

         /*
            allocate the uniform data
         */
         free(program->uniform_data);
         program->uniform_data = uniform_data;
         program->num_uniform_data = p->num_scalar_uniforms;

         /*
            copy the attributes
         */

         for (unsigned i = 0; i < p->num_attributes; i++) {
            attributes[i].offset = p->attributes[i].row;
            attributes[i].type = p->attributes[i].type;
            attributes[i].name = strdup(p->attributes[i].name);
         }
         free_attributes(program);
         program->attributes = attributes;
         program->num_attributes = p->num_attributes;

         program->linked = true;

         /* shader cache entry will be invalidated */
         for (unsigned i = 0; i < GL20_LINK_RESULT_CACHE_SIZE; i++)
         {
            program->result.cache[i].used   = false;
            program->result.cache[i].failed = false;
         }
      }
   }

   if (program->linked)
      program->info_log = strdup(error_buffer);

end:

   glsl_fastmem_term();
}

int gl20_is_program(GL20_PROGRAM_T *program)
{
   return program->sig == SIG_PROGRAM;
}

GLboolean gl20_validate_program(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *program)
{
   UNUSED(state);

   /*TODO */
   if (!program->linked) return 0;

   return 1;
}

GLboolean gl20_validate_current_program(GLXX_SERVER_STATE_T *state)
{
   GLboolean result = GL_TRUE;

   if(!IS_GL_11(state)) {
      if (state->program == NULL)
         return GL_FALSE;   //TODO what should we return here? Does it matter?

      result = gl20_validate_program(state, state->program);
   }
   return result;
}
