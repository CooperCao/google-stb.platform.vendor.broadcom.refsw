/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __PRIVATE_ANDROID_H__
#define __PRIVATE_ANDROID_H__

#include "egl_platform_abstract.h"

#include "gmem_abstract.h"
#include "sched_abstract.h"

typedef struct
{
   BEGL_InitInterface      *initInterface;
   BEGL_MemoryInterface    *memoryInterface;
   BEGL_SchedInterface     *schedInterface;
   BEGL_DisplayInterface   *displayInterface;

   bool                     drm;
} ANPL_InternalPlatformHandle;
#endif /* __PRIVATE_ANDROID_H__ */
