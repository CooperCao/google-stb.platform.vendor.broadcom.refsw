/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_binary_program.h"

BINARY_PROGRAM_T *glsl_binary_program_create() {
   BINARY_PROGRAM_T *ret = malloc(sizeof(*ret));
   if(!ret) return NULL;

   ret->fshader = NULL;
   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      ret->vstages[i][MODE_BIN]    = NULL;
      ret->vstages[i][MODE_RENDER] = NULL;
   }
   ret->has_point_size = false;
   return ret;
}

BINARY_PROGRAM_T *glsl_binary_program_from_dataflow(IR_PROGRAM_T                 *ir,
                                                    const GLXX_LINK_RESULT_KEY_T *key)
{
   BINARY_PROGRAM_T *ret = glsl_binary_program_create();
   if(!ret) return NULL;

   ret->fshader = glsl_binary_shader_from_dataflow(SHADER_FRAGMENT, false, &ret->vary_map, ir, key);
   if (ret->fshader == NULL) goto fail;

   // early out for compute shaders
   if (!ir->stage[SHADER_VERTEX].ir)
      return ret;

   ret->vstages[SHADER_VERTEX][MODE_BIN]    = glsl_binary_shader_from_dataflow(SHADER_VERTEX, true,  &ir->tf_vary_map, ir, key);
   ret->vstages[SHADER_VERTEX][MODE_RENDER] = glsl_binary_shader_from_dataflow(SHADER_VERTEX, false, &ret->vary_map,   ir, key);
   if (ret->vstages[SHADER_VERTEX][MODE_BIN] == NULL || ret->vstages[SHADER_VERTEX][MODE_RENDER] == NULL) goto fail;

   ret->has_point_size = (key->backend & GLXX_PRIM_M) == GLXX_PRIM_POINT;
   return ret;

fail:
   glsl_binary_program_free(ret);
   return NULL;
}

void glsl_binary_program_free(BINARY_PROGRAM_T *program) {
   if(!program) return;

   glsl_binary_shader_free(program->fshader);
   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      glsl_binary_shader_free(program->vstages[i][MODE_BIN]);
      glsl_binary_shader_free(program->vstages[i][MODE_RENDER]);
   }
   free(program);
}
