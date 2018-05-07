/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "glxx_tmu_blit_shaders.h"
#include "libs/util/log/log.h"
#include "libs/core/lfmt/lfmt.h"

#include "glxx_tmu_blit_qasm.h"

LOG_DEFAULT_CAT("tmu_blit_shaders")

static struct
{
   const uint64_t *code;
   size_t size;
   char name[100];
}shaders[] =
{
   {float_tlb_16_tmu_32, sizeof(float_tlb_16_tmu_32), "float_tlb_16_tmu_32"},
   {float_tlb_32_tmu_16, sizeof(float_tlb_32_tmu_16), "float_tlb_32_tmu_16"} ,

   {uint_int_tlb_16_tmu_32, sizeof(uint_int_tlb_16_tmu_32), "uint_int_tlb_16_tmu_32"},
   {uint_tlb_32_tmu_16, sizeof(uint_tlb_32_tmu_16), "uint_tlb_32_tmu_16"},

   {uint_int_tlb_16_tmu_32, sizeof(uint_int_tlb_16_tmu_32), "uint_int_tlb_16_tmu_32"},
   {int_tlb_32_tmu_16, sizeof(int_tlb_32_tmu_16), "int_tlb_32_tmu_16"},

   {tlb_16_tmu_16, sizeof(tlb_16_tmu_16), "tlb_16_tmu_16"},
   {tlb_32_tmu_32, sizeof(tlb_32_tmu_32), "tlb_32_tmu_32"},
};

void glxx_blit_fshader_and_unifs(uint32_t *shader_ptr, uint32_t *unif_ptr,
      GFX_LFMT_TYPE_T tex_fmt_type, bool tmu_ret_is_32bit, bool tlb_access_is_32bit,
      const uint32_t tmu_config[2], uint8_t tlb_config_byte)
{
   unsigned unif = 0;
   unsigned start_pos ;
   if (tmu_ret_is_32bit == tlb_access_is_32bit)
   {
      start_pos = 6;
   }
   else
   {
      switch(tex_fmt_type)
      {
      case GFX_LFMT_TYPE_INT:
         start_pos = 4;
         break;
      case GFX_LFMT_TYPE_UINT:
         start_pos = 2;
         break;
      default:
         start_pos = 0;
      }
   }

   unsigned idx =  start_pos + (tlb_access_is_32bit ? 1 : 0);
   assert(idx < sizeof(shaders)/sizeof(shaders[0]));
   assert(shaders[idx].size <= V3D_TMU_BLIT_SHADER_MAX_SIZE);
   memcpy(shader_ptr, shaders[idx].code, shaders[idx].size);
   log_trace("shader name = %s", shaders[idx].name);

   unif_ptr[unif++] = tmu_config[0];
   unif_ptr[unif++] = tmu_config[1];
   switch(idx)
   {
   case 0:
   case 1:
   case 6:
   case 7:
      unif_ptr[unif++] = 0xffffff00 | tlb_config_byte;
      break;
   case 2:
   case 4:
      unif_ptr[unif++] = 0xffff;
      unif_ptr[unif++] = 0x10;
      unif_ptr[unif++] = 0xffffff00 | tlb_config_byte;
      break;
   case 3:
      unif_ptr[unif++] = 0xffff;
      unif_ptr[unif++] = 0xffffff00 | tlb_config_byte;
      unif_ptr[unif++] = 0x10;
      break;
   case 5:
      unif_ptr[unif++] = 0x10;
      unif_ptr[unif++] = 0xffffff00 | tlb_config_byte;
      break;
   default:
      assert(0);
   }
   assert(unif <= V3D_TMU_BLIT_SHADER_MAX_UNIFS_SIZE/sizeof(uint32_t));
}

void glxx_blit_fshader_and_unifs_red_uint_with_mask(uint32_t *shader_ptr, uint32_t *unif_ptr,
      uint32_t mask_component, const uint32_t tmu_config[2], uint8_t tlb_config_byte)
{
   bool with_mask = mask_component != 0xffffffff;

   if (!with_mask)
   {
      assert(sizeof(red_uint) <= V3D_TMU_BLIT_SHADER_MAX_SIZE);
      memcpy(shader_ptr, red_uint, sizeof(red_uint));
   }
   else
   {
      assert(sizeof(red_uint_mask) <= V3D_TMU_BLIT_SHADER_MAX_SIZE);
      memcpy(shader_ptr, red_uint_mask, sizeof(red_uint_mask));
   }
   log_trace("shader red_uint%s", with_mask ? "_mask" : "");

   unsigned unif = 0;

   if (with_mask)
      unif_ptr[unif++] = mask_component;
   unif_ptr[unif++] = tmu_config[0];
   unif_ptr[unif++] = tmu_config[1];
   if (!with_mask)
      unif_ptr[unif++] = 0xffffff00 | tlb_config_byte;
   else
      unif_ptr[unif++] = 0xffff0000 | (tlb_config_byte << 8) | tlb_config_byte;
}
