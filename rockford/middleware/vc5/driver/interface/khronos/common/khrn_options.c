/*=============================================================================
Copyright (c) 2011 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  External interface

FILE DESCRIPTION
Environment configurable, one-shot options.
=============================================================================*/

#include "interface/khronos/common/khrn_options.h"
#include "helpers/gfx/gfx_options.h"
#include "helpers/v3d/v3d_limits.h"
#include "helpers/gfx/gfx_util_rand.h"
#include "middleware/khronos/glxx/glxx_texture.h"

#include <stdio.h>

#define VCOS_LOG_CATEGORY (&khrn_options_log)
static VCOS_LOG_CAT_T khrn_options_log = VCOS_LOG_INIT("khrn_options", VCOS_LOG_WARN);

KHRN_OPTIONS_T khrn_options;
static GFX_RAND_STATE_T random_centroid_state;

void khrn_init_options(void)
{
   /* TODO Make option naming a bit more consistent... */

   gfx_options_begin("khrn");

   /* NOTE: In Android, these option names are converted to lower-case, and the underscores are replaced
    * with periods. So, "V3D_GL_ERROR_ASSIST" becomes "v3d.gl.error.assist" under Android for example.
    * Since these key names are used in Android, remember to keep them <= 32 characters (including the
    * NULL terminator).
    */
#ifdef KHRN_CHANNEL_POSIX
   khrn_options.base_port                    = gfx_options_uint32("SIMPENROSE_BASE_PORT",          4538);
#endif

   khrn_options.nonms_double_buffer          = gfx_options_bool(  "GL_TILE_NONMS_DOUBLEBUFFER",    false);

   unsigned num_cores                        = gfx_options_uint32("KHRN_NUM_CORES",                0);
   khrn_options.render_cores                 = gfx_options_uint32("KHRN_RENDER_CORES",             num_cores);
   khrn_options.bin_cores                    = gfx_options_uint32("KHRN_BIN_CORES",                num_cores);
   khrn_options.no_tfu                       = gfx_options_bool(  "KHRN_NO_TFU",                   false);
   khrn_options.all_cores_same_st_order      = gfx_options_bool(  "GL_ALL_CORES_SAME_ST_ORDER",    false);
   khrn_options.partition_supertiles_in_sw   = gfx_options_bool(  "GL_PARTITION_SUPERTILES_IN_SW", false);
   khrn_options.z_prepass                    = gfx_options_bool(  "GL_Z_PREPASS",                  false);

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

#ifdef KHRN_AUTOCLIF
   khrn_options.autoclif_only_one_clif_i     = gfx_options_int32( "AUTOCLIF_ONLY_ONE_CLIF_I",      -1);
                                               gfx_options_str(   "AUTOCLIF_ONLY_ONE_CLIF_NAME",   NULL,
                                                     khrn_options.autoclif_only_one_clif_name,
                                                     sizeof(khrn_options.autoclif_only_one_clif_name));
   khrn_options.autoclif_bin_block_size      = gfx_options_uint32("AUTOCLIF_BIN_BLOCK_SIZE",       4 * 1024 * 1024);
#endif
                                               gfx_options_str(   "CHECKSUM_CAPTURE_FILENAME",     NULL,
                                                     khrn_options.checksum_capture_filename,
                                                     sizeof(khrn_options.checksum_capture_filename));
   khrn_options.checksum_start_buffer_index  = gfx_options_uint32("CHECKSUM_START_BUFFER_INDEX",   0);
   khrn_options.checksum_end_buffer_index    = gfx_options_uint32("CHECKSUM_END_BUFFER_INDEX",     UINT32_MAX);
   khrn_options.checksum_log_verbose         = gfx_options_bool(  "CHECKSUM_LOG_VERBOSE",          false);

   khrn_options.use_rgba5551_am              = gfx_options_bool(  "KHRN_USE_RGBA5551_AM",          false);
   khrn_options.prefer_yflipped              = gfx_options_bool(  "KHRN_PREFER_YFLIPPED",          false);
   khrn_options.force_multisample            = gfx_options_bool(  "KHRN_FORCE_MS",                 false);

   khrn_options.force_centroid               = gfx_options_bool(  "KHRN_FORCE_CENTROID",           false);
   khrn_options.random_centroid              = gfx_options_bool(  "KHRN_RANDOM_CENTROID",          false);
   khrn_options.random_centroid_seed         = gfx_options_uint32("KHRN_RANDOM_CENTROID_SEED",     42);

   khrn_options.gl_error_assist              = gfx_options_bool(  "V3D_GL_ERROR_ASSIST",           false);
   khrn_options.force_dither_off             = gfx_options_bool(  "V3D_FORCE_DITHER_OFF",          false);
   khrn_options.early_z                      = gfx_options_bool(  "GL_EARLY_Z",                    true);
   khrn_options.merge_attributes             = gfx_options_bool(  "GL_MERGE_ATTRIBUTES",           false);
   khrn_options.max_worker_threads           = gfx_options_uint32( "KHRN_MAX_WORKER_THREADS",      3);

   gfx_options_end();

   if (khrn_options.random_centroid)
      gfx_rand_init(&random_centroid_state, khrn_options.random_centroid_seed);
}

bool khrn_options_make_centroid(void)
{
   if (khrn_options.force_centroid)
      return true;

   if (khrn_options.random_centroid)
      return gfx_rand_with_prob(&random_centroid_state, 0.5f);

   return false;
}

/* TODO Why is this here? */
void khrn_error_assist(GLenum error, const char *func, const char *file, int line)
{
   if (khrn_options.gl_error_assist && error != GL_NO_ERROR)
   {
      switch (error)
      {
      case GL_INVALID_ENUM      : vcos_log_warn("V3D ERROR ASSIST : GL_INVALID_ENUM in %s (%s, %d)\n", func, file, line); break;
      case GL_INVALID_VALUE     : vcos_log_warn("V3D ERROR ASSIST : GL_INVALID_VALUE in %s (%s, %d)\n", func, file, line); break;
      case GL_INVALID_OPERATION : vcos_log_warn("V3D ERROR ASSIST : GL_INVALID_OPERATION in %s (%s, %d)\n", func, file, line); break;
      case GL_OUT_OF_MEMORY     : vcos_log_warn("V3D ERROR ASSIST : GL_OUT_OF_MEMORY in %s (%s, %d)\n", func, file, line); break;
      default                   : vcos_log_warn("V3D ERROR ASSIST : ERROR CODE %d in %s (%s, %d)\n", (int)error, func, file, line); break;
      }
   }
}
