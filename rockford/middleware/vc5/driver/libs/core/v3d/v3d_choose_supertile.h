/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#pragma once

#include "vcos_types.h"

VCOS_EXTERN_C_BEGIN

// Choose sensible supertile size
void v3d_choose_supertile_sizes(
   unsigned tiles_x, unsigned tiles_y,
   unsigned min_supertile_w, unsigned min_supertile_h, unsigned max_supertiles,
   unsigned *supertile_w, unsigned *supertile_h);

VCOS_EXTERN_C_END
