/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef V3D_DRIVER_API_H_
#define V3D_DRIVER_API_H_

#include "bcm_sched_api.h"
#include "bcm_sched_job.h"
#include "libs/core/v3d/v3d_limits.h"

#ifdef __cplusplus
extern "C" {
#endif

#define V3D_DEV_NAME "v3dv3"

#define V3D_IDENT_REGISTERS      4
#define V3D_HUB_IDENT_REGISTERS  4
struct v3d_idents
{
  uint32_t ident[V3D_MAX_CORES * V3D_IDENT_REGISTERS];
  uint32_t hubIdent[V3D_HUB_IDENT_REGISTERS];
  uint32_t ddrMapVer;
};
extern void v3d_get_info(struct v3d_idents *info);

#ifdef __cplusplus
}
#endif

#endif /* V3D_DRIVER_API_H_ */
