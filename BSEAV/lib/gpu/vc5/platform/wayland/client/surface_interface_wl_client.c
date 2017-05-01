/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "surface_interface_wl_client.h"

#include "wayland_nexus_client.h"
#include "../nexus/display_surface.h"

#include "nexus_platform.h"
#include "nexus_base_mmap.h"

#ifndef NDEBUG
#include <string.h>
#endif

typedef struct WaylandClientSurfaceInterface
{
   const WaylandClient *client;
   struct list buffers;
} WaylandClientSurfaceInterface;

WaylandClientBuffer *FindClientBuffer(WaylandClientSurfaceInterface *wcsi,
      struct wl_buffer *buffer)
{
   struct list *item = wcsi->buffers.next;
   while (item != &wcsi->buffers)
   {
      WaylandClientBuffer *cb = list_entry(item, WaylandClientBuffer, link);
      if (cb->buffer == buffer)
         return cb;
      item = item->next;
   }
   return NULL; /* not found */
}

static void destroy_surface(void *context, void *surface)
{
   (void)context;
   WaylandClientBuffer *cb = (WaylandClientBuffer *)surface;

   if (cb->buffer)
      wl_buffer_destroy(cb->buffer);
   list_del(&cb->link);

#ifndef NDEBUG
   memset(cb, 0, sizeof(*cb));
#endif
   /*the memory block isnt't ours to free */
}

static void buffer_created(void *data, struct wl_nexus *wl_nexus,
      struct wl_buffer *buffer, struct wl_array *physical, uint32_t pitch)
{
   WaylandClientSurfaceInterface *wcsi = (WaylandClientSurfaceInterface *)data;

   /* find a WaylandClientBuffer structure that matches this wl_buffer */
   WaylandClientBuffer *cb = FindClientBuffer(wcsi, buffer);
   assert(cb);

   cb->info.pitchBytes = pitch;
   cb->info.physicalOffset = *(const uint64_t *) physical->data;
   cb->info.cachedAddr = NEXUS_OffsetToCachedAddr(cb->info.physicalOffset);
   cb->info.byteSize = pitch * cb->info.height;
}

static void buffer_out_of_memory(void *data, struct wl_nexus *wl_nexus,
      struct wl_buffer *buffer)
{
   WaylandClientSurfaceInterface *wcsi = (WaylandClientSurfaceInterface *)data;

   /* find a WaylandClientBuffer structure that matches this wl_buffer */
   WaylandClientBuffer *cb = FindClientBuffer(wcsi, buffer);
   assert(cb);

   cb->info.pitchBytes = 0;
   cb->info.physicalOffset = 0;
   cb->info.cachedAddr = NULL;
   cb->info.byteSize = 0;
}

static bool create_surface(void *context, void *surface, uint32_t width,
      uint32_t height, BEGL_BufferFormat format, bool secure)
{
   WaylandClientSurfaceInterface *wcsi = (WaylandClientSurfaceInterface *)context;
   WaylandClientBuffer *cb = (WaylandClientBuffer *)surface;
   bool result = false;

   cb->buffer = wl_nexus_buffer_create(wcsi->client->nexus,
         width, height, format, secure);
   if (!cb->buffer)
      goto end;

   cb->info.width = width;
   cb->info.height = height;
   cb->info.format = (BEGL_BufferFormat)format;
   /* the rest will be set by an event */

   /* Remember each buffer - this is needed to handle Wayland callbacks.
    *
    * The only way to access platform-specific data for wl_buffer is to store
    * that data externally and find the right record based on wl_buffer pointer.
    *
    * Wayland allocates wl_buffers so it's not possible to embed the wl_buffer
    * structure in a larger structure and access additional data that way.
    *
    * It's also not possible to reliably attach any data to the wl_buffer
    * on the client side as the "data" pointer gets overwritten as soon
    * as a listener is added.
    */
   list_push_back(&wcsi->buffers, &cb->link);

   /* wait for the event - the buffer list will be used to find this cb */
   cb->info.physicalOffset = 0;
   wl_proxy_set_queue((struct wl_proxy *) cb->buffer, wcsi->client->events);
   wl_display_roundtrip_queue(wcsi->client->display, wcsi->client->events); // Blocking
   if (!cb->info.physicalOffset)
      goto no_physical;

   result = true;
   goto end;

no_physical:
   destroy_surface(context, surface);
end:
   return result;
}

static void destroy(void *context)
{
   WaylandClientSurfaceInterface *wcsi = (WaylandClientSurfaceInterface *)context;
   if (wcsi)
      assert(list_empty(&wcsi->buffers));
   free(wcsi);
}

bool SurfaceInterface_InitWlClient(SurfaceInterface *si,
      const WaylandClient *client)
{
   WaylandClientSurfaceInterface *wcsi = calloc(1, sizeof(*wcsi));
   if (!wcsi)
      return false;

   wcsi->client = client;
   list_init(&wcsi->buffers);

   si->base.context = wcsi;
   si->base.destroy = destroy;
   si->sizeof_surface = sizeof(struct WaylandClientBuffer);
   si->create = create_surface;
   si->destroy = destroy_surface;

   static const struct wl_nexus_listener nexusListener =
   {
         .buffer_created = buffer_created,
         .buffer_out_of_memory = buffer_out_of_memory,
   };
   wl_nexus_add_listener(client->nexus, &nexusListener, wcsi);
   return true;
}
