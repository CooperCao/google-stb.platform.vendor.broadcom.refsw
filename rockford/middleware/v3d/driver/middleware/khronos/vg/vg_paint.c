/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  VG paint

FILE DESCRIPTION
VG paint object handling implementation.
=============================================================================*/

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/vg/vg_image.h"
#include "middleware/khronos/vg/vg_paint.h"
#include "middleware/khronos/vg/vg_ramp.h"
#include "middleware/khronos/vg/vg_stem.h"

/* VG_PAINT_BPRINT_T doesn't actually exist (it would be empty) */

void vg_paint_bprint_term(MEM_HANDLE_T handle)
{
   UNUSED(handle);
}

void vg_paint_bprint_from_stem(MEM_HANDLE_T handle)
{
   vcos_assert(vg_is_stem(handle));

   mem_set_desc(handle, "VG_PAINT_BPRINT_T");
   mem_set_term(handle, vg_paint_bprint_term, NULL);
}

vcos_static_assert(sizeof(VG_PAINT_T) <= VG_STEM_SIZE);
vcos_static_assert(alignof(VG_PAINT_T) <= VG_STEM_ALIGN);

void vg_paint_term(MEM_HANDLE_T handle)
{
   VG_PAINT_T *paint = (VG_PAINT_T *)mem_lock(handle, NULL);

   if (paint->pattern != MEM_INVALID_HANDLE) {
      mem_release(paint->pattern);
   }

   if (paint->ramp != MEM_INVALID_HANDLE) {
      vg_ramp_release(paint->ramp, paint->ramp_i);
   }

   if (paint->ramp_stops != MEM_INVALID_HANDLE) {
      mem_release(paint->ramp_stops);
   }

   mem_unlock(handle);
}

static void init(VG_PAINT_T *paint)
{
   paint->type = VG_PAINT_TYPE_COLOR;

   paint->rgba = 0xff000000;

   paint->linear_gradient[0] = 0.0f;
   paint->linear_gradient[1] = 0.0f;
   paint->linear_gradient[2] = 1.0f;
   paint->linear_gradient[3] = 0.0f;
   paint->radial_gradient[0] = 0.0f;
   paint->radial_gradient[1] = 0.0f;
   paint->radial_gradient[2] = 0.0f;
   paint->radial_gradient[3] = 0.0f;
   paint->radial_gradient[4] = 1.0f;
   vg_paint_set_ramp_spread_mode(paint, VG_COLOR_RAMP_SPREAD_PAD);
   paint->ramp_pre = true;
   paint->ramp_stops = MEM_INVALID_HANDLE;
   paint->ramp = MEM_INVALID_HANDLE;

   paint->pattern_tiling_mode = VG_TILE_FILL;
   paint->pattern = MEM_INVALID_HANDLE;
}

MEM_HANDLE_T vg_paint_alloc(void)
{
   /*
      alloc struct
   */

   MEM_HANDLE_T handle = MEM_ALLOC_STRUCT_EX(VG_PAINT_T, MEM_COMPACT_DISCARD);
   if (handle == MEM_INVALID_HANDLE) {
      return MEM_INVALID_HANDLE;
   }

   /*
      fill in struct
   */

   init((VG_PAINT_T *)mem_lock(handle, NULL));
   mem_unlock(handle);

   mem_set_term(handle, vg_paint_term, NULL);

   return handle;
}

bool vg_paint_from_bprint(MEM_HANDLE_T handle)
{
   vcos_assert(vg_is_paint_bprint(handle));

   /*
      fill in struct
   */

   init((VG_PAINT_T *)mem_lock(handle, NULL));
   mem_unlock(handle);

   mem_set_desc(handle, "VG_PAINT_T");
   mem_set_term(handle, vg_paint_term, NULL);

   return true;
}

uint32_t vg_paint_get_ramp_degenerate_rgba(VG_PAINT_T *paint)
{
   /*
      degenerate means function has value 1 everywhere:
      - in pad and reflect spread modes, this maps to the last ramp stop color.
      - in repeat spread mode, this maps to the first ramp stop color.
   */

   return vg_ramp_get_rgba(paint->ramp_stops, vg_paint_get_ramp_spread_mode(paint) != VG_COLOR_RAMP_SPREAD_REPEAT);
}

bool vg_paint_retain_ramp(VG_PAINT_T *paint)
{
   if (paint->ramp == MEM_INVALID_HANDLE) {
      uint32_t ramp_i;
      if (!vg_ramp_alloc(&paint->ramp, &ramp_i)) { /* won't overwrite paint->ramp unless successful */
         return false;
      }
      paint->ramp_i = (uint8_t)ramp_i;
   }
   return vg_ramp_retain(paint->ramp, paint->ramp_i, paint->ramp_pre, paint->ramp_stops);
}
