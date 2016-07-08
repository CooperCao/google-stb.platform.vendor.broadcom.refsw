/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Contains per-process state and per-thread state
=============================================================================*/

#include <stdlib.h>
#include "vcos.h"
#include "khrn_options.h"
#include "khrn_int_common.h"
#include "khrn_int_process.h"
#include "khrn_mem.h"
#include "khrn_process_debug.h"
#include "khrn_fmem_debug_info.h"
#include "../glxx/glxx_server.h"
#include "../glxx/glxx_extensions.h"
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

static struct statics {
   gmem_handle_t dummy_texture_handle;
   gmem_handle_t gfxh_1320_buffer;
   bool initialized;
#if !V3D_HAS_NEW_TMU_CFG
   V3D_MISCCFG_T misccfg;
#endif
   char *extensions;
   gfx_lfmt_translate_gl_ext_t lfmt_translate_exts;
   GLXX_TEXTURE_SAMPLER_STATE_T image_unit_default_sampler;
} statics;

static bool init_v3d_client(void)
{
   bool mem = false, plat = false, ok = false;

   mem = khrn_mem_init();
   if (!mem) goto end;

   plat = v3d_platform_init();
   demand_msg(plat, "Failed to initialise platform");

#ifdef KHRN_GEOMD
   if (khrn_options.geomd)
      v3d_platform_set_debug_callback(khrn_debug_callback, NULL);
#endif

   if (!v3d_parallel_init(khrn_options.max_worker_threads))
      goto end;

   v3d_check_ident(v3d_scheduler_get_identity(), 0);

#if !V3D_HAS_NEW_TMU_CFG
   statics.misccfg.ovrtmuout = khrn_get_v3d_version() >= V3D_MAKE_VER(3,3,0,0);
#endif

   ok = true;
end:
   if (!ok)
   {
      /* SCHED_FIXME: completion_term, v3d_term, etc? */
      if (mem)
         khrn_mem_term();
      return false;
   }
   return true;
}

gmem_handle_t get_dummy_texture(void)
{
   return statics.dummy_texture_handle;
}

const GLXX_TEXTURE_SAMPLER_STATE_T *khrn_get_image_unit_default_sampler(void)
{
   return &statics.image_unit_default_sampler;
}

gmem_handle_t khrn_get_gfxh_1320_buffer(void)
{
   return statics.gfxh_1320_buffer;
}

char *khrn_get_extensions(void)
{
   return statics.extensions;
}

gfx_lfmt_translate_gl_ext_t khrn_get_lfmt_translate_exts(void)
{
   return statics.lfmt_translate_exts;
}

#if !V3D_HAS_NEW_TMU_CFG
const V3D_MISCCFG_T *khrn_get_hw_misccfg(void)
{
   return &statics.misccfg;
}
#endif

static void khrn_statics_shutdown(struct statics *s)
{
   gmem_free(s->dummy_texture_handle);
   gmem_free(s->gfxh_1320_buffer);
   free (s->extensions);
   s->dummy_texture_handle = 0;
   s->gfxh_1320_buffer = 0;
   s->extensions = NULL;
}

static void init_image_unit_default_sampler(GLXX_TEXTURE_SAMPLER_STATE_T *sampler)
{
   /* the important values are clamp to border, border_color[1-4]=0, min_lod = max_lod = 0;*/
   sampler->filter.min = sampler->filter.mag = GL_NEAREST;
   sampler->wrap.s = sampler->wrap.t = sampler->wrap.r = GL_CLAMP_TO_BORDER_BRCM;
   sampler->anisotropy = 1.0f;
   sampler->min_lod = sampler->max_lod = 0.0f;
   sampler->compare_mode = GL_NONE;
   sampler->compare_func = GL_LEQUAL;
   sampler->unnormalised_coords = false;
   memset(sampler->border_color, 0, sizeof(sampler->border_color));
   sampler->debug_label = NULL;
}

static bool khrn_statics_init(struct statics *s)
{
   unsigned int *p;
   unsigned int i;
   unsigned int num_extensions;
   unsigned int extensions_size;
   /* TODO - this size is assumed by glxx_calculate_and_hide() */
   const unsigned int texture_size = 64
      * 6;/* Alloc more space for cube map textures. */

   s->initialized = true;

   /* create a dummy texture */
   s->dummy_texture_handle = gmem_alloc(texture_size, V3D_TMU_ML_ALIGN,
      GMEM_USAGE_CPU_WRITE | GMEM_USAGE_V3D_READ, "khrn dummy texture");
   if (s->dummy_texture_handle == GMEM_HANDLE_INVALID)
      goto error;

   p = gmem_map_and_begin_cpu_access(s->dummy_texture_handle, GMEM_SYNC_CPU_WRITE);
   if (p == NULL)
      goto error;

   for (i = 0; i < texture_size / sizeof(*p); i++)
      p[i] = 0xff000000;

   gmem_end_cpu_access_and_unmap(s->dummy_texture_handle, GMEM_SYNC_CPU_WRITE);

   /* Create a dummy occlusion query buffer sized for each way in the cache. */
   if (khrn_get_v3d_version() < V3D_MAKE_VER(3,3,0,0)) // GFXH-1320
   {
      s->gfxh_1320_buffer = gmem_alloc(
         V3D_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN * 8,
         V3D_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN,
         GMEM_USAGE_V3D,
         "khrn dummy occlusion query buffer"
         );
      if (s->gfxh_1320_buffer == GMEM_HANDLE_INVALID)
         goto error;
   }

   /* create the supported extensions string */
   num_extensions = glxx_get_num_extensions();

   /* first calculate the space required to hold all names */
   extensions_size = 0;
   for (i = 0; i < num_extensions; i++)
   {
      extensions_size += strlen(glxx_get_extension(i));
   }

   s->extensions = calloc(1, extensions_size +
                           num_extensions +  /* each extension has a space delimitor */
                           1                 /* NULL terminator */);
   if (s->extensions == NULL)
      goto error;

   for (i = 0; i < num_extensions; i++)
   {
      strcat(s->extensions, glxx_get_extension(i));
      strcat(s->extensions, " ");
   }
   assert(strlen(s->extensions) == extensions_size + num_extensions);

   s->lfmt_translate_exts =
      (khrn_get_has_astc() ? GFX_LFMT_TRANSLATE_GL_EXT_ASTC : 0) |
      ((v3d_scheduler_get_v3d_ver() >= V3D_MAKE_VER(3,3,0,0)) ? GFX_LFMT_TRANSLATE_GL_EXT_SR8_SRG8 : 0);

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

      /* shutdown the driver */
#ifdef KHRN_GEOMD
      fmem_debug_info_term_process();
#endif
      khrn_statics_shutdown(&statics);
      khrn_mem_term();
      khrn_fmem_client_pool_deinit();
      v3d_parallel_term();
      v3d_platform_shutdown();
      profile_shutdown();
   }
}

bool khrn_process_init(void)
{
   verif(VCOS_SUCCESS==vcos_init());

   khrn_init_options();
   profile_init();

#ifdef KHRN_GEOMD
   if (!fmem_debug_info_init_process())
      return false;
#endif

   if (!init_v3d_client())
      return false;

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

   if (!khrn_fmem_client_pool_init())
      return false;

   return khrn_statics_init(&statics);
}
