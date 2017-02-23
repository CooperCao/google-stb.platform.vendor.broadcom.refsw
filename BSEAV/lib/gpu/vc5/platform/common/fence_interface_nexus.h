/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __FENCE_INTERFACE_NEXUS_H__
#define __FENCE_INTERFACE_NEXUS_H__

#include "fence_interface.h"
#include <EGL/begl_schedplatform.h>

/* A fence interface implementation that uses Nexus fences */
void FenceInteraface_InitNexus(FenceInterface *fi,
      const BEGL_SchedInterface *sched);

#endif /* __FENCE_INTERFACE_NEXUS_H__ */
