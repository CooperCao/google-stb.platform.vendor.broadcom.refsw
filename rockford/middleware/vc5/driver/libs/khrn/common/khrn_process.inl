/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once
#include "khrn_options.h"
#include "libs/platform/v3d_scheduler.h"

static inline uint32_t khrn_get_num_render_subjobs(void)
{
   assert(khrn_options.render_subjobs != 0);
   return khrn_options.render_subjobs;
}

static inline uint32_t khrn_get_num_bin_subjobs(void)
{
   assert(khrn_options.bin_subjobs != 0);
   return khrn_options.bin_subjobs;
}

static inline uint32_t khrn_get_num_cores(void)
{
   // Return the number of cores reported by the HW.
   return v3d_scheduler_get_hub_identity()->num_cores;
}

static inline uint32_t khrn_get_num_qpus_per_core(void)
{
   const V3D_IDENT_T *identity = v3d_scheduler_get_identity();
   return identity->num_slices * identity->num_qpus_per_slice;
}

static inline bool khrn_get_has_astc(void)
{
   return v3d_scheduler_get_identity()->has_astc;
}

static inline bool khrn_get_has_tfu(void)
{
   return v3d_scheduler_get_hub_identity()->has_tfu;
}

static inline uint32_t khrn_get_vpm_size(void)
{
   return v3d_scheduler_get_identity()->vpm_size_in_multiples_of_8kb * 8192;
}

static inline uint32_t khrn_get_num_slices_per_core(void)
{
   return v3d_scheduler_get_identity()->num_slices;
}

static inline uint32_t khrn_get_num_tmus_per_core(void)
{
   return v3d_scheduler_get_identity()->num_tmus;
}
