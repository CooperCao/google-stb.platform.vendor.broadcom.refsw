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

static WaylandClientPlatform s_platform;

__attribute__((constructor))
static void v3d_wlclient_load(void)
{
   if (NxClient_Join(NULL) == NEXUS_SUCCESS)
   {
      s_platform.joined = true;
      RegisterDisplayPlatform(&s_platform);
   }
   else
   {
      fprintf(stderr, "Wayland client platform constructor: NxClient_Join failed\n");
   }
}

__attribute__((destructor))
static void v3d_wlclient_unload(void)
{
   if (s_platform.joined)
   {
      UnregisterDisplayPlatform(&s_platform);
      NxClient_Uninit();
      s_platform.joined = false;
   }
}
