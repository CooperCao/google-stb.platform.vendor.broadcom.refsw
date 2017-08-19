/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <float.h>

#include "khrn_int_common.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

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
