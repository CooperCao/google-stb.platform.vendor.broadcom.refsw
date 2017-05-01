/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_common.h"
#include "v3d_gen.h"

static inline void v3d_log2_tile_size_pixels(
   uint32_t *log2_width, uint32_t *log2_height,
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

   *log2_width = 6; // TODO use ident
   *log2_height = 6;

   for (;;)
   {
      if (!split_count)
         break;

      --*log2_height;
      --split_count;

      if (!split_count)
         break;

      --*log2_width;
      --split_count;
   }

   assert(*log2_width >= 3);
   assert(*log2_height >= 3);
}

// Returns tile size in pixels - NOT subsamples
static inline void v3d_tile_size_pixels(
   uint32_t *width, uint32_t *height,
   bool ms, bool double_buffer,
   uint32_t num_rts, v3d_rt_bpp_t max_rt_bpp)
{
   uint32_t log2_width, log2_height;
   v3d_log2_tile_size_pixels(&log2_width, &log2_height,
      ms, double_buffer, num_rts, max_rt_bpp);
   *width = 1u << log2_width;
   *height = 1u << log2_height;
}
