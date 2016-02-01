/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef V3D_UTIL_H
#define V3D_UTIL_H

#include "helpers/v3d/v3d_common.h"
#include "helpers/gfx/gfx_util.h"
#include "helpers/assert.h"

VCOS_EXTERN_C_BEGIN

static inline v3d_addr_t v3d_addr_min(v3d_addr_t a, v3d_addr_t b)
{
   return (a < b) ? a : b;
}

static inline v3d_addr_t v3d_addr_max(v3d_addr_t a, v3d_addr_t b)
{
   return (a > b) ? a : b;
}

static inline v3d_addr_t v3d_addr_offset(v3d_addr_t addr, size_t offset)
{
   v3d_addr_t offset_addr;
   assert(offset <= (v3d_addr_t)-1);
   offset_addr = (v3d_addr_t)(addr + offset);
   assert(offset_addr >= addr);
   return offset_addr;
}

static inline v3d_addr_t v3d_addr_align_down(v3d_addr_t addr, uint32_t align)
{
   assert(gfx_is_power_of_2(align));
   return addr & ~(v3d_addr_t)(align - 1);
}

static inline v3d_addr_t v3d_addr_align_up(v3d_addr_t addr, uint32_t align)
{
   assert(gfx_is_power_of_2(align));
   return v3d_addr_offset(addr, align - 1) & ~(v3d_addr_t)(align - 1);
}

static inline bool v3d_addr_aligned(v3d_addr_t addr, uint32_t align)
{
   assert(gfx_is_power_of_2(align));
   return !(addr & (align - 1));
}

#ifndef NDEBUG
#define v3d_assert_addr_range_within(BEGIN, SIZE, PERMITTED_BEGIN, PERMITTED_SIZE)                 \
   do                                                                                              \
   {                                                                                               \
      v3d_addr_t begin_ = (BEGIN), permitted_begin_ = (PERMITTED_BEGIN);                           \
      size_t size_ = (SIZE), permitted_size_ = (PERMITTED_SIZE);                                   \
      assert_msg(                                                                                  \
         (begin_ >= permitted_begin_) &&                                                           \
         (v3d_addr_offset(begin_, size_) <= v3d_addr_offset(permitted_begin_, permitted_size_)),   \
         "Address range 0x%08x..0x%08x outside permitted range 0x%08x..0x%08x",                    \
         begin_, v3d_addr_offset(begin_, size_),                                                   \
         permitted_begin_, v3d_addr_offset(permitted_begin_, permitted_size_));                    \
   } while (0)
#else
#define v3d_assert_addr_range_within(BEGIN, SIZE, PERMITTED_BEGIN, PERMITTED_SIZE)
#endif

// Returns tile size in pixels - NOT subsamples
static inline void v3d_tile_size_pixels(
   uint32_t *width,
   uint32_t *height,
   uint32_t color_rendertarget_count,
   bool doublebuffer_enabled,
   uint32_t max_rt_bpp,
   bool ms_enabled)
{
   unsigned int split_count = 0;
   *width = 64; // TODO use ident
   *height = 64;

   if (ms_enabled)
      split_count += 2;

   if (doublebuffer_enabled)
      ++split_count;

   switch (max_rt_bpp) {
   case 32:
      break;
   case 64:
      ++split_count;
      break;
   case 128:
      split_count += 2;
      break;
   default:
      assert(0);
   }

   switch (color_rendertarget_count) {
   case 0: /* TODO 0 isn't really supported? */
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
      assert(0);
   }

   for (;;) {
      if (!split_count)
         break;

      *height /= 2;
      --split_count;

      if (!split_count)
         break;

      *width /= 2;
      --split_count;
   }
}

VCOS_EXTERN_C_END

#endif
