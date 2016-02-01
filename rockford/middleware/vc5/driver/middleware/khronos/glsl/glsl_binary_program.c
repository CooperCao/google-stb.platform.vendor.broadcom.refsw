/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "glsl_binary_program.h"

BINARY_PROGRAM_T *glsl_binary_program_create() {
   BINARY_PROGRAM_T *ret = malloc(sizeof(*ret));
   if(!ret) return NULL;

   ret->fshader        = NULL;
   ret->cshader        = NULL;
   ret->vshader        = NULL;
   ret->has_point_size = false;
   return ret;
}

BINARY_PROGRAM_T *glsl_binary_program_from_dataflow(IR_PROGRAM_T                 *ir,
                                                    const GLXX_LINK_RESULT_KEY_T *key,
                                                    int v3d_version)
{
   BINARY_PROGRAM_T *ret = glsl_binary_program_create();
   if(!ret) return NULL;

   ret->fshader = glsl_binary_shader_from_dataflow(BINARY_SHADER_FRAGMENT,   &ret->vary_map,   ir, key, v3d_version);
   if (ret->fshader == NULL) goto fail;

   // early out for compute shaders
   if (!ir->vshader)
      return ret;

   ret->cshader = glsl_binary_shader_from_dataflow(BINARY_SHADER_COORDINATE, &ir->tf_vary_map, ir, key, v3d_version);
   ret->vshader = glsl_binary_shader_from_dataflow(BINARY_SHADER_VERTEX,     &ret->vary_map,   ir, key, v3d_version);
   if (ret->cshader == NULL || ret->vshader == NULL) goto fail;

   ret->has_point_size = (key->backend & GLXX_PRIM_M) == GLXX_PRIM_POINT;
   return ret;

fail:
   glsl_binary_program_free(ret);
   return NULL;
}

void glsl_binary_program_free(BINARY_PROGRAM_T *program) {
   if(!program) return;

   glsl_binary_shader_free(program->fshader);
   glsl_binary_shader_free(program->cshader);
   glsl_binary_shader_free(program->vshader);
   free(program);
}
