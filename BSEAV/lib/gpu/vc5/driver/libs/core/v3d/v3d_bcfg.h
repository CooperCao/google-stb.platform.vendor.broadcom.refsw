/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_BCFG_H
#define V3D_BCFG_H

#include "v3d_cl.h"

struct v3d_bcfg
{
   uint32_t tile_alloc_size;
   v3d_addr_t tile_alloc_addr;

   uint32_t tile_alloc_ibs;
   uint32_t tile_alloc_bs;

   v3d_addr_t tile_state_addr;

   bool ms_mode;
   uint32_t log2_tile_w_px;
   uint32_t log2_tile_h_px;
   uint32_t tile_w_px;
   uint32_t tile_h_px;

   uint32_t frame_num_layers;
#if V3D_HAS_UNCONSTR_VP_CLIP
   uint32_t frame_w_in_pixels;
   uint32_t frame_h_in_pixels;
#endif
   uint32_t frame_w_in_tiles;
   uint32_t frame_h_in_tiles;
};

extern void v3d_bcfg_collect(
   struct v3d_bcfg *bcfg,
   const V3D_CL_TILE_BINNING_MODE_CFG_T *i);

#endif
