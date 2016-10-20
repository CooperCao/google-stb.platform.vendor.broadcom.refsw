/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "fence_interface.h"

#include <stddef.h>
#include <assert.h>

void FenceInterface_Create(const FenceInterface *fi, int *fence)
{
   assert(fi != NULL);
   assert(fence != NULL);

   if (fi->create)
      fi->create(fi->base.context, fence);
   else
      *fence = fi->invalid_fence;
}

void FenceInterface_Destroy(const FenceInterface *fi, int *fence)
{
   assert(fi != NULL);
   assert(fence != NULL);

   if (fi->destroy && *fence != fi->invalid_fence)
      fi->destroy(fi->base.context, *fence);
   *fence = fi->invalid_fence;
}

bool FenceInterface_Keep(const FenceInterface *fi, int fence)
{
   assert(fi != NULL);

   return fi->keep && fence != fi->invalid_fence ?
         fi->keep(fi->base.context, fence) : false;
}

bool FenceInterface_Wait(const FenceInterface *fi, int fence,
      uint32_t timeoutms)
{
   assert(fi != NULL);

   return fi->wait && fence != fi->invalid_fence ?
      fi->wait(fi->base.context, fence, timeoutms) : true;
}

void FenceInterface_Signal(const FenceInterface *fi, int fence)
{
   assert(fi != NULL);

   if (fi->signal && fence != fi->invalid_fence)
      fi->signal(fi->base.context, fence);
}

void FenceInterface_WaitAndDestroy(const FenceInterface *fi,
      int *fence)
{
   assert(fi != NULL);
   assert(fence != NULL);

   FenceInterface_Wait(fi, *fence, FENCE_WAIT_ININITE);
   FenceInterface_Destroy(fi, fence);
}
