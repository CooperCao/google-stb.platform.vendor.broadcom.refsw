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
   return ret;
}

BINARY_PROGRAM_T *glsl_binary_program_from_dataflow(IR_PROGRAM_T                 *ir,
                                                    const GLXX_LINK_RESULT_KEY_T *key)
{
   BINARY_PROGRAM_T *ret = glsl_binary_program_create();
   if(!ret) return NULL;

   ret->fshader = glsl_binary_shader_from_dataflow(SHADER_FRAGMENT, false, &ret->vary_map, ir, key);
   if (ret->fshader == NULL) goto fail;

   ShaderFlavour last_vtx = (ir->stage[SHADER_GEOMETRY].ir) ? SHADER_GEOMETRY : (ir->stage[SHADER_TESS_EVALUATION].ir) ? SHADER_TESS_EVALUATION : SHADER_VERTEX;

   // early out if there are no shaders left
   if (!ir->stage[last_vtx].ir)
      return ret;

   ret->vstages[last_vtx][MODE_BIN]    = glsl_binary_shader_from_dataflow(last_vtx, true,  &ir->tf_vary_map, ir, key);
   ret->vstages[last_vtx][MODE_RENDER] = glsl_binary_shader_from_dataflow(last_vtx, false, &ret->vary_map,   ir, key);
   if (ret->vstages[last_vtx][MODE_BIN] == NULL || ret->vstages[last_vtx][MODE_RENDER] == NULL) goto fail;

   for (int i=last_vtx-1; i >= SHADER_VERTEX; i--) {
      if (ir->stage[i].ir == NULL) continue;

      ret->vstages[i][MODE_BIN]    = glsl_binary_shader_from_dataflow(i, true,  NULL, ir, key);
      ret->vstages[i][MODE_RENDER] = glsl_binary_shader_from_dataflow(i, false, NULL, ir, key);
      if (ret->vstages[i][MODE_BIN] == NULL || ret->vstages[i][MODE_RENDER] == NULL) goto fail;
   }


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
