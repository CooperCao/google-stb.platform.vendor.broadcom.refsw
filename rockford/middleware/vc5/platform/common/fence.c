/*=============================================================================
Copyright (c) 2015 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "fence.h"

void MakeFence(BEGL_SchedInterface *schedIface, int *fence)
{
   if (schedIface != NULL && schedIface->MakeFence != NULL)
      schedIface->MakeFence(schedIface->context, fence);
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