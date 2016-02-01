/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#ifndef __PRIVATE_NEXUS_H__
#define __PRIVATE_NEXUS_H__

#include "egl_platform_abstract.h"

#include "gmem_abstract.h"
#include "sched_abstract.h"
#include "display_nexus.h"

#include "nexus_display.h"
#include "nexus_surface.h"
#include "nexus_memory.h"

typedef struct
{
   NEXUS_SurfaceHandle     surface;
   uint32_t                physicalOffset;
   void                   *cachedPtr;
   int                     fence;
   bool                    poisoned;
   int                     swapInterval;
   /* the window info flows through the pipeline with the buffer and is used
      at presentation time */
   NXPL_NativeWindowInfo   windowInfo;
   BEGL_BufferFormat       format;
} NXPL_Surface;

typedef struct
{
   /* Thread data and mutex */
   pthread_t               displayThread;
   pthread_barrier_t       barrier;
#ifdef NXPL_PLATFORM_EXCLUSIVE
   void                   *bufferOnDisplayEvent;
#endif
   void                   *vsyncEvent;
   bool                    terminating;

   void                   *displayQueue;
   void                   *fenceQueue;
#ifdef NXPL_PLATFORM_EXCLUSIVE
   int                     lastFence;
#endif

   /* data copied through from the owner */
   BEGL_SchedInterface    *schedIface;
   NXPL_NativeWindowInfo   windowInfo;
   BEGL_BufferFormat       format;
   unsigned int            numSurfaces;
   NEXUS_DISPLAYHANDLE     display;
   bool                    vsyncAvailable;

#ifdef NXPL_PLATFORM_NSC
   uint32_t                clientID;
   NEXUS_SurfaceClientHandle surfaceClient;
#endif

   NXPL_DisplayType        displayType;
   int                    *bound;

   /* opaque, do not access in the thread */
   void                   *nw;
} NXPL_NativeWindow_priv;

typedef struct
{
   BEGL_MemoryInterface    *memoryInterface;
   BEGL_SchedInterface     *schedInterface;
   BEGL_DisplayInterface   *displayInterface;
} NXPL_InternalPlatformHandle;

#endif /* __PRIVATE_NEXUS_H__ */
