/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#ifndef __DISPLAY_THREAD_H__
#define __DISPLAY_THREAD_H__

#include <EGL/begl_displayplatform.h>
#include "sched_abstract.h"

#include "display_nexus.h"

#ifdef NXPL_PLATFORM_EXCLUSIVE
void *CreateWorkerThread(void *nw,
                         BEGL_SchedInterface *schedIface,
                         int numSurfaces,
                         int *bound,
                         NXPL_NativeWindowInfoEXT *windowInfo,
                         BEGL_BufferFormat format,
                         NEXUS_DISPLAYHANDLE display,
                         NXPL_DisplayType displayType);
#else
void *CreateWorkerThread(void *nw,
                         BEGL_SchedInterface *schedIface,
                         int numSurfaces,
                         NXPL_NativeWindowInfoEXT *windowInfo,
                         BEGL_BufferFormat format,
                         uint32_t clientID,
                         NEXUS_SurfaceClientHandle surfaceClient,
                         NXPL_DisplayType displayType);
#endif

BEGL_Error DestroyWorkerThread(void *p);

BEGL_Error DequeueSurface(void *p, void **nativeSurface, int *fence);
BEGL_Error QueueSurface(void *p, void *nativeSurface, int fence, int swapInterval);
BEGL_Error CancelSurface(void *p, void *nativeSurface, int fence);

void *GetNativeWindow(void *p);

#endif /* __DISPLAY_THREAD_H__ */
