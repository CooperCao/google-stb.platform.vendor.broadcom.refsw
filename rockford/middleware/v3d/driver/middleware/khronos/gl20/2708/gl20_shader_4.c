/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Creates GLES2.0 shaders as dataflow graphs and passes them to the compiler
backend.
=============================================================================*/

#include "middleware/khronos/glsl/glsl_common.h"
#include "middleware/khronos/glsl/glsl_dataflow.h"
#include "middleware/khronos/glsl/2708/glsl_allocator_4.h"
#include "middleware/khronos/gl20/gl20_config.h"
//#include "v3d/verification/sw/tools/v3d/simpenrose/simpenrose.h"
#include "middleware/khronos/glxx/2708/glxx_shader_4.h"
#include "middleware/khronos/glxx/2708/glxx_attr_sort_4.h"
#include "middleware/khronos/gl20/2708/gl20_shader_4.h"
#define vclib_memcmp memcmp
#include <assert.h>

static Dataflow *fetch_all_es20_attributes(Dataflow **attrib, GLXX_ATTRIB_ABSTRACT_T *abstract, uint32_t attribs_live, uint32_t * attribs_order)
{
   uint32_t i;
   uint32_t uniform_index[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   Dataflow *dep;

   dep = NULL;

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      uniform_index[i] = GLXX_ATTRIB_OFFSET(i);

   return glxx_fetch_all_attributes(abstract, uniform_index, attrib, dep, attribs_live, attribs_order);

//   return dep;
}

static Dataflow * init_frag_vary(Dataflow **frag_vary, uint32_t num_varyings, uint32_t primitive_type)
{
   uint32_t i;
   Dataflow *dep = NULL;

   memset(frag_vary, 0, 4*64);
   if (primitive_type == 0)
   {
      /* points */
      dep = frag_vary[0] = glxx_vary(0, dep);
      dep = frag_vary[1] = glxx_vary(1, dep);
   }
   else
   {
      if (primitive_type == 1)  /* lines */
         dep = glxx_vary(0, dep);
      frag_vary[0] = glxx_cfloat(0.0f);
      frag_vary[1] = glxx_cfloat(0.0f);
   }

   for (i = 0; i < num_varyings; i++)
      frag_vary[32+i] = glxx_vary(32 + i, dep);

   return dep;
}

uint32_t xxx_shader;
static bool gl20_hw_emit_shaders(GL20_LINK_RESULT_T *link_result, GLXX_LINK_RESULT_KEY_T *key, GLXX_LINK_RESULT_DATA_T *data, void *base)
{
   Dataflow *shaded[GL20_LINK_RESULT_NODE_COUNT-5];
   Dataflow *frag_color[5];
   Dataflow *attrib[4*GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
   Dataflow *last_vpm_read, *last_vpm_write, *root;
   Dataflow *frag_vary[64];
   Dataflow *point_size;
   Dataflow *dep;
   uint32_t vary_map[32];
   uint32_t vary_count = 0;
   bool result = true;
   bool has_point_size;
   uint32_t fragment_shader_type = GLSL_BACKEND_TYPE_FRAGMENT;
   bool *texture_rb_swap = NULL;

#if GL_EXT_texture_format_BGRA8888
   texture_rb_swap = key->texture_rb_swap;
#endif

   /* fragment shader */
   dep = init_frag_vary(frag_vary, link_result->vary_count, key->primitive_type);
   glsl_dataflow_copy(5, frag_color, (Dataflow **)link_result->nodes, base, frag_vary, 64, DATAFLOW_VARYING, texture_rb_swap);
   root = glxx_backend(key->blend, glxx_vec4(frag_color[0], frag_color[1], frag_color[2], frag_color[3]), frag_color[4], key->stencil_config, key->use_depth, key->render_alpha, key->rgb565, key->fb_rb_swap);
   glxx_iodep(root, dep);
   data->threaded = true;
   if (!glxx_schedule(root, fragment_shader_type, &data->mh_fcode, &data->mh_funiform_map, &data->threaded, vary_map, &vary_count))
   {
      /* try again as non-threaded shader */
      init_frag_vary(frag_vary, link_result->vary_count, key->primitive_type);
      glsl_dataflow_copy(5, frag_color, (Dataflow **)link_result->nodes, base, frag_vary, 64, DATAFLOW_VARYING, texture_rb_swap);
      root = glxx_backend(key->blend, glxx_vec4(frag_color[0], frag_color[1], frag_color[2], frag_color[3]), frag_color[4], key->stencil_config, key->use_depth, key->render_alpha, key->rgb565, key->fb_rb_swap);
      data->threaded = false;
      result &= glxx_schedule(root, fragment_shader_type, &data->mh_fcode, &data->mh_funiform_map, &data->threaded, vary_map, &vary_count);
   }
   data->num_varyings = vary_count;

   /* coordinate shader */
   last_vpm_read = fetch_all_es20_attributes(attrib, key->attribs, link_result->cattribs_live, data->cattribs_order);
   glsl_dataflow_copy(5, shaded, (Dataflow **)link_result->nodes+5, base, attrib, 4*GLXX_CONFIG_MAX_VERTEX_ATTRIBS, DATAFLOW_ATTRIBUTE, texture_rb_swap);
   has_point_size = key->primitive_type == 0 && shaded[4] != NULL;
   point_size = has_point_size ? shaded[4] : NULL;
   last_vpm_write = glxx_vertex_backend(shaded[0], shaded[1], shaded[2], shaded[3], point_size, true, false, NULL, NULL, 0, key->egl_output);
   glxx_iodep(last_vpm_write, last_vpm_read);
   result &= glxx_schedule(last_vpm_write, GLSL_BACKEND_TYPE_COORD, &data->mh_ccode, &data->mh_cuniform_map, NULL, NULL, NULL);

   /* vertex shader */
   last_vpm_read = fetch_all_es20_attributes(attrib, key->attribs, link_result->vattribs_live, data->vattribs_order);
   glsl_dataflow_copy(5+link_result->vary_count, shaded, (Dataflow **)link_result->nodes+5, base, attrib, 4*GLXX_CONFIG_MAX_VERTEX_ATTRIBS, DATAFLOW_ATTRIBUTE, texture_rb_swap);
   point_size = has_point_size ? shaded[4] : NULL;
   last_vpm_write = glxx_vertex_backend(shaded[0], shaded[1], shaded[2], shaded[3], point_size, false, true, shaded+5, vary_map, vary_count, key->egl_output);
   glxx_iodep(last_vpm_write, last_vpm_read);
   result &= glxx_schedule(last_vpm_write, GLSL_BACKEND_TYPE_VERTEX, &data->mh_vcode, &data->mh_vuniform_map, NULL, NULL, NULL);

   data->has_point_size = has_point_size;

   return result;
}


void gl20_link_result_term(void *v, uint32_t size)
{
   GL20_LINK_RESULT_T *result = (GL20_LINK_RESULT_T *)v;
   uint32_t i;

   UNUSED(size);

   MEM_ASSIGN(result->mh_blob, MEM_INVALID_HANDLE);

   for (i = 0; i < GL20_LINK_RESULT_CACHE_SIZE; i++)
   {
      MEM_ASSIGN(result->cache[i].data.mh_vcode, MEM_INVALID_HANDLE);
      MEM_ASSIGN(result->cache[i].data.mh_ccode, MEM_INVALID_HANDLE);
      MEM_ASSIGN(result->cache[i].data.mh_fcode, MEM_INVALID_HANDLE);
      MEM_ASSIGN(result->cache[i].data.mh_vuniform_map, MEM_INVALID_HANDLE);
      MEM_ASSIGN(result->cache[i].data.mh_cuniform_map, MEM_INVALID_HANDLE);
      MEM_ASSIGN(result->cache[i].data.mh_funiform_map, MEM_INVALID_HANDLE);
   }
}

static void dump_key(GLXX_LINK_RESULT_KEY_T *key)
{
   uint32_t i;

   printf("use_depth         = %d\n", key->use_depth ? 1 : 0);
   printf("render_alpha      = %d\n", key->render_alpha ? 1 : 0);
   printf("rgb565            = %d\n", key->rgb565 ? 1 : 0);
   printf("fb_rb_swap        = %d\n", key->fb_rb_swap ? 1 : 0);
   printf("egl_output        = %d\n", key->egl_output ? 1 : 0);
   printf("primitive_type    = %d\n", key->primitive_type);
   printf("stencil_config    = %d\n", key->stencil_config);

   printf("blend equation                 = %d\n", key->blend.equation);
   printf("blend equation_alpha           = %d\n", key->blend.equation_alpha);
   printf("blend src_function             = %d\n", key->blend.src_function);
   printf("blend src_function_alpha       = %d\n", key->blend.src_function_alpha);
   printf("blend dst_function             = %d\n", key->blend.dst_function);
   printf("blend dst_function_alpha       = %d\n", key->blend.dst_function_alpha);
   printf("blend sample_alpha_to_coverage = %d\n", key->blend.sample_alpha_to_coverage);
   printf("blend sample_coverage          = %d\n", key->blend.sample_coverage);
   printf("blend sample_coverage_v.value  = %f\n", key->blend.sample_coverage_v.value);
   printf("blend sample_coverage_v.invert = %d\n", key->blend.sample_coverage_v.invert ? 1 : 0);
   printf("blend ms                       = %d\n", key->blend.ms ? 1 : 0);
   printf("blend logic_op                 = %d\n", key->blend.logic_op);
   printf("blend color_mask               = %d\n", key->blend.color_mask);

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
      printf("attribs[%d] size=%d, type=%d, norm=%d\n", i, key->attribs[i].size, key->attribs[i].type, key->attribs[i].norm ? 1 : 0);

#if GL_EXT_texture_format_BGRA8888
   for (i = 0; i < GLXX_CONFIG_MAX_TEXTURE_UNITS; i++)
      printf("texture_rb_swap[%d] = %d\n", i, key->texture_rb_swap[i] ? 1 : 0);
#endif

   printf("\n");
}

static void diff_keys(GLXX_LINK_RESULT_KEY_T *key1, GLXX_LINK_RESULT_KEY_T *key2)
{
   uint32_t i;

   if (key1->use_depth != key2->use_depth)
      printf("use_depth\n");

   if (key1->render_alpha != key2->render_alpha)
      printf("render_alpha\n");

   if (key1->rgb565 != key2->rgb565)
      printf("rgb565\n");

   if (key1->fb_rb_swap != key2->fb_rb_swap)
      printf("fb_rb_swap\n");

   if (key1->egl_output != key2->egl_output)
      printf("egl_output\n");

   if (key1->primitive_type != key2->primitive_type)
      printf("primitive_type %d != %d\n", key1->primitive_type, key2->primitive_type);

   if (key1->stencil_config != key2->stencil_config)
      printf("stencil_config %d != %d\n", key1->stencil_config, key2->stencil_config);

   if (key1->blend.equation != key2->blend.equation)
      printf("blend equation %d != %d\n", key1->blend.equation, key2->blend.equation);

   if (key1->blend.equation_alpha != key2->blend.equation_alpha)
      printf("blend equation_alpha %d != %d\n", key1->blend.equation_alpha, key2->blend.equation_alpha);

   if (key1->blend.src_function != key2->blend.src_function)
      printf("blend src_function %d != %d\n", key1->blend.src_function, key2->blend.src_function);

   if (key1->blend.src_function_alpha != key2->blend.src_function_alpha)
      printf("blend src_function_alpha %d != %d\n", key1->blend.src_function_alpha, key2->blend.src_function_alpha);

   if (key1->blend.dst_function != key2->blend.dst_function)
      printf("blend dst_function %d != %d\n", key1->blend.dst_function, key2->blend.dst_function);

   if (key1->blend.dst_function_alpha != key2->blend.dst_function_alpha)
      printf("blend dst_function_alpha %d != %d\n", key1->blend.dst_function_alpha, key2->blend.dst_function_alpha);

   if (key1->blend.sample_alpha_to_coverage != key2->blend.sample_alpha_to_coverage)
      printf("blend sample_alpha_to_coverage\n");

   if (key1->blend.sample_coverage != key2->blend.sample_coverage)
      printf("blend sample_coverage %d != %d\n", key1->blend.sample_coverage, key2->blend.sample_coverage);

   if (key1->blend.sample_coverage_v.value != key2->blend.sample_coverage_v.value)
      printf("blend sample_coverage_v.value %f != %f\n", key1->blend.sample_coverage_v.value, key2->blend.sample_coverage_v.value);

   if (key1->blend.sample_coverage_v.invert != key2->blend.sample_coverage_v.invert)
      printf("blend sample_coverage_v.invert %d != %d\n", key1->blend.sample_coverage_v.invert, key2->blend.sample_coverage_v.invert);

   if (key1->blend.ms != key2->blend.ms)
      printf("blend ms\n");

   if (key1->blend.logic_op != key2->blend.logic_op)
      printf("blend logic_op %d != %d\n", key1->blend.logic_op, key2->blend.logic_op);

   if (key1->blend.color_mask != key2->blend.color_mask)
      printf("blend color_mask %08X != %08X\n", key1->blend.color_mask, key2->blend.color_mask);

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      if (key1->attribs[i].size != key2->attribs[i].size ||
          key1->attribs[i].type != key2->attribs[i].type ||
          key1->attribs[i].norm != key2->attribs[i].norm)
         printf("attribs[%d] size=%d, type=%d, norm=%d != attribs[%d] size=%d, type=%d, norm=%d\n",
               i, key1->attribs[i].size, key1->attribs[i].type, key1->attribs[i].norm ? 1 : 0,
               i, key2->attribs[i].size, key2->attribs[i].type, key2->attribs[i].norm ? 1 : 0);
   }

#if GL_EXT_texture_format_BGRA8888
   for (i = 0; i < GLXX_CONFIG_MAX_TEXTURE_UNITS; i++)
   {
      if (key1->texture_rb_swap[i] != key2->texture_rb_swap[i])
         printf("texture_rb_swap[%d]\n", i);
   }
#endif

   printf("\n");
}

static bool create_shader(GL20_LINK_RESULT_T *link_result, uint32_t cache_index)
{
   void *base;
   bool result;

   glsl_fastmem_init();
   if (setjmp(g_ErrorHandlerEnv) != 0)
   {
      /* We must be jumping back from an error. */
      glsl_fastmem_term();
      mem_unlock(link_result->mh_blob);
      return false;
   }

   /* looks a little strange.  The block above is marking the error return point.  If gl20_hw_emit_shaders() dies it will jump back to
      this point with link_result->mh_blob locked from below. */
   base = mem_lock(link_result->mh_blob, NULL);
   glsl_init_primitive_values();

   result = gl20_hw_emit_shaders(link_result, &link_result->cache[cache_index].key, &link_result->cache[cache_index].data, base);

   /*For debug : dump_key(&link_result->cache[cache_index].key);*/

   glsl_fastmem_term();
   mem_unlock(link_result->mh_blob);
   return result;
}

bool gl20_link_result_get_shaders(
   GL20_LINK_RESULT_T *link_result,
   GLXX_HW_SHADER_RECORD_T *shader_out,
   MEM_HANDLE_T *cunifmap_out,
   MEM_HANDLE_T *vunifmap_out,
   MEM_HANDLE_T *funifmap_out,
   GLXX_SERVER_STATE_T *state,
   GLXX_ATTRIB_T *attrib,
   uint32_t *mergeable_attribs,
   uint32_t *cattribs_order_out,
   uint32_t *vattribs_order_out,
   bool     *wasInCache)
{
   uint32_t i, j;
   GLXX_LINK_RESULT_KEY_T *key;
   bool found;
   bool failed;
   static uint32_t totalShaders = 0;
   static uint32_t reschedShaders = 0;

   key = &state->shader.common;

   //TODO: vertex hiding
   //vcos_assert(0);
   /*for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      if ((link_result->vattribs_live & 15<<(4*i)) && attrib[i].enabled)
      {
         key.attribs[i].size = attrib[i].size;
         key.attribs[i].type = attrib[i].type;
         key.attribs[i].norm = !!attrib[i].normalized;
      }
      else
      {
         key.attribs[i].size = 0;
         key.attribs[i].type = 0;
         key.attribs[i].norm = false;
      }
   }*/

   found  = false;
   failed = false;

   for (i = 0; i < link_result->cache_used; i++)
   {
      if (link_result->cache[i].used && !vclib_memcmp(&link_result->cache[i].key, key, sizeof(GLXX_LINK_RESULT_KEY_T)))
      {
         failed = link_result->cache[i].failed;
         found  = true;
         break;
      }
   }

   *wasInCache = found;

   if (failed)
      return false;

   if (!found)
   {
/*
      if (link_result->cache_used > 0)
      {
         reschedShaders++;

         printf("RE_SCHEDULING SHADER %d of %d\n", reschedShaders, totalShaders);
         for (i = 0; i < link_result->cache_used; i++)
         {
            if (link_result->cache[i].used)
               diff_keys(&link_result->cache[i].key, key);
         }
      }
      else
      {
         printf("INITIAL SCHEDULING %d\n", totalShaders);
         totalShaders++;
      }
*/

      /* Compile new version of this shader and add it to the cache. */
      i = link_result->cache_next;
      link_result->cache_next = (i + 1) % GL20_LINK_RESULT_CACHE_SIZE;
      if (link_result->cache_used < GL20_LINK_RESULT_CACHE_SIZE) link_result->cache_used++;

      link_result->cache[i].key    = *key;
      link_result->cache[i].used   = true;
      link_result->cache[i].failed = false;

      glxx_sort_attributes(attrib, link_result->cattribs_live, link_result->vattribs_live, mergeable_attribs,
            link_result->cache[i].data.cattribs_order, link_result->cache[i].data.vattribs_order);

      if (!create_shader(link_result, i))
      {
         // Although the build failed, keep the line marked as used, but set
         // the "failed" flag.  Use this to prevent attempts to reschedule the
         // code if nothing else has changed.
         // link_result->cache[i].used = false;
         link_result->cache[i].failed = true;
         return false;
      }
   }

   for (j = 0; j < GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2; j++)
   {
      cattribs_order_out[j] = link_result->cache[i].data.cattribs_order[j];
      vattribs_order_out[j] = link_result->cache[i].data.vattribs_order[j];
   }

   shader_out->flags =
      (!link_result->cache[i].data.threaded) |
      link_result->cache[i].data.has_point_size<<1 |
      1<<2;
   shader_out->num_varyings = link_result->cache[i].data.num_varyings;

   /* SW-5891 hardware can only do 65536 vertices at a time */
   /* store copy of handles in here, so can properly copy shader record */
   shader_out->fshader = (uint32_t)link_result->cache[i].data.mh_fcode;
   shader_out->vshader = (uint32_t)link_result->cache[i].data.mh_vcode;
   /* */

   glxx_big_mem_insert(&shader_out->fshader, link_result->cache[i].data.mh_fcode, 0);
   glxx_big_mem_insert(&shader_out->vshader, link_result->cache[i].data.mh_vcode, 0);

   /* check big_mem_insert didn't change our handle copies */
   vcos_assert(shader_out->fshader == (uint32_t)link_result->cache[i].data.mh_fcode);
   vcos_assert(shader_out->vshader == (uint32_t)link_result->cache[i].data.mh_vcode);

   *vunifmap_out = link_result->cache[i].data.mh_vuniform_map;
   *funifmap_out = link_result->cache[i].data.mh_funiform_map;

   /* store copy of handle in here, so can properly copy shader record */
   shader_out->cshader = (uint32_t)link_result->cache[i].data.mh_ccode;

   glxx_big_mem_insert(&shader_out->cshader, link_result->cache[i].data.mh_ccode, 0);

   /* check big_mem_insert didn't change our handle copies */
   vcos_assert(shader_out->cshader == (uint32_t) link_result->cache[i].data.mh_ccode);

   *cunifmap_out = link_result->cache[i].data.mh_cuniform_map;

   return true;
}
