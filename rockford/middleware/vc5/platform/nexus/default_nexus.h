/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef __DEFAULT_NEXUS_H__
#define __DEFAULT_NEXUS_H__

#ifndef NXPL_EXPORT
#if defined(WIN32)
#define NXPL_EXPORT __declspec(dllexport)
#else
#define NXPL_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifdef SINGLE_PROCESS

#include "nexus_display.h"
#include "nexus_surface.h"
#include "nexus_memory.h"

#define NEXUS_DISPLAYHANDLE NEXUS_DisplayHandle

#else /* Multi-process client */

#include "nexus_platform_client.h"
#include "nexus_surface_client.h"
#include "nexus_surface.h"
#include "nexus_memory.h"

#define NEXUS_DISPLAYHANDLE void *

#endif /* SINGLE_PROCESS */

#define NEXUS_SURFACEHANDLE NEXUS_SurfaceHandle
#define NEXUS_HEAPHANDLE    NEXUS_HeapHandle

#ifdef __cplusplus
extern "C" {
#endif

struct BEGL_PixmapInfo;

typedef void *NXPL_PlatformHandle;

typedef struct
{
   uint32_t    width;
   uint32_t    height;
   uint32_t    x;
   uint32_t    y;
   bool        stretch;
   uint32_t    clientID;
   uint32_t    zOrder;
} NXPL_NativeWindowInfo;

typedef enum NXPL_DisplayType
{
   NXPL_2D = 0,
   NXPL_3D_LEFT_RIGHT,
   NXPL_3D_OVER_UNDER
} NXPL_DisplayType;

/* Register a display for exclusive use. The client application should not use the display until
 * calling NXPL_UnregisterNexusDisplayPlatform.
 * It will register its own memory, h/w and display APIs suitable for basic exclusive mode rendering on
 * a Nexus display.
 * Fills in the NXPL platform handle, which must be passed back into the unregister function.
 */
NXPL_EXPORT void NXPL_RegisterNexusDisplayPlatform(NXPL_PlatformHandle *handle, NEXUS_DISPLAYHANDLE display);

/* Unregister a display for exclusive use. The client application can the use the Nexus display again. */
NXPL_EXPORT void NXPL_UnregisterNexusDisplayPlatform(NXPL_PlatformHandle handle);

/*
extern bool NXPL_BufferGetRequirements(NXPL_PlatformHandle handle, BEGL_PixmapInfo *bufferRequirements, BEGL_BufferSettings * bufferConstrainedRequirements);
*/

/* Create a 'native window' of the given size. This is really just a small structure that holds the size
 * of the window that EGL will write into. */
NXPL_EXPORT void *NXPL_CreateNativeWindow(const NXPL_NativeWindowInfo *info);

/* Update the 'native window' with new settings */
NXPL_EXPORT void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info);

/* Destroy a 'native window' */
NXPL_EXPORT void NXPL_DestroyNativeWindow(void *nativeWin);

NXPL_EXPORT bool NXPL_CreateCompatiblePixmap(NXPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SURFACEHANDLE *surface, struct BEGL_PixmapInfo *info);
NXPL_EXPORT void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle handle, void *pixmapHandle);

NXPL_EXPORT void NXPL_SetDisplayType(NXPL_PlatformHandle handle, NXPL_DisplayType type);

#ifndef SINGLE_PROCESS
/* only valid for surface compositor */
/* gets the associated video pip window from the graphics window. */
NEXUS_SurfaceClientHandle NXPL_CreateVideoWindowClient(void *native, unsigned windowId);

/* releases the pip back */
NXPL_EXPORT void NXPL_ReleaseVideoWindowClient(NEXUS_SurfaceClientHandle handle);

/* returns the top surface Id for that nxclient or */
/* 0 if the native window doesn't exit */
NXPL_EXPORT uint32_t NXPL_GetClientID (void *native);

#endif

#ifdef __cplusplus
}
#endif

#endif /* __DEFAULT_NEXUS_H__ */
