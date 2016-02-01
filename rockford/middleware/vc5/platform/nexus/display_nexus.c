/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  Nexus platform API for EGL driver
Module   :  Nexus platform

FILE DESCRIPTION

This implementation can be configured for "exclusive" or "nexus surface
compositor mode"

NXPL_PLATFORM_EXCLUSIVE
The "exclusive display" is the fastest of all the display options.
It supports a SINGLE native rendering window, which corresponds to the physical
display. Only one native window can be created in this mode, with one
eglWindowSurface attached. The framebuffers are set one at a time as the
physical display framebuffers. No copying is done.

NXPL_PLATFORM_NSC
The "Nexus surface compositor" display platform supports multiple overlapped
relocatable and resizable display windows.  It supports multiple processes.
Unlike exclusive mode, surface compositor mode implies that buffers are
copied prior to display.  The front buffer from each client is copied into a
common composition surface before being displayed.  NO_NXCLIENT can be defined
to drop to using NSC instead of the reference server NxClient, which is useful
for customers who have developed their own servers.

=============================================================================*/

#include "default_nexus.h"
#include "display_nexus.h"
#include "display_thread.h"
#include "display_helpers.h"
#include "display_surface.h"
#include "fence.h"

#if !defined(NXPL_PLATFORM_EXCLUSIVE) && !defined(NXPL_PLATFORM_NSC)
#error "One (and only one) of NXPL_PLATFORM_EXCLUSIVE or NXPL_PLATFORM_NSC must be defined"
#endif

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <pthread.h>

#define MAX_SWAP_BUFFERS   3

/* NXPL_DisplayContext */
typedef struct
{
   NXPL_DisplayType     displayType;
   BEGL_SchedInterface *schedIface;

#ifdef NXPL_PLATFORM_EXCLUSIVE
   NEXUS_DISPLAYHANDLE  display;
   bool                 stretch;
#endif
} NXPL_DisplayContext;

/* NXPL_NativeWindow */
typedef struct
{
   /* Main thread data */
   NXPL_NativeWindowInfo   windowInfo;
   unsigned int            numSurfaces;

   bool                    initialized;
   int                     swapInterval;

#ifdef NXPL_PLATFORM_EXCLUSIVE
   /* create and delete can overlap, although exclusive access to
      the buffer is mandated.  This delete of one window can remove
      the callback of another */
   int                     bound;
#endif

#ifdef NXPL_PLATFORM_NSC
   uint32_t                clientID;
   /* NSC client handle */
   NEXUS_SurfaceClientHandle  surfaceClient;
#ifdef NXCLIENT_SUPPORT
   NxClient_AllocResults   allocResults;
#endif
#endif /* NXPL_PLATFORM_NSC */

} NXPL_NativeWindow;

/* Called to determine current size of the window referenced by the opaque window handle.
 * This is needed by EGL in order to know the size of a native 'window'. */
static BEGL_Error DispWindowGetInfo(void *context,
                                    void *nativeWindow,
                                    BEGL_WindowInfoFlags flags,
                                    BEGL_WindowInfo *info)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)GetNativeWindow(nativeWindow);

   if (nw != NULL)
   {
      if (flags & BEGL_WindowInfoWidth)
         info->width = nw->windowInfo.width;
      if (flags & BEGL_WindowInfoHeight)
         info->height = nw->windowInfo.height;

      if (flags & BEGL_WindowInfoSwapChainCount)
         info->swapchain_count = nw->numSurfaces;

#ifdef NXPL_PLATFORM_EXCLUSIVE
      if (flags & BEGL_WindowInfoRefreshRate)
      {
         NXPL_DisplayContext *ctx = (NXPL_DisplayContext*)context;
         NEXUS_DisplayStatus status;

         if (ctx->display != NULL)
         {
            NEXUS_Display_GetStatus(ctx->display, &status);
            info->refreshRateMilliHertz = status.refreshRate;
         }
         else
            info->refreshRateMilliHertz = 60;
      }
#endif
      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_Error DispSurfaceGetInfo(void *context, void *nativeSurface, BEGL_SurfaceInfo *info)
{
   NXPL_Surface  *s = (NXPL_Surface*)nativeSurface;
   bool          ok = false;

   /* TODO : figure out if we can actually verify the surface as a valid NEXUS handle */
   if (s != NULL && (void*)s != (void*)0xFFFFFFFF && info != NULL)
   {
      NEXUS_SurfaceStatus surfStatus;
      NEXUS_SurfaceMemory surfMemory;

      NEXUS_Surface_GetStatus(s->surface, &surfStatus);
      NEXUS_Surface_GetMemory(s->surface, &surfMemory);

      info->width          = surfStatus.width;
      info->height         = surfStatus.height;
      info->pitchBytes     = surfMemory.pitch;
      info->physicalOffset = s->physicalOffset;
      info->cachedAddr     = s->cachedPtr;
      info->byteSize       = surfMemory.bufferSize;

      ok = NexusToBeglFormat(&info->format, surfStatus.pixelFormat);

      /* sanity check */
      assert(s->format == info->format);
   }

   return ok ? BEGL_Success : BEGL_Fail;
}

static BEGL_Error DispSurfaceChangeRefCount(void *context, void *nativeSurface, BEGL_RefCountMode incOrDec)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(nativeSurface);
   BSTD_UNUSED(incOrDec);

   /* Nothing to do in Nexus platform */
   return BEGL_Success;
}

static BEGL_Error DispSurfaceVerifyImageTarget(void *context,
      void *nativeSurface, uint32_t eglTarget)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(nativeSurface);

   return eglTarget == EGL_NATIVE_PIXMAP_KHR ? BEGL_Success : BEGL_Fail;
}

static BEGL_Error DispGetNextSurface(
   void *context,
   void *nativeWindow,
   BEGL_BufferFormat format,
   BEGL_BufferFormat *actualFormat,
   void **nativeSurface,
   int *fence)
{
   NXPL_DisplayContext *ctx = (NXPL_DisplayContext*)context;
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)GetNativeWindow(nativeWindow);

   if (nw == NULL || nativeWindow == NULL || ctx == NULL || actualFormat == NULL)
      return BEGL_Fail;

   if (DequeueSurface(nativeWindow, nativeSurface, fence) != BEGL_Success)
   {
      *actualFormat = BEGL_BufferFormat_INVALID;
      return BEGL_Fail;
   }

   NXPL_Surface *s = *nativeSurface;

   /* if the new surface has different size, recreate */
   if (nw->windowInfo.width != s->windowInfo.width ||
       nw->windowInfo.height != s->windowInfo.height ||
       format != s->format)
   {
      /* wait on the fence, as it could be still on the display */
      WaitFence(ctx->schedIface, s->fence);

      DestroySurface(s);
      CreateSurface(s, format, nw->windowInfo.width, nw->windowInfo.height,
         "SwapChain Surface");
   }

   /* if any of the presenation settings have changed, copy the windowInfo into the surface */
   int windowTest = memcmp(&nw->windowInfo, &s->windowInfo, sizeof(NXPL_NativeWindowInfo));
   if (windowTest)
      s->windowInfo = nw->windowInfo;

   return BEGL_Success;
}

static BEGL_Error DispDisplaySurface(void *context, void *nativeWindow, void *nativeSurface, int fence)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)GetNativeWindow(nativeWindow);

   if (nw != NULL)
      return QueueSurface(nativeWindow, nativeSurface, fence, nw->swapInterval);
   else
      return BEGL_Fail;
}

static BEGL_Error DispCancelSurface(void *context, void *nativeWindow, void *nativeSurface, int fence)
{
   if (nativeWindow != NULL)
      return CancelSurface(nativeWindow, nativeSurface, fence);

   return BEGL_Fail;
}

static void DispSetSwapInterval(void *context, void *nativeWindow, int interval)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)GetNativeWindow(nativeWindow);

   if (interval < 0)
      return;

   nw->swapInterval = interval;
}

static bool  DisplayPlatformSupported(void *context, uint32_t platform)
{
   BSTD_UNUSED(context);
   return platform == EGL_PLATFORM_NEXUS_BRCM;
}

static bool DispSetDefaultDisplay(void *context, void *display)
{
#ifdef NXPL_PLATFORM_EXCLUSIVE
   NXPL_DisplayContext    *ctx = (NXPL_DisplayContext *)context;

   if (display == NULL || display == (void*)0xFFFFFFFF)
      return false;

   ctx->display = display;
#endif

   return true;
}

static void *DispGetDefaultDisplay(void *context)
{
#ifdef NXPL_PLATFORM_NSC
   return (void *)1;
#endif

#ifdef NXPL_PLATFORM_EXCLUSIVE
   NXPL_DisplayContext    *ctx = (NXPL_DisplayContext *)context;
   if (ctx->display == NULL)
   {
      /* Headless display mode has a NULL display, but that would map to EGL_NO_DISPLAY during
         eglGetDisplay, so fake up a display id */
      return (void *)1;
   }
   else
      return ctx->display;
#endif
}

static const char *GetClientExtensions(void *context)
{
   BSTD_UNUSED(context);
   return "EGL_BRCM_platform_nexus";
}

/* This is passed a NXPL_NativeWindow, generated from NXPL_CreateNativeWindow() and
   wraps it into an opaque type which can be used elsewhere as a window surface */
/* TODO! when the platform layer changes, pass in the color format for the eglwindow
   so we don't guess at startup and correct later */
static void *DispWindowPlatformStateCreate(void *context, void *native)
{
   NXPL_DisplayContext *ctx = (NXPL_DisplayContext *)context;
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;
   void *res = NULL;

   if (ctx && nw)
   {
      /* Thread uses only opaque types, so only pass NXPL_NativeWindow as
         a void*, so we can't be tempted to tinker inside the thread */

      /* Surface format is only know when dequeue is called, so create with a
         default type first */
#ifdef NXPL_PLATFORM_EXCLUSIVE
      res = CreateWorkerThread(nw, ctx->schedIface, nw->numSurfaces,
         &nw->bound, &nw->windowInfo, BEGL_BufferFormat_eA8B8G8R8, ctx->display, ctx->displayType);
#endif

#ifdef NXPL_PLATFORM_NSC
      res = CreateWorkerThread(nw, ctx->schedIface, nw->numSurfaces,
         &nw->windowInfo, BEGL_BufferFormat_eA8B8G8R8, nw->clientID, nw->surfaceClient,
         ctx->displayType);
#endif
   }

   return res;
}

static BEGL_Error DispWindowPlatformStateDestroy(void *context, void *windowState)
{
   return DestroyWorkerThread(windowState);
}

BEGL_DisplayInterface *CreateDisplayInterface(NEXUS_DISPLAYHANDLE display, BEGL_SchedInterface *schedIface)
{
   BEGL_DisplayInterface   *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));

   if (disp != NULL)
   {
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      NXPL_DisplayContext    *ctx = (NXPL_DisplayContext *)malloc(sizeof(NXPL_DisplayContext));

      if (ctx != NULL)
      {
         memset(ctx, 0, sizeof(NXPL_DisplayContext));
#ifdef NXPL_PLATFORM_EXCLUSIVE
         ctx->display = display;
#endif
         ctx->schedIface = schedIface;

         disp->context = (void*)ctx;

         disp->WindowGetInfo              = DispWindowGetInfo;
         disp->SurfaceGetInfo             = DispSurfaceGetInfo;
         disp->SurfaceChangeRefCount      = DispSurfaceChangeRefCount;
         disp->SurfaceVerifyImageTarget   = DispSurfaceVerifyImageTarget;
         disp->GetNextSurface             = DispGetNextSurface;
         disp->DisplaySurface             = DispDisplaySurface;
         disp->CancelSurface              = DispCancelSurface;
         disp->SetSwapInterval            = DispSetSwapInterval;
         disp->PlatformSupported          = DisplayPlatformSupported;
         disp->SetDefaultDisplay          = DispSetDefaultDisplay;
         disp->GetDefaultDisplay          = DispGetDefaultDisplay;
         disp->GetClientExtensions        = GetClientExtensions;
         disp->WindowPlatformStateCreate  = DispWindowPlatformStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowPlatformStateDestroy;
      }
      else
      {
         goto error0;
      }
   }
   return disp;

error0:
   free(disp);
   return NULL;
}

void DestroyDisplayInterface(BEGL_DisplayInterface *disp)
{
   if (disp != NULL)
   {
      NXPL_DisplayContext *ctx = (NXPL_DisplayContext *)disp->context;

      if (ctx != NULL)
         free(ctx);

      memset(disp, 0, sizeof(BEGL_DisplayInterface));
      free(disp);
   }
}

void NXPL_SetDisplayType(NXPL_PlatformHandle handle, NXPL_DisplayType type)
{
   NXPL_InternalPlatformHandle *data = (NXPL_InternalPlatformHandle*)handle;

   if (data != NULL && data->displayInterface != NULL && data->displayInterface->context != NULL)
   {
      NXPL_DisplayContext *dd = (NXPL_DisplayContext*)data->displayInterface->context;
      dd->displayType = type;
   }
}

#ifdef NXPL_PLATFORM_EXCLUSIVE
static uint32_t s_exclusiveNativeWindowCount = 0;
#endif

void *NXPL_CreateNativeWindow(const NXPL_NativeWindowInfo *info)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)malloc(sizeof(NXPL_NativeWindow));

   if (nw != NULL && info != NULL)
   {
      memset(nw, 0, sizeof(NXPL_NativeWindow));

#ifdef NXPL_PLATFORM_EXCLUSIVE
      if (__sync_fetch_and_add(&s_exclusiveNativeWindowCount, 1) != 0)
      {
         fprintf(stderr, "ERROR: Only one NXPL_NativeWindow is allowed in exclusive display mode. Second window create failed.\n");
         goto error;
      }
#endif

      nw->windowInfo = *info;

#ifdef NXPL_PLATFORM_NSC
#ifdef NXCLIENT_SUPPORT
      NxClient_AllocSettings allocSettings;
      NxClient_GetDefaultAllocSettings(&allocSettings);
      allocSettings.surfaceClient = 1;
      int rc = NxClient_Alloc(&allocSettings, &nw->allocResults);
      if (rc)
         goto error;

      /* Attach the surface client for this swapChain. There is one swapChain per native window, so
      we will have one client per native window. */
      nw->clientID = nw->allocResults.surfaceClient[0].id;
#else
      nw->clientID = info->clientID;
#endif /* NXPL_PLATFORM_NSC */

      nw->surfaceClient = NEXUS_SurfaceClient_Acquire(nw->clientID);
      if (!nw->surfaceClient)
      {
         printf("Failed to acquire compositing client surface for client id %d", nw->clientID);
         goto error;
      }

#ifdef NXCLIENT_SUPPORT
      /* default our SurfaceClient to blend */
      NEXUS_SurfaceComposition comp;
      static NEXUS_BlendEquation colorBlend = { NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eSourceAlpha, false, NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eInverseSourceAlpha, false, NEXUS_BlendFactor_eZero };
      static NEXUS_BlendEquation alphaBlend = { NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eDestinationAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero };
      NxClient_GetSurfaceClientComposition(nw->clientID, &comp);
      comp.colorBlend = colorBlend;
      comp.alphaBlend = alphaBlend;
      NxClient_SetSurfaceClientComposition(nw->clientID, &comp);
#endif
#endif /* NXPL_PLATFORM_NSC */

      char *val = getenv("V3D_DOUBLE_BUFFER");
      if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
         nw->numSurfaces = 2;
      else
         nw->numSurfaces = MAX_SWAP_BUFFERS;

      nw->swapInterval = 1;
   }

   return nw;

error:
   NXPL_DestroyNativeWindow(nw);

   return NULL;
}

void NXPL_DestroyNativeWindow(void *native)
{
   if (native != NULL)
   {
      NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;

#ifdef NXPL_PLATFORM_EXCLUSIVE
      __sync_fetch_and_sub(&s_exclusiveNativeWindowCount, 1);
#endif

#ifdef NXPL_PLATFORM_NSC
      if (nw->surfaceClient)
         NEXUS_SurfaceClient_Release(nw->surfaceClient);

#ifdef NXCLIENT_SUPPORT
      NxClient_Free(&nw->allocResults);
#endif
#endif /* NXPL_PLATFORM_NSC */

      free(nw);
   }
}

void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;

   if (info != NULL && nw != NULL)
      nw->windowInfo = *info;
}

#ifdef NXPL_PLATFORM_NSC

NEXUS_SurfaceClientHandle NXPL_CreateVideoWindowClient(void *native, unsigned windowId)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;
   NEXUS_SurfaceClientHandle ret = NULL;

   if (nw && nw->surfaceClient)
      ret = NEXUS_SurfaceClient_AcquireVideoWindow(nw->surfaceClient, windowId);

   return ret;
}

void NXPL_ReleaseVideoWindowClient(NEXUS_SurfaceClientHandle handle)
{
   NEXUS_SurfaceClient_ReleaseVideoWindow(handle);
}

uint32_t NXPL_GetClientID(void *native)
{
   uint32_t ret = 0;

   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;

   if (nw)
      ret = nw->clientID;

   return ret;
}

#endif /* NXPL_PLATFORM_NSC */

bool NXPL_CreateCompatiblePixmap(NXPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SurfaceHandle *surface, BEGL_PixmapInfo *info)
{
   bool ok = false;
   NXPL_Surface  *s = (NXPL_Surface*)calloc(1, sizeof(NXPL_Surface));

   if (info != NULL && s != NULL)
   {
      ok = CreateSurface(s, info->format, info->width, info->height, "Pixmap Surface");

      if (ok)
      {
         if (pixmapHandle != NULL)
            *pixmapHandle = s;

         if (surface != NULL)
            *surface = s->surface;
      }
   }

   return ok;
}

void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle handle, void *pixmapHandle)
{
   DestroySurface((NXPL_Surface*)pixmapHandle);
   free(pixmapHandle);
}
