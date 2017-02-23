/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __PRIVATE_NEXUS_H__
#define __PRIVATE_NEXUS_H__

#include <EGL/begl_displayplatform.h>

#include "gmem_abstract.h"
#include "sched_abstract.h"
#include "display_nexus.h"

#include "nexus_display.h"
#include "nexus_surface.h"
#include "nexus_memory.h"
#include "berr.h"
#include "bkni_multi.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

/* NXPL_NativeWindow */
typedef struct
{
   /* Main thread data */
   NXPL_NativeWindowInfoEXT   windowInfo;
   unsigned int               numSurfaces;

   bool                       initialized;
   int                        swapInterval;

#ifdef NXPL_PLATFORM_EXCLUSIVE
   /* create and delete can overlap, although exclusive access to
      the buffer is mandated.  This delete of one window can remove
      the callback of another */
   int                        bound;
#else
   uint32_t                   clientID;
   /* NSC client handle */
   NEXUS_SurfaceClientHandle  surfaceClient;
#ifdef NXCLIENT_SUPPORT
   NxClient_AllocResults      allocResults;
#endif
#endif /* NXPL_PLATFORM_EXCLUSIVE */

} NXPL_NativeWindow;

typedef struct
{
   uint32_t                   magic;
   NEXUS_SurfaceHandle        surface;
   uint32_t                   physicalOffset;
   void                      *cachedPtr;
   int                        fence;
   bool                       poisoned;
   int                        swapInterval;
   /* the window info flows through the pipeline with the buffer and is used
      at presentation time */
   NXPL_NativeWindowInfoEXT   windowInfo;
   BEGL_BufferFormat          format;
   bool                       secure;
} NXPL_Surface;

typedef void (*NXPL_SurfaceOffDisplay)(void *context, NEXUS_SurfaceHandle surface);

typedef struct
{
   /* Thread data and mutex */
   pthread_t                  displayThread;
   pthread_barrier_t          barrier;

   void                      *displayQueue;
   void                      *fenceQueue;

   /* data copied through from the owner */
   BEGL_SchedInterface       *schedIface;
   NXPL_NativeWindowInfoEXT   windowInfo;
   BEGL_BufferFormat          format;
   unsigned int               numSurfaces;

   /* opaque, do not access in the thread */
   void                      *nw;
} NXPL_NativeWindow_priv;

typedef struct
{
   BEGL_MemoryInterface    *memoryInterface;
   bool                     drm;
   BEGL_SchedInterface     *schedInterface;
   BEGL_DisplayInterface   *displayInterface;
} NXPL_InternalPlatformHandle;

#endif /* __PRIVATE_NEXUS_H__ */
