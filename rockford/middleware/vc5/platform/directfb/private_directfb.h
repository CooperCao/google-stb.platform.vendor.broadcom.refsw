/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#ifndef __PRIVATE_DIRECTFB_H__
#define __PRIVATE_DIRECTFB_H__

#include "egl_platform_abstract.h"

#include "gmem_abstract.h"
#include "sched_abstract.h"

typedef struct
{
   BEGL_MemoryInterface    *memoryInterface;
   BEGL_SchedInterface     *schedInterface;
   BEGL_DisplayInterface   *displayInterface;
} DBPL_InternalPlatformHandle;

#endif /* __PRIVATE_DIRECTFB_H__ */
