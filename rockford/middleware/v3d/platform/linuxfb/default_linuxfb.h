/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Linux framebuffer platform

FILE DESCRIPTION
DESC
=============================================================================*/

#ifndef _LFPL_DEFAULT_NEXUS_H__
#define _LFPL_DEFAULT_NEXUS_H__

#define NEXUS_SURFACEHANDLE void *
#define NEXUS_DISPLAYHANDLE void *
#define NEXUS_HEAPHANDLE    void *

#include <EGL/egl.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void *LFPL_PlatformHandle;

/* For linuxFb this structure is used only for the sake of
 * compatibility with existing test apps
 */
typedef struct
{
   uint32_t    width;
   uint32_t    height;
   uint32_t    x;
   uint32_t    y;
   bool        stretch;
   uint32_t    clientID;
   uint32_t    zOrder;
} LFPL_NativeWindowInfo;


/* Register a display for exclusive use. The client application should not use the display until
 * calling LFPL_UnregisterNexusDisplayPlatform.
 * It will register its own memory, h/w and display APIs suitable for basic exclusive mode rendering on
 * a Nexus display.
 * Fills in the LFPL platform handle, which must be passed back into the unregister function.
 */
extern void LFPL_RegisterNexusDisplayPlatform(LFPL_PlatformHandle *handle, NEXUS_DISPLAYHANDLE display);

/* Unregister a display for exclusive use. The client application can the use the Nexus display again. */
extern void LFPL_UnregisterNexusDisplayPlatform(LFPL_PlatformHandle handle);

extern bool LFPL_BufferGetRequirements(LFPL_PlatformHandle handle, BEGL_PixmapInfo *bufferRequirements, BEGL_BufferSettings * bufferConstrainedRequirements);

extern bool LFPL_CreateCompatiblePixmap(LFPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SURFACEHANDLE *surface, BEGL_PixmapInfo *info);

extern void LFPL_DestroyCompatiblePixmap(LFPL_PlatformHandle handle, void *pixmapHandle);


extern void *LFPL_CreateNativeWindow(const LFPL_NativeWindowInfo *info);

/* Destroy a 'native window' */
extern void LFPL_DestroyNativeWindow(void *nativeWin);


#ifdef __cplusplus
}
#endif

#endif /* _LFPL_DEFAULT_NEXUS_H__ */
