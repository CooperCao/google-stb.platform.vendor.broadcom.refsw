/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "default_wayland.h"

#include "display_nx/display_nx.h"
#include "display_wl/display_wl.h"
#include "sched_nexus.h"
#include "fence_interface.h"

#include "nxclient.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>

#include "memory_nexus.h" /* has to be included after EGL/egl.h */
#include "memory_drm.h"   /* has to be included after EGL/egl.h */

#include <wayland-client.h>

#include <stdbool.h>
#include <stdio.h>
#include <memory.h>
#include <assert.h>

#define MAX_DISPLAYS 4
#define UNUSED(x) (void)(x)

typedef struct WLPL_Display
{
   EGLenum platform;
   void *nativeDisplay;
   BEGL_DisplayInterface *interface;
} WLPL_Display;

typedef struct WLPL_Platform
{
   BEGL_MemoryInterface *memoryInterface;
   bool drm;
   BEGL_SchedInterface *schedInterface;
   BEGL_InitInterface initInterface;

   WLPL_Display displays[MAX_DISPLAYS];
   unsigned numDisplays;
} WLPL_Platform;

static BEGL_DisplayInterface *CreateDisplayInterface(EGLenum platform,
      void *nativeDisplay, BEGL_SchedInterface *schedInterface)
{
   switch (platform)
   {
   case EGL_PLATFORM_WAYLAND_EXT:
      return WLPL_CreateWaylandDisplayInterface(nativeDisplay, schedInterface);
   case BEGL_DEFAULT_PLATFORM:
      return WLPL_CreateNexusDisplayInterface(schedInterface);
   default:
      return NULL;
   }
}

static void DestroyDisplayInterface(BEGL_DisplayInterface *iface,
      EGLenum platform)
{
   assert(iface);

   switch (platform)
   {
   case EGL_PLATFORM_WAYLAND_EXT:
      WLPL_DestroyWaylandDisplayInterface(iface);
      break;
   case BEGL_DEFAULT_PLATFORM:
      WLPL_DestroyNexusDisplayInterface(iface);
      break;
   default:
      break;
   }
}

static WLPL_Display *FindDisplay(WLPL_Platform *platform,
      EGLenum eglPlatform, const void *nativeDisplay)
{
   for (unsigned i = 0; i < platform->numDisplays; i++)
   {
      WLPL_Display *display = &platform->displays[i];
      if (display->platform == eglPlatform
            && display->nativeDisplay == nativeDisplay)
         return display;
   }
   return NULL;
}

static WLPL_Display *AddDisplay(WLPL_Platform *platform, uint32_t eglPlatform,
      void *nativeDisplay, int32_t *error)
{
   WLPL_Display *display = NULL;
   switch (eglPlatform)
   {
   case EGL_PLATFORM_WAYLAND_EXT:
   case BEGL_DEFAULT_PLATFORM:
      if (platform->numDisplays < MAX_DISPLAYS)
      {
         display = &platform->displays[platform->numDisplays];
         ++platform->numDisplays;

         memset(display, 0, sizeof(*display));
         display->platform = eglPlatform;
         display->nativeDisplay = nativeDisplay;
         /* display->interface will be created in Initialise() */
         *error = EGL_SUCCESS; /* valid parameters */
      }
      else
      {
         fprintf(stderr, "Can't add another EGL display, maximum reached\n");
         display = NULL;
         *error = EGL_SUCCESS; /* valid parameters */
      }
      break;
   default:
      display = NULL;
      *error = EGL_BAD_PARAMETER; /* unknown platform */
      break;
   }
   return display;
}

static const char *GetClientExtensions(void *context)
{
   BSTD_UNUSED(context);
   return "EGL_KHR_platform_wayland EGL_EXT_platform_wayland";
}

static int32_t GetDisplay(void *context, uint32_t eglPlatform,
      void *nativeDisplay, const void *attribList, bool isEglAttrib,
      BEGL_DisplayHandle *handle)
{
   WLPL_Platform *platform = (WLPL_Platform *)context;

   if (attribList)
   {
      *handle = EGL_NO_DISPLAY;
      return EGL_BAD_ATTRIBUTE;
   }

   *handle = FindDisplay(platform, eglPlatform, nativeDisplay);
   if (*handle)
      return EGL_SUCCESS; /* success - found previously created display */

   /* create a new display */
   int32_t result;
   *handle = AddDisplay(platform, eglPlatform, nativeDisplay, &result);
   return result;
}

static BEGL_Error Initialize(void *context, BEGL_DisplayHandle handle)
{
   WLPL_Platform *platform = (WLPL_Platform *)context;
   WLPL_Display *display = (WLPL_Display *)handle;
   if (!display)
      return BEGL_Fail;

   if (!display->interface)
   {
      display->interface = CreateDisplayInterface(display->platform,
            display->nativeDisplay, platform->schedInterface);
      if (display->interface)
         BEGL_RegisterDisplayInterface(display->interface);
   }
   /* else already initialised */
   return display->interface ? BEGL_Success : BEGL_Fail;
}

static BEGL_Error Terminate(void *context, BEGL_DisplayHandle handle)
{
   WLPL_Platform *platform = (WLPL_Platform *)context;
   WLPL_Display *display = (WLPL_Display *)handle;
   if (!display)
      return BEGL_Fail;

   if (display->interface)
   {
      BEGL_RegisterDisplayInterface(NULL);
      DestroyDisplayInterface(display->interface, display->platform);
      display->interface = NULL;
   }
   /* else already terminated */
   return BEGL_Success;
}

static bool ExplicitSync(void *context)
{
   return true;
}

static void DestroyPlatform(WLPL_Platform *platform)
{
   assert(platform);

   /* Tell the 3D driver that the platform layer is about to go away
    * so it can perform final cleanup.
    */
   BEGL_PlatformAboutToShutdown();

   /* Clear out all of our interface pointers and register the empty ones.
    * Do this before destroying the interfaces in case the driver needs
    * to use them to shutdown correctly.
    */
   BEGL_RegisterDisplayInterface(NULL);
   BEGL_RegisterInitInterface(NULL);
   BEGL_RegisterMemoryInterface(NULL);
   BEGL_RegisterSchedInterface(NULL);

   for (unsigned i = 0; i < platform->numDisplays; i++)
   {
      WLPL_Display *display = &platform->displays[i];
      if (display->interface)
      {
         DestroyDisplayInterface(display->interface, display->platform);
         display->interface = NULL;
      }
   }
   platform->numDisplays = 0;

   if (platform->schedInterface)
   {
      DestroySchedInterface(platform->schedInterface);
      platform->schedInterface = NULL;
   }

   if (platform->memoryInterface)
   {
      if (platform->drm)
         DestroyDRMMemoryInterface(platform->memoryInterface);
      else
         DestroyMemoryInterface(platform->memoryInterface);
      platform->memoryInterface = NULL;
   }

   memset(platform, 0, sizeof(*platform));
}

static bool CreatePlatform(WLPL_Platform *platform)
{
   assert(platform);

   memset(platform, 0, sizeof(*platform));

   platform->memoryInterface = CreateDRMMemoryInterface();
   if (platform->memoryInterface != NULL)
      platform->drm = true;
   else
      platform->memoryInterface = CreateMemoryInterface();
   if (!platform->memoryInterface)
      goto error;

   platform->schedInterface = CreateSchedInterface(platform->memoryInterface);
   if (!platform->schedInterface)
      goto error;

   /* block GL calls until TFU conversion has finished */
   platform->schedInterface->ExplicitSync = ExplicitSync;

   platform->initInterface.context = platform;
   platform->initInterface.GetClientExtensions = GetClientExtensions;
   platform->initInterface.GetDisplay = GetDisplay;
   platform->initInterface.Initialize = Initialize;
   platform->initInterface.Terminate = Terminate;

   BEGL_RegisterMemoryInterface(platform->memoryInterface);
   BEGL_RegisterSchedInterface(platform->schedInterface);
   BEGL_RegisterInitInterface(&platform->initInterface);
   BEGL_RegisterDisplayInterface(NULL); /* called from Initialize(dpy) */
   return true;

   error: DestroyPlatform(platform);
   return false;
}

static bool s_joined;
WLPL_Platform s_platform;

__attribute__((constructor))
static void wlpl_load(void)
{
   if (NxClient_Join(NULL) == NEXUS_SUCCESS)
   {
      s_joined = true;
      if (!CreatePlatform(&s_platform))
         fprintf(stderr, "Wayland platform constructor failed!\n");
   }
   else
   {
      fprintf(stderr, "Wayland platform constructor: NxClient_Join failed!\n");
   }
}

__attribute__((destructor))
static void wlpl_unload(void)
{
   if (s_joined)
   {
      DestroyPlatform(&s_platform);
      NxClient_Uninit();
      s_joined = false;
   }
}
