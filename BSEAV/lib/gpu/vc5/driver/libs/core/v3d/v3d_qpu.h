/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_ver.h"
#include "v3d_limits.h"

#include <assert.h>
#include <stdint.h>

EXTERN_C_BEGIN

static inline uint32_t v3d_qpu_rf_row(uint32_t thread, uint32_t i)
{
   assert(thread < V3D_NUM_THREADS_PER_QPU);
   uint32_t min_rows_per_thread = V3D_QPU_RF_SIZE / V3D_NUM_THREADS_PER_QPU;
   return i ^ (thread * min_rows_per_thread);
}

EXTERN_C_END
