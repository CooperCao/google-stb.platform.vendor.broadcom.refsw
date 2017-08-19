/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <directfb_version.h>
#include <directfb.h>

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *DBPL_PlatformHandle;

/* Register directFB with the egl platform */
extern void DBPL_RegisterDirectFBDisplayPlatform(DBPL_PlatformHandle *handle, IDirectFB *dfb);

/* Unregister directFB */
extern void DBPL_UnregisterDirectFBDisplayPlatform(DBPL_PlatformHandle handle);

extern bool DBPL_BufferGetRequirements(DBPL_PlatformHandle handle, BEGL_PixmapInfoEXT *bufferRequirements, BEGL_BufferSettings * bufferConstrainedRequirements);

extern bool DBPL_CreateCompatiblePixmap(DBPL_PlatformHandle handle, void **pixmapHandle, IDirectFBSurface **surface, BEGL_PixmapInfo *info);

extern void DBPL_DestroyCompatiblePixmap(DBPL_PlatformHandle handle, void *pixmapHandle);

#ifdef __cplusplus
}
#endif
