/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "default_nexus.h"
#include "private_nexus.h"
#include "display_nexus.h"

#include "memory_nexus.h"
#include "memory_drm.h"
#include "sched_nexus.h"

#include <malloc.h>
#include <memory.h>
#include <EGL/egl.h>
#include <EGL/eglplatform.h>
#include <EGL/eglext_brcm.h>

#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <fnmatch.h>
#include <stdlib.h>

#define UNUSED(X) (void)X

static BEGL_DisplayHandle ToBeglDisplayHandle(NEXUS_DISPLAYHANDLE display)
{
   return display ? (BEGL_DisplayHandle)display : (BEGL_DisplayHandle)1;
}

static NEXUS_DISPLAYHANDLE ToNexusDisplayHandle(BEGL_DisplayHandle handle)
{
   return handle == (BEGL_DisplayHandle)1 ? NULL : (NEXUS_DISPLAYHANDLE)handle;
}

static const char * GetClientExtensions(void *context)
{
   UNUSED(context);
   return "EGL_PLATFORM_NEXUS_BRCM";
}

int32_t GetDisplay(void *context, uint32_t eglPlatform,
      void *nativeDisplay, const void *attribList, bool isEglAttrib,
      BEGL_DisplayHandle *handle)
{
#ifdef SINGLE_PROCESS
   NXPL_InternalPlatformHandle *platform = (NXPL_InternalPlatformHandle*)context;
#endif
   NEXUS_DISPLAYHANDLE display;
   UNUSED(isEglAttrib);

   if (attribList)
      return EGL_BAD_ATTRIBUTE; /* there should be no attributes */

   switch (eglPlatform)
   {
   case EGL_PLATFORM_NEXUS_BRCM:
   case BEGL_DEFAULT_PLATFORM:
#ifdef SINGLE_PROCESS
      if (nativeDisplay == EGL_DEFAULT_DISPLAY)
         display = platform->defaultDisplay;
      else
         display = (NEXUS_DISPLAYHANDLE)nativeDisplay;
#else
      /* Only a default display is supported in multiprocess mode. */
      if (nativeDisplay != EGL_DEFAULT_DISPLAY)
         return EGL_BAD_PARAMETER;
      display = NULL;
#endif
      *handle = ToBeglDisplayHandle(display);
      return EGL_SUCCESS;
   default:
      *handle = NULL;
      return EGL_BAD_PARAMETER;
   }
}

BEGL_Error Initialize(void *context, BEGL_DisplayHandle handle)
{
   NXPL_InternalPlatformHandle *platform = (NXPL_InternalPlatformHandle*)context;
   NEXUS_DISPLAYHANDLE display = ToNexusDisplayHandle(handle);

   if (platform->displayInterface) /* a display is already initialised */
   {
      if (display == platform->currentDisplay)
         return BEGL_Success; /* another eglInitialise() must do nothing and succeed */
      else
         return BEGL_Fail; /* only one display is supported */
   }

   platform->displayContext = (NXPL_DisplayContext *)malloc(
         sizeof(*platform->displayContext));
   if (!platform->displayContext)
      return BEGL_Fail; /* oom */

   platform->displayInterface = CreateDisplayInterface(display, platform->displayContext,
                                                       platform->schedInterface, &platform->eventContext);

   if (!platform->displayInterface)
   {
      free(platform->displayContext);
      platform->displayContext = NULL;
      return BEGL_Fail;
   }

   /* Remember the last initialised display for subsequent calls
    * to eglInitialize() or eglTerminate()
    */
   platform->currentDisplay = display;

   BEGL_RegisterDisplayInterface(platform->displayInterface);
   return BEGL_Success;
}

BEGL_Error Terminate(void *context, BEGL_DisplayHandle handle)
{
   NXPL_InternalPlatformHandle *platform = (NXPL_InternalPlatformHandle*)context;
   NEXUS_DISPLAYHANDLE display = ToNexusDisplayHandle(handle);

   if (display != platform->currentDisplay)
      return BEGL_Fail; /* not our display */

   BEGL_RegisterDisplayInterface(NULL);
   if (platform->displayInterface)
   {
      DestroyDisplayInterface(platform->displayInterface);
      platform->displayInterface = NULL;
   }
   free(platform->displayContext);
   platform->displayContext = NULL;

   /* platform->currentDisplay = unchanged for subsequent calls
    * to eglTermiate() - those should succeed but do nothing
    */
   return BEGL_Success;
}

BEGL_InitInterface *CreateNexusInitInterface(NXPL_InternalPlatformHandle *platform)
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

void DestroyNexusInitInterface(BEGL_InitInterface *init)
{
   if (init)
   {
      memset(init, 0, sizeof(*init));
      free(init);
   }
}

static inline bool opt_bool(const char *option)
{
   char *val = getenv(option);
   if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
      return true;

   return false;
}

static bool has_available_mmu(void)
{
   if (opt_bool("V3D_DRM_DISABLE"))
      return false;

   const char *path = "/proc/device-tree/rdb";
   bool rc = false;
   DIR *dir = opendir(path);
   if (dir)
   {
      struct dirent *ent;
      while ((ent = readdir(dir)) != NULL && !rc)
      {
         if (strstr(ent->d_name, "gpu-mmu") == ent->d_name)
         {
            char buf[256];
            snprintf(buf, sizeof(buf), "%s/%s/compatible", path, ent->d_name);
            FILE *f = fopen(buf, "r");
            if (f)
            {
               if (fread(buf, 1, sizeof(buf), f) > 0)
                  rc = (fnmatch("brcm,bcm*-v3d", buf, 0) == 0);

               fclose(f);
            }
         }
      }
      closedir(dir);
   }
   return rc;
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
      const char *err_msg = NULL;
      platform->initInterface = CreateNexusInitInterface(platform);
      if (has_available_mmu())
      {
         platform->memoryInterface = CreateDRMMemoryInterface();
         platform->drm = !!platform->memoryInterface;
         err_msg = "platform not configured for brcm_cma, or brcmv3d.ko not loaded.  See nexus/docs/Nexus_DtuUsage.pdf\n";
      }
      else
      {
         platform->memoryInterface = CreateMemoryInterface();
         err_msg = "unable to create memory interface\n";
      }

      if (platform->memoryInterface == NULL)
      {
         puts(err_msg);
         exit(0);
      }

      platform->schedInterface = CreateSchedInterface(platform->memoryInterface, &platform->eventContext);
      platform->displayInterface = NULL;
      platform->currentDisplay = NULL;
      platform->defaultDisplay = display;

      *handle = (NXPL_PlatformHandle)platform;

      BEGL_RegisterMemoryInterface(platform->memoryInterface);
      BEGL_RegisterSchedInterface(platform->schedInterface);
      BEGL_RegisterInitInterface(platform->initInterface);
      BEGL_RegisterDisplayInterface(NULL);
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
      BEGL_RegisterInitInterface(NULL);

      if (data->displayInterface)
         DestroyDisplayInterface(data->displayInterface);
      free(data->displayContext);
      data->displayContext = NULL;

      if (data->drm)
         DestroyDRMMemoryInterface(data->memoryInterface);
      else
         DestroyMemoryInterface(data->memoryInterface);
      DestroySchedInterface(data->schedInterface);
      DestroyNexusInitInterface(data->initInterface);

      memset(data, 0, sizeof(NXPL_InternalPlatformHandle));
      free(data);
   }
}
