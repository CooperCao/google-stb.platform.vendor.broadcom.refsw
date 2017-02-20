/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <limits.h>
#include <math.h>
#include <assert.h>

#include "libs/core/v3d/v3d_common.h"
#include "libs/core/v3d/v3d_gen.h"
#include "libs/util/gfx_util/gfx_util_conv.h"

/* TODO: MRT and other fanciness? */
static uint32_t clear_shader_2[] =
{
#if V3D_HAS_RELAXED_THRSW
   0xbb800000, 0x3c603186, // nop            ; thrsw ; ldunif
   0xb682d000, 0x3c202183, // mov rf3, r5    ; thrsw
   0xbb800000, 0x3c003186, // nop
   0xbb800000, 0x3c603186, // nop            ; thrsw ; ldunif
   0xb68360c0, 0x3c003188, // mov tlbu, rf3
   0xb682d000, 0x3c003187, // mov tlb, r5
#else
   0xbb800000, 0x3c403186, // nop            ; ldunif
# if V3D_VER_AT_LEAST(4,0,2,0)
   0xb682d000, 0x3c203188, // mov tlbu, r5   ; thrsw
   0xbb800000, 0x3c403186, // nop            ; ldunif
   0xb682d000, 0x3c003187, // mov tlb, r5
# else
   0xb682d000, 0x3c003188, // mov tlbu, r5
   0xbb800000, 0x3c403186, // nop            ; ldunif
   0xb682d000, 0x3c203187, // mov tlb, r5    ; thrsw
   0xbb800000, 0x3c003186, // nop
   0xbb800000, 0x3c003186, // nop
# endif
#endif
};

static uint32_t clear_shader_4[] =
{
#if V3D_HAS_RELAXED_THRSW
   0xbb800000, 0x3c603186, // nop           ; thrsw ; ldunif
   0xb682d000, 0x3c602183, // mov rf3, r5   ; thrsw ; ldunif
   0xb682d000, 0x3c002184, // mov rf4, r5
   0xbb800000, 0x3c003186, // nop
   0xb68360c0, 0x3c003188, // mov tlbu, rf3
   0xb6836100, 0x3c603187, // mov tlb, rf4  ; ldunif ; thrsw
   0xb682d000, 0x3c403187, // mov tlb, r5   ; ldunif
   0xb682d000, 0x3c003187, // mov tlb, r5
#else
   0xbb800000, 0x3c403186, // nop; ldunif
   0xb682d000, 0x3c003188, // mov tlbu, r5
   0xbb800000, 0x3c403186, // nop; ldunif
# if V3D_VER_AT_LEAST(4,0,2,0)
   0xb682d000, 0x3c603187, // mov tlb, r5; ldunif ; thrsw
   0xb682d000, 0x3c403187, // mov tlb, r5; ldunif
   0xb682d000, 0x3c003187, // mov tlb, r5
# else
   0xb682d000, 0x3c403187, // mov tlb, r5; ldunif
   0xb682d000, 0x3c403187, // mov tlb, r5; ldunif
   0xb682d000, 0x3c203187, // mov tlb, r5; thrsw
   0xbb800000, 0x3c003186, // nop
   0xbb800000, 0x3c003186, // nop
# endif
#endif
};

static uint32_t clear_shader_no_color[] =
{
#if V3D_HAS_RELAXED_THRSW
   0xbb800000, 0x3c203186, // nop ; thrsw
   0xbb800000, 0x3c203186, // nop ; thrsw
   0xbb800000, 0x3c003186, // nop
   0xbb800000, 0x3c203186, // nop ; thrsw
   0x00000000, 0x03003206, // mov tlbu, 0
   0xbb800000, 0x3c003186, // nop
#else
   0xbb800000, 0x3c003186, // nop
   0x00000000, 0x03003206, // mov tlbu, 0
   0xb7800000, 0x3c203187, // xor tlb, r0, r0 ; thrsw
   0x00000000, 0x030031c6, // mov tlb, 0
   0x00000000, 0x030031c6, // mov tlb, 0
#endif
};

/* TODO: Surely there should be a v3d_gen function for this? */
static v3d_tlb_rw_type_t get_write_type(v3d_rt_type_t rt)
{
   switch (rt)
   {
      case V3D_RT_TYPE_8I:
      case V3D_RT_TYPE_8UI:
      case V3D_RT_TYPE_16I:
      case V3D_RT_TYPE_16UI:
      case V3D_RT_TYPE_32I:
      case V3D_RT_TYPE_32UI:
      case V3D_RT_TYPE_32F:
         return V3D_TLB_RW_TYPE_COLOR_32;
      case V3D_RT_TYPE_8:
      case V3D_RT_TYPE_16F:
         return V3D_TLB_RW_TYPE_COLOR_F16;
      default:
         unreachable();
         return V3D_TLB_RW_TYPE_INVALID;
   }
}

static bool write_as_int(v3d_rt_type_t rt)
{
   assert(get_write_type(rt) == V3D_TLB_RW_TYPE_COLOR_32);
   return rt != V3D_RT_TYPE_32F;
}

/* Pack the colours into values suitable for the uniform stream. */
static void pack_colors_to_f16(const uint32_t color_value[4],
   uint32_t *uniform, v3d_rt_type_t rt_type)
{
   uint32_t color_f16[4];
   if (rt_type == V3D_RT_TYPE_8)
   {
      for (unsigned i = 0; i < 4; i++)
      {
         float f = gfx_float_from_bits(color_value[i]);
         uint32_t u = gfx_float_to_unorm8(f);
         color_f16[i] = gfx_unorm_to_float16(u, 8);
      }
   }
   else
   {
      for (unsigned i = 0; i < 4; i++)
         color_f16[i] = gfx_floatbits_to_float16(color_value[i]);
   }
   uniform[1] = gfx_pack_1616(color_f16[2], color_f16[3]);
   uniform[0] = gfx_pack_1616(color_f16[0], color_f16[1]);
}

void v3d_clear_shader_color(uint32_t **shader_code, uint32_t *shader_size, uint32_t *unif_ptr,
                            v3d_rt_type_t type, int rt, const uint32_t *clear_color)
{
   v3d_tlb_rw_type_t tlb_write_type = get_write_type(type);
   unsigned cfg_unif_pos = V3D_HAS_RELAXED_THRSW ? 2 : 1;
   uint8_t cfg;

   /* TODO: select shader based on MRT etc. */
   /* If we write f16 then we have 2 writes, f32 and i32 have 4 */
   if (tlb_write_type == V3D_TLB_RW_TYPE_COLOR_F16)
   {
      *shader_code = clear_shader_2;
      *shader_size = sizeof(clear_shader_2);   /* NB: sizeof(array) */

      /* Pack colours into values for the uniform stream */
      uint32_t col_unifs[2];
      pack_colors_to_f16(clear_color, col_unifs, type);
      /* Copy into the uniform stream, skipping the config location */
      for (unsigned i=0; i<2; i++)
         unif_ptr[i + (i>=cfg_unif_pos)] = col_unifs[i];

      V3D_TLB_CONFIG_COLOR_F16_T c = { .num_words = 2, .no_swap = true,
                                       .all_samples_same_data = true, .rt = rt };
      cfg = v3d_pack_tlb_config_color_f16(&c);
   }
   else
   {
      *shader_code = clear_shader_4;
      *shader_size = sizeof(clear_shader_4);
      /* Copy into the uniform stream, skipping the config location */
      for (unsigned i=0; i<4; i++)
         unif_ptr[i + (i>=cfg_unif_pos)] = clear_color[i];

      V3D_TLB_CONFIG_COLOR_32_T c = { .num_words = 4, .all_samples_same_data = true,
                                      .rt = rt, .as_int = write_as_int(type) };
      cfg = v3d_pack_tlb_config_color_32(&c);
   }

   /* set up the config uniform; unused config entries must be all 1s */
   unif_ptr[cfg_unif_pos] = 0xffffff00 | cfg;
}

void v3d_clear_shader_no_color(uint32_t **shader_code, uint32_t *shader_size, uint32_t *unif_ptr) {
   *shader_code = clear_shader_no_color;
   *shader_size = sizeof(clear_shader_no_color);

#if V3D_HAS_RELAXED_THRSW
   uint8_t cfg = 0x3C;
#else
   /* Configure tlb for f32 color writes, but disable the write masks later */
   /* GFXH-1212 means we must write 4 values to prevent a lockup */
   /* TODO: GFXH-1212 is now fixed so we can do better than this */
   uint8_t cfg = 0x3F;
#endif

   /* set up the config uniform; unused config entries must be all 1s */
   unif_ptr[0] = 0xffffff00 | cfg;
}
