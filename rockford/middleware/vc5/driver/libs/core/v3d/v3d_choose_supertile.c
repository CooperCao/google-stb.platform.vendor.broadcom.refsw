/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "v3d_common.h"
#include "v3d_limits.h"
#include "libs/util/gfx_util/gfx_util.h"

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
