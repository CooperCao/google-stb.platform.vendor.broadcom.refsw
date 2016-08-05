/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#pragma once

#include "v3d_gen.h"

VCOS_EXTERN_C_BEGIN

extern void v3d_check_ver(int v3d_ver);
#if !V3D_HAS_IDENT_WITH_L2T
extern int v3d_ver_from_ident(const V3D_IDENT_T *ident);
#endif
extern int v3d_ver_from_hub_ident(const V3D_HUB_IDENT_T *ident);

extern void v3d_check_ident(const V3D_IDENT_T *ident, uint32_t core);
extern void v3d_check_hub_ident(const V3D_HUB_IDENT_T *ident);

VCOS_EXTERN_C_END
