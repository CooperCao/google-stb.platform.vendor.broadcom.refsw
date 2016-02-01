/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#ifndef __SCHED_NEXUS_H__
#define __SCHED_NEXUS_H__

#include "sched_abstract.h"
#include "gmem_abstract.h"

BEGL_SchedInterface *CreateSchedInterface(BEGL_MemoryInterface *iface);
void DestroySchedInterface(BEGL_SchedInterface *iface);

#endif
