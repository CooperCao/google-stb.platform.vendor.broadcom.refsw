/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../glxx/glxx_int_attrib.h"
#include "../glsl/glsl_dataflow.h"
#include "../glsl/glsl_ir_program.h"
#include "gl11_shader.h"

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

IRShader *gl11_ir_shader_from_nodes(Dataflow **df, int count, int *out_bindings) {
   IRShader *r = glsl_ir_shader_create();

   r->num_cfg_blocks = 1;
   r->blocks = malloc(sizeof(CFGBlock));

   if (r->blocks == NULL) {
      glsl_ir_shader_free(r);
      return NULL;
   }

   Dataflow **df_squashed = malloc(count * sizeof(Dataflow *));
   if (df_squashed == NULL) {
      glsl_ir_shader_free(r);
      return NULL;
   }

   int n_outputs = 0;
   for (int i=0; i<count; i++) {
      if (df[i] == NULL)
         out_bindings[i] = -1;
      else {
         df_squashed[n_outputs] = df[i];
         out_bindings[i] = n_outputs++;
      }
   }

   if (!glsl_ir_copy_block(&r->blocks[0], df_squashed, n_outputs)) {
      free(df_squashed);
      glsl_ir_shader_free(r);
      return NULL;
   }
   free(df_squashed);

   r->blocks[0].successor_condition = -1;
   r->blocks[0].next_if_true  = -1;
   r->blocks[0].next_if_false = -1;
   r->blocks[0].barrier = false;

   r->num_outputs = n_outputs;
   r->outputs = malloc(n_outputs * sizeof(IROutput));

   if (r->outputs == NULL) {
      glsl_ir_shader_free(r);
      return NULL;
   }

   for (int i=0; i<n_outputs; i++) {
      r->outputs[i].block = 0;
      r->outputs[i].output = i;
   }

   return r;
}

LinkMap *gl11_link_map_from_bindings(int out_count, const int *out_bindings, int in_count, int unif_count, const int *unif_bindings) {
   LinkMap *l = glsl_link_map_alloc(in_count, out_count, unif_count, 0);
   if (l == NULL) return NULL;

   for (int i=0; i<l->num_outs;     i++) l->outs[i]     = out_bindings[i];
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

   blob->early_fragment_tests = true;
   blob->live_attr_set = gl11_get_live_attr_set(v);

   glsl_dataflow_begin_construction();
   /* TODO: Need a compiler error handler here */

   gl11_get_cvshader(v, &blob->stage[SHADER_VERTEX].ir, &blob->stage[SHADER_VERTEX].link_map);
   gl11_get_fshader (v, &blob->stage[SHADER_FRAGMENT].ir, &blob->stage[SHADER_FRAGMENT].link_map);

   glsl_dataflow_end_construction();

   if(!blob->stage[SHADER_VERTEX].ir   || !blob->stage[SHADER_VERTEX].link_map ||
      !blob->stage[SHADER_FRAGMENT].ir || !blob->stage[SHADER_FRAGMENT].link_map)
   {
      glsl_ir_program_free(blob);
      return NULL;
   }

   return blob;
}
