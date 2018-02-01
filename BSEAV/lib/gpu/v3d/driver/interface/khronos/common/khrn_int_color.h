/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_util.h"

extern const uint8_t COLOR_S_TO_LIN[256];
extern const uint16_t COLOR_S_TO_LIN16[256];
extern const uint8_t COLOR_LIN_TO_S[256];
extern const uint8_t COLOR_LIN16_TO_S[320];
extern const uint32_t COLOR_RECIP[256];

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

static inline uint32_t color_floats_to_rgba_clean(const float *color)
{
   return color_floats_to_rgba(clean_float(color[0]), clean_float(color[1]), clean_float(color[2]), clean_float(color[3]));
}

static inline void khrn_color_rgba_to_floats(float *floats, uint32_t rgba)
{
   floats[0] = (float)((rgba >> 0) & 0xff) * (1.0f / 255.0f);
   floats[1] = (float)((rgba >> 8) & 0xff) * (1.0f / 255.0f);
   floats[2] = (float)((rgba >> 16) & 0xff) * (1.0f / 255.0f);
   floats[3] = (float)((rgba >> 24) & 0xff) * (1.0f / 255.0f);
}

extern uint32_t khrn_color_rgba_pre(uint32_t rgba);
extern uint32_t khrn_color_rgba_unpre(uint32_t rgba);
extern uint32_t khrn_color_rgba_rz(uint32_t rgba);
extern uint32_t khrn_color_rgba_clamp_to_a(uint32_t rgba);
extern uint32_t khrn_color_rgba_s_to_lin(uint32_t rgba);
extern uint32_t khrn_color_rgba_lin_to_s(uint32_t rgba);
extern uint32_t khrn_color_rgba_to_la_lin(uint32_t rgba);
extern uint32_t khrn_color_rgba_to_la_s(uint32_t rgba);

static inline uint32_t khrn_color_rgba_flip(uint32_t rgba)
{
   return
      (((rgba >> 0) & 0xff) << 24) |
      (((rgba >> 8) & 0xff) << 16) |
      (((rgba >> 16) & 0xff) << 8) |
      (((rgba >> 24) & 0xff) << 0);
}

extern uint32_t khrn_color_rgba_add_dither(uint32_t rgba, int r, int g, int b, int a);

extern uint32_t khrn_color_rgba_transform(uint32_t rgba, const float *color_transform);
