/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  Default DirectFB platform API for EGL driver
Module   :  DirectFB platform on NEXUS

FILE DESCRIPTION
DESC
=============================================================================*/

#include "default_directfb.h"
#include "private_directfb.h"

#include "memory_nexus.h"
#include "sched_nexus.h"

#include <malloc.h>
#include <string.h>


/*****************************************************************************
 * Registration interface
 *****************************************************************************/
/* Register directFB with the egl platform */
void DBPL_RegisterDirectFBDisplayPlatform(DBPL_PlatformHandle *handle, IDirectFB *dfb)
{
   DBPL_InternalPlatformHandle *platform = (DBPL_InternalPlatformHandle*)malloc(sizeof(DBPL_InternalPlatformHandle));
   memset(platform, 0, sizeof(DBPL_InternalPlatformHandle));

   if (platform != NULL)
   {
      platform->memoryInterface  = CreateMemoryInterface();
      platform->schedInterface   = CreateSchedInterface(platform->memoryInterface);
      platform->displayInterface = CreateDirectFBDisplayInterface(dfb,
                                                         platform->schedInterface);

      *handle = (DBPL_PlatformHandle)platform;

      BEGL_RegisterMemoryInterface(platform->memoryInterface);
      BEGL_RegisterSchedInterface(platform->schedInterface);
      BEGL_RegisterDisplayInterface(platform->displayInterface);
   }
}

/* Unregister directFB */
void DBPL_UnregisterDirectFBDisplayPlatform(DBPL_PlatformHandle handle)
{
   DBPL_InternalPlatformHandle *platform = (DBPL_InternalPlatformHandle*)handle;

   if (platform != NULL)
   {
      /* Clear out all of our interface pointers and register the empty ones.
       * Do this before destroying the interfaces in case the driver needs to use them
       * to shutdown correctly */
      DBPL_InternalPlatformHandle nulliface;
      memset(&nulliface, 0, sizeof(DBPL_InternalPlatformHandle));

      BEGL_RegisterMemoryInterface(NULL);
      BEGL_RegisterSchedInterface(NULL);
      BEGL_RegisterDisplayInterface(NULL);

      DestroyDirectFBDisplayInterface(platform->displayInterface);
      DestroySchedInterface(platform->schedInterface);
      DestroyMemoryInterface(platform->memoryInterface);

      memset(platform, 0, sizeof(DBPL_InternalPlatformHandle));
      free(platform);
   }
}
