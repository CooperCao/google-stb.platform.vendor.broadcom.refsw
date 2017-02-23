/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_GADGETTYPE_H
#define GLSL_GADGETTYPE_H

#if !V3D_VER_AT_LEAST(3,3,0,0)

#include "libs/core/lfmt/lfmt_translate_v3d.h"

typedef uint16_t glsl_gadgettype_t;

#define GLSL_GADGETTYPE_AUTO                 (0<<12)

/* The non-AUTO gadgettypes are only used on 3.2 HW */

/* For regular (non-shadow) lookups:
 * - DEPTH_FIXED means the output from the TMU is 32-bit
 * - SWAP1632 means the output from the TMU is the wrong precision:
 *   - if the sampler is 16-bit, the TMU output is 32-bit
 *   - if the sampler is 32-bit, the TMU output is 16-bit
 *
 * For shadow lookups, the TMU output is *always* 16-bit. If DEPTH_FIXED, the
 * reference value for the comparison should be clamped to [0,1]. SWAP1632
 * should be treated just like AUTO. */
#define GLSL_GADGETTYPE_DEPTH_FIXED          (1<<12)
#define GLSL_GADGETTYPE_SWAP1632             (2<<12)

#define GLSL_GADGETTYPE_INT8                 (3<<12)
#define GLSL_GADGETTYPE_INT16                (4<<12)
#define GLSL_GADGETTYPE_INT32                (5<<12)
#define GLSL_GADGETTYPE_INT10_10_10_2        (6<<12)

/* Lowest 12 bits are filled in with swizzle when this is true */
#define GLSL_GADGETTYPE_NEEDS_SWIZZLE(g) ((g)>>12 >= 3)

static inline glsl_gadgettype_t glsl_make_shader_swizzled_gadgettype(
   gfx_lfmt_tmu_ret_t ret, const v3d_tmu_swizzle_t swizzles[4])
{
   glsl_gadgettype_t g;
   switch (ret) {
   case GFX_LFMT_TMU_RET_8:         g = GLSL_GADGETTYPE_INT8; break;
   case GFX_LFMT_TMU_RET_16:        g = GLSL_GADGETTYPE_INT16; break;
   case GFX_LFMT_TMU_RET_32:        g = GLSL_GADGETTYPE_INT32; break;
   case GFX_LFMT_TMU_RET_1010102:   g = GLSL_GADGETTYPE_INT10_10_10_2; break;
   default:                         unreachable();
   }
   assert(GLSL_GADGETTYPE_NEEDS_SWIZZLE(g));
   return
      ((uint16_t)swizzles[0] << 0) |
      ((uint16_t)swizzles[1] << 3) |
      ((uint16_t)swizzles[2] << 6) |
      ((uint16_t)swizzles[3] << 9) |
      g;
}

static inline glsl_gadgettype_t glsl_make_tmu_swizzled_gadgettype(
   bool tmu_output_32bit, bool sampler_32bit)
{
   return (tmu_output_32bit == sampler_32bit) ?
      GLSL_GADGETTYPE_AUTO : GLSL_GADGETTYPE_SWAP1632;
}

#endif
#endif
