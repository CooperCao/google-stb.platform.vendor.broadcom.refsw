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
#include "middleware/khronos/glsl/glsl_errors.h"

static void gl20_shader_free_source(GL20_SHADER_T *shader)
{
   for (int i=0; i<shader->sourcec; i++) free(shader->source[i]);
   free(shader->source);

   shader->source = NULL;
   shader->sourcec = 0;
}

static char *copy_source_string(const char *string, int length)
{
   char *str;

   if (string == NULL) string = "";

   if (length < 0)
      str = strdup(string);
   else {
      str = malloc(length + 1);

      if (str != NULL) {
         memcpy(str, string, length);
         str[length] = '\0';
      }
   }

   return str;
}

/** Public functions **/

void gl20_shader_init(GL20_SHADER_T *shader, int32_t name, GLenum type)
{
   assert(shader);

   shader->sig = SIG_SHADER;
   shader->refs = 0;
   shader->name = name;

   shader->deleted = GL_FALSE;

   shader->type = type;

   shader->sourcec = 0;
   shader->source = NULL;

   shader->info_log = NULL;

   shader->binary = NULL;

   shader->debug_label = NULL;
}

void gl20_shader_term(void *v, size_t size)
{
   GL20_SHADER_T *shader = v;

   UNUSED(size);

   gl20_shader_free_source(shader);

   free(shader->info_log);
   glsl_compiled_shader_free(shader->binary);

   free(shader->debug_label);
   shader->debug_label = NULL;
}

bool gl20_shader_set_source(GL20_SHADER_T *shader, unsigned count,
                            const char * const *string, const int *length)
{
   bool out_of_memory = false;

   /* Free any prior source */
   gl20_shader_free_source(shader);

   shader->source = malloc(count * sizeof(char *));
   if (shader->source == NULL) return false;

   shader->sourcec = count;

   for (unsigned i = 0; i < count; i++) {
      shader->source[i] = copy_source_string(string[i], length ? length[i] : -1);

      if (shader->source[i] == NULL) out_of_memory = true;
   }

   if (out_of_memory) gl20_shader_free_source(shader);
   return !out_of_memory;
}

void gl20_shader_acquire(GL20_SHADER_T *shader)
{
   assert(shader->refs >= 0);
   shader->refs++;
   assert(shader->refs >  0);
}

void gl20_shader_release(GL20_SHADER_T *shader)
{
   assert(shader->refs > 0);
   shader->refs--;
}

static ShaderFlavour get_shader_flavour(GLenum gl_type) {
   switch (gl_type) {
      case GL_VERTEX_SHADER:   return SHADER_VERTEX;
      case GL_FRAGMENT_SHADER: return SHADER_FRAGMENT;
      case GL_COMPUTE_SHADER:  return SHADER_COMPUTE;
   }
   unreachable();
   return 0;
}

void gl20_shader_compile(GL20_SHADER_T *shader)
{
   GLSL_SHADER_SOURCE_T source;

   source.sourcec = shader->sourcec;
   source.sourcev = (const char *const*)shader->source;
   source.name = shader->name;

   shader->binary = glsl_compile_shader(get_shader_flavour(shader->type), &source);

   free(shader->info_log);
   if (shader->binary != NULL)
      shader->info_log = strdup("");
   else
      shader->info_log = strdup(glsl_compile_error_get());
}

int gl20_is_shader(GL20_SHADER_T *shader)
{
   return shader->sig == SIG_SHADER;
}
