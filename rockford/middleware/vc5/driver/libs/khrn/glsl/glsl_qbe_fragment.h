/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  glsl
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GLSL_QBE_FRAGMENT_H__
#define GLSL_QBE_FRAGMENT_H__

#include "glsl_backflow.h"
#include "libs/core/v3d/v3d_limits.h"

typedef struct {
   bool ms;
   bool sample_alpha_to_coverage;
   bool sample_mask;
   bool fez_safe_with_discard;
   bool early_fragment_tests;
   bool requires_sbwait;

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

#endif
