/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#include <stdlib.h>

#include "glsl_ir_program.h"

static void init_vary_map(GLSL_VARY_MAP_T *map) {
   map->n = 0;
}

static void init_varying_info(VARYING_INFO_T *varying) {
   varying->centroid = false;
   varying->flat     = false;
}

LinkMap *glsl_link_map_alloc(int num_ins, int num_outs, int num_unifs, int num_buffers) {
   LinkMap *ret = malloc(sizeof(LinkMap));
   if (!ret) return NULL;

   ret->num_ins      = num_ins;
   ret->num_outs     = num_outs;
   ret->num_uniforms = num_unifs;
   ret->num_buffers  = num_buffers;

   ret->ins      = malloc(ret->num_ins      * sizeof(int));
   ret->outs     = malloc(ret->num_outs     * sizeof(int));
   ret->uniforms = malloc(ret->num_uniforms * sizeof(int));
   ret->buffers  = malloc(ret->num_buffers  * sizeof(int));

   if (!ret->ins || !ret->outs || !ret->uniforms || !ret->buffers) {
      glsl_link_map_free(ret);
      return NULL;
   }
   return ret;
}

void glsl_link_map_free(LinkMap *l) {
   if (!l) return;

   free(l->ins);
   free(l->outs);
   free(l->uniforms);
   free(l->buffers);

   free(l);
}

IR_PROGRAM_T *glsl_ir_program_create() {
   IR_PROGRAM_T *ret = malloc(sizeof(*ret));
   if(!ret) return NULL;

   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      ret->stage[i].ir = NULL;
      ret->stage[i].link_map = NULL;
   }

   ret->live_attr_set = 0;
   init_vary_map(&ret->tf_vary_map);

   ret->early_fragment_tests = false;
   for(int i=0; i<V3D_MAX_VARYING_COMPONENTS; i++) {
      init_varying_info(&ret->varying[i]);
   }
   return ret;
}

void glsl_ir_program_free(IR_PROGRAM_T *bin) {
   if(!bin) return;

   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      glsl_ir_shader_free(bin->stage[i].ir);
      glsl_link_map_free(bin->stage[i].link_map);
   }
   free(bin);
}
