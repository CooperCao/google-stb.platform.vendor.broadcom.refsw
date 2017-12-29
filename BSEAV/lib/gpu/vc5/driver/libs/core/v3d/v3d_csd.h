/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_gen.h"

#if V3D_VER_AT_LEAST(4,1,34,0)
#include "v3d_limits.h"
#include "libs/util/gfx_util/gfx_util.h"

static inline void v3d_csd_cfg_extract_shader_args(
   V3D_SHADER_ARGS_T *args, const V3D_CSD_CFG_T *cfg)
{
   args->threading = cfg->threading;
   args->single_seg = cfg->single_seg;
   args->propagate_nans = cfg->propagate_nans;
   args->addr = cfg->shader_addr;
   args->unifs_addr = cfg->unifs_addr;
}

static inline void v3d_csd_cfg_compute_num_batches(V3D_CSD_CFG_T* cfg)
{
   cfg->batches_per_sg = gfx_udiv_round_up(cfg->wgs_per_sg * cfg->wg_size, V3D_VPAR);
   uint64_t num_wgs = (uint64_t)cfg->num_wgs_x * cfg->num_wgs_y * cfg->num_wgs_z;
   uint64_t num_whole_sgs = num_wgs / cfg->wgs_per_sg;
   uint32_t rem_wgs = (uint32_t)(num_wgs - num_whole_sgs*cfg->wgs_per_sg);
   cfg->num_batches = cfg->batches_per_sg*num_whole_sgs + gfx_udiv_round_up(rem_wgs * cfg->wg_size, V3D_VPAR);
}

#endif
