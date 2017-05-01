/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_XY_H
#define V3D_XY_H

#include "libs/util/gfx_util/gfx_util.h"

/* Unpack xy and convert from "1/4 offset" 12.4 to 24.8 */
static inline void v3d_unpack_xy(int32_t *x, int32_t *y, uint32_t xy)
{
   *x = (gfx_sext((xy & 0xffff) - 0x4000, 16) + 0x4000) << 4;
   *y = (gfx_sext((xy >> 16) - 0x4000, 16) + 0x4000) << 4;
}

#endif
