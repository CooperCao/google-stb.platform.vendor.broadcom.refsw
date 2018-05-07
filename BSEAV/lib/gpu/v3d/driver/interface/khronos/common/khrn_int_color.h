/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_util.h"

static inline uint32_t color_clamp_times_256(float x)
{
   return (uint32_t)(_minf(_maxf(x, 0.0f), 255.0f / 256.0f) * 256.0f);
}

//Note: this will not do the same as hardware if you put NaN in.
static inline uint32_t color_floats_to_rgba(float r, float g, float b, float a)
{
   return
      (color_clamp_times_256(r) << 0) |
      (color_clamp_times_256(g) << 8) |
      (color_clamp_times_256(b) << 16) |
      (color_clamp_times_256(a) << 24);
}
