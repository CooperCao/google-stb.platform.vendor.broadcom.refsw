/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#if V3D_VER_AT_LEAST(3,3,0,0)
#include "libs/core/v3d/v3d_ver.h"
#include "libs/khrn/common/khrn_vector.h"
#include <stdbool.h>
#include <stdint.h>

typedef struct glxx_compute_render_state glxx_compute_render_state;

#if V3D_USE_CSD

#include "libs/platform/bcm_sched_job.h"

typedef v3d_compute_subjob glxx_compute_dispatch;

typedef struct glxx_compute_indirect
{
   const uint32_t* num_wgs;
   unsigned dispatch_index;
} glxx_compute_indirect;

void glxx_compute_process_indirect_dispatches(
   khrn_vector* dispatches_vec,
   const khrn_vector* indirect_vec);

#else

#include "libs/compute/compute.h"

typedef struct glxx_compute_dispatch
{
   compute_program program;
   v3d_addr_t unifs_addr;
   uint8_t* dispatch_cl;

   union
   {
      uint32_t immediate[3];
      uint32_t const* indirect;
   } num_work_groups;
   bool is_indirect;

} glxx_compute_dispatch;

compute_job_mem* glxx_compute_process_dispatches(const khrn_vector* dispatches_vec);

#endif

bool glxx_compute_render_state_flush(glxx_compute_render_state* rs);

#endif