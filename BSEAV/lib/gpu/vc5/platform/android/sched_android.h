/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __SCHED_ANDROID_H__
#define __SCHED_ANDROID_H__

#include "sched_abstract.h"
#include "gmem_abstract.h"

#ifdef __cplusplus
extern "C" {
#endif

BEGL_SchedInterface *CreateAndroidSchedInterface(BEGL_MemoryInterface *iface);
void DestroyAndroidSchedInterface(BEGL_SchedInterface *iface);

#ifdef __cplusplus
}
#endif

#endif
