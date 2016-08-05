/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "fence_interface_nexus.h"

/* A fence interface implementation that uses Nexus fences */

static void create(void *context, int *fence)
{
   const BEGL_SchedInterface *sched = (const BEGL_SchedInterface *)context;
   sched->MakeFence(sched->context, fence);
}

static void destroy(void *context, int fence)
{
   const BEGL_SchedInterface *sched = (const BEGL_SchedInterface *)context;
   sched->CloseFence(sched->context, fence);
}

static bool keep(void *context, int fence)
{
   const BEGL_SchedInterface *sched = (const BEGL_SchedInterface *)context;
   return sched->KeepFence(sched->context, fence) == BEGL_SchedSuccess;
}

static bool wait(void *context, int fence, uint32_t timeoutms)
{
   const BEGL_SchedInterface *sched = (const BEGL_SchedInterface *)context;
   if (timeoutms == FENCE_WAIT_ININITE)
   {
      sched->WaitFence(sched->context, fence);
      return true;
   }
   else
   {
      return sched->WaitFenceTimeout(sched->context, fence,
            timeoutms) == BEGL_FenceSignaled;
   }
}

static void signal(void *context, int fence)
{
   const BEGL_SchedInterface *sched = (const BEGL_SchedInterface *)context;
   sched->SignalFence(sched->context, fence);
}


void FenceInteraface_InitNexus(struct FenceInterface *fi,
      const BEGL_SchedInterface *sched)
{
   fi->base.context = (void*)sched;
   fi->base.destroy = NULL; /* unused */

   fi->invalid_fence = -1;
   fi->create = create;
   fi->destroy = destroy;
   fi->keep = keep;
   fi->wait = wait;
   fi->signal = signal;
}
