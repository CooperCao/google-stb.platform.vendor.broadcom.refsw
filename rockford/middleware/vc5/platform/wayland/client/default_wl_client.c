/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "default_wl_client.h"

#include "display_wl_client.h"
#include "private_nexus.h"
#include "memory_nexus.h"
#include "sched_nexus.h"

#include "nexus_platform_common.h"

#include <wayland-client.h>

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>

/*
 * Wayland platform boot is a 2 stage process. The 1st call to the
 * eglGetDisplay(wayland_display) connects client to the server and then
 * creates and registers the BEGL_DisplayInterface. But in order for that
 * to happen there must be a BEGL_DisplayInterface already registered.
 *
 * To sort out the egg and chicken problem we use 2 different interfaces.
 * When platform library is loaded the v3d_wlclient_load() is called which
 * registers a minimal bootstrap interface. The only purpose of the bootstrap
 * interface is to respond to eglGetDisplay() and create the actual Wayland
 * client platform and the real BEGL_DisplayInterface.
 */

static void RegisterDisplayPlatform(WaylandClient *wlc)
{
   WaylandClientDisplayPlatform *platform = &wlc->platform;

   /* use standard Nexus memory and sched interfaces */
   platform->memoryInterface = CreateMemoryInterface();
   platform->schedInterface  = CreateSchedInterface(platform->memoryInterface);

   /* Wayland overrides for display interface - client side*/
   platform->displayInterface = CreateDisplayInterfaceWaylandClient(wlc);

   BEGL_RegisterMemoryInterface(platform->memoryInterface);
   BEGL_RegisterSchedInterface(platform->schedInterface);
   BEGL_RegisterDisplayInterface(platform->displayInterface);
}

static void UnregisterDisplayPlatform(WaylandClientDisplayPlatform *platform)
{
   /* Tell the 3D driver that the platform layer is about to go away so it can perform
    * final cleanup. */
   BEGL_PlatformAboutToShutdown();

   /* Clear out all of our interface pointers and register the empty ones.
    * Do this before destroying the interfaces in case the driver needs to use them
    * to shutdown correctly */
   BEGL_RegisterDisplayInterface(NULL);
   BEGL_RegisterMemoryInterface(NULL);
   BEGL_RegisterSchedInterface(NULL);

   DestroyDisplayInterfaceWaylandClient(platform->displayInterface);
   platform->displayInterface = NULL;

   DestroySchedInterface(platform->schedInterface);
   platform->schedInterface = NULL;

   DestroyMemoryInterface(platform->memoryInterface);
   platform->memoryInterface = NULL;
}

static void Event_ClientCertificate(void *context, struct wl_nexus *wl_nexus,
      const char *data, uint32_t bytes)
{
   WaylandClient *wlc = (WaylandClient *)context;

   NEXUS_ClientAuthenticationSettings joinSettings;
   if (bytes <= sizeof(joinSettings.certificate.data))
   {
      NEXUS_Platform_GetDefaultClientAuthenticationSettings(&joinSettings);
      memcpy(joinSettings.certificate.data, data, bytes);
      joinSettings.certificate.length = bytes;
      wlc->joined = (NEXUS_Platform_AuthenticatedJoin(&joinSettings) == NEXUS_SUCCESS);
   }
}

static void RegistryGolbalAdd(void *data, struct wl_registry *registry,
      uint32_t name, const char *interface, uint32_t version)
{
   if (strcmp(interface, "wl_nexus") == 0)
   {
      WaylandClient *wlc = (WaylandClient *)data;

      wlc->nexus = (struct wl_nexus *) wl_registry_bind(registry, name,
            &wl_nexus_interface, version);

      static const struct wl_nexus_listener nexusListener =
      {
            .client_certificate = Event_ClientCertificate
      };

      wl_nexus_add_listener(wlc->nexus, &nexusListener, wlc);
      wl_proxy_set_queue((struct wl_proxy *) wlc->nexus,
            wlc->events);
   }
}

static void RegistryGlobalRemove(void *data, struct wl_registry *registry,
      uint32_t name)
{
   /* nothing to do here */
}

static void DestroyWaylandClient(WaylandClient *wlc)
{
   /* This function may be called on partially initialised client */

   UnregisterDisplayPlatform(&wlc->platform);

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

   if (wlc->display)
   {
      wl_display_disconnect(wlc->display);
      wlc->display = NULL;
   }

   NEXUS_Platform_Uninit();
}

static bool InitWaylandClient(WaylandClient *wlc,
      struct wl_display *display)
{
   assert(!wlc->joined);

   if (!display)
      return false;

   wlc->display = display;
   wlc->registry = wl_display_get_registry(wlc->display);
   wlc->events = wl_display_create_queue(wlc->display);
   if (wlc->events == NULL)
      goto error;

   static const struct wl_registry_listener registryGlobalListener = {
         RegistryGolbalAdd, RegistryGlobalRemove };

   wl_registry_add_listener(wlc->registry, &registryGlobalListener, wlc);
   wl_proxy_set_queue((struct wl_proxy *) wlc->registry,
         wlc->events);
   wl_display_roundtrip_queue(wlc->display, wlc->events); // Blocking
   if (wlc->nexus == NULL)
      goto error;

   wl_nexus_set_user_data(wlc->nexus, wlc);
   wl_nexus_client_join(wlc->nexus);
   wl_display_roundtrip_queue(wlc->display, wlc->events); // Blocking
   if (!wlc->joined)
      goto error;

   RegisterDisplayPlatform(wlc);
   return true;

error:
   DestroyWaylandClient(wlc);
   return false;
}

static WaylandClient s_wlclient;

/* called by eglGetDisplay() */
void *GetDefaultDisplay(void *context)
{
   return s_wlclient.display;
}

/* called by eglGetDisplay() */
bool SetDefaultDisplay(void *context, void *display)
{
   return InitWaylandClient(&s_wlclient, display);
}

__attribute__((constructor))
static void v3d_wlclient_load(void)
{
   /* a bootstrap display interface used only to respond to eglGetDisplay()
    * which in turn starts up and registers the full wayland client platform
    */
   static BEGL_DisplayInterface bootstrap =
   {
      .context = NULL, /* unused */
      .GetDefaultDisplay = GetDefaultDisplay,
      .SetDefaultDisplay = SetDefaultDisplay
   };
   BEGL_RegisterDisplayInterface(&bootstrap);
}

__attribute__((destructor))
static void v3d_wlclient_unload(void)
{
   DestroyWaylandClient(&s_wlclient);
}
