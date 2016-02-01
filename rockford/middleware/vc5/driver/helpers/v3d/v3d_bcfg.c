/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "helpers/v3d/v3d_common.h"
#include "helpers/v3d/v3d_align.h"
#include "helpers/v3d/v3d_bcfg.h"
#include "helpers/v3d/v3d_util.h"

void v3d_bcfg_collect(
   struct v3d_bcfg *bcfg,
   const V3D_CL_TILE_BINNING_MODE_CFG_T *i)
{
   switch (i->type)
   {
   case V3D_BCFG_TYPE_PART1:
   {
      const V3D_CL_TILE_BINNING_MODE_CFG_PART1_T *part1 = &i->u.part1;

      assert(part1->auto_init_tile_state);

      bcfg->tile_alloc_ibs = v3d_translate_from_tile_alloc_block_size(
         part1->tile_alloc_initial_block_size);
      bcfg->tile_alloc_bs = v3d_translate_from_tile_alloc_block_size(
         part1->tile_alloc_block_size);
      assert(part1->tile_state_addr);
      assert(v3d_addr_aligned(part1->tile_state_addr, V3D_TILE_STATE_ALIGN));
      bcfg->tile_state_addr = part1->tile_state_addr;

      bcfg->frame_w_in_tiles = part1->w_in_tiles;
      bcfg->frame_h_in_tiles = part1->h_in_tiles;
      bcfg->ms_mode = part1->ms_mode;
      v3d_tile_size_pixels(
         &bcfg->tile_w_px, &bcfg->tile_h_px,
         part1->num_rts, part1->double_buffer,
         v3d_translate_from_rt_bpp(part1->max_bpp), part1->ms_mode);

      break;
   }
   case V3D_BCFG_TYPE_PART2:
   {
      const V3D_CL_TILE_BINNING_MODE_CFG_PART2_T *part2 = &i->u.part2;

      bcfg->tile_alloc_size = part2->tile_alloc_size;
      assert(part2->tile_alloc_addr);
      bcfg->tile_alloc_addr = part2->tile_alloc_addr;

      break;
   }
   default:
      unreachable();
   }
}
