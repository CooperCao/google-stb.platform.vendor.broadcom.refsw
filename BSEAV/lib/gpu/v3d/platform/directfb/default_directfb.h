/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <directfb_version.h>
#include <directfb.h>

#include <EGL/egl.h>
#include <EGL/begl_platform.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *DBPL_PlatformHandle;

/* Register directFB with the egl platform */
extern void DBPL_RegisterDirectFBDisplayPlatform(DBPL_PlatformHandle *handle, IDirectFB *dfb);

/* Unregister directFB */
extern void DBPL_UnregisterDirectFBDisplayPlatform(DBPL_PlatformHandle handle);

extern void DBPL_GetDefaultPixmapInfoEXT(BEGL_PixmapInfoEXT *info)
   __attribute__((deprecated("Use DFB surface with DSCAPS_GL compatibility flag instead")));

extern bool DBPL_CreateCompatiblePixmapEXT(DBPL_PlatformHandle handle, void **pixmapHandle, IDirectFBSurface **surface, BEGL_PixmapInfoEXT *info)
   __attribute__((deprecated("Use DFB surface with DSCAPS_GL compatibility flag instead")));

extern void DBPL_DestroyCompatiblePixmap(DBPL_PlatformHandle handle, void *pixmapHandle)
   __attribute__((deprecated("Use DFB surface with DSCAPS_GL compatibility flag instead")));

#ifdef __cplusplus
}
#endif
