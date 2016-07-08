/*=============================================================================
Broadcom Proprietary and Confidential. (c)2011 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  External interface

FILE DESCRIPTION
Environment configurable, one-shot options.
=============================================================================*/

#ifndef KHRN_OPTIONS_H
#define KHRN_OPTIONS_H

#include "khrn_int_common.h"
#include "../glxx/gl_public_api.h"
#include "vcos.h"

VCOS_EXTERN_C_BEGIN

typedef struct {
#ifdef KHRN_CHANNEL_POSIX
   uint32_t base_port;
#endif

   bool nonms_double_buffer;

   uint32_t render_subjobs;
   uint32_t bin_subjobs;
   bool all_cores_same_st_order; /* Essentially gives the same render list to each core */
   bool partition_supertiles_in_sw; /* ie don't give all supertiles to all
                                       cores, give each core an exclusive subset */

   /* isolate_frame == -1 disables tile isolation. Otherwise, isolate_supertile_x &
    * isolate_supertile_y determine the tile to isolate in frame number
    * isolate_frame */
   uint32_t isolate_frame;
   uint32_t isolate_supertile_x;
   uint32_t isolate_supertile_y;

   uint32_t min_supertile_w;
   uint32_t min_supertile_h;
   uint32_t max_supertiles;

#ifdef KHRN_GEOMD
   bool geomd;
#endif

   int32_t              autoclif_only_one_clif_i; /* < 0 means capture all frames */
   char                 autoclif_only_one_clif_name[VCOS_PROPERTY_VALUE_MAX];
   uint32_t             autoclif_bin_block_size; /* Set the size of the binning memory block in recorded CLIFs */

   char                 checksum_capture_filename[VCOS_PROPERTY_VALUE_MAX];
   uint32_t             checksum_start_buffer_index;
   uint32_t             checksum_end_buffer_index;

   /* This is a hack for testing the alpha-masked RGBA5551 TLB output format in
    * GL. That output format is really only intended for use by VG.
    *
    * Essentially this forces GL to use the alpha-masked format whenever it
    * would have used the regular RGBA5551 format. */
   bool use_rgba5551_am;

   /* Prefer y-flipped buffers. This option is only intended to be used for
    * testing purposes. */
   bool prefer_yflipped;

   /* Force multisample mode. This option is only intended to be used for
    * testing purposes.
    *
    * This operates at a very low level, essentially just forcing on the
    * multisample-enable bit in various control list instructions. There are
    * almost certainly cases where this will cause something bad to happen...
    * hence "only for testing purposes". */
   bool force_multisample;

   /* Test modes for forcing centroid varyings */
   bool force_centroid;
   bool random_centroid;
   uint32_t random_centroid_seed;

   bool     gl_error_assist;           /* Outputs useful info when the error occurs */
   bool     force_dither_off;          /* Ensure dithering is always off */
   bool     z_prepass;                 /* Z-prepass enabled */
   bool     no_empty_tile_skip;        /* No empty tile skipping. */
   bool     no_empty_tile_fill;        /* No empty tile filling. */
   bool     no_gfxh_1385;              /* Disable workarounds for GFXH-1385. */
   bool     early_z;                   /* Use early-Z */
   bool     merge_attributes;
   bool     no_compute_batching;
   uint32_t max_worker_threads;        /* Maximum number of worker threads to spawn for computing in parallel */

} KHRN_OPTIONS_T;

extern KHRN_OPTIONS_T khrn_options;

extern void khrn_init_options(void);
extern bool khrn_options_make_centroid(void);
extern void khrn_error_assist(GLenum error, const char *func, const char *file, int line);

#ifdef WIN32
#undef __func__
#define __func__ __FUNCTION__
#endif

VCOS_EXTERN_C_END

#endif
