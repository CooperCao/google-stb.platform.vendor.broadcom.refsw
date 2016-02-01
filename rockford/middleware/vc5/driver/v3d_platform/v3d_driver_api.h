/*******************************************************************************
Copyright 2013 Broadcom Corporation.  All rights reserved.

Unless you and Broadcom execute a separate written software license
agreement governing use of this software, this software is licensed to
you under the terms of the GNU General Public License version 2,
available at http://www.gnu.org/copyleft/gpl.html (the "GPL").

Notwithstanding the above, under no circumstances may you combine this
software in any way with any other Broadcom software provided under a
license other than the GPL, without Broadcom's express prior written
consent.
*******************************************************************************/

#ifndef V3D_DRIVER_API_H_
#define V3D_DRIVER_API_H_

#include "bcm_sched_api.h"
#include "bcm_sched_job.h"
#include "helpers/v3d/v3d_limits.h"

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
};
extern void v3d_get_info(struct v3d_idents *info);

#ifdef __cplusplus
}
#endif

#endif /* V3D_DRIVER_API_H_ */
