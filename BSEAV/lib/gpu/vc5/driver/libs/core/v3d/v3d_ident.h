/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_gen.h"
#include "v3d_limits.h"

EXTERN_C_BEGIN

extern void v3d_check_ver(int v3d_ver);
#if !V3D_VER_AT_LEAST(4,1,34,0)
extern int v3d_ver_from_ident(const V3D_IDENT_T *ident);
#endif
extern int v3d_ver_from_hub_ident(const V3D_HUB_IDENT_T *ident);

extern void v3d_check_ident(const V3D_IDENT_T *ident, uint32_t core);
extern void v3d_check_hub_ident(const V3D_HUB_IDENT_T *ident);

extern size_t v3d_sprint_device_name(char *buf, size_t buf_size, size_t offset, const V3D_IDENT_T *ident);

static inline uint32_t v3d_l2t_size(uint32_t ways, uint32_t way_depth)
{
   return (V3D_L2T_CACHE_LINE_SIZE << way_depth) * ways;
}

static inline uint32_t v3d_l2t_size_from_ident(const V3D_IDENT_T *ident)
{
#if V3D_VER_AT_LEAST(4,1,34,0)
   return v3d_l2t_size(ident->l2t_ways, ident->l2t_way_depth);
#else
   // The L2T size is not stored in the ident, but small configurations of
   // V3D like 7250 and 7260 had a cut down L2T.
   uint32_t l2t_size_in_kb = (ident->num_slices == 1) ?
      (V3D_VER_AT_LEAST(3,3,0,0) ? 32 : 16) :
      (V3D_VER_AT_LEAST(3,3,0,0) ? 256 : 128);
   return l2t_size_in_kb * 1024;
#endif
}

EXTERN_C_END
