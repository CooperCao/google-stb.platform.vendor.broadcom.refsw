/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_fmem.h"
#include "../glxx/glxx_int_config.h"

#if V3D_VER_AT_LEAST(4,1,34,0)

#define ALLOC_SIZE (2048u)
static_assrt(V3D_TMU_CONFIG_CACHE_LINE_SIZE >= 32);
static_assrt(ALLOC_SIZE / 32 >= GLXX_CONFIG_MAX_SHADER_TEXTURE_IMAGE_UNITS * 2);   // need to allocate arrays up to this size
static_assrt((ALLOC_SIZE % V3D_TMU_CONFIG_CACHE_LINE_SIZE) == 0);

static bool tmu_cfg_new_block(khrn_fmem *fmem)
{
   khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;

   a->spare_8 = 0;
   a->spare_16 = 0;
   a->spare_32 = 0;
   a->num_32 = 0;

   a->ptr = khrn_fmem_data(&a->addr, fmem, ALLOC_SIZE, V3D_TMU_CONFIG_CACHE_LINE_SIZE);
   if (!a->ptr)
      return false;
   a->num_32 = ALLOC_SIZE / 32;
   return true;
}

static bool tmu_cfg_alloc_32(uint32_t* ret, khrn_fmem *fmem, bool reserved32)
{
   khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;
   if (!a->num_32)
   {
      assert(!reserved32);
      if (!tmu_cfg_new_block(fmem))
         return false;
   }

   *ret = a->spare_32;
   a->spare_32 += 32;
   a->num_32 -= 1;
   a->spare_8 <<= 4;
   a->spare_16 <<= 2;
   return true;
}

static_assrt(V3D_TMU_TEX_STATE_PACKED_SIZE == 16);
static_assrt(V3D_TMU_TEX_STATE_ALIGN == 16);
static_assrt((V3D_TMU_TEX_STATE_PACKED_SIZE + V3D_TMU_TEX_EXTENSION_PACKED_SIZE) == 24);
static_assrt(V3D_TMU_EXTENDED_TEX_STATE_ALIGN == 32);

bool khrn_fmem_reserve_tmu_cfg(khrn_fmem *fmem, unsigned num_32)
{
   khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;

   assert(num_32 <= ALLOC_SIZE / 32);
   return num_32 <= a->num_32 || tmu_cfg_new_block(fmem);
}

v3d_addr_t khrn_fmem_add_tmu_tex_state(khrn_fmem *fmem,
   const void *tex_state, bool extended, bool reserved32)
{
   khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;

   uint32_t offset;
   if (!extended && !reserved32 && a->spare_16)
   {
      unsigned bit = gfx_msb(a->spare_16);
      offset = a->spare_32 - (bit + 1)*16;
      a->spare_16 &= ~(1 << bit);
   }
   else
   {
      if (!tmu_cfg_alloc_32(&offset, fmem, reserved32))
         return 0;
      if (extended)
          a->spare_8 |= 1;
      else
         a->spare_16 |= 1;
   }

   memcpy((char*)a->ptr + offset, tex_state,
      V3D_TMU_TEX_STATE_PACKED_SIZE + (extended ? V3D_TMU_TEX_EXTENSION_PACKED_SIZE : 0));
   return a->addr + offset;
}

static_assrt(V3D_TMU_SAMPLER_PACKED_SIZE == 8);
static_assrt(V3D_TMU_SAMPLER_ALIGN == 8);
static_assrt(V3D_TMU_EXTENDED_SAMPLER_ALIGN == 32);

v3d_addr_t khrn_fmem_add_tmu_sampler(khrn_fmem *fmem,
   const void *sampler, bool extended, bool reserved32)
{
   khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;

   uint32_t offset;
   if (!extended && !reserved32)
   {
      if (a->spare_8)
      {
         unsigned bit = gfx_msb64(a->spare_8);
         offset = a->spare_32 - (bit + 1)*8;
         a->spare_8 &= ~((uint64_t) 1 << bit);
         goto end;
      }
      if (a->spare_16)
      {
         unsigned bit = gfx_msb(a->spare_16);
         offset = a->spare_32 - (bit + 1)*16;
         a->spare_16 &= ~(1 << bit);
         a->spare_8 |= (uint64_t)1 << bit*2;
         goto end;
      }
   }

   if (!tmu_cfg_alloc_32(&offset, fmem, reserved32))
      return 0;
   if (extended)
   {
      // Spare 8-byte chunk following p.
      a->spare_8 |= 1;
   }
   else
   {
      // Spare 8-byte and 16-byte chunks following p.
      a->spare_16 |= 1 << 0;
      a->spare_8 |= (uint64_t)1 << 2;
   }

end:
   memcpy((char*)a->ptr + offset, sampler, V3D_TMU_SAMPLER_PACKED_SIZE + (extended ? 16 : 0));
   return a->addr + offset;
}

#else

v3d_addr_t khrn_fmem_add_tmu_indirect(khrn_fmem *fmem, uint32_t const *tmu_indirect)
{
   khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;

   // allocate tmu records in blocks of 64
   const unsigned c_cache_size = 64;
   if (!a->num_spare)
   {
      a->spare_ptr = khrn_fmem_data(&a->spare_addr, fmem,
         V3D_TMU_INDIRECT_PACKED_SIZE*c_cache_size,
         V3D_TMU_CONFIG_CACHE_LINE_SIZE
         );
      if (!a->spare_ptr)
         return 0;
      a->num_spare = c_cache_size;
   }

   v3d_addr_t ret = a->spare_addr;
   memcpy(a->spare_ptr, tmu_indirect, V3D_TMU_INDIRECT_PACKED_SIZE);
   a->spare_ptr = (char*)a->spare_ptr + V3D_TMU_INDIRECT_PACKED_SIZE;
   a->spare_addr += V3D_TMU_INDIRECT_PACKED_SIZE;
   a->num_spare -= 1;
   return ret;
}

#endif