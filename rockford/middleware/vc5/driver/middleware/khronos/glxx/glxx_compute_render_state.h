/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/
#pragma once

#include "middleware/khronos/common/khrn_fmem.h"
#include "middleware/khronos/glxx/glxx_server_state.h"

typedef struct glxx_compute_render_state_s
{
   khrn_fmem fmem;

   GLXX_SERVER_STATE_T* server_state;
} glxx_compute_render_state;
