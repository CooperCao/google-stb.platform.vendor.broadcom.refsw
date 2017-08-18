/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_ver.h"
#include "v3d_gen.h"
#include "vcos_types.h"
#include <stdbool.h>
#include <stdint.h>

// See "VPM Usage" section in arch spec.

EXTERN_C_BEGIN

#if V3D_VER_AT_LEAST(4,0,2,0)

typedef struct v3d_vpm_cfg_v
{
   V3D_IN_SEG_ARGS_T input_size;
   V3D_OUT_SEG_ARGS_T output_size;
   uint8_t vcm_cache_size;
} v3d_vpm_cfg_v;

bool v3d_vpm_compute_cfg_tg(
   v3d_vpm_cfg_v v[2],
   V3D_VPM_CFG_TG_T tg[2], bool t, bool g,
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
   v3d_vpm_cfg_v const v[2],
   // tg may be NULL if !t && !g
   V3D_VPM_CFG_TG_T const tg[2], bool t, bool g,
   unsigned vpm_size_in_sectors,
   unsigned max_input_vertices,
   unsigned tes_patch_vertices);

#else

typedef struct v3d_vpm_cfg_v
{
   uint8_t input_size;
   uint8_t output_size;
   uint8_t vcm_cache_size;
} v3d_vpm_cfg_v;

#endif

void v3d_vpm_compute_cfg(
   v3d_vpm_cfg_v cfg[2],
   unsigned vpm_size_in_sectors,
   uint8_t const vs_input_words[2],
   uint8_t const vs_output_words[2],
   bool z_pre_pass);

EXTERN_C_END
