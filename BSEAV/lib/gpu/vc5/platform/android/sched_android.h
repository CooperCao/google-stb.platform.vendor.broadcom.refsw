/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __SCHED_ANDROID_H__
#define __SCHED_ANDROID_H__

#include "sched_abstract.h"
#include "gmem_abstract.h"

BEGL_SchedInterface *CreateAndroidSchedInterface(BEGL_MemoryInterface *iface);
void DestroyAndroidSchedInterface(BEGL_SchedInterface *iface);

#endif
