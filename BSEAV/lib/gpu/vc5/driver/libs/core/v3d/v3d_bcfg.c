/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_common.h"
#include "v3d_align.h"
#include "v3d_bcfg.h"
#include "v3d_util.h"
#include "v3d_tile_size.h"

void v3d_bcfg_collect(
   struct v3d_bcfg *bcfg,
   const V3D_CL_TILE_BINNING_MODE_CFG_T *i)
{
#if V3D_HAS_UNCONSTR_VP_CLIP
   bcfg->tile_alloc_ibs = v3d_translate_from_tile_alloc_block_size(
      i->tile_alloc_initial_block_size);
   bcfg->tile_alloc_bs = v3d_translate_from_tile_alloc_block_size(
      i->tile_alloc_block_size);

   bcfg->ms_mode = i->ms_mode;
   v3d_log2_tile_size_pixels(&bcfg->log2_tile_w_px, &bcfg->log2_tile_h_px,
      i->ms_mode, i->double_buffer,
      i->num_rts, i->max_bpp);
   bcfg->tile_w_px = 1u << bcfg->log2_tile_w_px;
   bcfg->tile_h_px = 1u << bcfg->log2_tile_h_px;

   bcfg->frame_w_in_pixels = i->w_in_pixels;
   bcfg->frame_h_in_pixels = i->h_in_pixels;
   bcfg->frame_w_in_tiles = gfx_udiv_round_up(i->w_in_pixels, bcfg->tile_w_px);
   bcfg->frame_h_in_tiles = gfx_udiv_round_up(i->h_in_pixels, bcfg->tile_h_px);
#else
   switch (i->type)
   {
   case V3D_BCFG_TYPE_PART1:
   {
      const V3D_CL_TILE_BINNING_MODE_CFG_PART1_T *part1 = &i->u.part1;

#if !V3D_HAS_QTS
      assert(part1->auto_init_tile_state);
#endif

      bcfg->tile_alloc_ibs = v3d_translate_from_tile_alloc_block_size(
         part1->tile_alloc_initial_block_size);
      bcfg->tile_alloc_bs = v3d_translate_from_tile_alloc_block_size(
         part1->tile_alloc_block_size);

#if V3D_HAS_QTS
      if (part1->set_tile_state_addr)
#endif
      {
         assert(part1->tile_state_addr);
         assert(v3d_addr_aligned(part1->tile_state_addr, V3D_TILE_STATE_ALIGN));
         bcfg->tile_state_addr = part1->tile_state_addr;
      }

      bcfg->ms_mode = part1->ms_mode;
      v3d_log2_tile_size_pixels(&bcfg->log2_tile_w_px, &bcfg->log2_tile_h_px,
         part1->ms_mode, part1->double_buffer,
         part1->num_rts, part1->max_bpp);
      bcfg->tile_w_px = 1u << bcfg->log2_tile_w_px;
      bcfg->tile_h_px = 1u << bcfg->log2_tile_h_px;

      bcfg->frame_w_in_tiles = part1->w_in_tiles;
      bcfg->frame_h_in_tiles = part1->h_in_tiles;

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
#endif
}
