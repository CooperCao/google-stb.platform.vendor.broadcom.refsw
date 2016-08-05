/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

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

   uint32_t frame_num_layers;
   uint32_t frame_w_in_tiles;
   uint32_t frame_h_in_tiles;
   bool ms_mode;
   uint32_t tile_w_px;
   uint32_t tile_h_px;
};

extern void v3d_bcfg_collect(
   struct v3d_bcfg *bcfg,
   const V3D_CL_TILE_BINNING_MODE_CFG_T *i);

#endif
