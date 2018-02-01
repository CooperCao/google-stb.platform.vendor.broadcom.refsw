/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_OPTIONS_H
#define KHRN_OPTIONS_H

#include "khrn_int_common.h"
#include "vcos.h"

EXTERN_C_BEGIN

struct khrn_options {
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

#if KHRN_DEBUG
   bool                 no_gmp;                   /* Disable GMP support. */
   bool                 no_ubo_to_unif;           /* Disable pre-loading UBO values into uniform stream. */
   bool                 save_crc_enabled;
   bool                 autoclif_enabled;
   int32_t              autoclif_single_frame; /* < 0 means capture all frames */
   char                 autoclif_filename[VCOS_PROPERTY_VALUE_MAX]; // Filename of recorded CLIF. Should only be set if autoclif_single_frame >= 0.
   char                 autoclif_prefix[VCOS_PROPERTY_VALUE_MAX]; // Prefix for recorded CLIF filenames. Ignored if autoclif_filename set.
   uint32_t             autoclif_bin_block_size; /* Set the size of the binning memory block in recorded CLIFs */
   bool                 flush_after_draw;
   bool                 no_invalidate_fb;
#endif

   char                 checksum_capture_filename[VCOS_PROPERTY_VALUE_MAX];
   bool                 checksum_capture_data;
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

   /* Test modes for forcing wireframe */
   bool force_wireframe_lines;
   bool force_wireframe_points;
   bool random_wireframe;
   uint32_t random_wireframe_seed;

   /* Test modes for forcing centroid varyings */
   bool force_centroid;
   bool random_centroid;
   uint32_t random_centroid_seed;

#if V3D_VER_AT_LEAST(4,1,34,0)
   bool force_noperspective;
   bool random_noperspective;
   uint32_t random_noperspective_seed;
#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
   /* For testing sample-rate shading */
   bool force_sample_rate_shading;
   bool random_sample_rate_shading;
   uint32_t random_sample_rate_shading_seed;
#endif

   bool     no_async_host_reads;       /* Force all host buffer reads to stall rather than happen asynchronously. */
   bool     force_async_host_reads;    /* Force all host buffer reads to happen asynchronously if possible. */
   bool     force_dither_off;          /* Ensure dithering is always off */
   bool     z_prepass;                 /* Z-prepass enabled */
   bool     no_empty_tile_skip;        /* No empty tile skipping. */
   bool     early_z;                   /* Use early-Z */
   bool     no_compute_batching;
   uint32_t max_worker_threads;        /* Maximum number of worker threads to spawn for computing in parallel */

};

extern struct khrn_options khrn_options;

extern void khrn_init_options(void);
extern bool khrn_options_make_wireframe(void);
extern bool khrn_options_make_wireframe_points(void);
extern bool khrn_options_make_centroid(void);
#if V3D_VER_AT_LEAST(4,1,34,0)
extern bool khrn_options_make_noperspective(void);
#endif
#if V3D_VER_AT_LEAST(4,1,34,0)
extern bool khrn_options_make_sample_rate_shaded(void);
#endif

#ifdef WIN32
#undef __func__
#define __func__ __FUNCTION__
#endif

EXTERN_C_END

#endif
