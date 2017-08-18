/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_gen.h"

#if V3D_HAS_CSD

static inline void v3d_csd_cfg_extract_shader_args(
   V3D_SHADER_ARGS_T *args, const V3D_CSD_CFG_T *cfg)
{
   args->threading = cfg->threading;
   args->single_seg = cfg->single_seg;
   args->propagate_nans = cfg->propagate_nans;
   args->addr = cfg->shader_addr;
   args->unifs_addr = cfg->unifs_addr;
}

#endif
