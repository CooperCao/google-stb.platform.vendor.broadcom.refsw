/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "default_nexus.h"
#include "display_nexus.h"
#include "display_helpers.h"
#include "display_surface.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

#include "nexus_surface.h"
#include "nexus_striped_surface.h"

#include "display_interface.h"
#include "display_framework.h"
#include "../common/surface_interface_nexus.h"
#include "../common/nexus_heap_selection.h"

#include "display_nexus_exclusive.h"
#ifndef NXPL_PLATFORM_EXCLUSIVE
#include "display_nexus_multi.h"
#endif

#include "sched_nexus.h"

#include "platform_common.h"
#include "fence_queue.h"
#include "../common/debug_helper.h"
#include "../common/perf_event.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>

#include <malloc.h>
#include <memory.h>
#include <assert.h>
#include <pthread.h>
#include <stdint.h>

enum
{
   NATIVE_WINDOW_INFO_MAGIC = 0xABBA601D,
   PIXMAP_INFO_MAGIC        = 0x15EEB1A5
};

typedef struct WindowState
{
   NXPL_NativeWindow *native_window;
   DisplayFramework display_framework;
} WindowState;

static BEGL_BufferFormat DispGetPixmapFormat(void *context, void *pixmap)
{
   BEGL_BufferFormat format = BEGL_BufferFormat_INVALID;
   NEXUS_SurfaceHandle surface = (NEXUS_SurfaceHandle)pixmap;
   if (surface)
   {
      NEXUS_SurfaceStatus surfStatus = {};
      NEXUS_Surface_GetStatus(surface, &surfStatus);
      NexusToBeglFormat(&format, surfStatus.pixelFormat);
   }
   return format;
}

static NEXUS_MemoryBlockHandle CloneMemoryBlock(NEXUS_MemoryBlockHandle block,
      NEXUS_Addr *phys)
{
   if (!block)
      return NULL;

   NEXUS_MemoryBlockTokenHandle token = NEXUS_MemoryBlock_CreateToken(block);
   NEXUS_MemoryBlockHandle clone = NEXUS_MemoryBlock_Clone(token);
   if (clone)
   {
      NEXUS_Error err;
      err = NEXUS_MemoryBlock_LockOffset(clone, phys);
      if (err != NEXUS_SUCCESS)
      {
         NEXUS_MemoryBlock_Free(clone);
         clone = NULL;
      }
   }
   return clone;
}

static BEGL_Colorimetry NexusToBEGLColorPrimaries(NEXUS_MatrixCoefficients ncp)
{
   switch (ncp)
   {
   case NEXUS_MatrixCoefficients_eHdmi_RGB              : return BEGL_Colorimetry_RGB;
   case NEXUS_MatrixCoefficients_eItu_R_BT_709          : return BEGL_Colorimetry_BT_709;
   case NEXUS_MatrixCoefficients_eUnknown               : return BEGL_Colorimetry_Unknown;
   case NEXUS_MatrixCoefficients_eDvi_Full_Range_RGB    : return BEGL_Colorimetry_Dvi_Full_Range_RGB;
   case NEXUS_MatrixCoefficients_eFCC                   : return BEGL_Colorimetry_FCC;
   case NEXUS_MatrixCoefficients_eItu_R_BT_470_2_BG     : return BEGL_Colorimetry_BT_470_2_BG;
   case NEXUS_MatrixCoefficients_eSmpte_170M            : return BEGL_Colorimetry_Smpte_170M;
   case NEXUS_MatrixCoefficients_eSmpte_240M            : return BEGL_Colorimetry_Smpte_240M;
   case NEXUS_MatrixCoefficients_eXvYCC_709             : return BEGL_Colorimetry_XvYCC_709;
   case NEXUS_MatrixCoefficients_eXvYCC_601             : return BEGL_Colorimetry_XvYCC_601;
   case NEXUS_MatrixCoefficients_eItu_R_BT_2020_NCL     : return BEGL_Colorimetry_BT_2020_NCL;
   case NEXUS_MatrixCoefficients_eItu_R_BT_2020_CL      : return BEGL_Colorimetry_BT_2020_CL;
   case NEXUS_MatrixCoefficients_eHdmi_Full_Range_YCbCr : return BEGL_Colorimetry_Hdmi_Full_Range_YCbCr;
   default                                              : return BEGL_Colorimetry_XvYCC_601;
   }
}

static NEXUS_MemoryBlockHandle CloneSurfaceMemory(
      NEXUS_SurfaceHandle surface, BEGL_SurfaceInfo *info)
{
   if (!surface)
      return NULL;

   //this will fail if surface is not a valid NEXUS_SurfaceHandle
   NEXUS_SurfaceMemory surfMemory;
   if (NEXUS_Surface_GetMemory(surface, &surfMemory) != NEXUS_SUCCESS)
      return NULL;

   NEXUS_SurfaceCreateSettings createSettings;
   NEXUS_Surface_GetCreateSettings(surface, &createSettings);
   if (!createSettings.compatibility.graphicsv3d)
      return NULL;

   NEXUS_SurfaceStatus surfStatus;
   NEXUS_Surface_GetStatus(surface, &surfStatus);
   BEGL_BufferFormat format;
   if (!NexusToBeglFormat(&format, surfStatus.pixelFormat))
      return NULL;

   NEXUS_SurfaceMemoryProperties memProperties;
   NEXUS_Addr physicalOffset = 0;
   NEXUS_Surface_GetMemoryProperties(surface, &memProperties);
   NEXUS_MemoryBlockHandle clone = CloneMemoryBlock(
         memProperties.pixelMemory, &physicalOffset);
   if (clone)
   {
      info->format         = format;
      info->colorimetry    = BEGL_Colorimetry_RGB;
      info->width          = surfStatus.width;
      info->height         = surfStatus.height;
      info->pitchBytes     = surfStatus.pitch;
      info->contiguous     = true; // Nexus surfaces are always contiguous
      info->physicalOffset = physicalOffset;
      info->cachedAddr     = surfMemory.buffer;
      info->secure         = surfMemory.buffer == NULL;
      info->byteSize       = surfMemory.bufferSize;
      info->miplevels      = createSettings.mipLevel + 1;
   }
   return clone;
}

static NEXUS_MemoryBlockHandle CloneStripedSurfaceMemory(
      NEXUS_StripedSurfaceHandle stripedSurface, uint32_t plane,
      BEGL_SurfaceInfo *info)
{
   if (!stripedSurface || plane > 1)
      return NULL;

   NEXUS_StripedSurfaceCreateSettings cs;
   NEXUS_StripedSurface_GetCreateSettings(stripedSurface, &cs);

   BEGL_BufferFormat lumaFormat, chromaFormat;
   if (!NexusToBeglFormat(&lumaFormat, cs.lumaPixelFormat) ||
         !NexusToBeglFormat(&chromaFormat, cs.chromaPixelFormat))
      return NULL;

   const uint32_t byteWidth = cs.stripedWidth
         * (cs.imageWidth + cs.stripedWidth - 1) / cs.stripedWidth;

   NEXUS_MemoryBlockHandle clone;
   switch (plane)
   {
   case 0:
   {
      NEXUS_Addr lumaPhys;
      clone = CloneMemoryBlock(cs.lumaBuffer, &lumaPhys);
      if (clone)
      {
         info->format = lumaFormat;
         info->physicalOffset = lumaPhys + cs.lumaBufferOffset;
         info->stripedHeight = cs.lumaStripedHeight;
         info->byteSize = byteWidth * cs.lumaStripedHeight;
      }
      break;
   }
   case 1:
   {
      NEXUS_Addr chromaPhys;
      clone = CloneMemoryBlock(cs.chromaBuffer, &chromaPhys);
      if (clone)
      {
         info->format = chromaFormat;
         info->physicalOffset = chromaPhys + cs.chromaBufferOffset;
         info->stripedHeight = cs.chromaStripedHeight;
         info->byteSize = byteWidth * cs.chromaStripedHeight;
      }
      break;
   }
   default:
      clone = NULL;
      break;
   }

   if (clone)
   {
      info->colorimetry    = NexusToBEGLColorPrimaries(cs.matrixCoefficients);
      info->width          = cs.imageWidth;
      info->height         = cs.imageHeight;
      info->pitchBytes     = 0;
      info->contiguous     = true;   // Nexus surfaces are always contiguous
      info->cachedAddr     = NULL;   // Sand video can't be mapped
      info->secure         = false;  // TODO : I can't find a way to determine this here
      info->miplevels      = 1;
      info->stripeWidth    = cs.stripedWidth;
   }
   return clone;
}

static BEGL_NativeBuffer DispSurfaceAcquire(void *context, uint32_t target,
      void *eglObject, uint32_t plane, BEGL_SurfaceInfo *info)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(plane);

   switch (target)
   {
   case BEGL_SWAPCHAIN_BUFFER:
      assert(plane == 0);
      return eglObject && plane == 0 ?
            (BEGL_NativeBuffer)CloneSurfaceMemory(
                  ((NXPL_Surface *)eglObject)->surface, info) : NULL;
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_PIXMAP_BUFFER:
      assert(plane == 0);
      return plane == 0 ? (BEGL_NativeBuffer)CloneSurfaceMemory(
            (NEXUS_SurfaceHandle)eglObject, info) : NULL;
   case EGL_NEXUS_STRIPED_SURFACE_BRCM:
      assert(plane < 2);
      return (BEGL_NativeBuffer)CloneStripedSurfaceMemory(
            (NEXUS_StripedSurfaceHandle)eglObject, plane, info);
   default:
      return NULL;
   }
}

static BEGL_Error DispSurfaceRelease(void *context, uint32_t target,
      uint32_t plane, BEGL_NativeBuffer buffer)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(plane);

   switch (target)
   {
   case EGL_NEXUS_STRIPED_SURFACE_BRCM:
   case EGL_NATIVE_PIXMAP_KHR:
   case BEGL_SWAPCHAIN_BUFFER:
   case BEGL_PIXMAP_BUFFER:
   {
      NEXUS_MemoryBlockHandle memoryBlock = (NEXUS_MemoryBlockHandle)buffer;
      NEXUS_MemoryBlock_UnlockOffset(memoryBlock);
      NEXUS_MemoryBlock_Free(memoryBlock);
      return BEGL_Success;
   }
   default:
      return BEGL_Fail;
   }
}

static_assert(sizeof(WindowInfo) == sizeof(NXPL_NativeWindowInfoEXT), "sizeof(WindowInfo) & sizeof(NXPL_NativeWindowInfoEXT) need to match");

static BEGL_SwapchainBuffer DispGetNextSurface(
   void *context,
   void *nativeWindow,
   BEGL_BufferFormat format,
   bool secure,
   int *age,
   int *fence
)
{
   NXPL_DisplayContext *dc    = (NXPL_DisplayContext*)context;
   WindowState       *state = (WindowState *)nativeWindow;

   if (state == NULL)
      return NULL;

   uint64_t before = PerfGetTimeNow();
   NXPL_Surface *nxpl_surface = (NXPL_Surface *)DisplayFramework_GetNextSurface(
         &state->display_framework, format, secure, age, fence,
         (WindowInfo *)&state->native_window->windowInfo);
   if (!nxpl_surface)
      return NULL;

   static uint32_t eventID = 0;
   NEXUS_SurfaceHandle surface = nxpl_surface->surface;

   PerfAddEventWithTime(dc->eventContext, PERF_EVENT_TRACK_QUEUE, PERF_EVENT_DEQUEUE, eventID,
                        BCM_EVENT_BEGIN, before, (uintptr_t)surface, *fence);
   PerfAddEvent(dc->eventContext, PERF_EVENT_TRACK_QUEUE, PERF_EVENT_DEQUEUE, eventID++,
                BCM_EVENT_END, (uintptr_t)surface, *fence);

   return (BEGL_SwapchainBuffer)nxpl_surface;
}

static BEGL_Error DispDisplaySurface(void *context, void *nativeWindow,
      BEGL_SwapchainBuffer nativeSurface, int fence, int interval)
{
   NXPL_DisplayContext *dc    = (NXPL_DisplayContext*)context;
   WindowState       *state = (WindowState *) nativeWindow;

   if (state && nativeSurface)
   {
      static uint32_t eventID = 0;
      NXPL_Surface *nxpl_surface = (NXPL_Surface *)nativeSurface;
      NEXUS_SurfaceHandle surface = nxpl_surface->surface;

      PerfAddEvent(dc->eventContext, PERF_EVENT_TRACK_QUEUE, PERF_EVENT_QUEUE, eventID, BCM_EVENT_BEGIN,
                   (uintptr_t)surface, (int32_t)fence, (int32_t)interval);

      DisplayFramework_DisplaySurface(&state->display_framework, nativeSurface, fence, interval,
         (WindowInfo *)&state->native_window->windowInfo);

      PerfAddEvent(dc->eventContext, PERF_EVENT_TRACK_QUEUE, PERF_EVENT_QUEUE, eventID++, BCM_EVENT_END,
                    (uintptr_t)surface, (int32_t)fence, (int32_t)interval);

      return BEGL_Success;
   }
   else
      return BEGL_Fail;
}

static BEGL_Error DispCancelSurface(void *context, void *nativeWindow,
      BEGL_SwapchainBuffer nativeSurface, int fence)
{
   UNUSED(context);
   WindowState *state = (WindowState *) nativeWindow;
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
   WindowState *state = NULL;
   unsigned buffers = MAX_SWAP_BUFFERS;

   char *val = getenv("V3D_DOUBLE_BUFFER");
   if (val && (val[0] == 't' || val[0] == 'T' || val[0] == '1'))
      buffers = 2;

   DisplayFramework *df = NULL;
   if (ctx && nw)
   {
      state = calloc(1, sizeof(*state));
      if (state)
      {
         df = (DisplayFramework *)&state->display_framework;
         InitFenceInterface(&df->fence_interface, ctx->schedIface);
         SurfaceInterface_InitNexus(&df->surface_interface);

#ifdef NXPL_PLATFORM_EXCLUSIVE
         if (!DisplayInterface_InitNexusExclusive(&df->display_interface,
               &df->fence_interface, &nw->windowInfo,
               ctx->display, &nw->bound, ctx->eventContext))
            goto error_display;
#else
         if (!DisplayInterface_InitNexusMulti(&df->display_interface,
               &df->fence_interface,
               buffers, nw, ctx->eventContext))
            goto error_display;
#endif

         state->native_window = nw;

         if (!DisplayFramework_Start(df, buffers))
            goto error_framework;
      }
   }
   return state;

error_framework:
   Interface_Destroy(&df->display_interface.base);
   Interface_Destroy(&df->surface_interface.base);
   Interface_Destroy(&df->fence_interface.base);
error_display:
   free(state);
   return NULL;
}

static BEGL_Error DispWindowPlatformStateDestroy(void *context, void *windowState)
{
   UNUSED(context);
   WindowState *state = (WindowState *) windowState;

   if (state)
   {
      DisplayFramework *df = (DisplayFramework *)&state->display_framework;
      DisplayFramework_Stop(df);
      Interface_Destroy(&df->display_interface.base);
      Interface_Destroy(&df->surface_interface.base);
      Interface_Destroy(&df->fence_interface.base);

#ifndef NDEBUG
      /* catch some cases of use after free */
      memset(state, 0, sizeof(*state));
#endif
      free(state);
   }
   return BEGL_Success;
}

static const char *DispGetDisplayExtensions(void *context)
{
   BSTD_UNUSED(context);
   return "EGL_BRCM_image_nexus_striped_surface";
}

BEGL_DisplayInterface *CreateDisplayInterface(
      NEXUS_DISPLAYHANDLE display,
      NXPL_DisplayContext *ctx,
      BEGL_SchedInterface *schedIface,
      EventContext *eventContext)
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
         ctx->eventContext = eventContext;

         disp->context = (void*)ctx;

         disp->GetPixmapFormat            = DispGetPixmapFormat;
         disp->SurfaceAcquire             = DispSurfaceAcquire;
         disp->SurfaceRelease             = DispSurfaceRelease;
         disp->GetNextSurface             = DispGetNextSurface;
         disp->DisplaySurface             = DispDisplaySurface;
         disp->CancelSurface              = DispCancelSurface;
         disp->WindowPlatformStateCreate  = DispWindowPlatformStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowPlatformStateDestroy;
         disp->GetDisplayExtensions       = DispGetDisplayExtensions;
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

#ifdef NXPL_PLATFORM_EXCLUSIVE
static uint32_t s_exclusiveNativeWindowCount = 0;
#endif

void NXPL_GetDefaultNativeWindowInfoEXT(NXPL_NativeWindowInfoEXT *info)
{
   if (info != NULL)
   {
      memset(info, 0, sizeof(NXPL_NativeWindowInfoEXT));
#if NEXUS_HAS_GRAPHICS2D
      static NEXUS_BlendEquation colorBlend = { NEXUS_BlendFactor_eSourceColor, NEXUS_BlendFactor_eSourceAlpha, false, NEXUS_BlendFactor_eDestinationColor, NEXUS_BlendFactor_eInverseSourceAlpha, false, NEXUS_BlendFactor_eZero };
      static NEXUS_BlendEquation alphaBlend = { NEXUS_BlendFactor_eSourceAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eDestinationAlpha, NEXUS_BlendFactor_eOne, false, NEXUS_BlendFactor_eZero };
      info->colorBlend = colorBlend;
      info->alphaBlend = alphaBlend;
#endif
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

      if (nw->videoClient)
         NEXUS_SurfaceClient_Release(nw->videoClient);

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

void NXPL_SetDisplayType(NXPL_PlatformHandle handle __attribute__((unused)), NXPL_DisplayType type __attribute__((unused)))
{
   /* NOOP */
}

NXPL_EXPORT void NXPL_UpdateNativeWindowDisplayType(void *native, NXPL_DisplayType type)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;

   if (nw != NULL)
      nw->windowInfo.type = type;
}

#ifndef NXPL_PLATFORM_EXCLUSIVE

NEXUS_SurfaceClientHandle NXPL_CreateVideoWindowClient(void *native, unsigned windowId)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;
   NEXUS_SurfaceClientHandle ret = NULL;

   if (nw && nw->surfaceClient)
   {
      ret = NEXUS_SurfaceClient_AcquireVideoWindow(nw->surfaceClient, windowId);
      nw->videoClient = ret;
   }
   return ret;
}

void NXPL_ReleaseVideoWindowClient(NEXUS_SurfaceClientHandle handle)
{
   NEXUS_SurfaceClient_ReleaseVideoWindow(handle);
}

void NXPL_ReleaseVideoWindowClientEXT(void *native)
{
   NXPL_NativeWindow *nw = (NXPL_NativeWindow*)native;
   NEXUS_SurfaceClientHandle videoClient = nw->videoClient;
   nw->videoClient = NULL;
   if (videoClient)
      NEXUS_SurfaceClient_ReleaseVideoWindow(videoClient);
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

bool NXPL_CreateCompatiblePixmapEXT(NXPL_PlatformHandle handle, void **pixmapHandle, NEXUS_SurfaceHandle *surface, BEGL_PixmapInfoEXT *info)
{
   if (!info || info->magic != PIXMAP_INFO_MAGIC)
      return false;

   NEXUS_SurfaceCreateSettings surfSettings;
   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);

   if (!BeglToNexusFormat(&surfSettings.pixelFormat, info->format))
      return false;

   // Note we allow NEXUS_Surface_Create to allocate the memory as
   // it will calculate the required size for UIF surfaces as well as
   // raster and we don't want to duplicate the logic for that here.
   surfSettings.compatibility.graphicsv3d = true;

   surfSettings.width = info->width;
   surfSettings.height = info->height;
   // NEXUS surface creation specifies the mip level number at the beginning
   // of the surface, not the number of miplevels.
   surfSettings.mipLevel = info->miplevels - 1;
   surfSettings.heap = info->secure ? GetSecureHeap() : GetDisplayHeap(0);
   surfSettings.alignment = 12; // log2(4096)

   NEXUS_SurfaceHandle nexusSurface = NEXUS_Surface_Create(&surfSettings);

   if (nexusSurface)
   {
      if (pixmapHandle != NULL)
         *pixmapHandle = nexusSurface;

      if (surface != NULL)
         *surface = nexusSurface;
   }

   return nexusSurface != NULL;
}

void NXPL_DestroyCompatiblePixmap(NXPL_PlatformHandle handle, void *pixmapHandle)
{
   NEXUS_SurfaceHandle nexusSurface = (NEXUS_SurfaceHandle)pixmapHandle;
   if (nexusSurface)
         NEXUS_Surface_Destroy(nexusSurface);
}
