/*=============================================================================
Broadcom Proprietary and Confidential. (c)2015 Broadcom.
All rights reserved.
=============================================================================*/

#include "event.h"
#include "fence.h"
#include "display_thread.h"
#include "display_surface.h"

#include "nexus_platform.h"
#include "nexus_display.h"

#include "private_nexus.h"

#include "message_queue.h"

#ifdef NXCLIENT_SUPPORT
#include "nxclient.h"
#endif

static void fatalError(const char *msg, const char *function, int line)
{
   printf("FATAL : %s (%s, %d)\n", msg, function, line);
   exit(0);
}
#define FATAL_ERROR(MSG) fatalError(MSG, __FUNCTION__, __LINE__)

static void vsyncCallback(void *context, int param)
{
   NXPL_NativeWindow_priv *priv = (NXPL_NativeWindow_priv *)context;

   int terminating = __sync_fetch_and_and(&priv->terminating, 1);
   if (!terminating)
      SetEvent(priv->vsyncEvent);
}

static void bufferOnDisplayCallback(void *context, int param)
{
   NXPL_NativeWindow_priv *priv = (NXPL_NativeWindow_priv *)context;

   int terminating = __sync_fetch_and_and(&priv->terminating, 1);
   if (!terminating)
   {
      size_t numRecycled = 1;
#ifndef NXPL_PLATFORM_EXCLUSIVE
      NEXUS_SurfaceHandle surface_list[priv->numSurfaces];
      NEXUS_SurfaceClient_RecycleSurface(priv->surfaceClient, surface_list, priv->numSurfaces, &numRecycled);
#endif

      for (size_t i = 0; i < numRecycled; i++)
      {
         /* SIGNAL FENCE! */
         NXPL_Surface *sFence = ReceiveMessage(priv->fenceQueue, IMMEDIATE);
         if (sFence)
         {
            /* this only triggers if a swap interval is required */
#ifdef NXPL_PLATFORM_EXCLUSIVE
            SignalFence(priv->schedIface, priv->lastFence);
            priv->lastFence = sFence->fence;
#else
            SignalFence(priv->schedIface, sFence->fence);
#endif

            ReleaseMessage(priv->fenceQueue, sFence);
         }
#ifdef NXPL_PLATFORM_EXCLUSIVE
         else
         {
            /* drain existing fence, if swap interval changes from >=1 to 0 */
            SignalFence(priv->schedIface, priv->lastFence);
            priv->lastFence = -1;
         }
#endif
      }

#ifdef NXPL_PLATFORM_EXCLUSIVE
      SetEvent(priv->bufferOnDisplayEvent);
#endif
   }
}

#if !defined(NXPL_PLATFORM_EXCLUSIVE) && defined(NXCLIENT_SUPPORT)
static void SetComposition(NXPL_NativeWindow_priv *priv)
{
   NEXUS_SurfaceComposition comp;

   NxClient_GetSurfaceClientComposition(priv->clientID, &comp);

   if (!priv->windowInfo.stretch)
   {
      comp.virtualDisplay.width = 0;
      comp.virtualDisplay.height = 0;
      comp.position.width = priv->windowInfo.width;
      comp.position.height = priv->windowInfo.height;
   }
   else
   {
      comp.virtualDisplay.width = priv->windowInfo.width;
      comp.virtualDisplay.height = priv->windowInfo.height;
      comp.position.width = priv->windowInfo.width - (2 * priv->windowInfo.x);
      comp.position.height = priv->windowInfo.height - (2 * priv->windowInfo.y);
   }
   comp.position.x = priv->windowInfo.x;
   comp.position.y = priv->windowInfo.y;
   comp.zorder = priv->windowInfo.zOrder;
   comp.colorBlend = priv->windowInfo.colorBlend;
   comp.alphaBlend = priv->windowInfo.alphaBlend;

   NxClient_SetSurfaceClientComposition(priv->clientID, &comp);
}
#endif /* !defined(NXPL_PLATFORM_EXCLUSIVE) && defined(NXCLIENT_SUPPORT) */

static bool InitializeDisplay(NXPL_NativeWindow_priv *priv)
{
   /* setup the display & callback */
   priv->vsyncEvent = CreateEvent();

#ifdef NXPL_PLATFORM_EXCLUSIVE
   priv->bufferOnDisplayEvent = CreateEvent();

   if (priv->display != NULL)
   {
      __sync_fetch_and_add(priv->bound, 1);

      NEXUS_GraphicsSettings  graphicsSettings;

      NEXUS_Display_GetGraphicsSettings(priv->display, &graphicsSettings);

      graphicsSettings.enabled = true;
      graphicsSettings.position.x = priv->windowInfo.x;
      graphicsSettings.position.y = priv->windowInfo.y;
      if (!priv->windowInfo.stretch)
      {
         graphicsSettings.position.width = priv->windowInfo.width;
         graphicsSettings.position.height = priv->windowInfo.height;
      }
      graphicsSettings.clip.width = priv->windowInfo.width;
      graphicsSettings.clip.height = priv->windowInfo.height;

      graphicsSettings.frameBufferCallback.context = priv;
      graphicsSettings.frameBufferCallback.callback = bufferOnDisplayCallback;

      NEXUS_Error err = NEXUS_Display_SetGraphicsSettings(priv->display, &graphicsSettings);
      if (err != NEXUS_SUCCESS)
         FATAL_ERROR("NEXUS_Display_SetGraphicsSettings failed");

      priv->vsyncAvailable = true;
   }
#else
   if (priv->surfaceClient != NULL)
   {
      /* You can have swap interval on a single NSC client, but it fails for more, as the
         display callback cannot be registered to multiple clients.  For this to properly
         work, we'd need something added to NEXUS_SurfaceClient_ API to expose a per client
         vsync callback */
#ifdef WANT_VSYNC_ON_SINGLE_NSC_CLIENT
#define NEXUS_DISPLAY_OBJECTS 4
      NEXUS_InterfaceName interfaceName;
      NEXUS_PlatformObjectInstance objects[NEXUS_DISPLAY_OBJECTS] = { { 0 } };
      size_t num = NEXUS_DISPLAY_OBJECTS;
      strcpy(interfaceName.name, "NEXUS_Display");
      NEXUS_Error rc = NEXUS_Platform_GetObjects(&interfaceName, &objects[0], num, &num);
      if (rc == NEXUS_SUCCESS)
      {
         priv->display = (NEXUS_DisplayHandle)objects[0].object;
         priv->vsyncAvailable = priv->display != NULL;
      }
      else
#endif /* WANT_VSYNC_ON_SINGLE_NSC_CLIENT */
         priv->vsyncAvailable = false;

      NEXUS_SurfaceClientSettings clientSettings;
      NEXUS_SurfaceClient_GetSettings(priv->surfaceClient, &clientSettings);
      clientSettings.recycled.context = priv;
      clientSettings.recycled.callback = bufferOnDisplayCallback;
      NEXUS_Error err = NEXUS_SurfaceClient_SetSettings(priv->surfaceClient, &clientSettings);
      if (err != NEXUS_SUCCESS)
         FATAL_ERROR("NEXUS_SurfaceClient_SetSettings failed");
   }
#endif /* NXPL_PLATFORM_EXCLUSIVE */

   if (priv->vsyncAvailable)
   {
      NEXUS_CallbackDesc vsync;
      /* vsync available, swap interval can be active */
      vsync.callback = vsyncCallback;
      vsync.context = priv;
      NEXUS_Error err = NEXUS_Display_SetVsyncCallback(priv->display, &vsync);
      if (err != NEXUS_SUCCESS)
         FATAL_ERROR("NEXUS_Display_SetVsyncCallback failed");
   }

   return true;
}

static void TerminateDisplay(NXPL_NativeWindow_priv *priv)
{
   /* mark as dying.  As callbacks are asynchronous in the nexus model, they can
      arrive after they have been uninstalled here, calling with destroyed resources */
   int res = __sync_fetch_and_or(&priv->terminating, 1);
   if (res != 0)
      FATAL_ERROR("TerminateDisplay called more than once");

#ifdef NXPL_PLATFORM_EXCLUSIVE
   if (priv->display != NULL)
   {
      int res = __sync_sub_and_fetch(priv->bound, 1);
      /* don't remove the callbacks until we've counted everything done */
      if (res == 0)
      {
         NEXUS_GraphicsSettings  graphicsSettings;
         NEXUS_Display_GetGraphicsSettings(priv->display, &graphicsSettings);

         graphicsSettings.enabled = false;
         graphicsSettings.frameBufferCallback.callback = NULL;

         NEXUS_Error err = NEXUS_Display_SetGraphicsSettings(priv->display, &graphicsSettings);
         if (err != NEXUS_SUCCESS)
            FATAL_ERROR("NEXUS_Display_SetGraphicsSettings failed");
      }
   }
#else
#if 0 /* temp hack for SWVC5-543.  Works round and application bug in webkit integration.  Will remove on 16.2. */
   if (priv->surfaceClient != NULL)
   {
      NEXUS_SurfaceClientSettings clientSettings;
      NEXUS_SurfaceClient_GetSettings(priv->surfaceClient, &clientSettings);
      clientSettings.recycled.context = 0;
      clientSettings.recycled.callback = NULL;
      NEXUS_Error err = NEXUS_SurfaceClient_SetSettings(priv->surfaceClient, &clientSettings);
      if (err != NEXUS_SUCCESS)
         FATAL_ERROR("NEXUS_SurfaceClient_SetSettings failed");
   }
#endif /* end of temp hack */
#endif /* NXPL_PLATFORM_EXCLUSIVE */

   if (priv->vsyncAvailable)
   {
      NEXUS_Error err = NEXUS_Display_SetVsyncCallback(priv->display, NULL);
      if (err != NEXUS_SUCCESS)
         FATAL_ERROR("NEXUS_Display_SetVsyncCallback failed");
   }

#ifdef NXPL_PLATFORM_EXCLUSIVE
   DestroyEvent(priv->bufferOnDisplayEvent);
#endif

   DestroyEvent(priv->vsyncEvent);
}

static void *RunDisplayThread(void *p)
{
   NXPL_NativeWindow_priv *priv = (NXPL_NativeWindow_priv *)p;

#ifdef NXPL_PLATFORM_EXCLUSIVE
   NEXUS_GraphicsFramebuffer3D fb3d;
   NEXUS_Graphics_GetDefaultFramebuffer3D(&fb3d);
#endif

   if (!InitializeDisplay(priv))
      FATAL_ERROR("InitializeDisplay failed");

   priv->displayQueue = CreateQueue(priv->numSurfaces, sizeof(NXPL_Surface));
   for (int i = 0; i < priv->numSurfaces; i++)
   {
      bool ok;
      NXPL_Surface *s = GetMessage(priv->displayQueue, INFINITE);

      ok = CreateSurface(s,
         priv->format, priv->windowInfo.width, priv->windowInfo.height, /*secure=*/false,
         "SwapChain Surface");

      if (!ok)
         FATAL_ERROR("CreateSurface failed");

      /* copy the creation info into the surface (STRUCTURE COPY!) */
      s->windowInfo = priv->windowInfo;

      ReleaseMessage(priv->displayQueue, s);
   }

#if !defined(NXPL_PLATFORM_EXCLUSIVE) && defined(NXCLIENT_SUPPORT)
   /* Set the default window position */
   SetComposition(priv);
#endif /* !defined(NXPL_PLATFORM_EXCLUSIVE) && defined(NXCLIENT_SUPPORT) */

   priv->fenceQueue = CreateQueue(priv->numSurfaces, sizeof(NXPL_Surface));

   /* main thread can proceed */
   pthread_barrier_wait(&priv->barrier);

   bool poisoned = false;
   while (!poisoned)
   {
      NXPL_Surface *s = ReceiveMessage(priv->displayQueue, INFINITE);
      if (!s->poisoned)
      {
         NXPL_Surface *sFence = GetMessage(priv->fenceQueue, INFINITE);

         WaitFence(priv->schedIface, s->fence);

#ifdef NXPL_PLATFORM_EXCLUSIVE
         fb3d.main = s->surface;
         fb3d.right = s->surface;

         if (priv->displayType == NXPL_2D)
            fb3d.orientation = NEXUS_VideoOrientation_e2D;
         else if (priv->displayType == NXPL_3D_LEFT_RIGHT)
            fb3d.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
         else if (priv->displayType == NXPL_3D_OVER_UNDER)
            fb3d.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
         else
            FATAL_ERROR("Invalid displayType");
#else
         NEXUS_SurfaceClientSettings clientSettings;
         NEXUS_SurfaceClient_GetSettings(priv->surfaceClient, &clientSettings);
         if (priv->displayType == NXPL_2D)
            clientSettings.orientation = NEXUS_VideoOrientation_e2D;
         else if (priv->displayType == NXPL_3D_LEFT_RIGHT)
            clientSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
         else if (priv->displayType == NXPL_3D_OVER_UNDER)
            clientSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
         else
            FATAL_ERROR("Invalid displayType");
         clientSettings.allowCompositionBypass = true;

         NEXUS_Error err = NEXUS_SurfaceClient_SetSettings(priv->surfaceClient, &clientSettings);
         if (err != NEXUS_SUCCESS)
            FATAL_ERROR("NEXUS_SurfaceClient_SetSettings failed");

#ifdef NXCLIENT_SUPPORT
         /* if the new surface has different composition settings, reprogram */
         int windowTest = memcmp(&priv->windowInfo, &s->windowInfo, sizeof(NXPL_NativeWindowInfoEXT));
         if (windowTest)
         {
            SetComposition(priv);
            /* (STRUCTURE COPY!) */
            priv->windowInfo = s->windowInfo;
         }
#endif /* NXCLIENT_SUPPORT */
#endif /* NXPL_PLATFORM_EXCLUSIVE */

         int swapInterval = s->swapInterval;
         if (swapInterval > 0)
         {
            MakeFence(priv->schedIface, &s->fence);

            /* copy the surface into the fence queue (STRUCTURE COPY!) */
            *sFence = *s;

            /* Send into the fence fifo */
            SendMessage(priv->fenceQueue, sFence);
         }
         else
         {
            s->fence = -1;

            /* dont send anything to the fence queue */
            ReleaseMessage(priv->fenceQueue, sFence);
         }

         /* and into the main queue */
         ReleaseMessage(priv->displayQueue, s);

#ifdef NXPL_PLATFORM_EXCLUSIVE
         ResetEvent(priv->bufferOnDisplayEvent);

         NEXUS_Error err = NEXUS_Display_SetGraphicsFramebuffer3D(priv->display, &fb3d);
         if (err != NEXUS_SUCCESS)
            FATAL_ERROR("NEXUS_Display_SetGraphicsFramebuffer3D failed");
#else
         err = NEXUS_SurfaceClient_PushSurface(priv->surfaceClient, s->surface, NULL, false);
         if (err != NEXUS_SUCCESS)
            FATAL_ERROR("NEXUS_SurfaceClient_PushSurface failed");
#endif

         if (swapInterval > 0)
         {
#ifdef NXPL_PLATFORM_EXCLUSIVE
            /* NSC mode is always frame locked, just count the correct number of vsyncs */
            WaitEvent(priv->bufferOnDisplayEvent);
            swapInterval--;
#endif

            if (priv->vsyncAvailable)
            {
               ResetEvent(priv->vsyncEvent);
               while (swapInterval > 0)
               {
                  /* Any additional vsyncs */
                  WaitEvent(priv->vsyncEvent);
                  swapInterval--;
               }
            }
         }
      }
      else
      {
         ReleaseMessage(priv->displayQueue, s);
         poisoned = true;
      }
   }

   TerminateDisplay(priv);

   for (int i = 0; i < priv->numSurfaces; i++)
   {
      NXPL_Surface *s = GetMessage(priv->displayQueue, INFINITE);

      DestroySurface(s);

      ReleaseMessage(priv->displayQueue, s);
   }

   DeleteQueue(priv->displayQueue);
   DeleteQueue(priv->fenceQueue);

   return NULL;
}

#ifdef NXPL_PLATFORM_EXCLUSIVE
void *CreateWorkerThread(void *nw,
                         BEGL_SchedInterface *schedIface,
                         int numSurfaces,
                         int *bound,
                         NXPL_NativeWindowInfoEXT *windowInfo,
                         BEGL_BufferFormat format,
                         NEXUS_DISPLAYHANDLE display,
                         NXPL_DisplayType displayType)
#else
void *CreateWorkerThread(void *nw,
                         BEGL_SchedInterface *schedIface,
                         int numSurfaces,
                         NXPL_NativeWindowInfoEXT *windowInfo,
                         BEGL_BufferFormat format,
                         uint32_t clientID,
                         NEXUS_SurfaceClientHandle surfaceClient,
                         NXPL_DisplayType displayType)
#endif
{
   /* Create a display thread for this native window */
   NXPL_NativeWindow_priv *priv = (NXPL_NativeWindow_priv *)malloc(sizeof(NXPL_NativeWindow_priv));

   if (priv == NULL)
      FATAL_ERROR("malloc failed");

   memset(priv, 0, sizeof(NXPL_NativeWindow_priv));

   priv->nw = nw;

   priv->schedIface = schedIface;
   priv->numSurfaces = numSurfaces;
   priv->windowInfo = *windowInfo;
   priv->format = format;
   priv->displayType = displayType;
#ifdef NXPL_PLATFORM_EXCLUSIVE
   priv->bound = bound;
   priv->display = display;
   priv->lastFence = -1;
#else
   priv->clientID = clientID;
   priv->surfaceClient = surfaceClient;
   priv->display = NULL;
#endif

   /* number of times the barrier needs to hit before proceeding = 2 */
   /* once here and once in the thread */
   pthread_barrier_init(&priv->barrier, NULL, 2);

   if (pthread_create(&priv->displayThread, NULL, &RunDisplayThread, priv) != 0)
      FATAL_ERROR("pthread_create failed");

   /* Wait for thread to really start */
   pthread_barrier_wait(&priv->barrier);

   pthread_barrier_destroy(&priv->barrier);

   return priv;
}

BEGL_Error DestroyWorkerThread(void *p)
{
   if (p != NULL)
   {
      NXPL_NativeWindow_priv *priv = (NXPL_NativeWindow_priv *)p;

      NXPL_Surface *s;
      s = GetMessage(priv->displayQueue, INFINITE);

      /* should be -1 from the cancel, but incase it isn't */
      WaitFence(priv->schedIface, s->fence);

      /* poison */
      s->poisoned = true;
      SendMessage(priv->displayQueue, s);

      /* Wait for display thread to end */
      if (priv->displayThread)
      {
         if (pthread_join(priv->displayThread, NULL) != 0)
            FATAL_ERROR("pthread_join failed");
      }

      free(priv);

      return BEGL_Success;
   }

   return BEGL_Fail;
}

BEGL_Error DequeueSurface(void *p, void **nativeSurface, int *fence)
{
   NXPL_NativeWindow_priv *priv = (NXPL_NativeWindow_priv *)p;

   if (priv != NULL && nativeSurface != NULL && fence != NULL)
   {
      NXPL_Surface *s = GetMessage(priv->displayQueue, INFINITE);
      *fence = s->fence;
      *nativeSurface = s;

      return BEGL_Success;
   }
   return BEGL_Fail;
}

BEGL_Error QueueSurface(void *p, void *nativeSurface, int fence, int swapInterval)
{
   NXPL_NativeWindow_priv *priv = (NXPL_NativeWindow_priv *)p;
   NXPL_Surface *s = (NXPL_Surface *)nativeSurface;

   if (priv != NULL && s != NULL)
   {
      s->poisoned = false;
      s->fence = fence;
#ifndef NXPL_PLATFORM_EXCLUSIVE
      swapInterval = 1;    /* Multi-process only works with swap interval 1 */
#endif
      /* copy the swapInterval at this point as it should only take effect for subsequent buffers */
      s->swapInterval = swapInterval;

      SendMessage(priv->displayQueue, s);

      return BEGL_Success;
   }
   return BEGL_Fail;
}

BEGL_Error CancelSurface(void *p, void *nativeSurface, int fence)
{
   NXPL_NativeWindow_priv *priv = (NXPL_NativeWindow_priv *)p;
   NXPL_Surface *s = (NXPL_Surface *)nativeSurface;

   if (priv != NULL && s != NULL)
   {
      WaitFence(priv->schedIface, fence);
      s->fence = -1;
      ReleaseMessage(priv->displayQueue, s);
      return BEGL_Success;
   }
   return BEGL_Fail;
}

void *GetNativeWindow(void *p)
{
   NXPL_NativeWindow_priv *priv = (NXPL_NativeWindow_priv *)p;

   if (priv != NULL)
      return priv->nw;

   return NULL;
}
