/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#if KHRN_GLES31_DRIVER
#include "libs/compute/compute.h"

typedef struct khrn_memaccess khrn_memaccess;
typedef struct glxx_compute_render_state glxx_compute_render_state;

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

bool glxx_compute_render_state_flush(glxx_compute_render_state* rs);

compute_job_mem* glxx_compute_process_dispatches(
   glxx_compute_dispatch const* dispatches,
   uint32_t num_dispatches);

#endif