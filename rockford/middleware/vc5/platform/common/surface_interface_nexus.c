/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "surface_interface_nexus.h"
#include "../nexus/display_surface.h"

static bool create_surface(void *context, void *surface, uint32_t width,
      uint32_t height, BEGL_BufferFormat format, bool secure)
{
   return CreateSurface((NXPL_Surface *) surface, format, width, height,
         secure, "swapchain surface");
}

static void destroy_surface(void *context, void *surface)
{
   DestroySurface((NXPL_Surface *) surface);
}

void SurfaceInterface_InitNexus(SurfaceInterface *si)
{
   si->base.context = NULL; /* unused */
   si->base.destroy = NULL; /* unused */
   si->sizeof_surface = sizeof(NXPL_Surface);
   si->create = create_surface;
   si->destroy = destroy_surface;
}
