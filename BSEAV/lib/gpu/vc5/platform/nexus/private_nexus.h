/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
   uint64_t                   physicalOffset;
   void                      *cachedPtr;
   int                        fence;
   bool                       poisoned;
   /* the window info flows through the pipeline with the buffer and is used
      at presentation time */
   NXPL_NativeWindowInfoEXT   windowInfo;
   BEGL_BufferFormat          format;
   bool                       secure;
} NXPL_Surface;

typedef struct
{
   BEGL_MemoryInterface    *memoryInterface;
   bool                     drm;
   BEGL_SchedInterface     *schedInterface;
   BEGL_DisplayInterface   *displayInterface;
   NXPL_DisplayContext     *displayContext;
} NXPL_InternalPlatformHandle;

#endif /* __PRIVATE_NEXUS_H__ */
