/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "default_wl_server.h"

#include "display_wl_server.h"
#include "private_nexus.h"
#include "memory_drm.h"
#include "memory_nexus.h"
#include "sched_nexus.h"

#include <malloc.h>
#include <memory.h>
#include <string.h>

static bool ExplicitSync(void *context)
{
   return true;
}

/*****************************************************************************
 * Registration interface
 *****************************************************************************/
/* Register a display for exclusive use. The client application should not use the display until
 * calling NXPL_UnregisterNexusDisplayPlatform.
 * It will register its own memory, h/w and display APIs suitable for basic exclusive mode rendering on
 * a Nexus display. */
void NXPL_RegisterNexusDisplayPlatform(NXPL_PlatformHandle *handle, NEXUS_DISPLAYHANDLE display)
{
   NXPL_InternalPlatformHandle *platform = (NXPL_InternalPlatformHandle*)malloc(sizeof(NXPL_InternalPlatformHandle));
   memset(platform, 0, sizeof(NXPL_InternalPlatformHandle));

   if (platform != NULL)
   {
      platform->memoryInterface = CreateDRMMemoryInterface();
      if (platform->memoryInterface != NULL)
         platform->drm = true;
      else
         platform->memoryInterface = CreateMemoryInterface();

      platform->schedInterface  = CreateSchedInterface(platform->memoryInterface);

      /* block GL calls until TFU conversion has finished */
      platform->schedInterface->ExplicitSync = ExplicitSync;

      /* Wayland overrides for display interface - server side*/
      platform->displayInterface = CreateDisplayInterfaceWaylandServer(display, platform->schedInterface);

      *handle = (NXPL_PlatformHandle)platform;

      BEGL_RegisterMemoryInterface(platform->memoryInterface);
      BEGL_RegisterSchedInterface(platform->schedInterface);
      BEGL_RegisterDisplayInterface(platform->displayInterface);
   }
}

/* Unregister a display for exclusive use. The client application can then use the Nexus display again. */
void NXPL_UnregisterNexusDisplayPlatform(NXPL_PlatformHandle handle)
{
   NXPL_InternalPlatformHandle *data = (NXPL_InternalPlatformHandle*)handle;

   if (data != NULL)
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

      /* Wayland overrides for display interface - server side*/
      DestroyDisplayInterfaceWaylandServer(data->displayInterface);

      if (data->drm)
         DestroyDRMMemoryInterface(data->memoryInterface);
      else
         DestroyMemoryInterface(data->memoryInterface);

      DestroySchedInterface(data->schedInterface);

      memset(data, 0, sizeof(NXPL_InternalPlatformHandle));
      free(data);
   }
}
