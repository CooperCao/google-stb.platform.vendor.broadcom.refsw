/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#if V3D_VER_AT_LEAST(3,3,0,0)

#include "../common/khrn_fmem.h"
#include "glxx_render_state.h"
#include "glxx_server_state.h"
#include "libs/util/assert_helpers.h"

static_assrt(offsetof(glxx_render_state, fmem) == 0);

typedef struct glxx_compute_render_state
{
   // Inherit from glxx_render_state.
   union
   {
      glxx_render_state base;
      khrn_fmem fmem;
   };

   GLXX_SERVER_STATE_T* server_state;

#if !V3D_USE_CSD
   v3d_addr_t clist_start;
#endif
} glxx_compute_render_state;

#endif
