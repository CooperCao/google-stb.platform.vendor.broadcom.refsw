/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include "../common/khrn_fmem.h"

typedef struct glxx_render_state
{
   khrn_fmem fmem;

   bool has_buffer_writes;
} glxx_render_state;
