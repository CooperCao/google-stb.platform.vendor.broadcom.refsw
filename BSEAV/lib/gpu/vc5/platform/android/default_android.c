/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "default_android.h"
#include "private_android.h"
#include "display_android.h"

#include "memory_android.h"
#include "memory_drm.h"
#include "sched_android.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <malloc.h>
#include <memory.h>

#include <gralloc_priv.h>

#define DEFAULT_DISPLAY ((BEGL_DisplayHandle)1)
#define UNUSED(X) (void)X

static const char * GetClientExtensions(void *context)
{
   UNUSED(context);
   return "EGL_KHR_platform_android";
}

int32_t GetDisplay(void *context, uint32_t eglPlatform,
      void *nativeDisplay, const void *attribList, bool isEglAttrib,
      BEGL_DisplayHandle *handle)
{
   ANPL_InternalPlatformHandle *platform = (ANPL_InternalPlatformHandle*)context;
   UNUSED(isEglAttrib);

   if (nativeDisplay != EGL_DEFAULT_DISPLAY)
      return EGL_BAD_PARAMETER; /* only default display is supported */

   if (attribList)
      return EGL_BAD_ATTRIBUTE; /* there should be no attributes */

   switch (eglPlatform)
   {
   case EGL_PLATFORM_ANDROID_KHR:
   case BEGL_DEFAULT_PLATFORM:
      *handle = DEFAULT_DISPLAY;
      return EGL_SUCCESS;
   default:
      *handle = NULL;
      return EGL_BAD_PARAMETER;
   }
}

BEGL_Error Initialize(void *context, BEGL_DisplayHandle handle)
{
   ANPL_InternalPlatformHandle *platform = (ANPL_InternalPlatformHandle*)context;
   if (handle == DEFAULT_DISPLAY)
   {
      if (!platform->displayInterface)
      {
         platform->displayInterface = CreateAndroidDisplayInterface(platform->schedInterface);
         BEGL_RegisterDisplayInterface(platform->displayInterface);
      }
      return platform->displayInterface ? BEGL_Success : BEGL_Fail;
   }
   else
      return BEGL_Fail;
}

BEGL_Error Terminate(void *context, BEGL_DisplayHandle handle)
{
   ANPL_InternalPlatformHandle *platform = (ANPL_InternalPlatformHandle*)context;
   if (handle == DEFAULT_DISPLAY)
   {
      BEGL_RegisterDisplayInterface(NULL);
      if (platform->displayInterface)
      {
         DestroyAndroidDisplayInterface(platform->displayInterface);
         platform->displayInterface = NULL;
      }
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

BEGL_InitInterface *CreateAndroidInitInterface(ANPL_InternalPlatformHandle *platform)
{
   BEGL_InitInterface *init = calloc(1, sizeof(*init));
   if (init)
   {
      init->context = platform;
      init->GetClientExtensions = GetClientExtensions;
      init->GetDisplay = GetDisplay;
      init->Initialize = Initialize;
      init->Terminate = Terminate;
   }
   return init;
}

void DestroyAndroidInitInterface(BEGL_InitInterface *init)
{
   if (init)
   {
      memset(init, 0, sizeof(*init));
      free(init);
   }
}

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
      platform->initInterface = CreateAndroidInitInterface(platform);
      platform->memoryInterface = CreateDRMMemoryInterface();
      if (platform->memoryInterface != NULL)
         platform->drm = true;
      else
         platform->memoryInterface = CreateAndroidMemoryInterface();
      platform->schedInterface = CreateAndroidSchedInterface(platform->memoryInterface);
      platform->displayInterface = NULL;

      *handle = (RSOANPL_PlatformHandle)platform;

      BEGL_RegisterMemoryInterface(platform->memoryInterface);
      BEGL_RegisterSchedInterface(platform->schedInterface);
      BEGL_RegisterInitInterface(platform->initInterface);
      BEGL_RegisterDisplayInterface(NULL);
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
      BEGL_RegisterInitInterface(NULL);

      if (data->displayInterface)
         DestroyAndroidDisplayInterface(data->displayInterface);
      if (data->drm)
         DestroyDRMMemoryInterface(data->memoryInterface);
      else
         DestroyAndroidMemoryInterface(data->memoryInterface);
      DestroyAndroidSchedInterface(data->schedInterface);
      DestroyAndroidInitInterface(data->initInterface);

      memset(data, 0, sizeof(ANPL_InternalPlatformHandle));
      free(data);
   }
}
