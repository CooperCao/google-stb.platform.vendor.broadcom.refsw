/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __BIND_WL_DISPLAY_H__
#define __BIND_WL_DISPLAY_H__

#include "display_nexus.h"
#include "private_nexus.h"

#include "libs/khrn/egl/egl_types.h"
#include <wayland-util.h>
#include <stdint.h>
#include <stdbool.h>

typedef struct WLServer_DisplayContext
{
   /* inherited from Nexus platform */
   NXPL_DisplayContext parent; /* for functions inherited from Nexus platform */
   BEGL_Error (*GetNativeSurface)(void *context, uint32_t eglTarget,
         void *eglClientBuffer, void **opaqueNativeSurface);
   BEGL_Error (*SurfaceChangeRefCount)(void *context, void *opaqueNativeSurface,
         BEGL_RefCountMode inOrDec);
   BEGL_Error (*SurfaceGetInfo)(void *context, void *opaqueNativeSurface,
         BEGL_SurfaceInfo *info);

   struct wl_display *display;
   struct wl_global *global;
   struct wl_resource *nexus;
} WLServer_DisplayContext;

typedef struct ClientBuffer
{
   uint32_t magic;
   NXPL_Surface surface; /* must NOT be the 1st member - see isNXPL_Surface() */
   int refcount;
} ClientBuffer;


bool BindWlDisplay(WLServer_DisplayContext *wl_context,
      struct wl_display *wl_display);

bool UnbindWlDisplay(WLServer_DisplayContext *wl_context,
      struct wl_display *wl_display);

bool IsClientBuffer(const void *buffer);

ClientBuffer *GetClientBuffer(struct wl_resource *buffer);

BEGL_Error ChangeWlBufferRefCount(ClientBuffer *cb, BEGL_RefCountMode incOrDec);

#endif /* __BIND_WL_DISPLAY_H__ */
