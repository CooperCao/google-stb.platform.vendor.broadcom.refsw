/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "libs/core/v3d/v3d_ver.h"
#include <stdint.h>

/*
33222222222211111111110000000000
10987654321098765432109876543210
--aaaauxdzwfffwfffwfffwfffpp-ccc   backend

a = advanced blend type
p = prim point
f = framebuffer type
w = fb alpha workaround
z = z only write
d = fez safe with discard
x = compute padding
u = disable UBO fetch optimization
*/

/* backend */
#define GLSL_SAMPLE_MS       (1<<0)
#define GLSL_SAMPLE_ALPHA    (1<<1)
#if !V3D_VER_AT_LEAST(4,1,34,0)
# define GLSL_SAMPLE_MASK     (1<<2)
# define GLSL_SAMPLE_OPS_M    (0x7<<0)
#else
# define GLSL_SAMPLE_OPS_M    (0x3<<0)
#endif

#define GLSL_PRIM_NOT_POINT_OR_LINE (0<<4)
#define GLSL_PRIM_POINT             (1<<4)
#define GLSL_PRIM_LINE              (2<<4)
#define GLSL_PRIM_M                 (3<<4)

#define GLSL_FB_GADGET_M    (0xF)
#define GLSL_FB_GADGET_S    6
#define GLSL_FB_16          (1 << 0)
#define GLSL_FB_32          (0 << 0)
#define GLSL_FB_INT         (1 << 1)
#define GLSL_FB_PRESENT     (1 << 2)

#if !V3D_VER_AT_LEAST(4,0,2,0)
/* V3Dv3.3 and earlier must write alpha if it's present for 16-bit RTs */
# define GLSL_FB_ALPHA_16_WORKAROUND (1<<3)
#endif

static inline void glsl_pack_fb_gadget(uint32_t *packed, uint32_t gadget, int i) {
   *packed |= gadget << (GLSL_FB_GADGET_S + 4*i);
}
static inline uint32_t glsl_unpack_fb_gadget(uint32_t packed, int i) {
   return (packed >> (GLSL_FB_GADGET_S + 4*i)) & GLSL_FB_GADGET_M;
}

/* Leave space for 4 fb gadgets. 6, 10, 14, 18 */

#define GLSL_Z_ONLY_WRITE              (1<<22)
#define GLSL_FEZ_SAFE_WITH_DISCARD     (1<<23)
#define GLSL_COMPUTE_PADDING           (1<<24)
#define GLSL_DISABLE_UBO_FETCH         (1<<25)

/* Advanced blend */
#define GLSL_ADV_BLEND_S              26
#define GLSL_ADV_BLEND_M              (0xf << GLSL_ADV_BLEND_S)
#define GLSL_ADV_BLEND_MULTIPLY       1
#define GLSL_ADV_BLEND_SCREEN         2
#define GLSL_ADV_BLEND_OVERLAY        3
#define GLSL_ADV_BLEND_DARKEN         4
#define GLSL_ADV_BLEND_LIGHTEN        5
#define GLSL_ADV_BLEND_COLORDODGE     6
#define GLSL_ADV_BLEND_COLORBURN      7
#define GLSL_ADV_BLEND_HARDLIGHT      8
#define GLSL_ADV_BLEND_SOFTLIGHT      9
#define GLSL_ADV_BLEND_DIFFERENCE     10
#define GLSL_ADV_BLEND_EXCLUSION      11
#define GLSL_ADV_BLEND_HSL_HUE        12
#define GLSL_ADV_BLEND_HSL_SATURATION 13
#define GLSL_ADV_BLEND_HSL_COLOR      14
#define GLSL_ADV_BLEND_HSL_LUMINOSITY 15

#if !V3D_HAS_SRS_CENTROID_FIX
#define GLSL_SAMPLE_SHADING_ENABLED    (1<<30)
#endif

#if !V3D_VER_AT_LEAST(3,3,0,0)

#include "libs/khrn/glxx/glxx_int_config.h"

typedef uint16_t glsl_gadgettype_t;

#define GLSL_GADGETTYPE_AUTO                 (0<<12)

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
   glsl_gadgettype_t base_gadget, const v3d_tmu_swizzle_t swizzles[4])
{
   assert(GLSL_GADGETTYPE_NEEDS_SWIZZLE(base_gadget));
   return
      ((uint16_t)swizzles[0] << 0) | ((uint16_t)swizzles[1] << 3) |
      ((uint16_t)swizzles[2] << 6) | ((uint16_t)swizzles[3] << 9) |
      base_gadget;
}

static inline glsl_gadgettype_t glsl_make_tmu_swizzled_gadgettype(
   bool tmu_output_32bit, bool sampler_32bit)
{
   return (tmu_output_32bit == sampler_32bit) ?
      GLSL_GADGETTYPE_AUTO : GLSL_GADGETTYPE_SWAP1632;
}
#endif

typedef struct glsl_backend_cfg
{
   uint32_t backend;

#if !V3D_VER_AT_LEAST(3,3,0,0)
   glsl_gadgettype_t gadgettype[GLXX_CONFIG_MAX_COMBINED_TEXTURE_IMAGE_UNITS];
   glsl_gadgettype_t img_gadgettype[GLXX_CONFIG_MAX_IMAGE_UNITS];
#endif
} GLSL_BACKEND_CFG_T;
