/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of OpenGL ES 2.0 program structure.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/gl20/gl20_shader.h"

#include "middleware/khronos/glsl/glsl_compiler.h"
#include "middleware/khronos/glsl/glsl_fastmem.h"

#include <string.h>

void gl20_program_init(GL20_PROGRAM_T *program, int32_t name)
{
   vcos_assert(program);

   /*
      we never re-init a program structure, so all these
      should be shiny and new
   */

   vcos_assert(program->mh_vertex == MEM_INVALID_HANDLE);
   vcos_assert(program->mh_fragment == MEM_INVALID_HANDLE);
   vcos_assert(program->mh_bindings == MEM_INVALID_HANDLE);
   vcos_assert(program->mh_sampler_info == MEM_INVALID_HANDLE);
   vcos_assert(program->mh_uniform_info == MEM_INVALID_HANDLE);
   vcos_assert(program->mh_uniform_data == MEM_INVALID_HANDLE);
   vcos_assert(program->mh_attrib_info == MEM_INVALID_HANDLE);
   vcos_assert(program->mh_info == MEM_INVALID_HANDLE);

   program->sig = SIG_PROGRAM;
   program->refs = 0;
   program->name = name;
   program->debug_save_count = 0;

   program->deleted = GL_FALSE;
   program->linked = GL_FALSE;
   program->validated = GL_FALSE;

   MEM_ASSIGN(program->mh_bindings, MEM_ZERO_SIZE_HANDLE);

   program->attribs_live = 0;

   MEM_ASSIGN(program->mh_sampler_info, MEM_ZERO_SIZE_HANDLE);
   MEM_ASSIGN(program->mh_uniform_info, MEM_ZERO_SIZE_HANDLE);
   MEM_ASSIGN(program->mh_attrib_info, MEM_ZERO_SIZE_HANDLE);
   MEM_ASSIGN(program->mh_info, MEM_EMPTY_STRING_HANDLE);
}

void gl20_bindings_term(void *v, uint32_t size)
{
   GL20_BINDING_T *base = (GL20_BINDING_T *)v;

   int i, count = size / sizeof(GL20_BINDING_T);

   vcos_assert(size % sizeof(GL20_BINDING_T) == 0);

   for (i = 0; i < count; i++)
      MEM_ASSIGN(base[i].mh_name, MEM_INVALID_HANDLE);
}

void gl20_uniform_info_term(void *v, uint32_t size)
{
   GL20_UNIFORM_INFO_T *base = (GL20_UNIFORM_INFO_T *)v;

   int i, count = size / sizeof(GL20_UNIFORM_INFO_T);

   vcos_assert(size % sizeof(GL20_UNIFORM_INFO_T) == 0);

   for (i = 0; i < count; i++)
      MEM_ASSIGN(base[i].mh_name, MEM_INVALID_HANDLE);
}

void gl20_attrib_info_term(void *v, uint32_t size)
{
   GL20_ATTRIB_INFO_T *base = (GL20_ATTRIB_INFO_T *)v;

   int i, count = size / sizeof(GL20_ATTRIB_INFO_T);

   vcos_assert(size % sizeof(GL20_ATTRIB_INFO_T) == 0);

   for (i = 0; i < count; i++)
      MEM_ASSIGN(base[i].mh_name, MEM_INVALID_HANDLE);
}

void gl20_program_term(void *v, uint32_t size)
{
   GL20_PROGRAM_T *program = (GL20_PROGRAM_T *)v;

   UNUSED(size);

   MEM_ASSIGN(program->mh_vertex, MEM_INVALID_HANDLE);
   MEM_ASSIGN(program->mh_fragment, MEM_INVALID_HANDLE);
   MEM_ASSIGN(program->mh_bindings, MEM_INVALID_HANDLE);

   gl20_link_result_term(&program->result, 0);

   MEM_ASSIGN(program->mh_sampler_info, MEM_INVALID_HANDLE);
   MEM_ASSIGN(program->mh_uniform_info, MEM_INVALID_HANDLE);
   MEM_ASSIGN(program->mh_uniform_data, MEM_INVALID_HANDLE);
   MEM_ASSIGN(program->mh_attrib_info, MEM_INVALID_HANDLE);
   MEM_ASSIGN(program->mh_info, MEM_INVALID_HANDLE);
}

bool gl20_program_bind_attrib(GL20_PROGRAM_T *program, uint32_t index, const char *name)
{
   MEM_HANDLE_T new_handle, name_handle;
   GL20_BINDING_T *new_bindings;
   GL20_BINDING_T *bindings = (GL20_BINDING_T *)mem_lock(program->mh_bindings, NULL);
   int count = mem_get_size(program->mh_bindings) / sizeof(GL20_BINDING_T);
   int i;

   /*
      try to replace an existing binding
   */

   for (i = 0; i < count; i++) {
      int c = strcmp(name, (char *)mem_lock(bindings[i].mh_name, NULL));
      mem_unlock(bindings[i].mh_name);

      if (!c) {
         bindings[i].index = index;
         mem_unlock(program->mh_bindings);
         return true;
      }
   }

   /*
      no existing binding, so allocate a new, larger binding table

      - copy the existing bindings across
      - stick the new binding on the end
   */

   new_handle = mem_alloc_ex((count + 1) * sizeof(GL20_BINDING_T), 4, MEM_FLAG_NONE, "GL20_PROGRAM_T.bindings", MEM_COMPACT_DISCARD);       // check, gl20_bindings_term
   if (new_handle == MEM_INVALID_HANDLE)
      return false;

   name_handle = mem_strdup_ex(name, MEM_COMPACT_DISCARD);                                                                                  // check
   if (name_handle == MEM_INVALID_HANDLE) {
      mem_release(new_handle);
      return false;
   }

   mem_set_term(new_handle, gl20_bindings_term);

   new_bindings = (GL20_BINDING_T *)mem_lock(new_handle, NULL);

   for (i = 0; i < count; i++) {
      new_bindings[i].index = bindings[i].index;
      MEM_ASSIGN(new_bindings[i].mh_name, bindings[i].mh_name);
   }

   mem_unlock(program->mh_bindings);

   new_bindings[count].index = index;
   MEM_ASSIGN(new_bindings[count].mh_name, name_handle);
   mem_release(name_handle);

   mem_unlock(new_handle);

   /*
      install the new binding table
   */

   MEM_ASSIGN(program->mh_bindings, new_handle);
   mem_release(new_handle);

   return true;
}

void gl20_program_acquire(GL20_PROGRAM_T *program)
{
   vcos_assert(program);
   vcos_assert(program->refs >= 0);

   program->refs++;

   vcos_assert(program->refs >= 0);
}

void gl20_program_release(GL20_PROGRAM_T *program)
{
   vcos_assert(program);
   vcos_assert(program->refs > 0);

   program->refs--;
}

void lock_sources_for_compiler(int count, const char **result, MEM_HANDLE_T hsources)
{
   int i;
   MEM_HANDLE_T *handles = (MEM_HANDLE_T *)mem_lock(hsources, NULL);

   for (i = 0; i < count; i++)
      result[i] = (char *)mem_lock(handles[i], NULL);

   mem_unlock(hsources);
}

void unlock_sources_for_compiler(MEM_HANDLE_T hsources)
{
   int i;
   MEM_HANDLE_T *handles = (MEM_HANDLE_T *)mem_lock(hsources, NULL);
   int count = mem_get_size(hsources) / sizeof(MEM_HANDLE_T);

   for (i = 0; i < count; i++)
      mem_unlock(handles[i]);

   mem_unlock(hsources);
}

static void lock_bindings_for_compiler(int count, slang_binding *result, MEM_HANDLE_T hbindings)
{
   int i;
   GL20_BINDING_T *bindings = (GL20_BINDING_T *)mem_lock(hbindings, NULL);

   for (i = 0; i < count; i++) {
      result[i].index = bindings[i].index;
      result[i].name = (char *)mem_lock(bindings[i].mh_name, NULL);
   }

   mem_unlock(hbindings);
}

static void unlock_bindings_for_compiler(MEM_HANDLE_T hbindings)
{
   int i;
   GL20_BINDING_T *bindings = (GL20_BINDING_T *)mem_lock(hbindings, NULL);
   int count = mem_get_size(hbindings) / sizeof(GL20_BINDING_T);

   for (i = 0; i < count; i++)
      mem_unlock(bindings[i].mh_name);

   mem_unlock(hbindings);
}

void gl20_program_link(GLXX_SERVER_STATE_T *state, GL20_PROGRAM_T *program)
{
   glsl_fastmem_init();

   program->linked = GL_FALSE;

   MEM_ASSIGN(program->mh_info, MEM_EMPTY_STRING_HANDLE);

   if (program->mh_vertex != MEM_INVALID_HANDLE) {
      GL20_SHADER_T *vertex = (GL20_SHADER_T *)mem_lock(program->mh_vertex, NULL);

      if (program->mh_fragment != MEM_INVALID_HANDLE) {
         GL20_SHADER_T *fragment = (GL20_SHADER_T *)mem_lock(program->mh_fragment, NULL);

         if (vertex->compiled && fragment->compiled) {
            slang_program *p = (slang_program *)glsl_fastmem_malloc(sizeof(slang_program), false);

            if (p) {
               p->vshader.sourcec = mem_get_size(vertex->mh_sources_compile) / sizeof(MEM_HANDLE_T);
               p->vshader.sourcev = (const char **)glsl_fastmem_malloc(p->vshader.sourcec * sizeof(char *), false);

               p->fshader.sourcec = mem_get_size(fragment->mh_sources_compile) / sizeof(MEM_HANDLE_T);
               p->fshader.sourcev = (const char **)glsl_fastmem_malloc(p->fshader.sourcec * sizeof(char *), false);

               p->num_bindings = mem_get_size(program->mh_bindings) / sizeof(GL20_BINDING_T);
               p->bindings = (slang_binding *)glsl_fastmem_malloc(p->num_bindings * sizeof(slang_binding), false);
               p->result = &program->result;

               if (p->vshader.sourcev && p->fshader.sourcev && p->bindings) {
                  /*
                     lock shader sources
                  */

                  lock_sources_for_compiler(p->vshader.sourcec, p->vshader.sourcev, vertex->mh_sources_compile);
                  lock_sources_for_compiler(p->fshader.sourcec, p->fshader.sourcev, fragment->mh_sources_compile);

                  /*
                     copy attribute bindings, leaving locks in place so we can use the name strings
                  */

                  lock_bindings_for_compiler(p->num_bindings, p->bindings, program->mh_bindings);

                  if (glsl_compile_and_link(p)) {
                     unsigned int i;

                     MEM_HANDLE_T hsampler_info = mem_alloc_ex(p->num_samplers * sizeof(GL20_SAMPLER_INFO_T), 4, MEM_FLAG_NONE, "GL20_PROGRAM_T.sampler_info", MEM_COMPACT_DISCARD);      // check, no term
                     MEM_HANDLE_T huniform_info = mem_alloc_ex(p->num_uniforms * sizeof(GL20_UNIFORM_INFO_T), 4, MEM_FLAG_NONE, "GL20_PROGRAM_T.uniform_info", MEM_COMPACT_DISCARD);      // check, gl20_uniform_info_term
                     MEM_HANDLE_T huniform_data = mem_alloc_ex(p->num_scalar_uniforms * 4, 4, MEM_FLAG_ZERO, "GL20_PROGRAM_T.uniform_data", MEM_COMPACT_DISCARD);                         // check, no term
                     MEM_HANDLE_T hattrib_info = mem_alloc_ex(p->num_attributes * sizeof(GL20_ATTRIB_INFO_T), 4, MEM_FLAG_NONE, "GL20_PROGRAM_T.attrib_info", MEM_COMPACT_DISCARD);       // check, gl20_attrib_info_term

                     bool out_of_memory = hsampler_info == MEM_INVALID_HANDLE ||
                                          huniform_info == MEM_INVALID_HANDLE ||
                                          huniform_data == MEM_INVALID_HANDLE ||
                                          hattrib_info == MEM_INVALID_HANDLE;

                     for (i = 0; i < p->num_uniforms; i++) {
                        p->uniforms[i].u.handle = mem_strdup_ex(p->uniforms[i].u.name, MEM_COMPACT_DISCARD);                              // check
                        out_of_memory = out_of_memory || p->uniforms[i].u.handle == MEM_INVALID_HANDLE;
                     }

                     for (i = 0; i < p->num_attributes; i++) {
                        p->attributes[i].u.handle = mem_strdup_ex(p->attributes[i].u.name, MEM_COMPACT_DISCARD);                          // check
                        out_of_memory = out_of_memory || p->attributes[i].u.handle == MEM_INVALID_HANDLE;
                     }

                     if (!out_of_memory) {
                        GL20_SAMPLER_INFO_T *samplers;
                        GL20_UNIFORM_INFO_T *uniforms;
                        GL20_ATTRIB_INFO_T *attribs;

                        /*
                           copy the attribute liveness mask
                        */

                        program->attribs_live = p->live_attributes;

                        /*
                           copy the samplers
                        */

                        MEM_ASSIGN(program->mh_sampler_info, hsampler_info);

                        samplers = (GL20_SAMPLER_INFO_T *)mem_lock(program->mh_sampler_info, NULL);

                        for (i = 0; i < p->num_samplers; i++) {
                           samplers[i].uniform = p->samplers[i].uniform;
                           samplers[i].index = p->samplers[i].array_index;
                           samplers[i].in_vshader = p->samplers[i].in_vshader;
                        }

                        mem_unlock(program->mh_sampler_info);

                        /*
                           copy the uniforms
                        */

                        mem_set_term(huniform_info, gl20_uniform_info_term);
                        MEM_ASSIGN(program->mh_uniform_info, huniform_info);

                        uniforms = (GL20_UNIFORM_INFO_T *)mem_lock(program->mh_uniform_info, NULL);

                        for (i = 0; i < p->num_uniforms; i++) {
                           vcos_assert(uniforms[i].mh_name == MEM_INVALID_HANDLE);

                           uniforms[i].offset = p->uniforms[i].row;
                           uniforms[i].size = p->uniforms[i].array_length;
                           uniforms[i].type = p->uniforms[i].type;
                           uniforms[i].is_array = p->uniforms[i].is_array;
                           MEM_ASSIGN(uniforms[i].mh_name, p->uniforms[i].u.handle);
                        }

                        mem_unlock(program->mh_uniform_info);

                        /*
                           allocate the uniform data
                        */

                        MEM_ASSIGN(program->mh_uniform_data, huniform_data);

                        /*
                           copy the attributes
                        */

                        mem_set_term(hattrib_info, gl20_attrib_info_term);
                        MEM_ASSIGN(program->mh_attrib_info, hattrib_info);

                        attribs = (GL20_ATTRIB_INFO_T *)mem_lock(program->mh_attrib_info, NULL);

                        for (i = 0; i < p->num_attributes; i++) {
                           vcos_assert(attribs[i].mh_name == MEM_INVALID_HANDLE);

                           attribs[i].offset = p->attributes[i].row;
                           attribs[i].type = p->attributes[i].type;

                           MEM_ASSIGN(attribs[i].mh_name, p->attributes[i].u.handle);
                        }

                        mem_unlock(program->mh_attrib_info);

                        program->linked = GL_TRUE;

                        /* shader cache entry will be invalidated */
                        for (i = 0; i < GL20_LINK_RESULT_CACHE_SIZE; i++)
                        {
                           program->result.cache[i].used   = false;
                           program->result.cache[i].failed = false;
                        }
                     }

                     if (hsampler_info != MEM_INVALID_HANDLE)
                        mem_release(hsampler_info);
                     if (huniform_info != MEM_INVALID_HANDLE)
                        mem_release(huniform_info);
                     if (huniform_data != MEM_INVALID_HANDLE)
                        mem_release(huniform_data);
                     if (hattrib_info != MEM_INVALID_HANDLE)
                        mem_release(hattrib_info);

                     for (i = 0; i < p->num_uniforms; i++)
                        mem_release(p->uniforms[i].u.handle);

                     for (i = 0; i < p->num_attributes; i++)
                        mem_release(p->attributes[i].u.handle);
                  } else {
                     MEM_HANDLE_T handle = mem_strdup_ex(error_buffer, MEM_COMPACT_DISCARD);

                     if (handle != MEM_INVALID_HANDLE) {
                        MEM_ASSIGN(program->mh_info, handle);
                        mem_release(handle);
                     }
                  }

                  /*
                     unlock attribute bindings
                  */

                  unlock_bindings_for_compiler(program->mh_bindings);

                  /*
                     unlock shader sources
                  */

                  unlock_sources_for_compiler(vertex->mh_sources_compile);
                  unlock_sources_for_compiler(fragment->mh_sources_compile);
               }
            }
         }

         mem_unlock(program->mh_fragment);
      }

      mem_unlock(program->mh_vertex);
   }

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
   GL20_PROGRAM_T *program;
   GLboolean result = GL_TRUE;

   if(!IS_GL_11(state)) {
      if (state->mh_program == MEM_INVALID_HANDLE)
         return GL_FALSE;   //TODO what should we return here? Does it matter?

      program = (GL20_PROGRAM_T *)mem_lock(state->mh_program, NULL);

      result = gl20_validate_program(state, program);

      mem_unlock(state->mh_program);
   }
   return result;
}
