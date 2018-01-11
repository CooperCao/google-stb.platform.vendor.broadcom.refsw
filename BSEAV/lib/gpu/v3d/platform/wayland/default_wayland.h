/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifndef WLPL_EXPORT
#define WLPL_EXPORT __attribute__((visibility("default")))
#endif

#ifdef SINGLE_PROCESS
#error Wayland requires multi-process support
#else
#include "nexus_platform_client.h"
#include "nexus_surface_client.h"
#endif
#include "nexus_surface.h"
#include "nexus_graphics2d.h"
#include "nexus_memory.h"

#define NEXUS_DISPLAYHANDLE void *
#define NEXUS_SURFACECLIENTHANDLE NEXUS_SurfaceClientHandle

#include "nexus_core_utils.h"

#include <EGL/egl.h>
#include <wayland-client.h>

#ifdef __cplusplus
extern "C"
{
#endif

/*typedef void *WLPL_PlatformHandle;*/

typedef enum WLPL_DisplayType
{
   WLPL_2D = 0,
   WLPL_3D_LEFT_RIGHT,
   WLPL_3D_OVER_UNDER
} WLPL_DisplayType;

typedef struct
{
   uint32_t width;
   uint32_t height;
   uint32_t x;
   uint32_t y;
   bool stretch;
   uint32_t clientID;
   uint32_t zOrder;
   NEXUS_BlendEquation colorBlend;
   NEXUS_BlendEquation alphaBlend;
   WLPL_DisplayType type;
   uint32_t magic;
} WLPL_NexusWindowInfoEXT;

/* Generate a default WLPL_NexusWindowInfoEXT */
WLPL_EXPORT void WLPL_GetDefaultNexusWindowInfoEXT(
      WLPL_NexusWindowInfoEXT *info);

/* Create a 'native window' of the given size. This is really just a small structure that holds the size
 * of the window that EGL will write into. */
WLPL_EXPORT void *WLPL_CreateNexusWindowEXT(
      const WLPL_NexusWindowInfoEXT *info);

/* Destroy a 'native window' */
WLPL_EXPORT void WLPL_DestroyNexusWindow(void *nativeWin);

#ifdef __cplusplus
}
#endif
