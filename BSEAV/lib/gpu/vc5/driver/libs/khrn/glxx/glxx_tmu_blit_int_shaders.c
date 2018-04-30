/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_tmu_blit_shaders.h"
#include "libs/util/log/log.h"

#if !V3D_VER_AT_LEAST(3,3,0,0)

#include "glxx_tmu_blit_int_qasm.h"

LOG_DEFAULT_CAT("tmu_blit_int_shaders")

// tlb access for int/uint on hw 3.2 can only be set to i32

static struct
{
   const uint64_t *code;
   size_t size;
   char name[100];
}shaders[] =
{
   {uint_r_ret_8_or_10_10_10_2, sizeof(uint_r_ret_8_or_10_10_10_2), "uint_r_ret_8_or_10_10_10_2"},
   {uint_rg_ret_8_or_10_10_10_2, sizeof(uint_rg_ret_8_or_10_10_10_2), "uint_rg_ret_8_or_10_10_10_2"},
   {uint_rgba_ret_8_or_10_10_10_2, sizeof(uint_rgba_ret_8_or_10_10_10_2), "uint_rgba_ret_8_or_10_10_10_2"},

   {int_r_ret_8, sizeof(int_r_ret_8), "int_r_ret_8"},
   {int_rg_ret_8, sizeof(int_rg_ret_8), "int_rg_ret_8" },
   {int_rgba_ret_8, sizeof(int_rgba_ret_8), "int_rgba_ret_8"},

   {uint_r_ret_16, sizeof(uint_r_ret_16), "uint_r_ret_16"},
   {uint_rg_ret_16, sizeof(uint_rg_ret_16), "uint_rg_ret_16"},
   {uint_rgba_ret_16, sizeof(uint_rgba_ret_16), "uint_rgba_ret_16"},

   {int_r_ret_16, sizeof(int_r_ret_16), "int_r_ret_16"},
   {int_rg_ret_16, sizeof(int_rg_ret_16), "int_rg_ret_16"},
   {int_rgba_ret_16, sizeof(int_rgba_ret_16), "int_rgba_ret_16"},

   {uint_int_r_ret_32, sizeof(uint_int_r_ret_32), "uint_int_r_ret_32"},
   {uint_int_rg_ret_32, sizeof(uint_int_rg_ret_32),"uint_int_rg_ret_32"},
   {uint_int_rgba_ret_32, sizeof(uint_int_rgba_ret_32), "uint_int_rgba_ret_32"},
};

/* decides how many words to read based on tmu_ret and swizzle,
 * write rgba to a 32 bit int/uint tlb, using (0,0,1) for missing GBA channels */
void glxx_blit_int_fshader_and_unifs(uint32_t *shader_ptr, uint32_t *unif_ptr,
      GFX_LFMT_TYPE_T tex_fmt_type, gfx_lfmt_tmu_ret_t tmu_ret,
      const v3d_tmu_swizzle_t swizzle[4],
      const uint32_t tmu_config[2], uint8_t tlb_config_byte)
{
   assert(tex_fmt_type == GFX_LFMT_TYPE_UINT || tex_fmt_type == GFX_LFMT_TYPE_INT);
   bool is_uint = tex_fmt_type == GFX_LFMT_TYPE_UINT;

#define SWIZZLES_EQ(R, G, B, A) (          \
      swizzle[0] == V3D_TMU_SWIZZLE_##R && \
      swizzle[1] == V3D_TMU_SWIZZLE_##G && \
      swizzle[2] == V3D_TMU_SWIZZLE_##B && \
      swizzle[3] == V3D_TMU_SWIZZLE_##A)

   assert(SWIZZLES_EQ(R,0,0,1) ||SWIZZLES_EQ(R,G,0,1) || SWIZZLES_EQ(R,G,B,A));

   unsigned unif = 0;

   unsigned start_pos = 0;
   bool is_ret_8 = tmu_ret == GFX_LFMT_TMU_RET_8;
   switch(tmu_ret)
   {
   case GFX_LFMT_TMU_RET_8:
   case GFX_LFMT_TMU_RET_1010102:
      if (is_uint)
      {
         unif_ptr[unif++] = is_ret_8 ? 0xff : 0x3ff;
         unif_ptr[unif++] = tmu_config[0];
         unif_ptr[unif++] = tmu_config[1];
         unif_ptr[unif++] = is_ret_8 ? 8 : 10;
      }
      else
      {
         assert(is_ret_8);
         unif_ptr[unif++] = tmu_config[0];
         unif_ptr[unif++] = tmu_config[1];
         unif_ptr[unif++] = 24;
      }
      start_pos = is_uint ? 0 : 3;
      break;
   case GFX_LFMT_TMU_RET_16:
      unif_ptr[unif++] = tmu_config[0];
      unif_ptr[unif++] = tmu_config[1];
      if (is_uint)
         unif_ptr[unif++] = 0xffff;
      start_pos = is_uint ? 6 : 9;
      break;
   case GFX_LFMT_TMU_RET_32:
      unif_ptr[unif++] = tmu_config[0];
      unif_ptr[unif++] = tmu_config[1];
      start_pos = 12;
      break;
   default:
      assert(0);
   }
   unif_ptr[unif++] = 0xffffff00 | tlb_config_byte;

   assert(unif <= V3D_TMU_BLIT_SHADER_MAX_UNIFS_SIZE/sizeof(uint32_t));

   unsigned idx;
   if (SWIZZLES_EQ(R,0,0,1))
      idx = start_pos;
   else if(SWIZZLES_EQ(R,G,0,1))
      idx = start_pos + 1;
   else
   {
      assert(SWIZZLES_EQ(R,G,B,A));
      idx  = start_pos + 2;
   }
   assert(idx < sizeof(shaders)/sizeof(shaders[0]));
   assert(shaders[idx].size <= V3D_TMU_BLIT_SHADER_MAX_SIZE);
   log_trace("shader name %s", shaders[idx].name);
   memcpy(shader_ptr, shaders[idx].code, shaders[idx].size);

#undef  SWIZZLES_EQ
}
#endif
