/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Implementation of OpenGL ES 2.0 shader structure.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"

#include "middleware/khronos/gl20/gl20_shader.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/glsl/glsl_compiler.h"
#include "middleware/khronos/glsl/glsl_fastmem.h"

void gl20_shader_init(GL20_SHADER_T *shader, int32_t name, GLenum type)
{
   vcos_assert(shader);

   vcos_assert(shader->mh_sources_current == MEM_INVALID_HANDLE);
   vcos_assert(shader->mh_sources_compile == MEM_INVALID_HANDLE);
   vcos_assert(shader->mh_info == MEM_INVALID_HANDLE);

   shader->sig = SIG_SHADER;
   shader->refs = 0;
   shader->name = name;

   shader->deleted = GL_FALSE;
   shader->compiled = GL_FALSE;

   shader->type = type;

   MEM_ASSIGN(shader->mh_sources_current, MEM_ZERO_SIZE_HANDLE);
   MEM_ASSIGN(shader->mh_sources_compile, MEM_ZERO_SIZE_HANDLE);
   MEM_ASSIGN(shader->mh_info, MEM_EMPTY_STRING_HANDLE);
}

void gl20_shader_term(void *v, uint32_t size)
{
   GL20_SHADER_T *shader = (GL20_SHADER_T *)v;

   UNUSED(size);

   MEM_ASSIGN(shader->mh_sources_current, MEM_INVALID_HANDLE);
   MEM_ASSIGN(shader->mh_sources_compile, MEM_INVALID_HANDLE);

   MEM_ASSIGN(shader->mh_info, MEM_INVALID_HANDLE);
}

void gl20_shader_sources_term(void *v, uint32_t size)
{
   MEM_HANDLE_T *base = (MEM_HANDLE_T *)v;

   int i, count = size / sizeof(MEM_HANDLE_T);

   vcos_assert(size % sizeof(MEM_HANDLE_T) == 0);

   for (i = 0; i < count; i++)
      MEM_ASSIGN(base[i], MEM_INVALID_HANDLE);
}

void gl20_shader_acquire(GL20_SHADER_T *shader)
{
   vcos_assert(shader);
   vcos_assert(shader->refs >= 0);

   shader->refs++;

   vcos_assert(shader->refs >= 0);
}

void gl20_shader_release(GL20_SHADER_T *shader)
{
   vcos_assert(shader);
   vcos_assert(shader->refs >= 0);

   shader->refs--;

   vcos_assert(shader->refs >= 0);
}

void gl20_shader_compile(GL20_SHADER_T *shader)
{
   int sourcec;
   const char** sourcev;

   glsl_fastmem_init();

   MEM_ASSIGN(shader->mh_sources_compile, shader->mh_sources_current);

   sourcec = mem_get_size(shader->mh_sources_compile) / sizeof(MEM_HANDLE_T);
   sourcev = (const char **)glsl_fastmem_malloc(sourcec * sizeof(char *), false);

   if (sourcev)
   {
      MEM_HANDLE_T handle;

      lock_sources_for_compiler(sourcec, sourcev, shader->mh_sources_compile);

      MEM_ASSIGN(shader->mh_info, MEM_EMPTY_STRING_HANDLE);

      if (glsl_compile(shader->type == GL_VERTEX_SHADER ? SHADER_VERTEX : SHADER_FRAGMENT, sourcec, sourcev))
         shader->compiled = GL_TRUE;
      else
         shader->compiled = GL_FALSE;

      handle = mem_strdup_ex(error_buffer, MEM_COMPACT_DISCARD);
      if (handle != MEM_INVALID_HANDLE) {
         MEM_ASSIGN(shader->mh_info, handle);
         mem_release(handle);
      }

      unlock_sources_for_compiler(shader->mh_sources_compile);
   }
   else
      shader->compiled = GL_FALSE;

   glsl_fastmem_term();
}

int gl20_is_shader(GL20_SHADER_T *shader)
{
   return shader->sig == SIG_SHADER;
}
