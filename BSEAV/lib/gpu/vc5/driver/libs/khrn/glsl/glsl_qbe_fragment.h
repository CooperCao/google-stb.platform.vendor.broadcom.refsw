/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "glsl_backflow.h"
#include "libs/core/v3d/v3d_ver.h"
#include "libs/core/v3d/v3d_limits.h"

typedef struct {
   bool ms;
   bool sample_alpha_to_coverage;
#if !V3D_HAS_FEP_SAMPLE_MASK
   bool sample_mask;
#endif
   bool fez_safe_with_discard;
   bool early_fragment_tests;
#if !V3D_HAS_RELAXED_THRSW
   bool requires_sbwait;
#endif

   struct {
      bool is_16;
      bool is_int;
      bool is_present;
#if !V3D_VER_AT_LEAST(4,0,2,0)
      bool alpha_16_workaround;
#endif
   } rt[V3D_MAX_RENDER_TARGETS];

   uint8_t  adv_blend;  /* 0=NONE, 1=MULTIPLY, ... */
} FragmentBackendState;

void glsl_fragment_backend(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   const FragmentBackendState *s,
   const bool *shader_outputs_used,
   bool *does_discard_out,
   bool *does_z_change_out);
