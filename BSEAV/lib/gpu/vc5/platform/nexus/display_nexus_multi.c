/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "display_nexus_multi.h"

#include "event.h"
#include "fatal_error.h"
#include "platform_common.h"
#include "fence_queue.h"
#include "private_nexus.h" /* NXPL_Surface */

#include "debug_helper.h"

typedef struct display
{
   const FenceInterface      *fenceInterface;
   pthread_mutex_t            mutex;
   struct fence_queue         fence_queue;

   NEXUS_DISPLAYHANDLE        display;
   DisplayType                displayType;
   unsigned int               numSurfaces;
   unsigned int               pushedSurfaces;
   NXPL_NativeWindow         *nw;

   void                      *vsyncEvent;
   int                        terminating;
   EventContext              *eventContext;

} display;

static void vsyncCallback(void *context, int param)
{
   display *self = (display *)context;

   int terminating = __sync_fetch_and_and(&self->terminating, 1);
   if (!terminating)
      SetEvent(self->vsyncEvent);
}

static void recycledCallback(void *context, int param)
{
   display *self = (display *)context;
   UNUSED(param);

   int terminating = __sync_fetch_and_and(&self->terminating, 1);

   platform_dbg_message_add("%s, terminating %s", __FUNCTION__, terminating ? "true" : "false");

   if (!terminating)
   {
      size_t numRecycled = 1;
      NEXUS_SurfaceHandle surface_list[self->numSurfaces];
      NXPL_NativeWindow *nw = self->nw;
      NEXUS_SurfaceClient_RecycleSurface(nw->surfaceClient, surface_list, self->numSurfaces, &numRecycled);

      platform_dbg_message_add("%s, numRecycled %zd", __FUNCTION__, numRecycled);

      pthread_mutex_lock(&self->mutex);
      for (size_t i = 0; i < numRecycled; i++)
      {
         platform_dbg_message_add("  %s %d - surface_list[%zd] = %p", __FUNCTION__, __LINE__, i, surface_list[i]);

         int display_fence;
         if (fence_queue_dequeue(&self->fence_queue, &display_fence, false))
            FenceInterface_Signal(self->fenceInterface, display_fence);
      }
      pthread_mutex_unlock(&self->mutex);
   }
}

static void SetDisplayComposition(display *self, const WindowInfo *windowInfo)
{
#if defined(NXCLIENT_SUPPORT)
   NEXUS_SurfaceComposition comp;

   NXPL_NativeWindow *nw = self->nw;
   NxClient_GetSurfaceClientComposition(nw->clientID, &comp);

   if (!windowInfo->stretch)
   {
      comp.virtualDisplay.width = 0;
      comp.virtualDisplay.height = 0;
      comp.position.width = windowInfo->width;
      comp.position.height = windowInfo->height;
   }
   else
   {
      comp.virtualDisplay.width = windowInfo->width;
      comp.virtualDisplay.height = windowInfo->height;
      comp.position.width = windowInfo->width - (2 * windowInfo->x);
      comp.position.height = windowInfo->height - (2 * windowInfo->y);
   }
   comp.position.x = windowInfo->x;
   comp.position.y = windowInfo->y;
   comp.zorder = windowInfo->zOrder;
   comp.colorBlend = windowInfo->colorBlend;
   comp.alphaBlend = windowInfo->alphaBlend;

   NxClient_SetSurfaceClientComposition(nw->clientID, &comp);
#endif
}

static DisplayInterfaceResult display_surface(void *context, void *s,
      const WindowInfo *windowInfo,
      int render_fence, bool create_display_fence, int *display_fence)
{
   UNUSED(create_display_fence);
   display *self = (display *)context;
   NXPL_Surface *nxpl_surface = (NXPL_Surface *)s;
   NEXUS_SurfaceHandle surface = nxpl_surface->surface;

   FenceInterface_WaitAndDestroy(
         self->fenceInterface, &render_fence);

   NEXUS_Error err;

   ResetEvent(self->vsyncEvent);

   pthread_mutex_lock(&self->mutex);

   platform_dbg_message_add("%s %d - surface = %p", __FUNCTION__, __LINE__, surface);

   NXPL_NativeWindow *nw = self->nw;
   if (windowInfo != NULL)
   {
      SetDisplayComposition(self, windowInfo);
      if (self->displayType != windowInfo->type)
      {
         NEXUS_SurfaceClientSettings clientSettings;
         NEXUS_SurfaceClient_GetSettings(nw->surfaceClient, &clientSettings);
         if (windowInfo->type == _2D)
            clientSettings.orientation = NEXUS_VideoOrientation_e2D;
         else if (windowInfo->type == _3D_LEFT_RIGHT)
            clientSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
         else if (windowInfo->type == _3D_OVER_UNDER)
            clientSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
         else
            FATAL_ERROR("Invalid displayType");
         clientSettings.allowCompositionBypass = true;

         err = NEXUS_SurfaceClient_SetSettings(nw->surfaceClient, &clientSettings);
         if (err != NEXUS_SUCCESS)
            FATAL_ERROR("NEXUS_SurfaceClient_SetSettings failed");

         self->displayType = windowInfo->type;
      }

      if (nw->videoClient)
      {
         NEXUS_SurfaceClientSettings clientSettings;
         NEXUS_SurfaceClient_GetSettings(nw->videoClient, &clientSettings);
         clientSettings.composition.position.x      = windowInfo->videoX;
         clientSettings.composition.position.y      = windowInfo->videoY;
         clientSettings.composition.position.width  = windowInfo->videoWidth;
         clientSettings.composition.position.height = windowInfo->videoHeight;
         clientSettings.synchronizeGraphics         = true;
         NEXUS_SurfaceClient_SetSettings(nw->videoClient, &clientSettings);
      }
   }

   err = NEXUS_SurfaceClient_PushSurface(nw->surfaceClient, surface, NULL, true);
   if (err == NEXUS_SUCCESS)
   {
      FenceInterface_Create(
            self->fenceInterface, display_fence);
      fence_queue_enqueue(&self->fence_queue, *display_fence);

      pthread_mutex_unlock(&self->mutex);
      return eDisplayPending;
   }
   else
   {
      pthread_mutex_unlock(&self->mutex);
      FATAL_ERROR("NEXUS_SurfaceClient_PushSurface failed");
      return eDisplayFailed;
   }
}

static bool wait_sync(void *context)
{
   display *self = (display *)context;
   platform_dbg_message_add("%s - WAIT FOR SYNC", __FUNCTION__);
   WaitEvent(self->vsyncEvent);
   platform_dbg_message_add("%s - GOT SYNC", __FUNCTION__);
   return true;
}

static void stop(void *context)
{
   display *self = (display *)context;
   /* mark as dying.  As callbacks are asynchronous in the nexus model, they can
      arrive after they have been uninstalled here, calling with destroyed resources */
   int res = __sync_fetch_and_or(&self->terminating, 1);
   if (res != 0)
      FATAL_ERROR("TerminateDisplay called more than once");

   NXPL_NativeWindow *nw = self->nw;
   if (nw->surfaceClient != NULL)
   {
      NEXUS_SurfaceClientSettings clientSettings;
      NEXUS_SurfaceClient_GetSettings(nw->surfaceClient, &clientSettings);

      clientSettings.vsync.callback = NULL;
      clientSettings.vsync.context = NULL;

      NEXUS_SurfaceClient_SetSettings(nw->surfaceClient, &clientSettings);

      /* current surface must be cleared from nxserver */
      NEXUS_SurfaceClient_Clear(nw->surfaceClient);
   }

   DestroyEvent(self->vsyncEvent);

   int display_fence;
   while (fence_queue_dequeue(&self->fence_queue, &display_fence, false))
      FenceInterface_Signal(self->fenceInterface, display_fence);
}


static void destroy(void *context)
{
   display *self = (display *)context;
   if (self)
   {
      pthread_mutex_destroy(&self->mutex);
      fence_queue_destroy(&self->fence_queue);
      free(self);
   }
}

static void SetVsyncCallback(NEXUS_SurfaceClientHandle surfaceClient,
      NEXUS_Callback callback, void *context)
{
   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(surfaceClient, &clientSettings);

   clientSettings.vsync.callback = callback;
   clientSettings.vsync.context = context;

   NEXUS_Error err = NEXUS_SurfaceClient_SetSettings(surfaceClient,
         &clientSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetVsyncCallback failed");
}

static void SetRecycledCallback(NEXUS_SurfaceClientHandle surfaceClient,
      NEXUS_Callback callback, void *context)
{
   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(surfaceClient, &clientSettings);

   clientSettings.recycled.callback = callback;
   clientSettings.recycled.context = context;
   NEXUS_Error err = NEXUS_SurfaceClient_SetSettings(surfaceClient,
         &clientSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("SetDisplayFrameBufferCallback failed");

}

bool DisplayInterface_InitNexusMulti(DisplayInterface *di,
      const FenceInterface *fi,
      unsigned int numSurfaces, NXPL_NativeWindow *nw,
      EventContext *eventContext)
{
   display *self = calloc(1, sizeof(*self));
   if (self)
   {
      self->fenceInterface = fi;
      fence_queue_init(&self->fence_queue, numSurfaces, self->fenceInterface);
      pthread_mutex_init(&self->mutex, NULL);

      self->displayType = NXPL_2D;
      self->numSurfaces = numSurfaces;
      self->nw = nw;
      self->display = NULL;
      self->eventContext = eventContext;

      /* setup the display & callback */
      self->vsyncEvent = CreateEvent();

      if (nw->surfaceClient != NULL)
      {
         SetVsyncCallback(nw->surfaceClient, vsyncCallback, self);
         SetRecycledCallback(nw->surfaceClient, recycledCallback, self);
      }

      di->base.context = self;
      di->base.destroy = destroy;

      di->display = display_surface;
      di->wait_sync = wait_sync;
      di->stop = stop;
   }
   return self != NULL;
}
