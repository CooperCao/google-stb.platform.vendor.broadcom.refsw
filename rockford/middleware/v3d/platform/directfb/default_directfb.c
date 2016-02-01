/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default DirectFB platform API for EGL driver
Module   :  DirectFB platform on NEXUS

FILE DESCRIPTION
DESC
=============================================================================*/

#include "default_directfb.h"

#include <EGL/egl.h>
#include <malloc.h>
#include <string.h>

BEGL_MemoryInterface  *NXPL_CreateMemInterface(BEGL_HWInterface *hwIface);
BEGL_HWInterface      *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks);
BEGL_DisplayInterface *DBPL_CreateDisplayInterface(IDirectFB *dfb,
                                                   BEGL_MemoryInterface *memIface,
                                                   BEGL_DisplayCallbacks *displayCallbacks);

void NXPL_DestroyMemInterface(BEGL_MemoryInterface *iface);
void NXPL_DestroyHWInterface(BEGL_HWInterface *iface);
void DBPL_DestroyDisplayInterface(BEGL_DisplayInterface * disp);

/*****************************************************************************
 * Registration interface
 *****************************************************************************/
/* Register directFB with the egl platform */
void DBPL_RegisterDirectFBDisplayPlatform(DBPL_PlatformHandle *handle, IDirectFB *dfb)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)malloc(sizeof(BEGL_DriverInterfaces));
   memset(data, 0, sizeof(BEGL_DriverInterfaces));

   if (data != NULL)
   {
      BEGL_GetDefaultDriverInterfaces(data);

      data->hwInterface = NXPL_CreateHWInterface(&data->hardwareCallbacks);
      data->memInterface = NXPL_CreateMemInterface(data->hwInterface);
      data->displayInterface = DBPL_CreateDisplayInterface(dfb,
                                                           data->memInterface,
                                                           &data->displayCallbacks);

      *handle = (DBPL_PlatformHandle)data;

      BEGL_RegisterDriverInterfaces(data);
   }
}

/* Unregister directFB */
void DBPL_UnregisterDirectFBDisplayPlatform(DBPL_PlatformHandle handle)
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

      NXPL_DestroyMemInterface(data->memInterface);
      NXPL_DestroyHWInterface(data->hwInterface);
      DBPL_DestroyDisplayInterface(data->displayInterface);

      memset(data, 0, sizeof(BEGL_DriverInterfaces));
      free(data);
   }
}
