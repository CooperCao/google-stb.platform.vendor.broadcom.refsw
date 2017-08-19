/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __SCHED_NEXUS_H__
#define __SCHED_NEXUS_H__

#include "sched_abstract.h"
#include "gmem_abstract.h"

#include "../common/fence_interface.h"

#ifdef __cplusplus
extern "C" {
#endif

BEGL_SchedInterface *CreateSchedInterface(BEGL_MemoryInterface *iface);
void DestroySchedInterface(BEGL_SchedInterface *iface);

/* Common code that can be used by other platforms */
int MakeFenceForJobs(void *context,
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps,
   bool force_create);

int MakeFenceForAnyNonFinalizedJob(void *session);

int MakeFenceForAnyJob(void *context,
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps);

void InitFenceInterface(FenceInterface *fi, const BEGL_SchedInterface *sched);

#ifdef __cplusplus
}
#endif

#endif
