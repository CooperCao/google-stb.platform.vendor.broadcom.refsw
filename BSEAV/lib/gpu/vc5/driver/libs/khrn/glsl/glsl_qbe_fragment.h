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
   bool sample_mask;
   bool fez_safe_with_discard;
   bool early_fragment_tests;
#if !V3D_HAS_RELAXED_THRSW
   bool requires_sbwait;
#endif

   struct {
      int type;
      bool alpha_16_workaround;
   } rt[V3D_MAX_RENDER_TARGETS];

   uint8_t  adv_blend;  /* 0=NONE, 1=MULTIPLY, ... */
} FragmentBackendState;

void glsl_fragment_backend(
   SchedBlock *block,
   int block_id,
   const IRShader *sh,
   const LinkMap *link_map,
   const FragmentBackendState *s,
   bool *does_discard_out,
   bool *does_z_change_out);
