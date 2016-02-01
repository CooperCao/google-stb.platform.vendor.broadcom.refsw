/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GFX_UTIL_RAND_H
#define GFX_UTIL_RAND_H

#include "helpers/gfx/gfx_util.h"

VCOS_EXTERN_C_BEGIN

/* http://en.wikipedia.org/wiki/Xorshift */
typedef struct
{
   uint32_t x, y, z, w;
} GFX_RAND_STATE_T;

static inline void gfx_rand_init(GFX_RAND_STATE_T *state, uint32_t seed)
{
   /* Avoid all 0s! */
   state->x = seed ^ 0x6565732b;
   state->y = seed ^ 0xb47a0d88;
   state->z = seed ^ 0x97b75092;
   state->w = seed ^ 0x8b529b4a;
}

static inline uint32_t gfx_rand(GFX_RAND_STATE_T *state)
{
   uint32_t t = state->x ^ (state->x << 11);
   state->x = state->y;
   state->y = state->z;
   state->z = state->w;
   state->w = state->w ^ (state->w >> 19) ^ t ^ (t >> 8);
   return state->w;
}

/* Generates a random uint32_t in the range [min, max] */
static inline uint32_t gfx_rand_ubetween(GFX_RAND_STATE_T *state,
   uint32_t min, uint32_t max)
{
   assert(min <= max);

   if ((min == 0) && ((max + 1) == 0))
      return gfx_rand(state);

   /* This is slightly biased towards small numbers... */
   return min + (gfx_rand(state) % (max + 1 - min));
}

static inline int32_t gfx_rand_sbetween(GFX_RAND_STATE_T *state,
   int32_t min, int32_t max)
{
   assert(min <= max);
   return min + gfx_rand_ubetween(state, 0, max - min);
}

static inline float gfx_rand_fbetween(GFX_RAND_STATE_T *state,
   float min, float max)
{
   assert(min <= max);
   return min + (((float)gfx_rand(state) / (float)0xffffffff) * (max - min));
}

static inline bool gfx_rand_with_prob(GFX_RAND_STATE_T *state, float p)
{
   return (p != 0.0f) && (gfx_rand_fbetween(state, 0.0f, 1.0f) <= p);
}

static inline float gfx_rand_std_normal(GFX_RAND_STATE_T *state)
{
   /* http://en.wikipedia.org/wiki/Box%E2%80%93Muller_transform */

   float u, v, s;
   do
   {
      u = gfx_rand_fbetween(state, -1.0f, 1.0f);
      v = gfx_rand_fbetween(state, -1.0f, 1.0f);
      s = (u * u) + (v * v);
   } while ((s <= 0.0f) || (s >= 1.0f));

   return u * sqrtf((-2.0f * logf(s)) / s);
}

static inline float gfx_rand_normal(GFX_RAND_STATE_T *state,
   float mean, float std_dev)
{
   return mean + (gfx_rand_std_normal(state) * std_dev);
}

static inline uint32_t gfx_rand_ubetween_normal(GFX_RAND_STATE_T *state,
   uint32_t min, uint32_t max, float std_dev)
{
   assert(min <= max);
   float range = (float)(max - min);
   float cutoff = (range + 1.0f) / (2.0f * std_dev);
   for (;;)
   {
      float x = gfx_rand_std_normal(state);
      if (fabsf(x) < cutoff)
      {
         x *= std_dev;
         x += min + (0.5f * range);
         return gfx_uclamp(gfx_float_to_uint32(x), min, max);
      }
   }
}

VCOS_EXTERN_C_END

#endif
