/******************************************************************************
*  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
******************************************************************************/

#include "default_RSO_android.h"
#include "../common/perf_event.h"

#include <EGL/begl_platform.h>
#include <malloc.h>
#include <string.h>

BEGL_MemoryInterface  *NXPL_CreateMemInterface(BEGL_HWInterface *hwIface);
BEGL_HWInterface      *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks, EventContext *eventContext);
BEGL_DisplayInterface *RSOANPL_CreateDisplayInterface(BEGL_MemoryInterface *mem, BEGL_HWInterface *hw);

void NXPL_DestroyMemInterface(BEGL_MemoryInterface *iface);
void NXPL_DestroyHWInterface(BEGL_HWInterface *iface);
void RSOANPL_DestroyDisplayInterface(BEGL_DisplayInterface * disp);

/*****************************************************************************
 * Registration interface
 *****************************************************************************/
/* Register Android with the egl platform */
__attribute__((visibility("default")))
void RSOANPL_RegisterAndroidDisplayPlatform(RSOANPL_PlatformHandle *handle, ANativeWindow *awin)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)malloc(sizeof(BEGL_DriverInterfaces));
   UNUSED(awin);
   memset(data, 0, sizeof(BEGL_DriverInterfaces));

   if (data != NULL)
   {
      BEGL_GetDefaultDriverInterfaces(data);

      data->hwInterface = NXPL_CreateHWInterface(&data->hardwareCallbacks, NULL);
      data->hwInterfaceFn = (void *) NXPL_CreateHWInterface;

      data->memInterface = NXPL_CreateMemInterface(data->hwInterface);
      data->memInterfaceFn = (void *) NXPL_CreateMemInterface;

      data->displayInterface = RSOANPL_CreateDisplayInterface(data->memInterface, data->hwInterface);

      *handle = (RSOANPL_PlatformHandle)data;

      BEGL_RegisterDriverInterfaces(data);
   }
}

/* Unregister Android */
__attribute__((visibility("default")))
void RSOANPL_UnregisterAndroidDisplayPlatform(RSOANPL_PlatformHandle handle)
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
      RSOANPL_DestroyDisplayInterface(data->displayInterface);

      memset(data, 0, sizeof(BEGL_DriverInterfaces));
      free(data);
   }
}
