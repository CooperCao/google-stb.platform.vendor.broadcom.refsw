/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>

#include "nexus_surface.h"
#include "nexus_striped_surface.h"
#include "nexus_graphics2d.h"
#include "berr.h"
#include "bkni.h"

/* Things that may be costly to make every frame */
typedef struct
{
   NEXUS_Graphics2DHandle  gfx2d;
   BKNI_EventHandle        gfxEvent;
   bool                    mipmapper;  /* Is this a mipmap-M2MC? */
   bool                    secure;     /* Is the gfx2d secure?   */
} MemConvertCache;

BEGL_Error MemoryConvertSurface(const BEGL_SurfaceConversionInfo *info,
                                NEXUS_StripedSurfaceHandle srcStripedSurf,
                                NEXUS_SurfaceHandle srcSurf,
                                bool validateOnly, MemConvertCache *cache);

void MemoryConvertClearCache(MemConvertCache *cache);

extern bool DisplayAcquireNexusSurfaceHandles(NEXUS_StripedSurfaceHandle *stripedSurf,
                                              NEXUS_SurfaceHandle *surf, void *nativeSurface);
extern void DisplayReleaseNexusSurfaceHandles(NEXUS_StripedSurfaceHandle stripedSurf,
                                              NEXUS_SurfaceHandle surf);
