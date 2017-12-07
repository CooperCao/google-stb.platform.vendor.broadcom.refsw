/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "fence_interface.h"

#include <stddef.h>
#include <assert.h>

#include "debug_helper.h"

void FenceInterface_Create(const FenceInterface *fi, int *fence)
{
   assert(fi != NULL);
   assert(fence != NULL);

   if (fi->create)
      fi->create(fi->base.context, fence);
   else
      *fence = fi->invalid_fence;

   platform_dbg_message_add("%s - fence = %d", __FUNCTION__, *fence);
}

void FenceInterface_Destroy(const FenceInterface *fi, int *fence)
{
   assert(fi != NULL);
   assert(fence != NULL);

   platform_dbg_message_add("%s - fence = %d", __FUNCTION__, *fence);

   if (fi->destroy && *fence != fi->invalid_fence)
      fi->destroy(fi->base.context, *fence);
   *fence = fi->invalid_fence;
}

bool FenceInterface_Keep(const FenceInterface *fi, int fence)
{
   assert(fi != NULL);

   platform_dbg_message_add("%s %d - fence = %d", __FUNCTION__, __LINE__, fence);

   return fi->keep && fence != fi->invalid_fence ?
      fi->keep(fi->base.context, fence) : false;
}

bool FenceInterface_Wait(const FenceInterface *fi, int fence,
      uint32_t timeoutms)
{
   assert(fi != NULL);

   platform_dbg_message_add("%s %d - fence = %d, timeout = %d", __FUNCTION__, __LINE__, fence, timeoutms);

   return fi->wait && fence != fi->invalid_fence ?
      fi->wait(fi->base.context, fence, timeoutms) : true;
}

void FenceInterface_Signal(const FenceInterface *fi, int fence)
{
   assert(fi != NULL);

   platform_dbg_message_add("%s %d - fence = %d", __FUNCTION__, __LINE__, fence);

   if (fi->signal && fence != fi->invalid_fence)
      fi->signal(fi->base.context, fence);
}

void FenceInterface_WaitAndDestroy(const FenceInterface *fi,
      int *fence)
{
   assert(fi != NULL);
   assert(fence != NULL);

   FenceInterface_Wait(fi, *fence, FENCE_WAIT_INFINITE);
   FenceInterface_Destroy(fi, fence);
}
