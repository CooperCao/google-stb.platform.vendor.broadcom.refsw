/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "surface_interface_wl_client.h"

#include "wayland_nexus_client.h"
#include "../nexus/display_surface.h"

static bool create_surface(void *context, void *surface, uint32_t width,
      uint32_t height, BEGL_BufferFormat format, bool secure)
{
   const WaylandClient *client = (const WaylandClient *)context;
   WaylandClientBuffer *wlcb = (WaylandClientBuffer *)surface;
   bool result = false;

   wlcb->created = CreateSurface(&wlcb->surface, format, width, height,
         secure, "swapchain surface");
   if (!wlcb->created)
      goto end;

   NEXUS_SurfaceStatus status;
   NEXUS_Surface_GetStatus(wlcb->surface.surface, &status); /* pitch */

   wlcb->surface.windowInfo.width = status.width;
   wlcb->surface.windowInfo.height = status.height;

   struct wl_array array;
   wl_array_init(&array);
   uint64_t *offset = (uint64_t *)wl_array_add(&array, sizeof(*offset));
   *offset = wlcb->surface.physicalOffset;

   wlcb->buffer = wl_nexus_buffer_create(client->nexus,
         status.width, status.pitch , status.height, format, &array);

   wl_array_release(&array);

   if (!wlcb->buffer)
      goto no_buffer;

   wl_proxy_set_queue((struct wl_proxy *) wlcb->buffer, client->events);

   result = true;
   goto end;

no_buffer:
   DestroySurface(&wlcb->surface);
end:
   return result;
}

static void destroy_surface(void *context, void *surface)
{
   const WaylandClient *client = (const WaylandClient *)context;
   WaylandClientBuffer *wlcb = (WaylandClientBuffer *)surface;

   wlcb->buffer_release_callback = NULL;

   if (wlcb->buffer)
   {
      wl_buffer_destroy(wlcb->buffer);
      wlcb->buffer = NULL;
   }

   if (wlcb->created)
   {
      DestroySurface(&wlcb->surface);
      memset(&wlcb->surface, 0, sizeof(wlcb->surface));
      wlcb->created = false;
   }
}

void SurfaceInterface_InitWlClient(SurfaceInterface *si,
      const WaylandClient *client)
{
   si->base.context = (void*)client;
   si->base.destroy = NULL; /* unused */
   si->sizeof_surface = sizeof(WaylandClientBuffer);
   si->create = create_surface;
   si->destroy = destroy_surface;
}
