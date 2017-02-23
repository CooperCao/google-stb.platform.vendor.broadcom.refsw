/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once
#include "../common/khrn_fmem.h"

typedef struct glxx_render_state
{
   KHRN_FMEM_T fmem;

   bool has_buffer_writes;
} glxx_render_state;
