/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <memory.h>
#include <assert.h>
#include <semaphore.h>

#include "default_directfb.h"
#include "private_directfb.h"

#include "../common/fence_interface.h"

#include "nexus_memory.h"
#include "nexus_platform.h"
#include "nexus_base_mmap.h"
#include "sched_nexus.h"

#include <bkni.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_brcm.h>

#define MAX_SWAP_BUFFERS   3

/* We will queue ahead this many frames. This doesn't have to match MAX_SWAP_BUFFERS. */
#define DISPLAY_QUEUE_SIZE MAX_SWAP_BUFFERS

typedef struct
{
   IDirectFB               *dfb;
   BEGL_SchedInterface     *schedIface;
   FenceInterface          fenceIface;
} DBPL_DisplayContext;

/* DBPL_DisplatQueueItem */
typedef struct
{
   bool                 endDisplayProcess;
   int                  isRenderedFence;
   int                  interval;
} DBPL_DisplayQueueItem;

/* DBPL_SurfaceContainer */
typedef struct
{
   IDirectFBSurface        *windowsurface;
   BEGL_SurfaceInfo        settings;
   int                     availableToRenderFence;
} DBPL_SurfaceContainer;

typedef struct {
   /* Main thread data */
   IDirectFBSurface        *windowsurface;
   DBPL_SurfaceContainer   surfaces[MAX_SWAP_BUFFERS];
   unsigned int            numSurfaces;
   unsigned int            nextSurface;
   unsigned int            displayQueueWritePt;
   unsigned int            numSurfacesGivenAway;

   /* Display thread data */
   unsigned int            displayQueueReadPt;


   /* Shared data */
   DBPL_DisplayQueueItem   displayQueue[DISPLAY_QUEUE_SIZE];   /* The display queue                      */
   volatile unsigned int   displayQueueEntries;                /* Number of entries in the display queue */
   volatile unsigned int   owners;                             /* Count of threads that own this window  */
   volatile unsigned int   vsyncCount;
   BEGL_BufferFormat       format;


   /* Thread data and mutex */
   pthread_t               displayThread;
   BKNI_MutexHandle        mutex;
   BKNI_EventHandle        queuedEvent;
   BKNI_EventHandle        dequeuedEvent;
   BKNI_EventHandle        referenceDecEvent;


} DBPL_NativeWindow;

/* DBPL_DisplayThreadParams */
typedef struct
{
   DBPL_DisplayContext     *context;
   DBPL_NativeWindow       *nativeWindow;
} DBPL_DisplayThreadParams;

/*****************************************************************************
 * Display driver interface
 *****************************************************************************/

/* macro for a safe call to DirectFB functions */
#define DFBCHECK(x...) \
     {                                                                \
          int err = x;                                                \
          if (err != DFB_OK) {                                        \
               fprintf( stderr, "%s <%d>:\n\t", __FILE__, __LINE__ ); \
               DirectFBErrorFatal( #x, err );                         \
          }                                                           \
     }

static void WaitFence(DBPL_DisplayContext *ctx, int fence)
{
   if (fence < 0)
      return;

   assert(ctx->schedIface != NULL);
   FenceInterface_Wait(&ctx->fenceIface, fence, 2000);
   FenceInterface_Destroy(&ctx->fenceIface, &fence);
}

static bool DirectFBToBeglFormat(DFBSurfacePixelFormat dfbFormat, BEGL_BufferFormat *bufferFormat)
{
   bool ok = true;
   *bufferFormat = BEGL_BufferFormat_INVALID;

   switch (dfbFormat)
   {
   case DSPF_ABGR:
      *bufferFormat = BEGL_BufferFormat_eA8B8G8R8;
      break;
   case DSPF_RGB32:
      *bufferFormat = BEGL_BufferFormat_eX8B8G8R8;
      break;
   case DSPF_RGB16:
      *bufferFormat = BEGL_BufferFormat_eR5G6B5;
      break;
//   case DSPF_UYVY:
//      *bufferFormat = BEGL_BufferFormat_eYUV422;
//      break;
   default:
      ok = false;
      *bufferFormat = BEGL_BufferFormat_INVALID;
      break;
   }

   return ok;
}

static void ReleaseWindow(DBPL_NativeWindow *nw)
{
   bool doFree = false;

   BKNI_AcquireMutex(nw->mutex);
   nw->owners--;
   if (nw->owners == 0)
      doFree = true;
   BKNI_ReleaseMutex(nw->mutex);

   if (doFree)
   {
      if (nw->mutex != NULL)
         BKNI_DestroyMutex(nw->mutex);

      if (nw->queuedEvent != NULL)
         BKNI_DestroyEvent(nw->queuedEvent);

      if (nw->dequeuedEvent != NULL)
         BKNI_DestroyEvent(nw->dequeuedEvent);

      if (nw->referenceDecEvent != NULL)
         BKNI_DestroyEvent(nw->referenceDecEvent);

      free(nw);
   }
}

static void *RunDisplayThread(void *p)
{
   DBPL_DisplayThreadParams   *params  = (DBPL_DisplayThreadParams *)p;
   DBPL_NativeWindow          *nw      = params->nativeWindow;
   DBPL_DisplayContext        *ctx     = (DBPL_DisplayContext *)params->context;

   IDirectFBSurface           *surface;
   unsigned int               popPoint;

   BKNI_AcquireMutex(nw->mutex);

   /* Reset the native window -- this could be a second invokation of the display loop */
   nw->displayQueueEntries = 0;
   nw->displayQueueReadPt  = 0;
   nw->vsyncCount          = 0;
   nw->nextSurface         = 0;
   nw->displayQueueWritePt = 0;  /* In main thread? */

   nw->owners++;

   BKNI_ReleaseMutex(nw->mutex);

   while (1)
   {
      unsigned int entries;

      /* Is there anything to display? */
      BKNI_AcquireMutex(nw->mutex);

      entries = nw->displayQueueEntries;

      /* Wait until there is a surface in the queues */
      /* something to display */
      while (entries == 0)
      {
         BKNI_ReleaseMutex(nw->mutex);
         BKNI_WaitForEvent(nw->queuedEvent, BKNI_INFINITE);
         BKNI_AcquireMutex(nw->mutex);

         entries = nw->displayQueueEntries;
      }

      /* Is this the termination entry? */
      if (nw->displayQueue[nw->displayQueueReadPt].endDisplayProcess)
      {
         BKNI_ReleaseMutex(nw->mutex);
         /* End of the display thread */
         break;
      }

      /* Wait for the buffer rendering to complete */
      BKNI_ReleaseMutex(nw->mutex);
      WaitFence(ctx, nw->displayQueue[nw->displayQueueReadPt].isRenderedFence);
      BKNI_AcquireMutex(nw->mutex);

      surface = nw->windowsurface;

      DFBCHECK(nw->windowsurface->Unlock( nw->windowsurface ));

      nw->numSurfacesGivenAway--;
      BKNI_SetEvent(nw->referenceDecEvent);

      /* only >=1 or 0 */
      if (nw->displayQueue[nw->displayQueueReadPt].interval > 0)
      {
         DFBCHECK(surface->Flip( surface, NULL, DSFLIP_WAITFORSYNC ));
      }
      else
      {
         DFBCHECK(surface->Flip( surface, NULL, DSFLIP_NONE ));
      }

      nw->displayQueueEntries--;

      BKNI_SetEvent(nw->dequeuedEvent);

      nw->displayQueueReadPt++;
      if (nw->displayQueueReadPt >= DISPLAY_QUEUE_SIZE)
         nw->displayQueueReadPt = 0;

      BKNI_ReleaseMutex(nw->mutex);
   }

   {
      int i;

      for (i = 0; i < nw->numSurfaces; ++i)
      {
         int   onDisplayFence = nw->surfaces[i].availableToRenderFence;

         FenceInterface_Signal(&ctx->fenceIface, onDisplayFence);
      }
   }

   ReleaseWindow(nw);
   free(params);

   return NULL;
}

/* Called to determine current size of the window referenced by the opaque window handle.
 * This is needed by EGL in order to know the size of a native 'window'. */
static BEGL_Error DispWindowGetInfo(void *context,
                                    void *nativeWindow,
                                    BEGL_WindowInfoFlags flags,
                                    BEGL_WindowInfo *info)
{
   DBPL_NativeWindow    *nw  = (DBPL_NativeWindow*)nativeWindow;
   BEGL_Error           ret  = BEGL_Fail;

   if (info != NULL)
   {
      /* Make sure we return something sensible */
      info->width  = 0;
      info->height = 0;
      info->swapchain_count = 0;

      if (nw != NULL)
      {
         IDirectFBSurface *dfbSurface = nw->windowsurface;
         DFBCHECK(dfbSurface->GetSize( dfbSurface, &info->width, &info->height ));

         if (dfbSurface)
         {
            DFBSurfaceCapabilities ret_caps;
            DFBCHECK(dfbSurface->GetCapabilities( dfbSurface, &ret_caps ));
            if (ret_caps & DSCAPS_DOUBLE)
               info->swapchain_count = 2;
            else if (ret_caps & DSCAPS_TRIPLE)
               info->swapchain_count = 3;
            else
               info->swapchain_count = 1;

            ret = BEGL_Success;
         }
      }
   }

   /* printf("[%s] - width = %d, height = %d, swapchain_count = %d, ret_format = %d\n",
      __FUNCTION__, info->width, info->height, info->swapchain_count, info->format); */

   return ret;
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
   DBPL_SurfaceContainer      *sc = (DBPL_SurfaceContainer*)nativeSurface;
   bool                       ok       = false;
   DBPL_DisplayContext        *ctx     = (DBPL_DisplayContext*)context;

   if (sc != NULL && info != NULL)
   {
      *info = sc->settings;

      ok = true;
   }

   return ok ? BEGL_Success : BEGL_Fail;
}

static BEGL_Error DispGetNextSurface(
   void *context,
   void *nativeWindow,
   BEGL_BufferFormat format,
   BEGL_BufferFormat *actualFormat,
   void **nativeSurface,
   bool secure,
   int *fence)
{
   DBPL_NativeWindow       *nw  = (DBPL_NativeWindow*)nativeWindow;
   DBPL_DisplayContext     *ctx = (DBPL_DisplayContext*)context;
   DBPL_SurfaceContainer   *sc;
   DFBSurfacePixelFormat   dfbFormat;

   if (nw == NULL || nativeSurface == NULL)
      return BEGL_Fail;

   BKNI_AcquireMutex(nw->mutex);

   sc = &nw->surfaces[nw->nextSurface];
   *nativeSurface = (void*)sc;

   if (nw->numSurfacesGivenAway == 0)
   {
      *fence = -1;
      sc->availableToRenderFence = -1;
   }
   else
   {
      /* We have given out all our buffers, wait for a free one */
      /* We need to keep one surface on the display */
      /* Only double buffered for the moment */
      while (nw->numSurfacesGivenAway >= 1)
      {
         BKNI_ResetEvent(nw->referenceDecEvent);
         BKNI_ReleaseMutex(nw->mutex);
         BKNI_WaitForEvent(nw->referenceDecEvent, 20);
         BKNI_AcquireMutex(nw->mutex);
      }
   }

   DFBCHECK(sc->windowsurface->Lock( sc->windowsurface, DSLF_WRITE, &sc->settings.cachedAddr, &sc->settings.pitchBytes));
   sc->settings.physicalOffset   = NEXUS_AddrToOffset(sc->settings.cachedAddr);
   sc->settings.byteSize = sc->settings.pitchBytes * sc->settings.height;

   DFBCHECK(sc->windowsurface->GetSize( sc->windowsurface, &sc->settings.width, &sc->settings.height ));
   DFBCHECK(sc->windowsurface->GetPixelFormat ( sc->windowsurface, &dfbFormat));

   if (!DirectFBToBeglFormat(dfbFormat, &sc->settings.format))
   {
      BKNI_ReleaseMutex(nw->mutex);
      return BEGL_Fail;
   }

   nw->numSurfacesGivenAway++;

   /* Increment nextSurface */
   nw->nextSurface++;
   if (nw->nextSurface >= nw->numSurfaces)
      nw->nextSurface = 0;

   BKNI_ReleaseMutex(nw->mutex);

   return BEGL_Success;
}

static BEGL_Error QueueSurface(void *context, void *nativeWindow, void *nativeSurface, int fence, int interval, bool endDisplayProcess)
{
   DBPL_NativeWindow       *nw = (DBPL_NativeWindow*)nativeWindow;
   DBPL_SurfaceContainer   *nc = (DBPL_SurfaceContainer*)nativeSurface;
   int                     i;
   unsigned int            surfIndex = 0;
   unsigned int            queueSize = DISPLAY_QUEUE_SIZE;

   BKNI_AcquireMutex(nw->mutex);
   queueSize = nw->displayQueueEntries;
   BKNI_ReleaseMutex(nw->mutex);

   /* Wait for the queue to not be full */
   while (queueSize == DISPLAY_QUEUE_SIZE)
   {
      BKNI_WaitForEvent(nw->dequeuedEvent, BKNI_INFINITE);

      BKNI_AcquireMutex(nw->mutex);
      queueSize = nw->displayQueueEntries;
      BKNI_ReleaseMutex(nw->mutex);
   }

   BKNI_AcquireMutex(nw->mutex);

   /* Push to display queue */
   nw->displayQueue[nw->displayQueueWritePt].endDisplayProcess = endDisplayProcess;
   nw->displayQueue[nw->displayQueueWritePt].isRenderedFence = fence;
   nw->displayQueue[nw->displayQueueWritePt].interval = interval;

   nw->displayQueueEntries++;

   nw->displayQueueWritePt++;
   if (nw->displayQueueWritePt >= DISPLAY_QUEUE_SIZE)
      nw->displayQueueWritePt = 0;

   BKNI_SetEvent(nw->queuedEvent);

   BKNI_ReleaseMutex(nw->mutex);

   return BEGL_Success;
}


static BEGL_Error DispDisplaySurface(void *context, void *nativeWindow, void *nativeSurface, int fence, int interval)
{
   BSTD_UNUSED(context);
   return QueueSurface(context, nativeWindow, nativeSurface, fence, interval, false);
}

/* This is called only once per native window */
static BEGL_Error CreateSwapChainSurfaces(DBPL_DisplayContext *context, DBPL_NativeWindow *nw)
{
   unsigned int             i;
   DBPL_DisplayThreadParams *params = NULL;

   for (i = 0; i < nw->numSurfaces; i++)
   {
      if (nw->surfaces[i].windowsurface == NULL)
      {
         DFBSurfacePixelFormat    dfbFormat;
         DBPL_SurfaceContainer   *sc = &nw->surfaces[i];

         BKNI_Memset(sc, 0, sizeof(DBPL_SurfaceContainer));

         sc->windowsurface = nw->windowsurface;
         sc->availableToRenderFence = -1;

         DFBCHECK(sc->windowsurface->GetSize( sc->windowsurface, &sc->settings.width, &sc->settings.height ));
         DFBCHECK(sc->windowsurface->GetPixelFormat ( sc->windowsurface, &dfbFormat));

         if (!DirectFBToBeglFormat(dfbFormat, &sc->settings.format))
            goto cleanup;

         if (sc->windowsurface == NULL)
            goto cleanup;
      }
   }

   /* Create a display thread for this native window */
   params = (DBPL_DisplayThreadParams*)malloc(sizeof(DBPL_DisplayThreadParams));
   params->context      = context;
   params->nativeWindow = nw;

   if (pthread_create(&nw->displayThread, NULL, &RunDisplayThread, params) != 0)
      goto cleanup;

   /* Wait for thread to really start */
   {
      uint32_t owners = 0;

      while (owners < 2)
      {
         BKNI_AcquireMutex(nw->mutex);
         owners = nw->owners;
         BKNI_ReleaseMutex(nw->mutex);

         if (owners < 2)
            usleep(10000);
      }
   }

   return BEGL_Success;

cleanup:
   if (params)
      free(params);

   for (i = 0; i < nw->numSurfaces; i++)
        nw->surfaces[i].windowsurface = NULL;

   return BEGL_Fail;
}

static void DestroySwapChainSurfaces(DBPL_DisplayContext *context, DBPL_NativeWindow *nw)
{
   unsigned int   i;

   if (context == NULL || nw == NULL)
      return;

   /* Wait for display thread to end */
   pthread_join(nw->displayThread, NULL);

   /* Release all the surfaces */
   for (i = 0; i < nw->numSurfaces; i++)
        nw->surfaces[i].windowsurface = NULL;
}


static void *DispWindowStateCreate(void *context, void *nativeWindow)
{
   DBPL_NativeWindow       *nw  = (DBPL_NativeWindow*)malloc(sizeof(DBPL_NativeWindow));
   DBPL_DisplayContext     *ctx = (DBPL_DisplayContext*)context;

   if (nw != NULL)
   {
      char                 *val;
      BERR_Code            err = BERR_SUCCESS;

      memset(nw, 0, sizeof(DBPL_NativeWindow));

      err = BKNI_CreateMutex(&nw->mutex);
      if (err != BERR_SUCCESS)
         goto error;

      err = BKNI_CreateEvent(&nw->queuedEvent);
      if (err != BERR_SUCCESS)
         goto error;

      err = BKNI_CreateEvent(&nw->dequeuedEvent);
      if (err != BERR_SUCCESS)
         goto error;

      err = BKNI_CreateEvent(&nw->referenceDecEvent);
      if (err != BERR_SUCCESS)
         goto error;

      IDirectFBSurface *dfbSurface = (IDirectFBSurface *)nativeWindow;
      {
         DFBSurfaceCapabilities ret_caps;
         DFBCHECK(dfbSurface->GetCapabilities( dfbSurface, &ret_caps ));
         if (ret_caps & DSCAPS_DOUBLE)
            nw->numSurfaces = 2;
         else if (ret_caps & DSCAPS_TRIPLE)
            nw->numSurfaces = 3;
         else
            nw->numSurfaces = 1;
      }

      nw->windowsurface          = dfbSurface;
      nw->owners                 = 1;
      nw->numSurfacesGivenAway   = 0;

      if (CreateSwapChainSurfaces(ctx, nw) != BEGL_Success)
         goto error;
   }

   return nw;

error:
   if (nw)
   {
      if (nw->mutex != NULL)
         BKNI_DestroyMutex(nw->mutex);

      if (nw->queuedEvent != NULL)
         BKNI_DestroyEvent(nw->queuedEvent);

      if (nw->dequeuedEvent != NULL)
         BKNI_DestroyEvent(nw->dequeuedEvent);

      if (nw->referenceDecEvent != NULL)
         BKNI_DestroyEvent(nw->referenceDecEvent);

      free(nw);
   }

   return NULL;
}

static BEGL_Error DispWindowStateDestroy(void *context, void *windowState)
{
   if (windowState != NULL)
   {
      DBPL_NativeWindow *nw = (DBPL_NativeWindow*)windowState;

      bool doFree = false;

      BKNI_AcquireMutex(nw->mutex);

      nw->owners--;
      if (nw->owners == 0)
         doFree = true;
      BKNI_ReleaseMutex(nw->mutex);

      if (doFree)
      {
         if (nw->mutex != NULL)
            BKNI_DestroyMutex(nw->mutex);

         if (nw->queuedEvent != NULL)
            BKNI_DestroyEvent(nw->queuedEvent);

         if (nw->dequeuedEvent != NULL)
            BKNI_DestroyEvent(nw->dequeuedEvent);

         if (nw->referenceDecEvent != NULL)
            BKNI_DestroyEvent(nw->referenceDecEvent);

         free(nw);
      }

      return BEGL_Success;
   }
   return BEGL_Fail;
}

static BEGL_Error PostPoison(void *context, void *nativeWindow)
{
   return QueueSurface(context, nativeWindow, NULL, -1, 1, true);
}

static BEGL_Error DispCancelSurface(void *context, void *nativeWindow,
      void *nativeSurface, int fence)
{
   DBPL_DisplayContext  *ctx = (DBPL_DisplayContext *)context;
   DBPL_NativeWindow *nw  = (DBPL_NativeWindow*)nativeWindow;

   if (context == NULL || nw == NULL)
      return BEGL_Fail;

   WaitFence(ctx, fence);

   /* Send poison into the display queue */
   PostPoison(context, nw);

   /* Wait for display thread to end */
   if (nw->displayThread)
      pthread_join(nw->displayThread, NULL);

   return BEGL_Success;
}

static bool  DisplayPlatformSupported(void *context, uint32_t platform)
{
   BSTD_UNUSED(context);
   return platform == EGL_PLATFORM_NEXUS_BRCM;
}

bool DispSetDefaultDisplay(void *context, void *display)
{
   BSTD_UNUSED(context);
   BSTD_UNUSED(display);
   return true;
}

static const char *GetClientExtensions(void *context)
{
   BSTD_UNUSED(context);
   return "EGL_BRCM_platform_nexus";
}

BEGL_DisplayInterface *CreateDirectFBDisplayInterface(IDirectFB *dfb,
                                             BEGL_SchedInterface *schedIface)
{
   DBPL_DisplayContext *context;
   BEGL_DisplayInterface *disp = (BEGL_DisplayInterface*)malloc(sizeof(BEGL_DisplayInterface));

   if (disp != NULL)
   {
      context = (DBPL_DisplayContext *)malloc(sizeof(DBPL_DisplayContext));
      memset(disp, 0, sizeof(BEGL_DisplayInterface));

      if (context != NULL)
      {
         memset(context, 0, sizeof(DBPL_DisplayContext));

         context->schedIface = schedIface;

         disp->context = (void*)context;

         disp->WindowGetInfo              = DispWindowGetInfo;
         disp->GetNativeSurface           = DispGetNativeSurface;
         disp->SurfaceGetInfo             = DispSurfaceGetInfo;
         disp->GetNextSurface             = DispGetNextSurface;
         disp->DisplaySurface             = DispDisplaySurface;
         disp->CancelSurface              = DispCancelSurface;
         disp->PlatformSupported          = DisplayPlatformSupported;
         disp->SetDefaultDisplay          = DispSetDefaultDisplay;
         disp->WindowPlatformStateCreate  = DispWindowStateCreate;
         disp->WindowPlatformStateDestroy = DispWindowStateDestroy;
         disp->GetClientExtensions        = GetClientExtensions;

         /* stash the dfb */
         context->dfb = dfb;

         InitFenceInterface(&context->fenceIface, schedIface);
      }
      else
      {
         free(disp);
         return NULL;
      }
   }

   return disp;
}

void DestroyDirectFBDisplayInterface(BEGL_DisplayInterface * disp)
{
   if (disp)
   {
      DBPL_DisplayContext *context = (DBPL_DisplayContext *)disp->context;
      if (context)
         free(context);

      free(disp);
   }
}

static DBPL_SurfaceContainer *CreateSurface(IDirectFB *dfb, BEGL_BufferFormat format, uint32_t width, uint32_t height)
{
   DBPL_SurfaceContainer            *sc = NULL;

   sc = (DBPL_SurfaceContainer *)malloc(sizeof(DBPL_SurfaceContainer));
   if (sc)
   {
      DFBSurfaceDescription            desc;

      BKNI_Memset(sc, 0, sizeof(DBPL_SurfaceContainer));

      desc.flags = DSDESC_CAPS |
                   DSDESC_WIDTH |
                   DSDESC_HEIGHT |
                   DSDESC_PIXELFORMAT;
      desc.caps = DSCAPS_GL;
      desc.width = width;
      desc.height = height;
      sc->settings.width = width;
      sc->settings.height = height;
      sc->settings.format = format;

      switch (format)
      {
      case BEGL_BufferFormat_eA8B8G8R8 :
         desc.pixelformat = DSPF_ABGR;
         break;
      case BEGL_BufferFormat_eX8B8G8R8 :
         desc.pixelformat = DSPF_RGB32;
         break;
      case BEGL_BufferFormat_eR5G6B5 :
         desc.pixelformat = DSPF_RGB16;
         break;
      case BEGL_BufferFormat_eR8G8B8A8 :
      case BEGL_BufferFormat_eR8G8B8X8 :
         fprintf(stderr, "DirectFB does not currently support RGBA8888 surface formats which is required "
                         "to support 3D graphics in bigendian mode\n");
         return NULL;
         break;
      default:
         return NULL;
      }

      DFBCHECK(dfb->CreateSurface( dfb, &desc, &sc->windowsurface ));

      /* These lock and unlocks are required  DFB has some lazy surface creation */
      DFBCHECK(sc->windowsurface->Lock( sc->windowsurface, DSLF_WRITE, &sc->settings.cachedAddr, &sc->settings.pitchBytes));
      DFBCHECK(sc->windowsurface->Unlock( sc->windowsurface ));

      sc->settings.physicalOffset   = NEXUS_AddrToOffset(sc->settings.cachedAddr);
      sc->settings.byteSize         = sc->settings.pitchBytes * sc->settings.height;
      sc->availableToRenderFence    = 1;
   }

   return sc;
}

bool DBPL_CreateCompatiblePixmap(DBPL_PlatformHandle handle, void **pixmapHandle, IDirectFBSurface **surface, BEGL_PixmapInfo *info)
{
   DBPL_InternalPlatformHandle *platform = (DBPL_InternalPlatformHandle*)handle;

   DBPL_SurfaceContainer       *sc = NULL;

   if (info != NULL)
   {
      DBPL_DisplayContext *context = (DBPL_DisplayContext *)platform->displayInterface->context;
      sc = CreateSurface(context->dfb, info->format, info->width, info->height);

      if (pixmapHandle != NULL)
         *pixmapHandle = sc;

      if (surface != NULL)
         *surface = sc->windowsurface;
   }

   return sc != NULL;
}

void DBPL_DestroyCompatiblePixmap(DBPL_PlatformHandle handle, void *pixmapHandle)
{
   DBPL_SurfaceContainer *sc = (DBPL_SurfaceContainer *)pixmapHandle;

   if (sc != NULL)
   {
      DFBCHECK(sc->windowsurface->Release( sc->windowsurface ));
      free(sc);
   }
}
