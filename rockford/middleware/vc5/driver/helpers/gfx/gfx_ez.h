/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  helpers
Module   :

FILE DESCRIPTION
=============================================================================*/

#ifndef GFX_EZ_H
#define GFX_EZ_H

#include "helpers/v3d/v3d_common.h"
#include "helpers/v3d/v3d_cl.h"

typedef struct
{
   /* Stick these in the CFG_BITS instruction */
   bool cfg_bits_ez;
   bool cfg_bits_ez_update;

   /* Stick these in the rendering mode config instruction */
   v3d_ez_direction_t rcfg_ez_direction;
   bool rcfg_ez_disable;

   /* For internal use only */
   bool disabled_for_rest_of_frame;
   bool chosen_direction;
} GFX_EZ_STATE_T;

/* Call at the start of a frame. If enable is false, early Z will be completely
 * disabled for the frame */
extern void gfx_ez_init(GFX_EZ_STATE_T *state, bool enable);

/* This should be called any time one of the arguments changes. It should also
 * be called once at the start of a frame (after gfx_ez_init) to provide the
 * initial config. If true is returned, the cfg_bits values have changed and a
 * new CFG_BITS instruction should be written to the control list */
extern bool gfx_ez_update_cfg(GFX_EZ_STATE_T *state,
   v3d_compare_func_t depth_func, bool depth_update,
   bool stencil,
   v3d_stencil_op_t front_fail_op, v3d_stencil_op_t front_depth_fail_op,
   v3d_stencil_op_t back_fail_op, v3d_stencil_op_t back_depth_fail_op);

#endif
