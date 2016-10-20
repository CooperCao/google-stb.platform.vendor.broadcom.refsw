/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "v3d_ver.h"
#include "v3d_gen.h"
#include "vcos_types.h"
#include <stdbool.h>
#include <stdint.h>

// See "VPM Usage" section in arch spec.

#if V3D_HAS_TNG

typedef struct v3d_vpm_cfg_v
{
   V3D_IN_SEG_ARGS_T input_size[2];
   V3D_OUT_SEG_ARGS_T output_size[2];
   uint8_t vcm_cache_size[2];
} v3d_vpm_cfg_v;

typedef struct v3d_vpm_cfg_t
{
   uint8_t per_patch_depth[2];
   uint8_t min_per_patch_segs[2];
   V3D_SEG_ARGS_T tcs_output[2];
   uint8_t max_extra_vert_segs_per_tcs_batch[2];
   uint8_t max_patches_per_tcs_batch;
   v3d_cl_tcs_batch_flush_mode_t tcs_batch_flush;
   uint8_t min_tcs_segs[2];
   V3D_SEG_ARGS_T tes_output[2];
   uint8_t max_extra_vert_segs_per_tes_batch[2];
   uint8_t max_tcs_segs_per_tes_batch[2];
   uint8_t max_patches_per_tes_batch;
   uint8_t min_tes_segs[2];
} v3d_vpm_cfg_t;

typedef struct v3d_vpm_cfg_g
{
   V3D_GEOM_SEG_ARGS_T geom_output[2];
   uint8_t max_extra_vert_segs_per_gs_batch[2];
   uint8_t min_gs_segs[2];
} v3d_vpm_cfg_g;

void v3d_vpm_default_cfg_t(v3d_vpm_cfg_t* cfg_t);
void v3d_vpm_default_cfg_g(v3d_vpm_cfg_g* cfg_g);

bool v3d_vpm_compute_cfg_tg(
   v3d_vpm_cfg_v* v,
   v3d_vpm_cfg_t* t,
   v3d_vpm_cfg_g* g,
   unsigned vpm_size_in_sectors,
   uint8_t const vs_input_words[2],
   uint8_t const vs_output_words[2],
   uint8_t const tcs_patch_vertices,
   uint8_t const tcs_patch_words[2],
   uint8_t const tcs_words[2],
   bool tcs_barriers,
   uint8_t tes_patch_vertices,
   uint8_t const tes_words[2],
   v3d_cl_tess_type_t tess_type,
   uint8_t gs_max_prim_vertices,
   uint16_t const gs_output_words[2]);

void v3d_vpm_cfg_validate(
   v3d_vpm_cfg_v const* v,
   v3d_vpm_cfg_t const* t,
   v3d_vpm_cfg_g const* g,
   unsigned vpm_size_in_sectors,
   unsigned max_input_vertices,
   unsigned tes_patch_vertices);

#else

typedef struct v3d_vpm_cfg_v
{
   uint8_t input_size[2];
   uint8_t output_size[2];
   uint8_t vcm_cache_size[2];
} v3d_vpm_cfg_v;

#endif

void v3d_vpm_compute_cfg(
   v3d_vpm_cfg_v* cfg,
   unsigned vpm_size_in_sectors,
   uint8_t const vs_input_words[2],
   uint8_t const vs_output_words[2],
   bool z_pre_pass);
