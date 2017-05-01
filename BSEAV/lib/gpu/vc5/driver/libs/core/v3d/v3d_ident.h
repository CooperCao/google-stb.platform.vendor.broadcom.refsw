/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "v3d_gen.h"

VCOS_EXTERN_C_BEGIN

extern void v3d_check_ver(int v3d_ver);
#if !V3D_VER_AT_LEAST(4,0,2,0)
extern int v3d_ver_from_ident(const V3D_IDENT_T *ident);
#endif
extern int v3d_ver_from_hub_ident(const V3D_HUB_IDENT_T *ident);

extern void v3d_check_ident(const V3D_IDENT_T *ident, uint32_t core);
extern void v3d_check_hub_ident(const V3D_HUB_IDENT_T *ident);

extern size_t v3d_sprint_device_name(char *buf, size_t buf_size, size_t offset, const V3D_IDENT_T *ident);

VCOS_EXTERN_C_END
