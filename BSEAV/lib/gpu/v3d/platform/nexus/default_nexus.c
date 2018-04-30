/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "default_nexus.h"
#include "display_nexus.h"

#include <EGL/begl_platform.h>

#include <malloc.h>
#include <memory.h>

typedef struct NXPL_Platform
{
   BEGL_DriverInterfaces interfaces;
   EventContext eventContext;
} NXPL_Platform;

/*****************************************************************************
 * Registration interface
 *****************************************************************************/
void NXPL_RegisterNexusDisplayPlatform(NXPL_PlatformHandle *handle, NEXUS_DISPLAYHANDLE display)
{
   NXPL_Platform *platform = (NXPL_Platform *)malloc(sizeof(*platform));
   if (platform != NULL)
   {
      memset(platform, 0, sizeof(*platform));
      BEGL_DriverInterfaces *data = &platform->interfaces;

      BEGL_GetDefaultDriverInterfaces(data);

      data->hwInterface      = NXPL_CreateHWInterface(&data->hardwareCallbacks, &platform->eventContext);
      data->memInterface     = NXPL_CreateMemInterface(data->hwInterface);
      data->displayInterface = NXPL_CreateDisplayInterface(data->memInterface, data->hwInterface, display,
                                                           &platform->eventContext);

      *handle = (NXPL_PlatformHandle)platform;

      BEGL_RegisterDriverInterfaces(data);
   }
}

/* Unregister a display for exclusive use. The client application can then use the Nexus display again. */
void NXPL_UnregisterNexusDisplayPlatform(NXPL_PlatformHandle handle)
{
   NXPL_Platform *platform = (NXPL_Platform *)handle;
   if (platform != NULL)
   {
      BEGL_DriverInterfaces *data = &platform->interfaces;

      /* Clear out all of our interface pointers and register the empty ones.
       * Do this before destroying the interfaces in case the driver needs to use them
       * to shutdown correctly */
      BEGL_DriverInterfaces nulliface;
      memset(&nulliface, 0, sizeof(BEGL_DriverInterfaces));
      BEGL_GetDefaultDriverInterfaces(&nulliface);
      BEGL_RegisterDriverInterfaces(&nulliface);

      NXPL_DestroyDisplayInterface(data->displayInterface);
      NXPL_DestroyMemInterface(data->memInterface);
      NXPL_DestroyHWInterface(data->hwInterface);

      memset(platform, 0, sizeof(*platform));
      free(platform);
   }
}
