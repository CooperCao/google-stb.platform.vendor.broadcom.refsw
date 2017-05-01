/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_options.h"
#include "libs/util/gfx_options/gfx_options.h"
#include "libs/core/v3d/v3d_limits.h"
#include "libs/util/gfx_util/gfx_util_rand.h"
#include "../glxx/glxx_texture.h"

#include <stdio.h>

LOG_DEFAULT_CAT("khrn_options")

struct khrn_options khrn_options;
static GFX_RAND_STATE_T random_centroid_state;
#if V3D_HAS_VARY_NO_PERSP
static GFX_RAND_STATE_T random_noperspective_state;
#endif
#if V3D_VER_AT_LEAST(4,0,2,0)
static GFX_RAND_STATE_T random_sample_rate_shading_state;
#endif

void khrn_init_options(void)
{
   /* TODO Make option naming a bit more consistent... */

   /* NOTE: In Android, these option names are converted to lower-case, and the underscores are replaced
    * with periods. So, "V3D_GL_ERROR_ASSIST" becomes "v3d.gl.error.assist" under Android for example.
    * Since these key names are used in Android, remember to keep them <= 32 characters (including the
    * NULL terminator).
    */

   khrn_options.nonms_double_buffer          = gfx_options_bool(  "GL_TILE_NONMS_DOUBLEBUFFER",    false);

   unsigned num_subjobs                      = gfx_options_uint32("KHRN_NUM_SUBJOBS",              0);
   khrn_options.render_subjobs               = gfx_options_uint32("KHRN_RENDER_SUBJOBS",           num_subjobs);
   khrn_options.bin_subjobs                  = gfx_options_uint32("KHRN_BIN_SUBJOBS",              num_subjobs);
   khrn_options.all_cores_same_st_order      = gfx_options_bool(  "GL_ALL_CORES_SAME_ST_ORDER",    false);
   khrn_options.partition_supertiles_in_sw   = gfx_options_bool(  "GL_PARTITION_SUPERTILES_IN_SW", false);
   khrn_options.z_prepass                    = gfx_options_bool(  "GL_Z_PREPASS",                  false);
   khrn_options.no_empty_tile_skip           = gfx_options_bool("GL_NO_EMPTY_TILE_SKIP",           false);
   khrn_options.no_compute_batching          = gfx_options_bool("GL_NO_COMPUTE_BATCHING",          false);

   khrn_options.isolate_frame                = gfx_options_int32( "GL_ISOLATE_FRAME",              ~0u); /* Disable by default */
   if (khrn_options.isolate_frame != ~0u) {
      khrn_options.isolate_supertile_x       = gfx_options_uint32("GL_ISOLATE_TILE_X",             0); // remove these after updating appclifs
      khrn_options.isolate_supertile_y       = gfx_options_uint32("GL_ISOLATE_TILE_Y",             0);
      khrn_options.isolate_supertile_x       = gfx_options_uint32("GL_ISOLATE_SUPERTILE_X",        khrn_options.isolate_supertile_x);
      khrn_options.isolate_supertile_y       = gfx_options_uint32("GL_ISOLATE_SUPERTILE_Y",        khrn_options.isolate_supertile_y);
   }

   khrn_options.min_supertile_w              = gfx_options_uint32("GL_MIN_SUPERTILE_W",            1);
   khrn_options.min_supertile_h              = gfx_options_uint32("GL_MIN_SUPERTILE_H",            1);
   khrn_options.max_supertiles               = gfx_options_uint32("GL_MAX_SUPERTILES",             V3D_MAX_SUPERTILES);

#ifdef KHRN_GEOMD
   khrn_options.geomd                        = gfx_options_bool(  "GL_GEOMD",                      false);
#endif

#if KHRN_DEBUG
   khrn_options.no_gmp                       = gfx_options_bool("KHRN_NO_GMP", false);
   khrn_options.save_crc_enabled             = gfx_options_bool("KHRN_SAVE_CRCS", false);
   khrn_options.autoclif_enabled             = gfx_options_bool(  "KHRN_AUTOCLIF",                 false);
   khrn_options.autoclif_only_one_clif_i     = gfx_options_int32( "AUTOCLIF_ONLY_ONE_CLIF_I",      -1);
                                               gfx_options_str(   "AUTOCLIF_ONLY_ONE_CLIF_NAME",   "",
                                                     khrn_options.autoclif_only_one_clif_name,
                                                     sizeof(khrn_options.autoclif_only_one_clif_name));
   khrn_options.autoclif_bin_block_size      = gfx_options_uint32("AUTOCLIF_BIN_BLOCK_SIZE",       4 * 1024 * 1024);
#endif
                                               gfx_options_str(   "CHECKSUM_CAPTURE_FILENAME",     "",
                                                     khrn_options.checksum_capture_filename,
                                                     sizeof(khrn_options.checksum_capture_filename));
   khrn_options.checksum_start_buffer_index  = gfx_options_uint32("CHECKSUM_START_BUFFER_INDEX",   0);
   khrn_options.checksum_end_buffer_index    = gfx_options_uint32("CHECKSUM_END_BUFFER_INDEX",     UINT32_MAX);

   khrn_options.use_rgba5551_am              = gfx_options_bool(  "KHRN_USE_RGBA5551_AM",          false);
   khrn_options.prefer_yflipped              = gfx_options_bool(  "KHRN_PREFER_YFLIPPED",          false);
   khrn_options.force_multisample            = gfx_options_bool(  "KHRN_FORCE_MS",                 false);

   khrn_options.force_centroid               = gfx_options_bool(  "KHRN_FORCE_CENTROID",           false);
   khrn_options.random_centroid              = gfx_options_bool(  "KHRN_RANDOM_CENTROID",          false);
   khrn_options.random_centroid_seed         = gfx_options_uint32("KHRN_RANDOM_CENTROID_SEED",     42);

#if V3D_HAS_VARY_NO_PERSP
   khrn_options.force_noperspective          = gfx_options_bool(  "KHRN_FORCE_NOPERSPECTIVE",      false);
   khrn_options.random_noperspective         = gfx_options_bool(  "KHRN_RANDOM_NOPERSPECTIVE",     false);
   khrn_options.random_noperspective_seed    = gfx_options_uint32("KHRN_RANDOM_NOPERSPECTIVE_SEED",42);
#endif

#if V3D_VER_AT_LEAST(4,0,2,0)
   khrn_options.force_sample_rate_shading    = gfx_options_bool(  "KHRN_FORCE_SAMPLE_RATE_SHADING", false);
   khrn_options.random_sample_rate_shading   = gfx_options_bool(  "KHRN_RANDOM_SAMPLE_RATE_SHADING", false);
   khrn_options.random_sample_rate_shading_seed = gfx_options_uint32("KHRN_RANDOM_SAMPLE_RATE_SHADING_SEED", 42);
#endif

   khrn_options.force_dither_off             = gfx_options_bool(  "V3D_FORCE_DITHER_OFF",          false);
   khrn_options.early_z                      = gfx_options_bool(  "GL_EARLY_Z",                    true);
   khrn_options.max_worker_threads           = gfx_options_uint32( "KHRN_MAX_WORKER_THREADS",      3);

   khrn_options.no_async_host_reads    = gfx_options_bool("KHRN_NO_ASYNC_HOST_READS", false);
   khrn_options.force_async_host_reads = gfx_options_bool("KHRN_FORCE_ASYNC_HOST_READS", false);

   if (khrn_options.random_centroid)
      gfx_rand_init(&random_centroid_state, khrn_options.random_centroid_seed);
#if V3D_HAS_VARY_NO_PERSP
   if (khrn_options.random_noperspective)
      gfx_rand_init(&random_noperspective_state, khrn_options.random_noperspective_seed);
#endif
#if V3D_VER_AT_LEAST(4,0,2,0)
   if (khrn_options.random_sample_rate_shading)
      gfx_rand_init(&random_sample_rate_shading_state, khrn_options.random_sample_rate_shading_seed);
#endif
}

bool khrn_options_make_centroid(void)
{
   if (khrn_options.force_centroid)
      return true;

   if (khrn_options.random_centroid)
      return gfx_rand_with_prob(&random_centroid_state, 0.5f);

   return false;
}

#if V3D_HAS_VARY_NO_PERSP
bool khrn_options_make_noperspective(void)
{
   if (khrn_options.force_noperspective)
      return true;

   if (khrn_options.random_noperspective)
      return gfx_rand_with_prob(&random_noperspective_state, 0.5f);

   return false;
}
#endif

#if V3D_VER_AT_LEAST(4,0,2,0)
bool khrn_options_make_sample_rate_shaded(void)
{
   if (khrn_options.force_sample_rate_shading)
      return true;

   if (khrn_options.random_sample_rate_shading)
      return gfx_rand_with_prob(&random_sample_rate_shading_state, 0.5f);

   return false;
}
#endif
