/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "display_nexus_multi.h"

#include "event.h"
#include "fatal_error.h"
#include "platform_common.h"
#include "fence_queue.h"
#include "private_nexus.h" /* NXPL_Surface */

typedef struct display
{
   const FenceInterface      *fenceInterface;
   pthread_mutex_t            mutex;
   struct fence_queue         fence_queue;

   NEXUS_DISPLAYHANDLE        display;
   NXPL_DisplayType           displayType;
   unsigned int               numSurfaces;
   unsigned int               pushedSurfaces;
   uint32_t                   clientID;
   NEXUS_SurfaceClientHandle  surfaceClient;

   void                      *vsyncEvent;
   bool                       terminating;
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
   if (!terminating)
   {
      size_t numRecycled = 1;
      NEXUS_SurfaceHandle surface_list[self->numSurfaces];
      NEXUS_SurfaceClient_RecycleSurface(self->surfaceClient, surface_list, self->numSurfaces, &numRecycled);
      pthread_mutex_lock(&self->mutex);
      for (size_t i = 0; i < numRecycled; i++)
      {
         int display_fence;
         if (fence_queue_dequeue(&self->fence_queue, &display_fence, false))
            FenceInterface_Signal(self->fenceInterface, display_fence);
      }
      pthread_mutex_unlock(&self->mutex);
   }
}


static DisplayInterfaceResult display_surface(void *context, void *s,
      int render_fence, int *display_fence)
{
   display *self = (display *)context;
   NXPL_Surface *nxpl_surface = (NXPL_Surface *)s;
   NEXUS_SurfaceHandle surface = nxpl_surface->surface;

   FenceInterface_WaitAndDestroy(
         self->fenceInterface, &render_fence);

   NEXUS_SurfaceClientSettings clientSettings;
   NEXUS_SurfaceClient_GetSettings(self->surfaceClient, &clientSettings);
   if (self->displayType == NXPL_2D)
      clientSettings.orientation = NEXUS_VideoOrientation_e2D;
   else if (self->displayType == NXPL_3D_LEFT_RIGHT)
      clientSettings.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
   else if (self->displayType == NXPL_3D_OVER_UNDER)
      clientSettings.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
   else
      FATAL_ERROR("Invalid displayType");
   clientSettings.allowCompositionBypass = true;

   NEXUS_Error err = NEXUS_SurfaceClient_SetSettings(self->surfaceClient, &clientSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_SurfaceClient_SetSettings failed");

   ResetEvent(self->vsyncEvent);

   pthread_mutex_lock(&self->mutex);
   err = NEXUS_SurfaceClient_PushSurface(self->surfaceClient, surface, NULL, false);
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
   WaitEvent(self->vsyncEvent);
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

   if (self->surfaceClient != NULL)
   {
      NEXUS_SurfaceClientSettings clientSettings;
      NEXUS_SurfaceClient_GetSettings(self->surfaceClient, &clientSettings);

      clientSettings.vsync.callback = NULL;
      clientSettings.vsync.context = NULL;

      NEXUS_SurfaceClient_SetSettings(self->surfaceClient, &clientSettings);
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

static void SetDisplayComposition(display *self, const NXPL_NativeWindowInfoEXT *windowInfo)
{
#if defined(NXCLIENT_SUPPORT)
   NEXUS_SurfaceComposition comp;

   NxClient_GetSurfaceClientComposition(self->clientID, &comp);

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

   NxClient_SetSurfaceClientComposition(self->clientID, &comp);
#endif
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
      const NXPL_NativeWindowInfoEXT *windowInfo, NXPL_DisplayType displayType,
      unsigned int numSurfaces, uint32_t clientID,
      NEXUS_SurfaceClientHandle surfaceClient)
{
   display *self = calloc(1, sizeof(*self));
   if (self)
   {
      self->fenceInterface = fi;
      fence_queue_init(&self->fence_queue, numSurfaces, self->fenceInterface);
      pthread_mutex_init(&self->mutex, NULL);

      self->displayType = displayType;
      self->numSurfaces = numSurfaces;
      self->clientID = clientID;
      self->surfaceClient = surfaceClient;
      self->display = NULL;

      /* setup the display & callback */
      self->vsyncEvent = CreateEvent();

      if (self->surfaceClient != NULL)
      {
         SetDisplayComposition(self, windowInfo);
         SetVsyncCallback(self->surfaceClient, vsyncCallback, self);
         SetRecycledCallback(self->surfaceClient, recycledCallback, self);
      }

      di->base.context = self;
      di->base.destroy = destroy;

      di->display = display_surface;
      di->wait_sync = wait_sync;
      di->stop = stop;
   }
   return self != NULL;
}
