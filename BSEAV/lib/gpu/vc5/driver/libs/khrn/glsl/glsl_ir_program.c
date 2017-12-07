/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdlib.h>

#include "glsl_ir_program.h"
#include "libs/util/gfx_util/gfx_util.h"

static void init_vary_map(GLSL_VARY_MAP_T *map) {
   map->n = 0;
}

static void init_varying_info(VARYING_INFO_T *varying) {
   varying->centroid      = false;
   varying->noperspective = false;
   varying->flat          = false;
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

bool glsl_wg_size_requires_barriers(const unsigned wg_size[3]) {
   unsigned wg_n_items = wg_size[0] * wg_size[1] * wg_size[2];
   return wg_n_items > 16 || !gfx_is_power_of_2(wg_n_items);
}

IR_PROGRAM_T *glsl_ir_program_create() {
   IR_PROGRAM_T *ret = malloc(sizeof(*ret));
   if(!ret) return NULL;

   for (int i=0; i<SHADER_FLAVOUR_COUNT; i++) {
      ret->stage[i].ir = NULL;
      ret->stage[i].link_map = NULL;
   }

   ret->live_attr_set = 0;

   ret->tess_vertices   = 0;
   ret->tess_mode       = V3D_CL_TESS_TYPE_INVALID;
   ret->tess_spacing    = V3D_CL_TESS_EDGE_SPACING_INVALID;
   ret->tess_point_mode = false;
   ret->tess_cw         = false;

   ret->max_known_layers = 1;
   ret->gs_out           = V3D_CL_GEOM_PRIM_TYPE_INVALID;
   ret->gs_in            = GS_IN_INVALID;
   ret->gs_n_invocations = 0;
   ret->gs_max_vertices  = 0;

   init_vary_map(&ret->tf_vary_map);

   ret->early_fragment_tests = false;
   ret->abq                  = 0;
   ret->varyings_per_sample  = false;
   for(int i=0; i<V3D_MAX_VARYING_COMPONENTS; i++) {
      init_varying_info(&ret->varying[i]);
   }

   for (int i=0; i<3; i++) ret->cs_wg_size[i] = 0;
   ret->cs_shared_block_size = ~0u;

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
