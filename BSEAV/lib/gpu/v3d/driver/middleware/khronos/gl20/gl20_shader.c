/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/khronos/common/khrn_int_common.h"

#include "middleware/khronos/gl20/gl20_shader.h"
#include "middleware/khronos/gl20/gl20_program.h"
#include "middleware/khronos/glsl/glsl_compiler.h"
#include "middleware/khronos/glsl/glsl_fastmem.h"

#include <string.h>

void gl20_shader_init(GL20_SHADER_T *shader, int32_t name, unsigned type)
{
   assert(shader);

   assert(shader->sourcev == NULL);
   assert(shader->info_log == NULL);

   shader->sig = SIG_SHADER;
   shader->refs = 0;
   shader->name = name;

   shader->deleted = false;
   shader->compiled = false;

   shader->type = type;

   shader->sourcev = NULL;
   shader->sourcec = 0;

   shader->info_log = NULL;
}

static void gl20_shader_free_source(GL20_SHADER_T *shader)
{
   for (unsigned i = 0; i<shader->sourcec; i++)
      free(shader->sourcev[i]);
   free(shader->sourcev);

   shader->sourcev = NULL;
   shader->sourcec = 0;
}

static char *copy_source_string(const char *string, int length)
{
   char *str = NULL;

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

void gl20_shader_term(void *p)
{
   GL20_SHADER_T *shader = p;
   gl20_shader_free_source(shader);
   free(shader->info_log);
   shader->info_log = NULL;
}

void gl20_shader_acquire(GL20_SHADER_T *shader)
{
   assert(shader);
   assert(shader->refs >= 0);

   shader->refs++;

   assert(shader->refs >= 0);
}

void gl20_shader_release(GL20_SHADER_T *shader)
{
   assert(shader);
   assert(shader->refs >= 0);

   shader->refs--;

   assert(shader->refs >= 0);
}

bool gl20_shader_set_source(GL20_SHADER_T *shader, unsigned count,
   const char * const *string, const int *length)
{
   bool out_of_memory = false;

   /* Free any prior source */
   gl20_shader_free_source(shader);

   shader->sourcev = malloc(count * sizeof(char *));
   if (shader->sourcev == NULL) return false;

   shader->sourcec = count;

   for (unsigned i = 0; i < count; i++) {
         shader->sourcev[i] = copy_source_string(string[i], length ? length[i] : -1);

         if (shader->sourcev[i] == NULL)
            out_of_memory = true;
   }

   if (out_of_memory)
      gl20_shader_free_source(shader);
   return !out_of_memory;
}

void gl20_shader_compile(GL20_SHADER_T *shader)
{
   glsl_fastmem_init();

   shader->compiled = false;
   if (!shader->sourcev) goto end;

   free(shader->info_log);
   shader->info_log = NULL;

   shader->compiled = glsl_compile(shader->type == GL_VERTEX_SHADER ? SHADER_VERTEX : SHADER_FRAGMENT, shader->sourcec, (const char **)shader->sourcev);

   if (!shader->compiled)
      shader->info_log = strdup(error_buffer);

end:
   glsl_fastmem_term();
}

int gl20_is_shader(GL20_SHADER_T *shader)
{
   return shader->sig == SIG_SHADER;
}
