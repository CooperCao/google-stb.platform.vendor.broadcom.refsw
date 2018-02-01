/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <stdlib.h>
#include "vcos.h"
#include "khrn_options.h"
#include "khrn_int_common.h"
#include "khrn_int_process.h"
#include "khrn_mem.h"
#include "khrn_process_debug.h"
#include "khrn_fmem_debug_info.h"
#if KHRN_DEBUG
#include "khrn_capture_buffer.h"
#endif
#include "../gl11/gl11_exts.h"
#include "../gl20/gl20_exts.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_texture.h"
#include "libs/platform/gmem.h"
#include "libs/core/v3d/v3d_ident.h"
#include "libs/core/v3d/v3d_debug.h"
#include "libs/util/profile/profile.h"
#include "libs/util/demand.h"
#include "libs/platform/bcm_sched_api.h"
#include "libs/platform/v3d_platform.h"
#include "libs/platform/v3d_parallel.h"
#include "libs/platform/v3d_scheduler.h"
#include "libs/platform/v3d_driver_api.h"
#include "libs/core/lfmt_translate_gl/lfmt_translate_gl.h"
#if V3D_VER_AT_LEAST(3,3,0,0)
#include "libs/compute/compute.h"
#endif

static struct statics {
   gmem_handle_t dummy_texture_handle;
#if !V3D_VER_AT_LEAST(4,2,13,0)
   gmem_handle_t dummy_ocq_buffer;
#endif
   bool initialized;
   char device_name[32];
   char gl11_exts_str[GL11_EXTS_STR_MAX_SIZE];
   char gl3x_exts_str[GL20_EXTS_STR_MAX_SIZE];
   unsigned num_gl3x_exts;
   const char *gl3x_exts[GL20_MAX_EXTS];
   gfx_lfmt_translate_gl_ext_t lfmt_translate_exts;
   GLXX_TEXTURE_SAMPLER_STATE_T image_unit_default_sampler;
} statics;

gmem_handle_t get_dummy_texture(void)
{
   return statics.dummy_texture_handle;
}

const GLXX_TEXTURE_SAMPLER_STATE_T *khrn_get_image_unit_default_sampler(void)
{
   return &statics.image_unit_default_sampler;
}

#if !V3D_VER_AT_LEAST(4,2,13,0)
gmem_handle_t khrn_get_dummy_ocq_buffer(void)
{
   return statics.dummy_ocq_buffer;
}
#endif

const char *khrn_get_device_name(void)
{
   return statics.device_name;
}

const char *khrn_get_gl11_exts_str(void)
{
   return statics.gl11_exts_str;
}

const char *khrn_get_gl3x_exts_str(void)
{
   return statics.gl3x_exts_str;
}

unsigned khrn_get_num_gl3x_exts(void)
{
   return statics.num_gl3x_exts;
}

const char *khrn_get_gl3x_ext(unsigned i)
{
   assert(i < statics.num_gl3x_exts);
   return statics.gl3x_exts[i];
}

gfx_lfmt_translate_gl_ext_t khrn_get_lfmt_translate_exts(void)
{
   return statics.lfmt_translate_exts;
}

static void khrn_statics_shutdown(struct statics *s)
{
   gmem_free(s->dummy_texture_handle);
   s->dummy_texture_handle = 0;
#if !V3D_VER_AT_LEAST(4,2,13,0)
   gmem_free(s->dummy_ocq_buffer);
   s->dummy_ocq_buffer = 0;
#endif
}

static void init_image_unit_default_sampler(GLXX_TEXTURE_SAMPLER_STATE_T *sampler)
{
   /* the important values are clamp to border, border_color[1-4]=0, min_lod = max_lod = 0
    * and GL_NEAREST for min/mag;*/
   sampler->filter.min = sampler->filter.mag = GL_NEAREST;
   sampler->wrap.s = sampler->wrap.t = sampler->wrap.r = GL_CLAMP_TO_BORDER;
   sampler->anisotropy = 1.0f;
   sampler->min_lod = sampler->max_lod = 0.0f;
   sampler->compare_mode = GL_NONE;
   sampler->compare_func = GL_LEQUAL;
   sampler->unnormalised_coords = false;
#if V3D_VER_AT_LEAST(4,1,34,0)
   memset(sampler->border_color, 0, sizeof(sampler->border_color));
#endif
   sampler->debug_label = NULL;
}

static bool khrn_statics_init(struct statics *s)
{
   unsigned int *p;
   unsigned int i;
   /* This size is assumed by compute_texture_uniforms() */
   const unsigned int texture_size = 64
      * 6;/* Alloc more space for cube map textures. */

   s->initialized = true;

   /* create a dummy texture */
   s->dummy_texture_handle = gmem_alloc(texture_size, V3D_TMU_ML_ALIGN,
      GMEM_USAGE_V3D_READ, "khrn dummy texture");
   if (s->dummy_texture_handle == GMEM_HANDLE_INVALID)
      goto error;

   p = gmem_map_and_get_ptr(s->dummy_texture_handle);
   if (p == NULL)
      goto error;

   for (i = 0; i < texture_size / sizeof(*p); i++)
      p[i] = 0xff000000;

   gmem_flush_mapped_buffer(s->dummy_texture_handle);

#if !V3D_VER_AT_LEAST(4,2,13,0)
   /* GFXH-1320 & GFXH-1636 workaround: create a dummy occlusion query buffer sized for each way in the cache. */
   s->dummy_ocq_buffer = gmem_alloc(
      V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN * 8,
      V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN,
      GMEM_USAGE_V3D_RW,
      "khrn dummy occlusion query buffer"
      );
   if (s->dummy_ocq_buffer == GMEM_HANDLE_INVALID)
      goto error;
#endif

   v3d_sprint_device_name(statics.device_name, sizeof(statics.device_name), 0, v3d_scheduler_get_identity());

   verif((gl11_exts_str(statics.gl11_exts_str) - statics.gl11_exts_str) < sizeof(statics.gl11_exts_str));
   verif((gl20_exts_str(statics.gl3x_exts_str) - statics.gl3x_exts_str) < sizeof(statics.gl3x_exts_str));
   statics.num_gl3x_exts = gl20_exts(statics.gl3x_exts);
   verif(statics.num_gl3x_exts <= countof(statics.gl3x_exts));

   s->lfmt_translate_exts =
      (khrn_get_has_astc() ? GFX_LFMT_TRANSLATE_GL_EXT_ASTC : 0);

   init_image_unit_default_sampler(&s->image_unit_default_sampler);

   return true;

error:
   khrn_statics_shutdown(s);
   return false;
}

void khrn_process_shutdown(void)
{
   if (statics.initialized)
   {
      statics.initialized = false;

      /* ensure that all job completion handlers run first */
      v3d_scheduler_wait_all();

#if KHRN_DEBUG
      khrn_capture_wait();
#endif

      /* shutdown the driver */
#ifdef KHRN_GEOMD
      fmem_debug_info_term_process();
#endif
      khrn_statics_shutdown(&statics);
      khrn_mem_term();
      khrn_fmem_client_pool_deinit();
#if !V3D_VER_AT_LEAST(4,1,34,0)
      khrn_tile_state_deinit();
#endif
      v3d_parallel_term();
#if V3D_VER_AT_LEAST(3,3,0,0) && !V3D_USE_CSD
      compute_term();
#endif
      v3d_platform_shutdown();
      profile_shutdown();
   }
}

bool khrn_process_init(void)
{
   verif(VCOS_SUCCESS==vcos_init());

#ifdef KHRN_GEOMD
   verif(fmem_debug_info_init_process());
#endif

   khrn_init_options();
   khrn_fmem_static_init();
   profile_init();

   demand_msg(v3d_platform_init(), "Failed to initialise platform");
   v3d_check_ident(v3d_scheduler_get_identity(), 0);

#if V3D_VER_AT_LEAST(3,3,0,0) && !V3D_USE_CSD
   compute_init();
#endif

#ifdef KHRN_GEOMD
   if (khrn_options.geomd)
      v3d_platform_set_debug_callback(khrn_debug_callback, NULL);
#endif

   if (v3d_parallel_init(khrn_options.max_worker_threads))
   {
      // Default to using all cores, or number specified by environment.
      // Need to do this after the scheduler session has been brought up.
      uint32_t num_cores = khrn_get_num_cores();
      if (!khrn_options.render_subjobs)
         khrn_options.render_subjobs = num_cores;
      if (!khrn_options.bin_subjobs)
         khrn_options.bin_subjobs = num_cores;

      // Further clamp to max subjobs of type...
      khrn_options.render_subjobs = gfx_umin(khrn_options.render_subjobs,  V3D_MAX_RENDER_SUBJOBS);
      khrn_options.bin_subjobs = gfx_umin(khrn_options.bin_subjobs, V3D_MAX_BIN_SUBJOBS);

      if (khrn_mem_init())
      {
         if (khrn_fmem_client_pool_init())
         {
            if (khrn_statics_init(&statics))
            {
             #if !V3D_VER_AT_LEAST(4,1,34,0)
               khrn_tile_state_init();
             #endif
               return true;
            }
            khrn_fmem_client_pool_deinit();
         }
         khrn_mem_term();
      }
      v3d_parallel_term();
   }

#if V3D_VER_AT_LEAST(3,3,0,0) && !V3D_USE_CSD
   compute_term();
#endif
   v3d_platform_shutdown();
   profile_shutdown();

   return false;
}
