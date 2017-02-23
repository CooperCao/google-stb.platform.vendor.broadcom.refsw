/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG tessellation

FILE DESCRIPTION
Non-platform-specific tessellation stuff.
=============================================================================*/

#ifndef VG_TESS_H
#define VG_TESS_H

#include "interface/khronos/vg/vg_int_config.h"

extern void vg_tess_subd_cubic(float *q, float *r, const float *p, float t);

/* generates up to 4 cubics that approximate the given arc */
extern uint32_t vg_tess_arc_to_cubics(
   float *p,
   float p0_x, float p0_y, float p1_x, float p1_y, float p1_m_p0_x, float p1_m_p0_y,
   float r_x, float r_y, float angle,
   bool large, bool cw);

typedef struct {
   float pattern[VG_CONFIG_MAX_DASH_COUNT]; /* dash points in [0, 1], must be monotonic increasing and last must be 1 */
   uint32_t pattern_count;                  /* must be even */
   float oo_scale;

   float phase;      /* start phase, must be in [0, 1] */
   uint32_t phase_i; /* pattern index corresponding to start phase */
   bool phase_reset; /* reset phase to start phase at the start of each subpath? */
} VG_TESS_STROKE_DASH_T;

extern bool vg_tess_stroke_dash_prepare(VG_TESS_STROKE_DASH_T *dash,
   const float *pattern, uint32_t pattern_n, float phase, bool phase_reset);

extern void vg_tess_clerp_approx(float *x, float *y, float x0, float y0, float x1, float y1, float t);

/* a should have space for at least (a_n + 4) verts. b should have space for at least (a_n + 3) verts */
extern uint32_t vg_tess_poly_clip(float *a, float *b, uint32_t a_n, const float *clip);

#endif
