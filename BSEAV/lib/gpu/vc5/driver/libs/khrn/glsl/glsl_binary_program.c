/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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

BINARY_PROGRAM_T *glsl_binary_program_from_dataflow(IR_PROGRAM_T                  *ir,
                                                    const struct glsl_backend_cfg *key)
{
   BINARY_PROGRAM_T *ret = glsl_binary_program_create();
   if(!ret) return NULL;

   if (!glsl_binary_shader_from_dataflow(&ret->fshader, SHADER_FRAGMENT, false, &ret->vary_map, ir, key))
      goto fail;

   ShaderFlavour last_vtx = (ir->stage[SHADER_GEOMETRY].ir) ? SHADER_GEOMETRY : (ir->stage[SHADER_TESS_EVALUATION].ir) ? SHADER_TESS_EVALUATION : SHADER_VERTEX;

   // early out if there are no shaders left
   if (!ir->stage[last_vtx].ir)
      return ret;

   if (  !glsl_binary_shader_from_dataflow(&ret->vstages[last_vtx][MODE_BIN], last_vtx, true,  &ir->tf_vary_map, ir, key)
      || !glsl_binary_shader_from_dataflow(&ret->vstages[last_vtx][MODE_RENDER], last_vtx, false, &ret->vary_map, ir, key)  )
   {
      goto fail;
   }

   for (int i=last_vtx-1; i >= SHADER_VERTEX; i--) {
      if (ir->stage[i].ir == NULL) continue;

      if (  !glsl_binary_shader_from_dataflow(&ret->vstages[i][MODE_BIN], i, true,  NULL, ir, key)
         || !glsl_binary_shader_from_dataflow(&ret->vstages[i][MODE_RENDER], i, false, NULL, ir, key)  )
      {
         goto fail;
      }
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
