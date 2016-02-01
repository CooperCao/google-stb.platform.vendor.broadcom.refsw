/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  VG paint

FILE DESCRIPTION
VG paint object handling interface.
=============================================================================*/

#ifndef VG_PAINT_H
#define VG_PAINT_H

#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/include/VG/openvg.h"

/* VG_PAINT_BPRINT_T doesn't actually exist (it would be empty) */

extern void vg_paint_bprint_from_stem(MEM_HANDLE_T handle);

extern void vg_paint_bprint_term(void *, uint32_t);

static INLINE bool vg_is_paint_bprint(MEM_HANDLE_T handle)
{
   return mem_get_term(handle) == vg_paint_bprint_term;
}

typedef struct {
   VGPaintType type;

   /*
      solid
   */

   uint32_t rgba;

   /*
      linear/radial
   */

   float linear_gradient[4]; /* as input */
   float radial_gradient[5]; /* as input */
   uint8_t ramp_spread_mode; /* actually (ramp_spread_mode - VG_COLOR_RAMP_SPREAD_PAD) */
   bool ramp_pre;
   uint8_t ramp_i;
   MEM_HANDLE_T ramp_stops; /* array of floats, MEM_INVALID_HANDLE means empty. r/g/b/a in [0, 1] */
   MEM_HANDLE_T ramp; /* VG_RAMP_T */

   /*
      pattern
   */

   VGTilingMode pattern_tiling_mode;
   MEM_HANDLE_T pattern; /* VG_IMAGE_T or VG_CHILD_IMAGE_T */
} VG_PAINT_T;

extern MEM_HANDLE_T vg_paint_alloc(void);
extern bool vg_paint_from_bprint(MEM_HANDLE_T handle);

extern void vg_paint_term(void *, uint32_t);

static INLINE bool vg_is_paint(MEM_HANDLE_T handle)
{
   return mem_get_term(handle) == vg_paint_term;
}

static INLINE void vg_paint_set_ramp_spread_mode(VG_PAINT_T *paint, VGColorRampSpreadMode ramp_spread_mode)
{
   paint->ramp_spread_mode = (uint8_t)(ramp_spread_mode - VG_COLOR_RAMP_SPREAD_PAD);
}

static INLINE VGColorRampSpreadMode vg_paint_get_ramp_spread_mode(VG_PAINT_T *paint)
{
   return (VGColorRampSpreadMode)(VG_COLOR_RAMP_SPREAD_PAD + paint->ramp_spread_mode);
}

extern uint32_t vg_paint_get_ramp_degenerate_rgba(VG_PAINT_T *paint);
extern bool vg_paint_retain_ramp(VG_PAINT_T *paint);

#endif
