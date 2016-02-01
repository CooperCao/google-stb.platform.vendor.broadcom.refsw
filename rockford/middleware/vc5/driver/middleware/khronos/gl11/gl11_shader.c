/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Creates GLES1.1 shaders as dataflow graphs and passes them to the compiler
backend.
=============================================================================*/

#include "interface/khronos/common/khrn_int_math.h"
#include "interface/khronos/glxx/glxx_int_attrib.h"
#include "middleware/khronos/glsl/glsl_dataflow.h"
#include "middleware/khronos/glsl/glsl_ir_program.h"
#include "middleware/khronos/gl11/gl11_shaders.h"
#include "middleware/khronos/gl11/gl11_shader.h"

uint32_t gl11_get_live_attr_set(const GL11_CACHE_KEY_T *shader)
{
   bool points = shader->points;
   bool have_lights = !!(shader->vertex & GL11_LIGHTING);

   uint32_t attribs_live = 0;

   attribs_live |= (1 << GL11_IX_VERTEX);

   if (!have_lights || (shader->vertex & GL11_COLORMAT)) {
      attribs_live |= (1 << GL11_IX_COLOR);
   }
   if (have_lights) {
      attribs_live |= (1 << GL11_IX_NORMAL);
   }
   for (int i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (shader->texture[i] & GL11_TEX_ENABLE)
         attribs_live |= ( 1 << (GL11_IX_TEXTURE_COORD+i) );
   }
   if ( (shader->vertex & GL11_MPAL_M) != 0 ) {
      attribs_live |= (1 << GL11_IX_MATRIX_INDEX);
      attribs_live |= (1 << GL11_IX_MATRIX_WEIGHT);
   }
   if(points) {
      attribs_live |= (1 << GL11_IX_POINT_SIZE);
   }

   return attribs_live;
}

IRShader *gl11_ir_shader_from_nodes(Dataflow **df, int count) {
   IRShader *r = glsl_ir_shader_create();

   r->num_cfg_blocks = 1;
   r->blocks = malloc(sizeof(CFGBlock));

   if (r->blocks == NULL) {
      glsl_ir_shader_free(r);
      return NULL;
   }

   if (!glsl_ir_copy_block(&r->blocks[0], df, count)) {
      glsl_ir_shader_free(r);
      return NULL;
   }
   r->blocks[0].successor_condition = -1;
   r->blocks[0].next_if_true  = -1;
   r->blocks[0].next_if_false = -1;

   r->num_outputs = count;
   r->outputs = malloc(count * sizeof(IROutput));

   if (r->outputs == NULL) {
      glsl_ir_shader_free(r);
      return NULL;
   }

   for (int i=0; i<count; i++) {
      r->outputs[i].block = 0;
      r->outputs[i].output = i;
   }
   return r;
}

/* TODO: This link map is inefficient and hacky */
LinkMap *gl11_link_map_from_bindings(int out_count, int in_count, int unif_count, int *unif_bindings) {
   LinkMap *l = glsl_link_map_alloc(in_count, out_count, unif_count, 0);
   if (l == NULL) return NULL;

   for (int i=0; i<l->num_outs;     i++) l->outs[i]     = i;
   for (int i=0; i<l->num_ins;      i++) l->ins[i]      = i;
   for (int i=0; i<l->num_uniforms; i++) l->uniforms[i] = unif_bindings[i];

   return l;
}

IR_PROGRAM_T *gl11_shader_get_dataflow(const GL11_CACHE_KEY_T *v)
{
   IR_PROGRAM_T *blob = glsl_ir_program_create();
   if(!blob) {
      return NULL;
   }

   if (v->fragment & GL11_FLATSHADE) {
      for (int i=0; i<4; i++) {
         blob->varying[4*GL11_VARYING_COLOR      + i].flat = true;
         blob->varying[4*GL11_VARYING_BACK_COLOR + i].flat = true;
      }
   }

   blob->live_attr_set = gl11_get_live_attr_set(v);

   glsl_dataflow_begin_construction();
   /* TODO: Need a compiler error handler here */

   gl11_get_cvshader(v, &blob->vshader, &blob->vlink_map);
   gl11_get_fshader (v, &blob->fshader, &blob->flink_map);

   glsl_dataflow_end_construction();

   if(!blob->vshader || !blob->vlink_map || !blob->fshader || !blob->flink_map) {
      glsl_ir_program_free(blob);
      return NULL;
   }

   return blob;
}
