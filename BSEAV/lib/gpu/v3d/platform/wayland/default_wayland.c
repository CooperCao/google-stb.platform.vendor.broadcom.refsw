/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "default_wayland.h"
#include "nxclient.h"
#include "../common/perf_event.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/begl_platform.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>

#include "display_nx/display_nx.h"
#include "display_wl/display_wl.h"

#define UNUSED(x) (void)(x)

BEGL_MemoryInterface *NXPL_CreateMemInterface(BEGL_HWInterface *hwIface);
BEGL_HWInterface *NXPL_CreateHWInterface(BEGL_HardwareCallbacks *callbacks, EventContext *eventContext);

void NXPL_DestroyMemInterface(BEGL_MemoryInterface *iface);
void NXPL_DestroyHWInterface(BEGL_HWInterface *iface);

typedef struct WLPL_Display
{
   EGLenum platform;
   void *nativeDisplay;
   BEGL_DisplayInterface *interface;
} WLPL_Display;

#define MAX_DISPLAYS 4
typedef struct WLPL_Platform
{
   BEGL_MemoryInterface *memInterface;
   BEGL_HWInterface *hwInterface;
   WLPL_Display displays[MAX_DISPLAYS];
   unsigned numDisplays;
} WLPL_Platform;

static bool CreateDisplayInterface(WLPL_Display *display,
      BEGL_MemoryInterface *memInterface, BEGL_HWInterface *hwInterface)
{
   assert(!display->interface);

   BEGL_DriverInterfaces temp;
   BEGL_GetDefaultDriverInterfaces(&temp);

   switch (display->platform)
   {
   case EGL_PLATFORM_WAYLAND_EXT:
      display->interface = WLPL_CreateWaylandDisplayInterface(memInterface,
            hwInterface, display->nativeDisplay);
      break;

   case BEGL_DEFAULT_PLATFORM:
   default:
      display->interface = WLPL_CreateNexusDisplayInterface(memInterface,
            hwInterface);
      break;
   }
   return display->interface != NULL;
}

static void DestroyDisplayInterface(WLPL_Display *display)
{
   assert(display->interface);

   switch (display->platform)
   {
   case EGL_PLATFORM_WAYLAND_EXT:
      WLPL_DestroyWaylandDisplayInterface(display->interface);
      break;
   case BEGL_DEFAULT_PLATFORM:
   default:
      WLPL_DestroyNexusDisplayInterface(display->interface);
      break;
   }

   display->interface = NULL;
}

static void CreatePlatform(WLPL_Platform *platform)
{
   BEGL_DriverInterfaces temp;
   BEGL_GetDefaultDriverInterfaces(&temp);

   memset(platform, 0, sizeof(*platform));
   platform->hwInterface = NXPL_CreateHWInterface(&temp.hardwareCallbacks, NULL);
   platform->memInterface = NXPL_CreateMemInterface(platform->hwInterface);

   BEGL_DriverInterfaces interfaces;
   BEGL_GetDefaultDriverInterfaces(&interfaces);
   interfaces.hwInterface = platform->hwInterface;
   interfaces.memInterface = platform->memInterface;
   interfaces.displayInterface = NULL;
   BEGL_RegisterDriverInterfaces(&interfaces);
}

static void DestroyPlatform(WLPL_Platform *platform)
{
   BEGL_DriverInterfaces nulliface;
   memset(&nulliface, 0, sizeof(BEGL_DriverInterfaces));
   BEGL_GetDefaultDriverInterfaces(&nulliface);
   BEGL_RegisterDriverInterfaces(&nulliface);

   for (unsigned i = 0; i < platform->numDisplays; i++)
   {
      WLPL_Display *display = &platform->displays[i];
      if (display->interface)
         DestroyDisplayInterface(display);
   }
   memset(&platform->displays, 0, sizeof(platform->displays));
   platform->numDisplays = 0;

   NXPL_DestroyMemInterface(platform->memInterface);
   platform->memInterface = NULL;
   NXPL_DestroyHWInterface(platform->hwInterface);
   platform->hwInterface = NULL;
}

static EGLDisplay FindPlatformDisplay(const WLPL_Platform *platform,
      EGLenum eglPlatform, const void *nativeDisplay)
{
   for (unsigned i = 0; i < platform->numDisplays; i++)
   {
      const WLPL_Display *display = &platform->displays[i];
      if (display->platform == eglPlatform
            && display->nativeDisplay == nativeDisplay)
         return (EGLDisplay)(i + 1);
   }
   return EGL_NO_DISPLAY;
}

static WLPL_Display *GetPlatformDisplay(WLPL_Platform *platform,
      BEGL_DisplayHandle handle)
{
   if (handle != EGL_NO_DISPLAY)
   {
      uintptr_t i = ((uintptr_t)handle) - 1;
      if (i < platform->numDisplays)
         return &platform->displays[i];
   }
   return NULL;
}

static BEGL_Error AddPlatformDisplay(WLPL_Platform *platform,
      uint32_t eglPlatform, void *nativeDisplay, BEGL_DisplayHandle *handle)
{
   if (platform->numDisplays == MAX_DISPLAYS)
   {
      *handle = EGL_NO_DISPLAY;
      return BEGL_Success; /* valid parameters but can't create another display */
   }

   WLPL_Display *display = &platform->displays[platform->numDisplays];
   display->platform = eglPlatform;
   display->nativeDisplay = nativeDisplay;
   assert(!display->interface); /* will be created in Initialise() */

   switch (eglPlatform)
   {
   case EGL_PLATFORM_WAYLAND_EXT:
   case BEGL_DEFAULT_PLATFORM:
      *handle = (BEGL_DisplayHandle *)++platform->numDisplays;
      return BEGL_Success; /* valid parameters */
   default:
      *handle = EGL_NO_DISPLAY;
      return BEGL_Fail; /* invalid parameters: unknown platform */
   }
}

static BEGL_Error GetDisplay(void *context, uint32_t eglPlatform,
      void *nativeDisplay, const int32_t *attribList, BEGL_DisplayHandle *handle)
{
   WLPL_Platform *platform = (WLPL_Platform *)context;

   if (attribList && *attribList != EGL_NONE)
   {
      *handle = EGL_NO_DISPLAY;
      return BEGL_Fail; /* invalid parameters: attributes present */
   }

   *handle = FindPlatformDisplay(platform, eglPlatform, nativeDisplay);
   if (*handle)
      return BEGL_Success; /* success - found previously created display */

   /* create a new display */
   return AddPlatformDisplay(platform, eglPlatform, nativeDisplay, handle);
}

static BEGL_Error Initialize(void *context, BEGL_DisplayHandle handle)
{
   WLPL_Platform *platform = (WLPL_Platform *)context;
   WLPL_Display *display = GetPlatformDisplay(platform, handle);
   if (!display)
      return BEGL_Fail;

   if (display->interface)
      return BEGL_Success; /* already initialised */

   if (CreateDisplayInterface(display, platform->memInterface,
         platform->hwInterface))
   {
      BEGL_DriverInterfaces interfaces;
      BEGL_GetDefaultDriverInterfaces(&interfaces);
      interfaces.hwInterface = platform->hwInterface;
      interfaces.memInterface = platform->memInterface;
      interfaces.displayInterface = display->interface;
      BEGL_RegisterDriverInterfaces(&interfaces);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error Terminate(void *context, BEGL_DisplayHandle handle)
{
   WLPL_Platform *platform = (WLPL_Platform *)context;
   WLPL_Display *display = GetPlatformDisplay(platform, handle);
   if (!display)
      return BEGL_Fail;

   if (!display->interface)
      return BEGL_Success; /* already terminated */

   BEGL_DriverInterfaces interfaces;
   BEGL_GetDefaultDriverInterfaces(&interfaces);
   interfaces.hwInterface = platform->hwInterface;
   interfaces.memInterface = platform->memInterface;
   interfaces.displayInterface = NULL;
   BEGL_RegisterDriverInterfaces(&interfaces);

   DestroyDisplayInterface(display);
   return BEGL_Success;
}

static bool s_joined;
WLPL_Platform s_platform;

__attribute__((constructor))
static void v3d_wlclient_load(void)
{
   if (NxClient_Join(NULL) == NEXUS_SUCCESS)
   {
      s_joined = true;
      CreatePlatform(&s_platform);

      BEGL_InitInterface init;
      memset(&init, 0, sizeof(init));
      init.context = &s_platform;
      init.GetDisplay = GetDisplay;
      init.Initialize = Initialize;
      init.Terminate = Terminate;
      BEGL_RegisterInitInterface(&init);
   }
   else
   {
      fprintf(stderr, "Wayland platform constructor: NxClient_Join failed\n");
   }
}

__attribute__((destructor))
static void v3d_wlclient_unload(void)
{
   if (s_joined)
   {
      BEGL_RegisterInitInterface(NULL);

      DestroyPlatform(&s_platform);
      NxClient_Uninit();
      s_joined = false;
   }
}
