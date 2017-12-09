/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "default_nexus.h"
#include "display_nexus.h"
#include "display_helpers.h"
#include "display_surface.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include "display_interface.h"
#include "display_framework.h"
#include "../common/surface_interface_nexus.h"
#include "../common/nexus_heap_selection.h"

#include "display_nexus_exclusive.h"
#include "display_nexus_multi.h"

#include "sched_nexus.h"

#include "platform_common.h"
#include "fence_queue.h"
#include "../common/debug_helper.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <pthread.h>

#define MAX_SWAP_BUFFERS   3

enum
{
   NATIVE_WINDOW_INFO_MAGIC = 0xABBA601D,
   PIXMAP_INFO_MAGIC        = 0x15EEB1A5
};

typedef struct PlatformState
{
   SurfaceInterface surface_interface;
   FenceInterface fence_interface;
   DisplayInterface display_interface;
   NXPL_NativeWindow *native_window;
   DisplayFramework display_framework;
} PlatformState;

/* Called to determine current size of the window referenced by the opaque window handle.
 * This is needed by EGL in order to know the size of a native 'window'. */
static BEGL_Error DispWindowGetInfo(void *context,
                                    void *nativeWindow,
                                    BEGL_WindowInfoFlags flags,
                                    BEGL_WindowInfo *info)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;
   NXPL_NativeWindow *nw = state->native_window;

   if (nw != NULL)
   {
      if (flags & BEGL_WindowInfoWidth)
         info->width = nw->windowInfo.width;
      if (flags & BEGL_WindowInfoHeight)
         info->height = nw->windowInfo.height;

      if (flags & BEGL_WindowInfoSwapChainCount)
         info->swapchain_count = nw->numSurfaces;

      return BEGL_Success;
   }

   return BEGL_Fail;
}

static BEGL_Error DispGetNativeSurface(void *context,
      uint32_t eglTarget, void *eglSurface, void **nativeSurface)
{
   BSTD_UNUSED(context);

   if (eglTarget != EGL_NATIVE_PIXMAP_KHR)
      return BEGL_Fail;
   *nativeSurface = eglSurface;
   return BEGL_Success;
}

static BEGL_Error DispSurfaceGetInfo(void *context, void *nativeSurface, BEGL_SurfaceInfo *info)
{
   NXPL_Surface  *s = (NXPL_Surface*)nativeSurface;
   bool          ok = false;

   /* TODO : figure out if we can actually verify the surface as a valid NEXUS handle */
   if (s != NULL && (void*)s != (void*)0xFFFFFFFF && info != NULL)
   {
      if (s->format == BEGL_BufferFormat_eSAND8 || s->format == BEGL_BufferFormat_eSAND10)
      {
         if (s->surface == NULL)
            return BEGL_Fail;

         NEXUS_StripedSurfaceCreateSettings cs;
         NEXUS_StripedSurface_GetCreateSettings((NEXUS_StripedSurfaceHandle)s->surface, &cs);

         NEXUS_Addr lumaPhys, chromaPhys;
         NEXUS_MemoryBlock_LockOffset(cs.lumaBuffer, &lumaPhys);

         if (cs.chromaBuffer == cs.lumaBuffer)
            chromaPhys = lumaPhys;
         else
            NEXUS_MemoryBlock_LockOffset(cs.chromaBuffer, &chromaPhys);

         uint32_t byteWidth = cs.stripedWidth * (cs.imageWidth + cs.stripedWidth - 1) / cs.stripedWidth;

         info->format         = s->format;
         info->colorimetry    = s->colorimetry;
         info->width          = cs.imageWidth;
         info->height         = cs.imageHeight;
         info->pitchBytes     = 0;
         info->contiguous     = true;   // Nexus surfaces are always contiguous
         info->physicalOffset = lumaPhys + cs.lumaBufferOffset;
         info->chromaOffset   = chromaPhys + cs.chromaBufferOffset;
         info->cachedAddr     = NULL;   // Sand video can't be mapped
         info->secure         = false;  // TODO : I can't find a way to determine this here
         info->miplevels           = 1;
         info->stripeWidth         = cs.stripedWidth;
         info->lumaStripedHeight   = cs.lumaStripedHeight;
         info->chromaStripedHeight = cs.chromaStripedHeight;
         if (cs.chromaBuffer == cs.lumaBuffer)
         {
            info->lumaAndChromaInSameAllocation = true;
            info->chromaByteSize = byteWidth * cs.chromaStripedHeight;
            // byteSize represents the combined luma/chroma buffer when lumaAndChromaInSameAllocation
            info->byteSize = info->chromaOffset - info->physicalOffset + info->chromaByteSize;
         }
         else
         {
            info->lumaAndChromaInSameAllocation = false;
            // When chroma & luma are in different allocations, byteSize is just for luma
            info->byteSize = byteWidth * cs.lumaStripedHeight;
            info->chromaByteSize = byteWidth * cs.chromaStripedHeight;
         }

         NEXUS_MemoryBlock_UnlockOffset(cs.lumaBuffer);
         if (cs.chromaBuffer == cs.lumaBuffer)
            NEXUS_MemoryBlock_UnlockOffset(cs.chromaBuffer);

         ok = true;
      }
      else
      {
         NEXUS_SurfaceStatus surfStatus;
         NEXUS_Surface_GetStatus(s->surface, &surfStatus);

         NEXUS_SurfaceMemory surfMemory;
         NEXUS_Surface_GetMemory(s->surface, &surfMemory);

         NEXUS_SurfaceCreateSettings surfSettings;
         NEXUS_Surface_GetCreateSettings(s->surface, &surfSettings);

         info->colorimetry    = s->colorimetry;
         info->width          = surfStatus.width;
         info->height         = surfStatus.height;
         info->pitchBytes     = surfStatus.pitch;
         info->contiguous     = true; // Nexus surfaces are always contiguous
         info->physicalOffset = s->physicalOffset;
         info->cachedAddr     = s->cachedPtr;
         info->secure         = surfMemory.buffer == NULL;
         info->byteSize       = surfMemory.bufferSize;
         info->miplevels      = surfSettings.mipLevel + 1;

         ok = NexusToBeglFormat(&info->format, surfStatus.pixelFormat);

         /* sanity check */
         assert(s->format == info->format);
      }
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

static BEGL_Error DispGetNextSurface(
   void *context,
   void *nativeWindow,
   BEGL_BufferFormat format,
   BEGL_BufferFormat *actualFormat,
   void **nativeSurface,
   bool secure,
   int *fence
)
{
   UNUSED(context);

   PlatformState *state = (PlatformState *) nativeWindow;

   if (state == NULL || nativeSurface == NULL || actualFormat == NULL)
      return BEGL_Fail;

   *nativeSurface = DisplayFramework_GetNextSurface(
         &state->display_framework, format, secure, fence);

   if (!*nativeSurface)
   {
      *actualFormat = BEGL_BufferFormat_INVALID;
      return BEGL_Fail;
   }
   return BEGL_Success;
}

static BEGL_Error DispDisplaySurface(void *context, void *nativeWindow, void *nativeSurface, int fence, int interval)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;
   if (state && nativeSurface)
   {
      DisplayFramework_DisplaySurface(&state->display_framework,
            nativeSurface, fence, interval);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error DispCancelSurface(void *context, void *nativeWindow, void *nativeSurface, int fence)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) nativeWindow;
   if (state && nativeSurface)
   {
      DisplayFramework_CancelSurface(&state->display_framework, nativeSurface,
            fence);
      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}


/* This is passed a NXPL_NativeWindow, generated from NXPL_CreateNativeWindow() and
   wraps it into an opaque type which can be used elsewhere as a window surface */
/* TODO! when the platform layer changes, pass in the color format for the eglwindow
   so we don't guess at startup and correct later */
static void *DispWindowPlatformStateCreate(void *context, void *native)
{
   NXPL_DisplayContext *ctx = (NXPL_DisplayContext *)context;
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;
   PlatformState *state = NULL;

   if (ctx && nw)
   {
      state = calloc(1, sizeof(*state));
      if (state)
      {
         InitFenceInterface(&state->fence_interface, ctx->schedIface);
         SurfaceInterface_InitNexus(&state->surface_interface);

#ifdef NXPL_PLATFORM_EXCLUSIVE
         if (!DisplayInterface_InitNexusExclusive(&state->display_interface,
               &state->fence_interface, &nw->windowInfo, ctx->displayType,
               ctx->display, &nw->bound))
            goto error_display;
#else
         if (!DisplayInterface_InitNexusMulti(&state->display_interface,
               &state->fence_interface, &nw->windowInfo, ctx->displayType,
               nw->numSurfaces, nw->clientID, nw->surfaceClient))
            goto error_display;
#endif

         state->native_window = nw;

         if (!DisplayFramework_Start(&state->display_framework,
               &state->display_interface,
               &state->fence_interface,
               &state->surface_interface,
               state->native_window->windowInfo.width,
               state->native_window->windowInfo.height,
               state->native_window->numSurfaces))
            goto error_framework;
      }
   }
   return state;

error_framework:
   Interface_Destroy(&state->display_interface.base);
   Interface_Destroy(&state->surface_interface.base);
   Interface_Destroy(&state->fence_interface.base);
error_display:
   free(state);
   return NULL;
}

static BEGL_Error DispWindowPlatformStateDestroy(void *context, void *windowState)
{
   UNUSED(context);
   PlatformState *state = (PlatformState *) windowState;

   if (state)
   {
      DisplayFramework_Stop(&state->display_framework);
      Interface_Destroy(&state->display_interface.base);
      Interface_Destroy(&state->surface_interface.base);
      Interface_Destroy(&state->fence_interface.base);

#ifndef NDEBUG
      /* catch some cases of use after free */
      memset(state, 0, sizeof(*state));
#endif
      free(state);
   }
   return BEGL_Success;
}

BEGL_DisplayInterface *CreateDisplayInterface(
      NEXUS_DISPLAYHANDLE display,
      NXPL_DisplayContext *ctx,
      struct BEGL_SchedInterface *schedIface)
{
   BEGL_DisplayInterface *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));

   if (disp != NULL)
   {
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      if (ctx != NULL)
      {
         memset(ctx, 0, sizeof(NXPL_DisplayContext));

#ifdef NXPL_PLATFORM_EXCLUSIVE
         ctx->display = display;
#endif
         ctx->schedIface = schedIface;

         disp->context = (void*)ctx;

         disp->WindowGetInfo              = DispWindowGetInfo;
         disp->GetNativeSurface           = DispGetNativeSurface;
         disp->SurfaceGetInfo             = DispSurfaceGetInfo;
         disp->SurfaceChangeRefCount      = DispSurfaceChangeRefCount;
         disp->GetNextSurface             = DispGetNextSurface;
         disp->DisplaySurface             = DispDisplaySurface;
         disp->CancelSurface              = DispCancelSurface;
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

void NXPL_GetDefaultNativeWindowInfoEXT(NXPL_NativeWindowInfoEXT *info)
{
   if (info != NULL)
   {
      memset(info, 0, sizeof(NXPL_NativeWindowInfoEXT));
      static NEXUS_BlendEquation colorBlend = { NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eSourceAlpha, false, NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eInverseSourceAlpha, false, NEXUS_BlendFactor_eZero };
      static NEXUS_BlendEquation alphaBlend = { NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eDestinationAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero };
      info->colorBlend = colorBlend;
      info->alphaBlend = alphaBlend;
      info->magic      = NATIVE_WINDOW_INFO_MAGIC;
   }
}

void *NXPL_CreateNativeWindowEXT(const NXPL_NativeWindowInfoEXT *info)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)malloc(sizeof(NXPL_NativeWindow));

   if (nw != NULL && info != NULL)
   {
      assert(info->magic == NATIVE_WINDOW_INFO_MAGIC);

      memset(nw, 0, sizeof(NXPL_NativeWindow));

#ifdef NXPL_PLATFORM_EXCLUSIVE
      if (__sync_fetch_and_add(&s_exclusiveNativeWindowCount, 1) != 0)
      {
         fprintf(stderr, "ERROR: Only one NXPL_NativeWindow is allowed in exclusive display mode. Second window create failed.\n");
         goto error;
      }
#endif

      nw->windowInfo = *info;

#ifndef NXPL_PLATFORM_EXCLUSIVE
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
#endif /* NXCLIENT_SUPPORT */

      nw->surfaceClient = NEXUS_SurfaceClient_Acquire(nw->clientID);
      if (!nw->surfaceClient)
      {
         printf("Failed to acquire compositing client surface for client id %d", nw->clientID);
         goto error;
      }

#ifdef NXCLIENT_SUPPORT
      /* default our SurfaceClient to blend */
      NEXUS_SurfaceComposition comp;
      NxClient_GetSurfaceClientComposition(nw->clientID, &comp);
      comp.colorBlend = info->colorBlend;
      comp.alphaBlend = info->alphaBlend;
      NxClient_SetSurfaceClientComposition(nw->clientID, &comp);
#endif /* NXCLIENT_SUPPORT */
#endif /* !defined(NXPL_PLATFORM_EXCLUSIVE) */

      char *val = getenv("V3D_DOUBLE_BUFFER");
      if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
         nw->numSurfaces = 2;
      else
         nw->numSurfaces = MAX_SWAP_BUFFERS;
   }

   return nw;

error:
   NXPL_DestroyNativeWindow(nw);

   return NULL;
}

void *NXPL_CreateNativeWindow(const NXPL_NativeWindowInfo *info)
{
   /* deprecated API */
   NXPL_NativeWindowInfoEXT infoEXT;
   NXPL_GetDefaultNativeWindowInfoEXT(&infoEXT);

   infoEXT.width    = info->width;
   infoEXT.height   = info->height;
   infoEXT.x        = info->x;
   infoEXT.y        = info->y;
   infoEXT.stretch  = info->stretch;
   infoEXT.clientID = info->clientID;
   infoEXT.zOrder   = info->zOrder;

   return NXPL_CreateNativeWindowEXT(&infoEXT);
}

void NXPL_DestroyNativeWindow(void *native)
{
   if (native != NULL)
   {
      NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;

#ifdef NXPL_PLATFORM_EXCLUSIVE
      __sync_fetch_and_sub(&s_exclusiveNativeWindowCount, 1);
#else
      if (nw->surfaceClient)
         NEXUS_SurfaceClient_Release(nw->surfaceClient);

#ifdef NXCLIENT_SUPPORT
      NxClient_Free(&nw->allocResults);
#endif
#endif /* NXPL_PLATFORM_EXCLUSIVE */

      free(nw);
   }
}

void NXPL_UpdateNativeWindowEXT(void *native, const NXPL_NativeWindowInfoEXT *info)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;

   if (info != NULL && nw != NULL)
      nw->windowInfo = *info;
}

void NXPL_UpdateNativeWindow(void *native, const NXPL_NativeWindowInfo *info)
{
   /* deprecated API */
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;

   if (info != NULL && nw != NULL)
   {
      nw->windowInfo.width = info->width;
      nw->windowInfo.height = info->height;
      nw->windowInfo.x = info->x;
      nw->windowInfo.y = info->y;
      nw->windowInfo.stretch = info->stretch;
      nw->windowInfo.clientID = info->clientID;
      nw->windowInfo.zOrder = info->zOrder;
   }
}

#ifndef NXPL_PLATFORM_EXCLUSIVE

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

#endif /* NXPL_PLATFORM_EXCLUSIVE */

void NXPL_GetDefaultPixmapInfoEXT(BEGL_PixmapInfoEXT *info)
{
   if (info != NULL)
   {
      memset(info, 0, sizeof(BEGL_PixmapInfoEXT));

      info->format    = BEGL_BufferFormat_INVALID;
      info->miplevels = 1;
      info->magic     = PIXMAP_INFO_MAGIC;
   }
}

void NXPL_SetStripedSurface(void *pixmapHandle, NEXUS_StripedSurfaceHandle striped,
                            BEGL_Colorimetry srcColorimetry)
{
   NXPL_Surface *s = (NXPL_Surface*)pixmapHandle;
   s->surface      = (NEXUS_SurfaceHandle)striped;
   s->colorimetry  = srcColorimetry;
}

bool NXPL_CreateCompatiblePixmapEXT(NXPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SurfaceHandle *surface, BEGL_PixmapInfoEXT *info)
{
   bool ok = false;
   NXPL_Surface  *s = (NXPL_Surface*)calloc(1, sizeof(NXPL_Surface));

   if (info != NULL && s != NULL)
   {
      assert(info->magic == PIXMAP_INFO_MAGIC);

      ok = CreateSurface(s, info->format, info->width, info->height, info->miplevels, info->secure, "Pixmap Surface");

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

bool NXPL_CreateCompatiblePixmap(NXPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SurfaceHandle *surface, BEGL_PixmapInfo *info)
{
   BEGL_PixmapInfoEXT   infoEXT;

   NXPL_GetDefaultPixmapInfoEXT(&infoEXT);

   infoEXT.width  = info->width;
   infoEXT.height = info->height;
   infoEXT.format = info->format;

   return NXPL_CreateCompatiblePixmapEXT(handle, pixmapHandle, surface, &infoEXT);
}

void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle handle, void *pixmapHandle)
{
   DestroySurface((NXPL_Surface*)pixmapHandle);
   free(pixmapHandle);
}
