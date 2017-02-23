/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "v3d_common.h"
#include "v3d_gen.h"

// Returns tile size in pixels - NOT subsamples
static inline void v3d_tile_size_pixels(
   uint32_t *width, uint32_t *height,
   bool ms, bool double_buffer,
   uint32_t num_rts, v3d_rt_bpp_t max_rt_bpp)
{
   unsigned int split_count = 0;

   if (ms)
      split_count += 2;

   if (double_buffer)
      ++split_count;

   switch (max_rt_bpp)
   {
   case V3D_RT_BPP_32:
      break;
   case V3D_RT_BPP_64:
      ++split_count;
      break;
   case V3D_RT_BPP_128:
      split_count += 2;
      break;
   default:
      unreachable();
   }

   switch (num_rts)
   {
   case 1:
      break;
   case 2:
      ++split_count;
      break;
   case 3:
   case 4:
      split_count += 2;
      break;
   default:
      unreachable();
   }

   *width = 64; // TODO use ident
   *height = 64;

   for (;;)
   {
      if (!split_count)
         break;

      *height /= 2;
      --split_count;

      if (!split_count)
         break;

      *width /= 2;
      --split_count;
   }

   assert(*width >= 8);
   assert(*height >= 8);
}
