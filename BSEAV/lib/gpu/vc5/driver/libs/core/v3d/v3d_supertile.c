/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "v3d_common.h"
#include "v3d_limits.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/util/gfx_util/gfx_util_morton.h"

/* Both dims in tiles */
static unsigned next_valid_supertile_dim(unsigned supertile_dim, unsigned frame_dim)
{
   assert(supertile_dim <= frame_dim);
#if !V3D_VER_AT_LEAST(3,3,0,0)
   /* Avoid supertile sizes which do not divide evenly into the frame size. See GFXH-1325. */
   while ((frame_dim % supertile_dim) != 0)
      ++supertile_dim;
#endif
   return supertile_dim;
}

void v3d_choose_supertile_sizes(
   unsigned tiles_x, unsigned tiles_y,
   unsigned min_supertile_w, unsigned min_supertile_h, unsigned max_supertiles,
   unsigned *supertile_w, unsigned *supertile_h)
{
   // TODO Searching for supertile shape can be better than this
   unsigned frame_w_in_supertiles, frame_h_in_supertiles;

   // Start from min supertile sizes and try growing until we use fewer than the
   // maximum allowed
   *supertile_w = next_valid_supertile_dim(min_supertile_w, tiles_x);
   *supertile_h = next_valid_supertile_dim(min_supertile_h, tiles_y);
   for (;;)
   {
      frame_w_in_supertiles = gfx_udiv_round_up(tiles_x, *supertile_w);
      frame_h_in_supertiles = gfx_udiv_round_up(tiles_y, *supertile_h);
      unsigned supertile_count = frame_w_in_supertiles * frame_h_in_supertiles;
      if (supertile_count <= max_supertiles)
         break;

      if ((*supertile_h < *supertile_w) && (*supertile_h < tiles_y))
         *supertile_h = next_valid_supertile_dim(*supertile_h + 1, tiles_y);
      else
         *supertile_w = next_valid_supertile_dim(*supertile_w + 1, tiles_x);
   }

   // The hardware doesn't support max x 1 supertile configurations
   if (frame_w_in_supertiles == V3D_MAX_SUPERTILES)
      *supertile_w = next_valid_supertile_dim(*supertile_w + 1, tiles_x);
   if (frame_h_in_supertiles == V3D_MAX_SUPERTILES)
      *supertile_h = next_valid_supertile_dim(*supertile_h + 1, tiles_y);

   assert(*supertile_w <= tiles_x);
   assert(*supertile_h <= tiles_y);
}

void v3d_supertile_range(
   uint32_t *morton_flags, uint32_t *begin_supertile, uint32_t *end_supertile,
   uint32_t num_cores, uint32_t core,
   unsigned int num_supertiles_x, unsigned int num_supertiles_y,
   bool partition_supertiles_in_sw,
   bool all_cores_same_st_order)
{
   uint32_t num_supertiles = num_supertiles_y * num_supertiles_x;

   *morton_flags = 0;

   if (partition_supertiles_in_sw)
   {
      /* Same supertile order for all cores, but give each core an exclusive
       * subset of the supertiles... */

      uint32_t supertiles_per_core = num_supertiles / num_cores;
      uint32_t extra_supertiles = num_supertiles - (num_cores * supertiles_per_core);

      *begin_supertile = core * supertiles_per_core;
      *end_supertile = (core + 1) * supertiles_per_core;

      /* Distribute extra supertiles */
      *begin_supertile += gfx_umin(core, extra_supertiles);
      *end_supertile += gfx_umin(core + 1, extra_supertiles);

      assert(*begin_supertile <= *end_supertile);
      assert(*end_supertile <= num_supertiles);
   }
   else
   {
      /* All cores are given all supertiles, but in a different order */

      /* TODO Experiment with this */
      switch (all_cores_same_st_order ? 0 :
         /* TODO Quick hack for 8 cores */
         (core & 3))
      {
      case 0:  break;
      case 1:  *morton_flags |= GFX_MORTON_REVERSE; break;
      case 2:  *morton_flags |= GFX_MORTON_FLIP_Y; break;
      case 3:  *morton_flags |= GFX_MORTON_FLIP_Y | GFX_MORTON_REVERSE; break;
      default: not_impl();
      }

      *begin_supertile = 0;
      *end_supertile = num_supertiles;
   }

   /* TODO Is this sensible? */
   if (num_supertiles_x < num_supertiles_y)
      *morton_flags |= GFX_MORTON_TRANSPOSE;
}
