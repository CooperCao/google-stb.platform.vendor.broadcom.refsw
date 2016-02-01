/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "sched_nexus.h"
#include <cutils/log.h>
#include <sync/sync.h>

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


BEGL_SchedInterface *CreateAndroidSchedInterface(BEGL_MemoryInterface *memIface)
{
   /* Get a standard Nexus scheduler interface */
   BEGL_SchedInterface *iface = CreateSchedInterface(memIface);

   if (iface) /* Override a couple of functions with Android-specific ones */
   {
      iface->WaitFence        = WaitFence;
      iface->WaitFenceTimeout = WaitFenceTimeout;
      iface->CloseFence       = CloseFence;
      iface->MakeFence        = NULL;
      iface->SignalFence      = NULL;
   }
   return iface;
}

void DestroyAndroidSchedInterface(BEGL_SchedInterface *iface)
{
   DestroySchedInterface(iface);
}
