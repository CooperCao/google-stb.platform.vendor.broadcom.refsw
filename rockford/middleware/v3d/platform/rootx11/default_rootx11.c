/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  Default rootx11 platform API for EGL driver
Module   :  rootx11 platform

FILE DESCRIPTION

WARNING! This only renders to the root window of the X11 project for Chromium
OS.  Do not expect to run multiclient or any other X apps via this.

DESC
=============================================================================*/

#include "default_rootx11.h"
#include "display_rootx11.h"

#include <xf86drm.h>
#include <X11/Xutil.h>
#include <X11/Xmd.h>
#include <X11/extensions/dri2proto.h>
#include <X11/extensions/dri2.h>

#include <EGL/egl.h>

#include <malloc.h>
#include <memory.h>


#define UNUSED(X) (void)X

#include "nexus_platform.h"
#ifdef ROOTX11_USE_NXCLIENT
#include "nxclient.h"
#endif
static bool joined_nexus = false;

/*****************************************************************************
 * Registration interface
 *****************************************************************************/
void RXPL_RegisterRootX11DisplayPlatform(RXPL_PlatformHandle *handle, Display *display)
{
   BEGL_DriverInterfaces *data = (BEGL_DriverInterfaces*)malloc(sizeof(BEGL_DriverInterfaces));
   memset(data, 0, sizeof(BEGL_DriverInterfaces));

   fprintf(stderr, "Connecting to nxserver %d\n", joined_nexus);

   /* join nexus */
   if (!joined_nexus)
   {
#ifdef ROOTX11_USE_NXCLIENT
      NEXUS_Error rc;
      NxClient_JoinSettings joinSettings;

      NxClient_GetDefaultJoinSettings(&joinSettings);
      snprintf(joinSettings.name, NXCLIENT_MAX_NAME, "%s", "es2tri app");
      rc = NxClient_Join(&joinSettings);
      if (rc)
      {
         fprintf(stderr, "Connecting to nxserver failed in NxClient_Join\n");
         return;
      }
#else
      NEXUS_Error rc;
      NEXUS_ClientAuthenticationSettings authSettings;
      NEXUS_Platform_GetDefaultClientAuthenticationSettings(&authSettings);
      rc = NEXUS_Platform_AuthenticatedJoin(&authSettings);
      if (rc)
      {
         fprintf(stderr, "NEXUS_Platform_AuthenticatedJoin failed err=0x%x\n", rc);
         return;
      }
#endif
      joined_nexus = true;

      fprintf(stderr, "Connected to nxserver\n");
   }

   if (data != NULL)
   {
      BEGL_GetDefaultDriverInterfaces(data);

      /* Uses NEXUS for Memory and HW interface.  Must be Joined in the app */
      data->hwInterface      = NXPL_CreateHWInterface(&data->hardwareCallbacks);
      data->memInterface     = NXPL_CreateMemInterface(data->hwInterface);
      /* The rootX11 is fairly basic.  It only supports a single window with a predefined swap chain */
      data->displayInterface = RXPL_CreateDisplayInterface(data->memInterface, display,
                                                           &data->displayCallbacks);

      *handle = (RXPL_PlatformHandle)data;

      BEGL_RegisterDriverInterfaces(data);
   }
}

void RXPL_UnregisterRootX11DisplayPlatform(RXPL_PlatformHandle handle)
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

      RXPL_DestroyDisplayInterface(data->displayInterface);
      NXPL_DestroyMemInterface(data->memInterface);
      NXPL_DestroyHWInterface(data->hwInterface);

      memset(data, 0, sizeof(BEGL_DriverInterfaces));
      free(data);
   }
}
