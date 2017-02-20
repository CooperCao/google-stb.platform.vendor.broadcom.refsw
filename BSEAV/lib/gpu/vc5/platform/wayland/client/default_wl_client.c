/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "default_wl_client.h"

#include "display_wl_client.h"
#include "private_nexus.h"
#include "memory_drm.h"
#include "memory_nexus.h"
#include "sched_nexus.h"

#include "nexus_platform_common.h"

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

static void RegisterDisplayPlatform(WaylandClientPlatform *platform)
{
   /* use standard Nexus memory and sched interfaces */
   platform->drm = false;
   platform->memoryInterface = CreateDRMMemoryInterface();
   if (platform->memoryInterface != NULL)
      platform->drm = true;
   else
      platform->memoryInterface = CreateMemoryInterface();

   platform->schedInterface  = CreateSchedInterface(platform->memoryInterface);

   /* Wayland overrides for display interface - client side*/
   platform->displayInterface = CreateDisplayInterfaceWaylandClient(platform);

   BEGL_RegisterMemoryInterface(platform->memoryInterface);
   BEGL_RegisterSchedInterface(platform->schedInterface);
   BEGL_RegisterDisplayInterface(platform->displayInterface);
}

static void UnregisterDisplayPlatform(WaylandClientPlatform *platform)
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

   if (platform->drm)
      DestroyDRMMemoryInterface(platform->memoryInterface);
   else
      DestroyMemoryInterface(platform->memoryInterface);
   platform->memoryInterface = NULL;
}

static void DestroyWaylandClientPlatform(
      WaylandClientPlatform *platform)
{
   /* This function may be called on partially initialised client */

   if (platform->joined)
   {
      UnregisterDisplayPlatform(platform);
      NxClient_Uninit();
      platform->joined = false;
   }
   DestroyWaylandClient(&platform->client);
}

static bool InitWaylandClientPlatform(
      WaylandClientPlatform *platform,
      struct wl_display *display)
{
   assert(!platform->joined);

   if (!InitWaylandClient(&platform->client, display))
      return false;

   if (NxClient_Join(NULL) != NEXUS_SUCCESS)
      goto error;
   RegisterDisplayPlatform(platform);
   platform->joined = true;

   return true;

error:
   DestroyWaylandClientPlatform(platform);
   return false;
}

static WaylandClientPlatform s_platform;

/* called by eglGetDisplay() */
void *GetDefaultDisplay(void *context)
{
   return s_platform.client.display;
}

/* called by eglGetDisplay() */
bool SetDefaultDisplay(void *context, void *display)
{
   return InitWaylandClientPlatform(&s_platform, display);
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
   DestroyWaylandClientPlatform(&s_platform);
}
