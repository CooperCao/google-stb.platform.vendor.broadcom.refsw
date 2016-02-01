/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#include "helpers/gfx/gfx_util_morton.h"

void gfx_morton_to_xy(uint32_t *x_out, uint32_t *y_out, uint32_t i)
{
   uint32_t x, y;

   x = i >> 0;      y = i >> 1;
   x &= 0x55555555; y &= 0x55555555;
   x |= (x >> 1);   y |= (y >> 1);
   x &= 0x33333333; y &= 0x33333333;
   x |= (x >> 2);   y |= (y >> 2);
   x &= 0x0f0f0f0f; y &= 0x0f0f0f0f;
   x |= (x >> 4);   y |= (y >> 4);
   x &= 0x00ff00ff; y &= 0x00ff00ff;
   x |= (x >> 8);   y |= (y >> 8);
   x &= 0x0000ffff; y &= 0x0000ffff;

   *x_out = x;
   *y_out = y;
}

uint32_t gfx_morton_from_xy(uint32_t x, uint32_t y)
{
   assert(!(x & ~0xffff));
   assert(!(y & ~0xffff));

   x |= (x << 8);   y |= (y << 8);
   x &= 0x00ff00ff; y &= 0x00ff00ff;
   x |= (x << 4);   y |= (y << 4);
   x &= 0x0f0f0f0f; y &= 0x0f0f0f0f;
   x |= (x << 2);   y |= (y << 2);
   x &= 0x33333333; y &= 0x33333333;
   x |= (x << 1);   y |= (y << 1);
   x &= 0x55555555; y &= 0x55555555;

   return x | (y << 1);
}

/** Iterating over a rectangle in Morton order */

void gfx_morton_init(GFX_MORTON_STATE_T *state,
   uint32_t w, uint32_t h, uint32_t flags)
{
   uint32_t tw, th, first, last;

   state->w = w;
   state->h = h;
   state->flags = flags;

   if (flags & GFX_MORTON_TRANSPOSE) {
      tw = h; th = w;
   } else {
      tw = w; th = h;
   }
   first = 0;
   last = gfx_morton_from_xy(tw - 1, th - 1);
   if (flags & GFX_MORTON_REVERSE) {
      state->i = last;
      state->inc_i = -1;
      state->end_i = first - 1;
   } else {
      state->i = first;
      state->inc_i = 1;
      state->end_i = last + 1;
   }
}

bool gfx_morton_next(GFX_MORTON_STATE_T *state,
   uint32_t *x_out, uint32_t *y_out, bool *last)
{
   uint32_t x, y;

   if (state->i == state->end_i) {
      return false;
   }

   do {
      assert(state->i != state->end_i);
      gfx_morton_to_xy(&x, &y, state->i);
      state->i += state->inc_i;

      if (state->flags & GFX_MORTON_TRANSPOSE) {
         uint32_t temp = x;
         x = y;
         y = temp;
      }
   } while ((x >= state->w) || (y >= state->h));

   if (state->flags & GFX_MORTON_FLIP_X) {
      x = state->w - x - 1;
   }
   if (state->flags & GFX_MORTON_FLIP_Y) {
      y = state->h - y - 1;
   }

   *x_out = x;
   *y_out = y;
   if (last) {
      *last = state->i == state->end_i;
   }
   return true;
}
