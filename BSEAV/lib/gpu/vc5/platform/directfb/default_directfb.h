/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef _DBPL_DEFAULT_DIRECTFB_H__
#define _DBPL_DEFAULT_DIRECTFB_H__

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

extern bool DBPL_CreateCompatiblePixmap(DBPL_PlatformHandle handle, void **pixmapHandle, IDirectFBSurface **surface, BEGL_PixmapInfo *info);

extern void DBPL_DestroyCompatiblePixmap(DBPL_PlatformHandle handle, void *pixmapHandle);


#ifdef __cplusplus
}
#endif

#endif /* _DBPL_DEFAULT_DIRECTFB_H__ */
