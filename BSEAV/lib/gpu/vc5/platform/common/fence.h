/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __DISPLAY_FENCE_H__
#define __DISPLAY_FENCE_H__

#include "sched_abstract.h"

void MakeFence(BEGL_SchedInterface *schedIface, int *fence);
int KeepFence(BEGL_SchedInterface *schedIface, int fence);
void SignalFence(BEGL_SchedInterface *schedIface, int fence);
void WaitFence(BEGL_SchedInterface *schedIface, int fence);

#endif /* __DISPLAY_FENCE_H__ */
