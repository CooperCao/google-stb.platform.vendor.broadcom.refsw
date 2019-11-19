/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
      *fence = INVALID_FENCE;

   platform_dbg_message_add("%s - fence = %d", __FUNCTION__, *fence);
}

void FenceInterface_Destroy(const FenceInterface *fi, int *fence)
{
   assert(fi != NULL);
   assert(fence != NULL);

   platform_dbg_message_add("%s - fence = %d", __FUNCTION__, *fence);

   if (fi->destroy && *fence != INVALID_FENCE)
      fi->destroy(fi->base.context, *fence);
   *fence = INVALID_FENCE;
}

bool FenceInterface_Keep(const FenceInterface *fi, int fence)
{
   assert(fi != NULL);

   platform_dbg_message_add("%s %d - fence = %d", __FUNCTION__, __LINE__, fence);

   return fi->keep && fence != INVALID_FENCE ?
      fi->keep(fi->base.context, fence) : false;
}

bool FenceInterface_Wait(const FenceInterface *fi, int fence,
      uint32_t timeoutms)
{
   assert(fi != NULL);

   platform_dbg_message_add("%s %d - fence = %d, timeout = %d", __FUNCTION__, __LINE__, fence, timeoutms);

   return fi->wait && fence != INVALID_FENCE ?
      fi->wait(fi->base.context, fence, timeoutms) : true;
}

void FenceInterface_Signal(const FenceInterface *fi, int fence)
{
   assert(fi != NULL);

   platform_dbg_message_add("%s %d - fence = %d", __FUNCTION__, __LINE__, fence);

   if (fi->signal && fence != INVALID_FENCE)
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
