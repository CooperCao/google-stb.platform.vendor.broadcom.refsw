/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include "libs/core/v3d/v3d_addr.h"
#include "libs/core/v3d/v3d_ver.h"
#include "libs/util/common.h"
#include <stdint.h>

EXTERN_C_BEGIN

typedef struct khrn_fmem khrn_fmem;

typedef struct khrn_fmem_tmu_cfg_alloc
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   v3d_addr_t addr;
   void* ptr;
   uint64_t spare_8;    // Mask of spare 8-byte aligned, 8-byte chunks backwards from spare_32;
   uint32_t spare_16;   // Mask of spare 16-byte aligned, 16-byte chunks backwards from spare_32;
   uint32_t spare_32;   // Offset of remaining 32-byte chunks.
   uint32_t num_32;     // Number of 32-byte chunks at ptr/addr + spare_32.
#else
   void*       spare_ptr;
   v3d_addr_t  spare_addr;
   uint32_t    num_spare;
#endif
} khrn_fmem_tmu_cfg_alloc;

#if V3D_VER_AT_LEAST(4,0,2,0)

// Reserve a contiguous block of 32-byte tmu tex-state/sampler configs.
// Subsequent calls to add_tmu_tex_state/add_tmu_sampler with the reserved32
// flag are guaranteed to fill the next 32-byte slot.
bool khrn_fmem_reserve_tmu_cfg(khrn_fmem *fmem, unsigned num_32);

v3d_addr_t khrn_fmem_add_tmu_tex_state(khrn_fmem *fmem, const void *tex_state,
      bool extended, bool reserved32);
v3d_addr_t khrn_fmem_add_tmu_sampler(khrn_fmem *fmem, const void *sampler,
      bool extended, bool reserved32);
#else
v3d_addr_t khrn_fmem_add_tmu_indirect(khrn_fmem *fmem, uint32_t const *tmu_indirect);
#endif
EXTERN_C_END
