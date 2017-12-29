/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "middleware/khronos/gl11/2708/gl11_shadercache_4.h"
#include "interface/khronos/common/khrn_int_hash.h"
#include "middleware/khronos/gl11/2708/gl11_shader_4.h"
#include "middleware/khronos/glxx/2708/glxx_shader_4.h"
#include "middleware/khronos/glxx/2708/glxx_attr_sort_4.h"
#include "middleware/khronos/gl11/2708/gl11_support_4.h"
#define vclib_memcmp khrn_memcmp
/*///////////////////// */
/* Vertex shader */
/*///////////////////// */

#define GL11_CACHE_SIZE 1024
//#define GL11_DUMP_VERTEX_KEY

static GL11_CACHE_ENTRY_T cache[GL11_CACHE_SIZE];

static void clear_entry(uint32_t hash)
{
   cache[hash].used = false;
   MEM_ASSIGN(cache[hash].data.mh_fcode, MEM_HANDLE_INVALID);
   MEM_ASSIGN(cache[hash].data.mh_vcode, MEM_HANDLE_INVALID);
   MEM_ASSIGN(cache[hash].data.mh_ccode, MEM_HANDLE_INVALID);
   MEM_ASSIGN(cache[hash].data.mh_cuniform_map, MEM_HANDLE_INVALID);
   MEM_ASSIGN(cache[hash].data.mh_vuniform_map, MEM_HANDLE_INVALID);
   MEM_ASSIGN(cache[hash].data.mh_funiform_map, MEM_HANDLE_INVALID);
}

/* Restore cache to its initial empty state */

void gl11_hw_shader_cache_reset()
{
   int i;
   for (i = 0; i < GL11_CACHE_SIZE; i++) {
      /* TODO: this is a slightly ugly way of restoring a line to its initial state */
      clear_entry(i);
      memset(&cache[i], 0, sizeof(GL11_CACHE_ENTRY_T));
      cache[i].data.mh_vcode = MEM_HANDLE_INVALID;   //naughty
      cache[i].data.mh_ccode = MEM_HANDLE_INVALID;   //naughty
      cache[i].data.mh_fcode = MEM_HANDLE_INVALID;   //naughty
      cache[i].data.mh_vuniform_map = MEM_HANDLE_INVALID;   //naughty
      cache[i].data.mh_cuniform_map = MEM_HANDLE_INVALID;   //naughty
      cache[i].data.mh_funiform_map = MEM_HANDLE_INVALID;   //naughty
   }
}

#define ASSIGN(a, b) do { changed |= (a) != (b); (a) = (b); } while (0)

#ifdef GL11_DUMP_VERTEX_KEY
static void dump_key(GL11_CACHE_KEY_T *entry)
{
/*
   GLXX_LINK_RESULT_KEY_T common;
      GLXX_ATTRIB_ABSTRACT_T attribs[GLXX_CONFIG_MAX_VERTEX_ATTRIBS];
      uint32_t primitive_type;
      GLXX_HW_BLEND_T blend;
         GLenum equation;
         GLenum equation_alpha;
         GLenum src_function;
         GLenum src_function_alpha;
         GLenum dst_function;
         GLenum dst_function_alpha;
         GLenum sample_alpha_to_coverage;
         GLenum sample_coverage;
         GLXX_SAMPLE_COVERAGE_T sample_coverage_v;
         bool ms;
         GLenum logic_op;
         uint32_t color_mask;
      bool stencil_config;
      bool render_alpha;
      bool rgb565;
   GL11_CACHE_LIGHT_ABSTRACT_T lights[GL11_CONFIG_MAX_LIGHTS];
      bool enabled;
      bool position_w_is_0;
      bool spot_cutoff_is_180;
   GL11_CACHE_TEXUNIT_ABSTRACT_T texunits[GL11_CONFIG_MAX_TEXTURE_UNITS];
      GLenum mode;
      GL11_COMBINER_T rgb;
         GLenum combine;
         GLenum source[3];
         GLenum operand[3];
         GLfloat scale;
      GL11_COMBINER_T alpha;
      GL11_TEXUNIT_PROPS_T props;
         bool active;
         bool complex;
         bool has_color;
         bool has_alpha;
      bool coord_replace;
   bool two_side;
   bool normalize;
   bool rescale_normal;
   bool lighting;
   bool color_material;
   bool point_smooth;
   bool line_smooth;
   GLenum alpha_func;
   GLenum fog_mode;
   int user_clip_plane;
   uint32_t cattribs_live;
   uint32_t vattribs_live;
*/

   int i;
   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS; i++)
   {
      printf("attrib:%d type:%d size:%d norm:%d \n",i,
         entry->common.attribs[i].type,
         entry->common.attribs[i].size,
         entry->common.attribs[i].norm);
   }
   printf("blend: not dumped (TODO)\n");
   printf("primitive_type:%d stencil_config:%d\n",entry->common.primitive_type,entry->common.stencil_config);
   printf("render_alpha:%d rgb565:%d\n",entry->common.render_alpha,entry->common.rgb565);
   for (i = 0; i < GL11_CONFIG_MAX_LIGHTS; i++) {
      printf("light:%d enabled:%d position_w_is_0:%d spot_cutoff_is_180:%d\n",i,entry->lights[i].enabled,
         entry->lights[i].position_w_is_0,entry->lights[i].spot_cutoff_is_180);
   }
   for (i = 0; i < GL11_CONFIG_MAX_TEXTURE_UNITS; i++) {
      printf("texunit:%d mode:%d coord_replace:%d\n",i,entry->texunits[i].mode,entry->texunits[i].coord_replace);
      printf("   active:%d complex:%d has_color:%d has_alpha:%d\n",entry->texunits[i].props.active,entry->texunits[i].props.complex,
         entry->texunits[i].props.has_color, entry->texunits[i].props.has_alpha);
      printf("   rgb: combine:%d scale:%f source:%d %d %d operand:%d %d %d\n",entry->texunits[i].rgb.combine,entry->texunits[i].rgb.scale,
         entry->texunits[i].rgb.source[0],entry->texunits[i].rgb.source[1],entry->texunits[i].rgb.source[2],
         entry->texunits[i].rgb.operand[0],entry->texunits[i].rgb.operand[1],entry->texunits[i].rgb.operand[2]);
      printf("   alpha: combine:%d scale:%f source:%d %d %d operand:%d %d %d\n",entry->texunits[i].alpha.combine,entry->texunits[i].alpha.scale,
         entry->texunits[i].alpha.source[0],entry->texunits[i].alpha.source[1],entry->texunits[i].alpha.source[2],
         entry->texunits[i].alpha.operand[0],entry->texunits[i].alpha.operand[1],entry->texunits[i].alpha.operand[2]);
   }
   printf("two_side:%d normalize:%d rescale_normal:%d\n",entry->two_side,entry->normalize,entry->rescale_normal);
   printf("lighting:%d color_material:%d\n",entry->lighting,entry->color_material);
   printf("point_smooth:%d line_smooth:%d\n",entry->point_smooth,entry->line_smooth);
   printf("alpha_func:%d fog_mode:%d user_clip_plane:%d\n",entry->alpha_func,entry->fog_mode,entry->user_clip_plane);
   printf("cattribs_live:0x%x vattribs_live:0x%x\n",entry->cattribs_live,entry->vattribs_live);
}
#endif

///////////////////////
// Fragment shader
///////////////////////


///////////////////
//
// Combined
//
///////////////////

static uint32_t hashkey(const GL11_CACHE_KEY_T *key)
{
   return khrn_hashword((const uint32_t *)key, sizeof(GL11_CACHE_KEY_T)/4, 0xa28abf82);
}

static uint32_t bump(uint32_t hash)
{
   return hash * 0x23456789;
}

bool gl11_hw_get_shaders(
    GLXX_HW_SHADER_RECORD_T *shader_out,
    MEM_HANDLE_T *cunifmap_out,
    MEM_HANDLE_T *vunifmap_out,
    MEM_HANDLE_T *funifmap_out,
    uint32_t *color_varyings_out,
    GLXX_SERVER_STATE_T *state,
    GLXX_ATTRIB_T *attrib,
    uint32_t *mergeable_attribs,
    uint32_t * cattribs_order,
    uint32_t * vattribs_order)
{
   uint32_t hash, hash_word, original_hash_word;
   GL11_CACHE_KEY_T *key;

   int i;

   key = &state->shader;

   hash_word = hashkey(key);
   original_hash_word = hash_word;

   hash = hash_word % GL11_CACHE_SIZE;

   if (cache[hash].used && vclib_memcmp(&cache[hash].key, key, sizeof(GL11_CACHE_KEY_T)))
   {
      hash_word = bump(hash_word);
      hash = hash_word % GL11_CACHE_SIZE;
      if (cache[hash].used && vclib_memcmp(&cache[hash].key, key, sizeof(GL11_CACHE_KEY_T)))
      {
         hash_word = bump(hash_word);
         hash = hash_word % GL11_CACHE_SIZE;
         if (cache[hash].used && vclib_memcmp(&cache[hash].key, key, sizeof(GL11_CACHE_KEY_T)))
         {
            /* Three attempts at finding a cache have all yielded something which is being used by someone else. */
            hash_word = original_hash_word;
            hash = hash_word % GL11_CACHE_SIZE;
            clear_entry(hash);
         }
      }
   }

   if (!cache[hash].used)
   {
#ifdef GL11_DUMP_VERTEX_KEY
      dump_key(key);
#endif

      cache[hash].used = true;
      khrn_memcpy(&cache[hash].key, key, sizeof(GL11_CACHE_KEY_T));

      glxx_sort_attributes(attrib, state->batch.cattribs_live, state->batch.vattribs_live, mergeable_attribs, cache[hash].data.cattribs_order, cache[hash].data.vattribs_order);

      if (!gl11_hw_emit_shaders(&cache[hash].key, &cache[hash].data, &cache[hash].color_varyings))
      {
         cache[hash].used = false;
         return false;
      }
   }

   for (i = 0; i < GLXX_CONFIG_MAX_VERTEX_ATTRIBS*2; i++)
   {
      cattribs_order[i] = cache[hash].data.cattribs_order[i];
      vattribs_order[i] = cache[hash].data.vattribs_order[i];
   }

   shader_out->flags =
      (!cache[hash].data.threaded) |
      cache[hash].data.has_point_size<<1 |
      1<<2;
   shader_out->num_varyings = cache[hash].data.num_varyings;

   /* SW-5891 hardware can only do 65536 vertices at a time */
   /* store copy of handles in here, so can properly copy shader record */
   shader_out->fshader = cache[hash].data.mh_fcode;
   shader_out->vshader = cache[hash].data.mh_vcode;
   shader_out->cshader = cache[hash].data.mh_ccode;

   if (!glxx_big_mem_insert(&shader_out->fshader, cache[hash].data.mh_fcode, 0)) return false;
   if (!glxx_big_mem_insert(&shader_out->vshader, cache[hash].data.mh_vcode, 0)) return false;
   if (!glxx_big_mem_insert(&shader_out->cshader, cache[hash].data.mh_ccode, 0)) return false;

   *cunifmap_out = cache[hash].data.mh_cuniform_map;
   *vunifmap_out = cache[hash].data.mh_vuniform_map;
   *funifmap_out = cache[hash].data.mh_funiform_map;

   /* check big_mem_insert didn't change our handle copies */
   assert(shader_out->fshader == cache[hash].data.mh_fcode);
   assert(shader_out->vshader == cache[hash].data.mh_vcode);
   assert(shader_out->cshader == cache[hash].data.mh_ccode);

   *color_varyings_out = cache[hash].color_varyings;
   return true;
}