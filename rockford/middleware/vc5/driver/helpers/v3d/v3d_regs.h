/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

#include "v3d_reg_map.h"
#include "v3d_limits.h"
#include "vcos_types.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

static inline bool v3d_is_hub_reg(uint32_t addr)
{
   return addr < V3D_CTL_0_BASE;
}

extern bool v3d_is_bin_cle_reg(uint32_t addr);
extern bool v3d_is_render_cle_reg(uint32_t addr);

static inline uint32_t v3d_reg_core(uint32_t addr)
{
   assert(!v3d_is_hub_reg(addr));
   return (addr - V3D_CTL_0_BASE) / V3D_REG_CORE_SEP;
}

static inline uint32_t v3d_reg_to_core0(uint32_t addr)
{
   return addr - (v3d_reg_core(addr) * V3D_REG_CORE_SEP);
}

static inline uint32_t v3d_reg_to_core(uint32_t core, uint32_t core0_addr)
{
   assert((core0_addr >= V3D_CTL_0_BASE) && (core0_addr < V3D_CTL_1_BASE));
   return (core * V3D_REG_CORE_SEP) + core0_addr;
}

#if V3D_VER_AT_LEAST(3,3)

#if V3D_VER_AT_LEAST(3,4)
static inline bool v3d_is_mmu_reg(uint32_t addr)
{
   return addr >= V3D_MMU_REG_START && addr <= V3D_MMU_REG_END;
}

#else
static inline bool v3d_is_mmu_reg(uint32_t addr)
{
   return addr >= V3D_MMU_0_REG_START && addr <= (V3D_MMU_0_REG_END + ((V3D_MAX_CORES - 1) * V3D_MMU_REG_CORE_SEP));
}

static inline uint32_t v3d_mmu_reg_core(uint32_t addr)
{
   assert(v3d_is_mmu_reg(addr));
   return (addr - V3D_MMU_0_BASE) / V3D_MMU_REG_CORE_SEP;
}

static inline uint32_t v3d_mmu_reg_to_core(uint32_t core, uint32_t core0_addr)
{
   assert(core0_addr >= V3D_MMU_0_BASE && core0_addr < V3D_MMU_1_BASE);
   return (core * V3D_MMU_REG_CORE_SEP) + core0_addr;
}

static inline uint32_t v3d_mmu_reg_to_tfu(uint32_t core0_addr)
{
   assert(core0_addr >= V3D_MMU_0_BASE && core0_addr < V3D_MMU_1_BASE);
   return (core0_addr - V3D_MMU_0_REG_START) + V3D_MMU_T_REG_START;
}

static inline uint32_t v3d_mmu_reg_to_core0(uint32_t addr)
{
   assert(v3d_is_mmu_reg(addr));
   return addr - (v3d_mmu_reg_core(addr) * (V3D_MMU_REG_CORE_SEP));
}
#endif

#endif
