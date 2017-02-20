/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "wl_client.h"

#include <wayland-client.h>
#include <memory.h>

static void RegistryGlobalAdd(void *data, struct wl_registry *registry,
      uint32_t name, const char *interface, uint32_t version)
{
   if (strcmp(interface, "wl_nexus") == 0)
   {
      WaylandClient *wlc = (WaylandClient *)data;

      wlc->nexus = (struct wl_nexus *) wl_registry_bind(registry, name,
            &wl_nexus_interface, version);

      wl_proxy_set_queue((struct wl_proxy *) wlc->nexus,
            wlc->events);
   }
}

static void RegistryGlobalRemove(void *data, struct wl_registry *registry,
      uint32_t name)
{
   /* nothing to do here */
}

void DestroyWaylandClient(WaylandClient *wlc)
{
   /* This function may be called on partially initialised client */

   if (wlc->nexus) /* uses wlc->events */
   {
      wl_nexus_destroy(wlc->nexus);
      wlc->nexus = NULL;
   }

   if (wlc->registry) /* uses wlc->events */
   {
      wl_registry_destroy(wlc->registry);
      wlc->registry = NULL;
   }

   if (wlc->events)
   {
      wl_event_queue_destroy(wlc->events);
      wlc->events = NULL;
   }

   /* display isn't ours to disconnect */
   wlc->display = NULL;
}

bool InitWaylandClient(WaylandClient *wlc,
      struct wl_display *display)
{
   if (!display)
      return false;
   wlc->display = display;

   wlc->registry = wl_display_get_registry(wlc->display);
   wlc->events = wl_display_create_queue(wlc->display);
   if (wlc->events == NULL)
      goto error;

   static const struct wl_registry_listener registryGlobalListener = {
         RegistryGlobalAdd, RegistryGlobalRemove };

   wl_registry_add_listener(wlc->registry, &registryGlobalListener, wlc);
   wl_proxy_set_queue((struct wl_proxy *) wlc->registry,
         wlc->events);
   wl_display_roundtrip_queue(wlc->display, wlc->events); // Blocking
   if (wlc->nexus == NULL)
      goto error;

   wl_nexus_set_user_data(wlc->nexus, wlc);

   return true;

error:
   DestroyWaylandClient(wlc);
   return false;
}
