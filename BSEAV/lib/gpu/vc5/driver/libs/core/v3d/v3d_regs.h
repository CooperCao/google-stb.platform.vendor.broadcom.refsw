/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_reg_map.h"
#include "v3d_limits.h"
#include "libs/util/common.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

static inline bool v3d_is_noncore_reg(uint32_t addr)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   return ((addr >= V3D_WRAP_REG_START) && (addr < V3D_MMU_REG_END)) ||
      ((addr >= V3D_TOP_GR_BRIDGE_REG_START) && (addr < V3D_TOP_GR_BRIDGE_REG_END));
#elif V3D_VER_AT_LEAST(4,1,34,0)
   return (addr >= V3D_TOP_GR_BRIDGE_REG_START) && (addr < V3D_MMU_REG_END);
#elif V3D_VER_AT_LEAST(3,3,0,0)
   return (addr >= V3D_HUB_CTL_REG_START) && (addr < V3D_GCA_REG_END);
#else
   return ((addr >= V3D_GCA_REG_START) && (addr < V3D_GCA_REG_END)) ||
      ((addr >= V3D_HUB_CTL_REG_START) && (addr < V3D_L3C_REG_END));
#endif
}

static inline bool v3d_is_core_reg(uint32_t addr)
{
   return !v3d_is_noncore_reg(addr);
}

extern bool v3d_is_bin_cle_reg(uint32_t addr);
extern bool v3d_is_render_cle_reg(uint32_t addr);
extern bool v3d_is_l2t_reg(uint32_t addr);

#define V3D_CORE0_REG_START V3D_CTL_0_REG_START
#define V3D_CORE1_REG_START V3D_CTL_1_REG_START
#define V3D_REG_CORE_SEP (V3D_CORE1_REG_START - V3D_CORE0_REG_START)

static inline bool v3d_is_core0_reg(uint32_t addr)
{
#if V3D_VER_AT_LEAST(3,3,0,0)
   return (addr >= V3D_CORE0_REG_START) && (addr < V3D_QPUDBG_0_REG_END);
#else
   return ((addr >= V3D_CORE0_REG_START) && (addr < V3D_GMP_0_REG_END)) ||
      ((addr >= V3D_ERR_0_REG_START) && (addr < V3D_QPUDBG_0_REG_END));
#endif
}

static inline uint32_t v3d_reg_core(uint32_t addr)
{
   assert(v3d_is_core_reg(addr));
   return (addr - V3D_CORE0_REG_START) / V3D_REG_CORE_SEP;
}

static inline uint32_t v3d_reg_to_core0(uint32_t addr)
{
   return addr - (v3d_reg_core(addr) * V3D_REG_CORE_SEP);
}

static inline uint32_t v3d_reg_to_core(uint32_t core, uint32_t core0_addr)
{
   assert(v3d_is_core0_reg(core0_addr));
   return (core * V3D_REG_CORE_SEP) + core0_addr;
}

#if V3D_VER_AT_LEAST(3,3,0,0) && !V3D_VER_AT_LEAST(4,1,34,0)

static inline uint32_t v3d_mmu_reg_to_tfu(uint32_t core0_addr)
{
   assert(core0_addr >= V3D_MMU_REG_START && core0_addr < V3D_MMU_REG_END);
   return (core0_addr - V3D_MMU_REG_START) + V3D_MMU_T_REG_START;
}

#endif

#if V3D_VER_AT_LEAST(4,1,34,0)
#define V3D_PCTR_SRC(x) V3D_PCTR_0_SRC_0_3_PCTRS0_##x
#else
#define V3D_PCTR_SRC(x) V3D_PCTR_0_PCTRS0_PCTRS_##x
#endif

#define V3D_PCTR_SRC_NUM V3D_PCTR_SRC(num)
