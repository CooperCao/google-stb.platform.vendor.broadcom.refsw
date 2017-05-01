/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "fence.h"

void MakeFence(BEGL_SchedInterface *schedIface, int *fence)
{
   if (schedIface != NULL && schedIface->MakeFence != NULL)
      schedIface->MakeFence(schedIface->context, fence);
}

int KeepFence(BEGL_SchedInterface *schedIface, int fence)
{
   if (fence < 0)
      return fence;

   return (schedIface != NULL && schedIface->KeepFence != NULL) ?
      schedIface->KeepFence(schedIface->context, fence) : -1;
}

void SignalFence(BEGL_SchedInterface *schedIface, int fence)
{
   if (fence < 0)
      return;

   if (schedIface != NULL && schedIface->SignalFence != NULL)
      schedIface->SignalFence(schedIface->context, fence);
}

void WaitFence(BEGL_SchedInterface *schedIface, int fence)
{
   if (fence < 0)
      return;

   if (schedIface != NULL &&
      schedIface->WaitFence != NULL &&
      schedIface->CloseFence != NULL)
   {
      schedIface->WaitFence(schedIface->context, fence);
      schedIface->CloseFence(schedIface->context, fence);
   }
}
