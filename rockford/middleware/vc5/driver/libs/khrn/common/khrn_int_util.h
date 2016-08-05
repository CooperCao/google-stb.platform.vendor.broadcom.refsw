/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include <float.h>

#include "khrn_int_common.h"
#include "libs/util/gfx_util/gfx_util.h"

/* Returns false iff all indices are restart indices, ie there are no real indices */
static inline bool find_max(uint32_t *max_out, int count, int per_index_size, const void *indices, bool primitive_restart_enabled)
{
   uint32_t max = 0;
   bool any = false;
   switch (per_index_size) {
   case 1:
   {
      uint8_t *u = (uint8_t *)indices;
      for (int i = 0; i < count; i++)
         if (!primitive_restart_enabled || u[i] != 0xff)
         {
            max = gfx_umax(max, u[i]);
            any = true;
         }
      break;
   }
   case 2:
   {
      uint16_t *u = (uint16_t *)indices;
      for (int i = 0; i < count; i++)
         if (!primitive_restart_enabled || u[i] != 0xffff)
         {
            max = gfx_umax(max, u[i]);
            any = true;
         }
      break;
   }
   case 4:
   {
      uint32_t *u = (uint32_t *)indices;
      for (int i = 0; i < count; i++)
         if (!primitive_restart_enabled || u[i] != 0xffffffff)
         {
            max = gfx_umax(max, u[i]);
            any = true;
         }
      break;
   }
   default:
      unreachable();
      break;
   }

   *max_out = max;
   return any;
}

/* OpenGL ES 1.1 makes use of the datatype 'fixed' which is defined as a:
 *
 *    signed 2s complement 16.16 scaled integer
 *
 * This function is used to convert a fixed value to its floating point
 * equivalent, with some loss of precision. We believe this is justified as the
 * state tables in the spec talk about storing all these values as type R
 * (floating-point number). */
static inline float fixed_to_float(int f)
{
   return (float)f / 65536.0f;
}

/*
   convert float to 16.16 fixed point value
   saturating, round to nearest

   Khronos documentation:

   If a value is so large in magnitude that it cannot be represented with the
   requested type, then the nearest value representable using the requested type
   is returned.
*/

static inline int32_t float_to_fixed(float f)
{
   return gfx_float_to_int32(f * (1 << 16));
}
