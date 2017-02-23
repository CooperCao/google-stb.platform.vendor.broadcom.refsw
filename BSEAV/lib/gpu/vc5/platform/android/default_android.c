/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  Default Android platform API for EGL driver
Module   :  RSO Android platform on NEXUS

FILE DESCRIPTION
DESC
=============================================================================*/

#include "default_android.h"
#include "private_android.h"
#include "display_android.h"

#include "memory_android.h"
#include "memory_drm.h"
#include "sched_android.h"

#include <EGL/egl.h>
#include <malloc.h>
#include <memory.h>

#define UNUSED(X) (void)X

/*****************************************************************************
 * Registration interface
 *****************************************************************************/
/* Register Android with the EGL platform */
__attribute__((visibility("default")))
void RSOANPL_RegisterAndroidDisplayPlatform(RSOANPL_PlatformHandle *handle, ANativeWindow *awin)
{
   ANPL_InternalPlatformHandle *platform = (ANPL_InternalPlatformHandle*)malloc(sizeof(ANPL_InternalPlatformHandle));
   memset(platform, 0, sizeof(ANPL_InternalPlatformHandle));

   if (platform != NULL)
   {
      platform->memoryInterface = CreateDRMMemoryInterface();
      if (platform->memoryInterface != NULL)
         platform->drm = true;
      else
         platform->memoryInterface = CreateAndroidMemoryInterface();
      platform->schedInterface = CreateAndroidSchedInterface(platform->memoryInterface);
      platform->displayInterface = CreateAndroidDisplayInterface(platform->schedInterface);

      *handle = (RSOANPL_PlatformHandle)platform;

      BEGL_RegisterMemoryInterface(platform->memoryInterface);
      BEGL_RegisterSchedInterface(platform->schedInterface);
      BEGL_RegisterDisplayInterface(platform->displayInterface);
   }
}

/* Unregister Android */
__attribute__((visibility("default")))
void RSOANPL_UnregisterAndroidDisplayPlatform(RSOANPL_PlatformHandle handle)
{
   ANPL_InternalPlatformHandle *data = (ANPL_InternalPlatformHandle*)handle;

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

      DestroyAndroidDisplayInterface(data->displayInterface);
      if (data->drm)
         DestroyDRMMemoryInterface(data->memoryInterface);
      else
         DestroyAndroidMemoryInterface(data->memoryInterface);
      DestroyAndroidSchedInterface(data->schedInterface);

      memset(data, 0, sizeof(ANPL_InternalPlatformHandle));
      free(data);
   }
}
