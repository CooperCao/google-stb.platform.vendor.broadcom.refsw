/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>

#include "nexus_surface.h"
#include "nexus_striped_surface.h"
#if NEXUS_HAS_GRAPHICS2D
#include "nexus_graphics2d.h"
#endif
#include "berr.h"
#include "bkni.h"

/* Things that may be costly to make every frame */
typedef struct
{
#if NEXUS_HAS_GRAPHICS2D
   NEXUS_Graphics2DHandle  gfx2d;
#endif
   BKNI_EventHandle        gfxEvent;
   BKNI_EventHandle        packetSpaceAvailableEvent;
   bool                    mipmapper;  /* Is this a mipmap-M2MC? */
   bool                    secure;     /* Is the gfx2d secure?   */
} MemConvertCache;

BEGL_Error MemoryConvertSurface(const BEGL_SurfaceConversionInfo *info,
                                bool validateOnly, MemConvertCache *cache);

void MemoryConvertClearCache(MemConvertCache *cache);
