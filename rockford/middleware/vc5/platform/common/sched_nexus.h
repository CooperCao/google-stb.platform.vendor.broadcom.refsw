/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __SCHED_NEXUS_H__
#define __SCHED_NEXUS_H__

#include "sched_abstract.h"
#include "gmem_abstract.h"

#ifdef __cplusplus
extern "C" {
#endif

BEGL_SchedInterface *CreateSchedInterface(BEGL_MemoryInterface *iface);
void DestroySchedInterface(BEGL_SchedInterface *iface);

#ifdef __cplusplus
}
#endif

#endif
