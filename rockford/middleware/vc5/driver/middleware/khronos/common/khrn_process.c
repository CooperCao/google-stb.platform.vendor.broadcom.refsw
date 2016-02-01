/*=============================================================================
Copyright (c) 2011 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
Contains per-process state and per-thread state
=============================================================================*/
#define VCOS_LOG_CATEGORY (&khrn_process_log)

#include <stdlib.h>
#include "vcos.h"
#include "interface/khronos/common/khrn_options.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_process.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "middleware/khronos/common/khrn_process_debug.h"
#include "middleware/khronos/common/khrn_fmem_debug_info.h"
#include "middleware/khronos/glxx/glxx_server.h"
#include "middleware/khronos/glxx/glxx_extensions.h"
#include "gmem.h"
#include "helpers/v3d/v3d_config.h"
#include "helpers/v3d/v3d_debug.h"
#include "bcm_sched_api.h"
#include "v3d_platform.h"
#include "v3d_parallel.h"
#include "v3d_scheduler.h"
#include "v3d_driver_api.h"
#include "helpers/gfx/gfx_lfmt_translate_gl.h"

static VCOS_LOG_CAT_T khrn_process_log;

static struct statics {
   gmem_handle_t dummy_texture_handle;
   gmem_handle_t gfxh_1320_buffer;
   bool initialized;
#if !V3D_HAS_NEW_TMU_CFG
   V3D_MISCCFG_T misccfg;
#endif
   char *gl30_extensions;
   gfx_lfmt_translate_gl_ext_t lfmt_translate_exts;
} statics;

static void check_hw_version(void)
{
   const V3D_IDENT_T *identity = v3d_scheduler_get_identity();
   vcos_log_warn("HW version %d.%d:%d:%d",
         identity->v3d_tech_version,
         identity->v3d_revision,
         identity->ip_recipient,
         identity->ip_recipient_revision);
   assert_msg(
      (identity->v3d_tech_version == V3D_TECH_VERSION) &&
      (identity->v3d_revision == V3D_REVISION),
      "Unsupported HW version!");
}

static bool init_v3d_client(void)
{
   bool mem = false, ok = false;

   mem = khrn_mem_init();
   if (!mem) goto end;

   if (!v3d_platform_init())
   {
      vcos_log_error("Failed to initialise platform\n");
      exit(EXIT_FAILURE);
   }

#ifdef KHRN_GEOMD
   if (khrn_options.geomd)
      v3d_platform_set_debug_callback(khrn_debug_callback, NULL);
#endif

   if (!v3d_parallel_init(khrn_options.max_worker_threads))
      goto end;

   check_hw_version();

#if !V3D_HAS_NEW_TMU_CFG
   statics.misccfg.ovrtmuout = khrn_get_v3d_version() >= V3D_MAKE_VER(3, 3);
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

gmem_handle_t khrn_get_gfxh_1320_buffer(void)
{
   return statics.gfxh_1320_buffer;
}

char *khrn_get_gl30_extensions(void)
{
   return statics.gl30_extensions;
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
   free (s->gl30_extensions);
   s->dummy_texture_handle = 0;
   s->gfxh_1320_buffer = 0;
   s->gl30_extensions = NULL;
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
   if (khrn_get_v3d_version() == V3D_MAKE_VER(3,2)) // GFXH-1320
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
   num_extensions = glxx_get_num_gl30_extensions();

   /* first calculate the space required to hold all names */
   extensions_size = 0;
   for (i = 0; i < num_extensions; i++)
   {
      extensions_size += strlen(glxx_get_gl30_extension(i));
   }

   s->gl30_extensions = calloc(1, extensions_size +
                           num_extensions +  /* each extension has a space delimitor */
                           1                 /* NULL terminator */);
   if (s->gl30_extensions == NULL)
      goto error;

   for (i = 0; i < num_extensions; i++)
   {
      strcat(s->gl30_extensions, glxx_get_gl30_extension(i));
      strcat(s->gl30_extensions, " ");
   }
   assert(strlen(s->gl30_extensions) == extensions_size + num_extensions);

   s->lfmt_translate_exts =
      (khrn_get_has_astc() ? GFX_LFMT_TRANSLATE_GL_EXT_ASTC : 0) |
      ((v3d_scheduler_get_v3d_ver() >= V3D_MAKE_VER(3, 3)) ? GFX_LFMT_TRANSLATE_GL_EXT_SR8_SRG8 : 0);

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
   }
}

bool khrn_process_init(void)
{
   verif(VCOS_SUCCESS==vcos_init());

   vcos_log_set_level(&khrn_process_log, VCOS_LOG_WARN);
   vcos_log_register("khrn_process", &khrn_process_log);
   khrn_init_options();

#ifdef KHRN_GEOMD
   if (!fmem_debug_info_init_process())
      return false;
#endif

   if (!init_v3d_client())
      return false;

   // Default to using all cores, clamp environment setting to available.
   // Need to do this after the scheduler session has been brought up.
   uint32_t num_cores = khrn_get_num_cores_uncapped();
   khrn_options.render_cores  = khrn_options.render_cores ? gfx_umin(khrn_options.render_cores, num_cores) : num_cores;
   khrn_options.bin_cores     = khrn_options.bin_cores    ? gfx_umin(khrn_options.bin_cores,    num_cores) : num_cores;
   // Further clamp to max subjobs of type...
   khrn_options.render_cores  = gfx_umin(khrn_options.render_cores,  V3D_MAX_RENDER_SUBJOBS);
   khrn_options.bin_cores     = gfx_umin(khrn_options.bin_cores,     V3D_MAX_BIN_SUBJOBS);

   if (!khrn_fmem_client_pool_init())
      return false;

   return khrn_statics_init(&statics);
}
