/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef _NXPL_DEFAULT_NEXUS_H__
#define _NXPL_DEFAULT_NEXUS_H__

#ifndef NXPL_EXPORT
#if defined(WIN32)
#define NXPL_EXPORT __declspec(dllexport)
#else
#define NXPL_EXPORT __attribute__((visibility("default")))
#endif
#endif

#ifdef NULL_DISPLAY

#define NEXUS_SURFACEHANDLE void *
#define NEXUS_DISPLAYHANDLE void *
#define NEXUS_HEAPHANDLE    void *
#define NEXUS_BLENDEQUATION void *

#else

#ifdef SINGLE_PROCESS

#include "nexus_display.h"
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
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
#define NEXUS_BLENDEQUATION NEXUS_BlendEquation

#endif /* NULL_DISPLAY */

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *NXPL_PlatformHandle;

/* WARNING, deprecated */
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
/* END, deprecated */

typedef struct
{
   uint32_t             width;
   uint32_t             height;
   uint32_t             x;
   uint32_t             y;
   bool                 stretch;
   uint32_t             clientID;
   uint32_t             zOrder;
   NEXUS_BLENDEQUATION  colorBlend;
   NEXUS_BLENDEQUATION  alphaBlend;
   uint32_t             magic;
} NXPL_NativeWindowInfoEXT;

/* Register a display for exclusive use. The client application should not use the display until
 * calling NXPL_UnregisterNexusDisplayPlatform.
 * It will register its own memory, h/w and display APIs suitable for basic exclusive mode rendering on
 * a Nexus display.
 * Fills in the NXPL platform handle, which must be passed back into the unregister function.
 */
NXPL_EXPORT void NXPL_RegisterNexusDisplayPlatform(NXPL_PlatformHandle *handle, NEXUS_DISPLAYHANDLE display);

/* Unregister a display for exclusive use. The client application can the use the Nexus display again. */
NXPL_EXPORT void NXPL_UnregisterNexusDisplayPlatform(NXPL_PlatformHandle handle);

NXPL_EXPORT bool NXPL_BufferGetRequirements(NXPL_PlatformHandle handle, BEGL_PixmapInfoEXT *bufferRequirements, BEGL_BufferSettings * bufferConstrainedRequirements);

/* Generate a default NXPL_NativeWindowInfoEXT */
NXPL_EXPORT void NXPL_GetDefaultNativeWindowInfoEXT(NXPL_NativeWindowInfoEXT *info);

/* Create a 'native window' of the given size. This is really just a small structure that holds the size
 * of the window that EGL will write into. */
NXPL_EXPORT void *NXPL_CreateNativeWindow(const NXPL_NativeWindowInfo *info) __attribute__((deprecated("Use NXPL_GetDefaultNativeWindowEXT()/NXPL_CreateNativeWindowEXT() instead")));
NXPL_EXPORT void *NXPL_CreateNativeWindowEXT(const NXPL_NativeWindowInfoEXT *info);

/* Update the 'native window' with new settings */
NXPL_EXPORT void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info) __attribute__((deprecated("Use NXPL_UpdateNativeWindowEXT() instead")));
NXPL_EXPORT void NXPL_UpdateNativeWindowEXT(void *native, const NXPL_NativeWindowInfoEXT *info);

/* Destroy a 'native window' */
NXPL_EXPORT void NXPL_DestroyNativeWindow(void *nativeWin);

NXPL_EXPORT void NXPL_GetDefaultPixmapInfoEXT(BEGL_PixmapInfoEXT *info);

NXPL_EXPORT bool NXPL_CreateCompatiblePixmap(NXPL_PlatformHandle handle, void **pixmapHandle,
                                             NEXUS_SURFACEHANDLE *surface, BEGL_PixmapInfo *info) __attribute__((deprecated("Use NXPL_GetDefaultPixmapInfoEXT()/NXPL_CreateCompatiblePixmapEXT() instead")));

NXPL_EXPORT bool NXPL_CreateCompatiblePixmapEXT(NXPL_PlatformHandle handle, void **pixmapHandle,
                                                NEXUS_SURFACEHANDLE *surface, BEGL_PixmapInfoEXT *info);

NXPL_EXPORT void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle handle, void *pixmapHandle);

#ifndef SINGLE_PROCESS
/* only valid for surface compositor */
/* gets the associated video pip window from the graphics window. */
NXPL_EXPORT NEXUS_SurfaceClientHandle NXPL_CreateVideoWindowClient(void *native, unsigned windowId);

/* releases the pip back */
NXPL_EXPORT void NXPL_ReleaseVideoWindowClient(NEXUS_SurfaceClientHandle handle);

/* returns the top surface Id for that nxclient or */
/* 0 if the native window doesn't exit */
NXPL_EXPORT uint32_t NXPL_GetClientID(void *native);

#endif

#ifdef __cplusplus
}
#endif

#endif /* _NXPL_DEFAULT_NEXUS_H__ */
