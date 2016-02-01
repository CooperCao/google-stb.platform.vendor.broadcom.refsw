/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION
DESC
=============================================================================*/

#include "default_linuxfb.h"
#include "display_linuxfb.h"

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>

/*****************************************************************************
 * Registration interface
 *****************************************************************************/
/* Register a display for exclusive use. The client application should not use the display until
 * calling LFPL_UnregisterNexusDisplayPlatform.
 * It will register its own memory, h/w and display APIs suitable for basic exclusive mode rendering on
 * a Nexus display. */
void LFPL_RegisterNexusDisplayPlatform(LFPL_PlatformHandle *handle, NEXUS_DISPLAYHANDLE display)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)malloc(sizeof(BEGL_DriverInterfaces));
   memset(data, 0, sizeof(BEGL_DriverInterfaces));

   if (data != NULL)
   {
      BEGL_GetDefaultDriverInterfaces(data);

      data->hwInterface      = NXPL_CreateHWInterface(&data->hardwareCallbacks);
      data->memInterface     = NXPL_CreateMemInterface(data->hwInterface);
      data->displayInterface = LFPL_CreateDisplayInterface(data->memInterface, data->hwInterface,
                                                           &(data->displayCallbacks));
      *handle = (LFPL_PlatformHandle)data;

      BEGL_RegisterDriverInterfaces(data);
   }
}

/* Unregister a display for exclusive use. The client application can then use the Nexus display again. */
void LFPL_UnregisterNexusDisplayPlatform(LFPL_PlatformHandle handle)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)handle;

   if (data != NULL)
   {
      /* Clear out all of our interface pointers and register the empty ones.
       * Do this before destroying the interfaces in case the driver needs to use them
       * to shutdown correctly */
      BEGL_DriverInterfaces nulliface;
      memset(&nulliface, 0, sizeof(BEGL_DriverInterfaces));
      BEGL_GetDefaultDriverInterfaces(&nulliface);
      BEGL_RegisterDriverInterfaces(&nulliface);

      LFPL_DestroyDisplayInterface(data->displayInterface);
      NXPL_DestroyMemInterface(data->memInterface);
      NXPL_DestroyHWInterface(data->hwInterface);

      memset(data, 0, sizeof(BEGL_DriverInterfaces));
      free(data);
   }
}
