/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "sched_nexus.h"
#include <cutils/log.h>
#include <sync/sync.h>

#include "nexus_graphicsv3d.h"

static void WaitFence(void *context, int fence)
{
   /* wait 3 seconds for this fence; if we do timeout, print a message and
    * only then wait forever */

   unsigned msg_timeout = 3000;
   int err = sync_wait(fence, msg_timeout);
   if (err < 0 && errno == ETIME)
   {
      ALOGE("%s: fence %d didn't signal in %u ms", __FUNCTION__, fence, msg_timeout);
      err = sync_wait(fence, -1);
   }
   if (err < 0)
      ALOGE("%s: error while waiting for fence %d", __FUNCTION__, fence);
}

static BEGL_FenceStatus WaitFenceTimeout(void *context, int fence, uint32_t timeoutms)
{
   BEGL_FenceStatus     ret;
   int err;

   err = sync_wait(fence, timeoutms);
   ret = err >= 0 ? BEGL_FenceSignaled : BEGL_FenceTimeout;

   return ret;
}

static void CloseFence(void *context, int fence)
{
   close(fence);
}

static bool WaitForAnyNonFinalisedJob(void *context)
{
   int fence = MakeFenceForAnyNonFinalizedJob(context);

   if (fence != -1)
   {
      WaitFence(context, fence);
      CloseFence(context, fence);
   }

   return fence != -1;
}

static void WaitJobs(void *context,
      const struct bcm_sched_dependencies *completed_deps,
      const struct bcm_sched_dependencies *finalised_deps)
{
   int fence = MakeFenceForJobs(context, completed_deps, finalised_deps, false);

   if (fence != -1)
   {
      WaitFence(context, fence);
      CloseFence(context, fence);
   }
}

static BEGL_FenceStatus WaitJobsTimeout(void *context,
      const struct bcm_sched_dependencies *completed_deps,
      const struct bcm_sched_dependencies *finalised_deps,
      uint32_t timeoutms)
{
   int fence = MakeFenceForJobs(context, completed_deps, finalised_deps, false);
   if (fence == -1)
      return BEGL_FenceSignaled; /* dependency done */

   BEGL_FenceStatus status = WaitFenceTimeout(context, fence, timeoutms);
   CloseFence(context, fence);
   return status;
}

static BEGL_FenceStatus WaitForAnyJobTimeout(void *context,
      const struct bcm_sched_dependencies *completed_deps,
      const struct bcm_sched_dependencies *finalised_deps,
      uint32_t timeoutms)
{
   int fence = MakeFenceForAnyJob(context, completed_deps, finalised_deps);
   if (fence == -1)
      return BEGL_FenceSignaled; /* dependency done */

   BEGL_FenceStatus status = WaitFenceTimeout(context, fence, timeoutms);
   CloseFence(context, fence);
   return status;
}


BEGL_SchedInterface *CreateAndroidSchedInterface(BEGL_MemoryInterface *memIface)
{
   /* Get a standard Nexus scheduler interface */
   BEGL_SchedInterface *iface = CreateSchedInterface(memIface);

   if (iface) /* Override a couple of functions with Android-specific ones */
   {
      iface->WaitForAnyNonFinalisedJob = WaitForAnyNonFinalisedJob;
      iface->WaitJobs                  = WaitJobs;
      iface->WaitJobsTimeout           = WaitJobsTimeout;
      iface->WaitForAnyJobTimeout      = WaitForAnyJobTimeout;
   }
   return iface;
}

void DestroyAndroidSchedInterface(BEGL_SchedInterface *iface)
{
   DestroySchedInterface(iface);
}
