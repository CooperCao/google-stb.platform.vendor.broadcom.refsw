/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __SCHED_ABSTRACT_H__
#define __SCHED_ABSTRACT_H__

#include "../bcm_sched_job.h"
#include "../bcm_perf_structs.h"
#include "gmem_abstract.h"
#include "../v3d_driver_api.h"

#include <EGL/begl_schedplatform.h>

/*
 Signals a fence;
 */
void v3d_platform_fence_signal(int fence);

#endif
