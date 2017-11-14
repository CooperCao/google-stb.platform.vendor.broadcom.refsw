/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_ver.h"

/* Sample x offset from pixel center, in 1/8ths of a pixel */
static inline int v3d_sample_x_offset(unsigned i)
{
#if V3D_HAS_STD_4X_RAST_PATT
   static const int offsets[] = {-1, 3, -3, 1};
#else
   static const int offsets[] = {1, -3, 3, -1};
#endif
   assert(i < countof(offsets));
   return offsets[i];
}

/* Sample y offset from pixel center, in 1/8ths of a pixel */
static inline int v3d_sample_y_offset(unsigned i)
{
   assert(i < 4);
   return -3 + ((int)i * 2);
}
