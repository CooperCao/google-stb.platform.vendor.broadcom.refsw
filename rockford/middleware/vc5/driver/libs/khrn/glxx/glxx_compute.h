/*=============================================================================
 * Broadcom Proprietary and Confidential. (c)2015 Broadcom.
 * All rights reserved.
 * =============================================================================*/
#pragma once
#include "libs/platform/gmem.h"

typedef struct glxx_compute_render_state glxx_compute_render_state;
typedef struct glxx_compute_shared glxx_compute_shared;

bool glxx_compute_render_state_flush(glxx_compute_render_state* rs);

void glxx_compute_shared_init(glxx_compute_shared* cs);
void glxx_compute_shared_term(glxx_compute_shared* cs);

struct glxx_compute_shared
{
   gmem_handle_t shared_buf;
};