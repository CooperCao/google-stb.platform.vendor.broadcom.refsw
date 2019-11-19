/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifndef NXPL_EXPORT
#define NXPL_EXPORT __attribute__((visibility("default")))
#endif

#ifdef SINGLE_PROCESS
#include "nexus_display.h"
#else
#include "nexus_platform_client.h"
#include "nexus_surface_client.h"
#endif
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_memory.h"

#ifdef SINGLE_PROCESS
#define NEXUS_DISPLAYHANDLE NEXUS_DisplayHandle
#define NEXUS_SURFACECLIENTHANDLE void *
#else
#define NEXUS_DISPLAYHANDLE void *
#define NEXUS_SURFACECLIENTHANDLE NEXUS_SurfaceClientHandle
#endif

#include "nexus_core_utils.h"

#include <EGL/begl_platform.h>

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

typedef enum NXPL_DisplayType
{
   NXPL_2D = 0,
   NXPL_3D_LEFT_RIGHT,
   NXPL_3D_OVER_UNDER
} NXPL_DisplayType;

typedef struct
{
   uint32_t             width;
   uint32_t             height;
   uint32_t             x;
   uint32_t             y;
   bool                 stretch;
   uint32_t             clientID;
   uint32_t             zOrder;
   NEXUS_BlendEquation  colorBlend;
   NEXUS_BlendEquation  alphaBlend;
   NXPL_DisplayType     type;
   uint32_t             videoWidth;
   uint32_t             videoHeight;
   uint32_t             videoX;
   uint32_t             videoY;
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

/* Generate a default NXPL_NativeWindowInfoEXT */
NXPL_EXPORT void NXPL_GetDefaultNativeWindowInfoEXT(NXPL_NativeWindowInfoEXT *info);

/* Create a 'native window' of the given size. This is really just a small structure that holds the size
 * of the window that EGL will write into. */
NXPL_EXPORT void *NXPL_CreateNativeWindow(const NXPL_NativeWindowInfo *info) __attribute__((deprecated("Use NXPL_GetDefaultNativeWindowEXT()/NXPL_CreateNativeWindowEXT() instead")));
NXPL_EXPORT void *NXPL_CreateNativeWindowEXT(const NXPL_NativeWindowInfoEXT *info);

/* Update the 'native window' with new settings */
NXPL_EXPORT void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info) __attribute__((deprecated("Use NXPL_UpdateNativeWindowEXT() instead")));
NXPL_EXPORT void NXPL_UpdateNativeWindowEXT(void *native, const NXPL_NativeWindowInfoEXT *info);
NXPL_EXPORT void NXPL_UpdateNativeWindowDisplayType(void *native, NXPL_DisplayType type);

/* Destroy a 'native window' */
NXPL_EXPORT void NXPL_DestroyNativeWindow(void *nativeWin);

NXPL_EXPORT void NXPL_GetDefaultPixmapInfoEXT(BEGL_PixmapInfoEXT *info)
   __attribute__((deprecated("Use Nexus surface with 3D compatibility flag instead")));

NXPL_EXPORT bool NXPL_CreateCompatiblePixmapEXT(NXPL_PlatformHandle handle, void **pixmapHandle,
                                                NEXUS_SurfaceHandle *surface, BEGL_PixmapInfoEXT *info)
   __attribute__((deprecated("Use Nexus surface with 3D compatibility flag instead")));

NXPL_EXPORT void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle handle, void *pixmapHandle)
   __attribute__((deprecated("Use Nexus surface with 3D compatibility flag instead")));

/* only valid for surface compositor */
/* gets the associated video pip window from the graphics window. */
NXPL_EXPORT NEXUS_SURFACECLIENTHANDLE NXPL_CreateVideoWindowClient(void *native, unsigned windowId);

/* releases the pip back */
NXPL_EXPORT void NXPL_ReleaseVideoWindowClient(NEXUS_SURFACECLIENTHANDLE handle) __attribute__((deprecated("Use NXPL_ReleaseVideoWindowClientEXT() instead")));
NXPL_EXPORT void NXPL_ReleaseVideoWindowClientEXT(void *native);

/* returns the top surface Id for that nxclient or */
/* 0 if the native window doesn't exit */
NXPL_EXPORT uint32_t NXPL_GetClientID(void *native);

#ifdef __cplusplus
}
#endif
