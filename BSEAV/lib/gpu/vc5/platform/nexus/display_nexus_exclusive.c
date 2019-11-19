/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "display_nexus_exclusive.h"

#include "event.h"
#include "fatal_error.h"
#include "platform_common.h"
#include "../nexus/private_nexus.h" /* NXPL_Surface */
#include "debug_helper.h"

struct buffer
{
   NEXUS_SurfaceHandle surface;
   int                 fence;
   uint32_t            debugId;
};

typedef struct display
{
   const FenceInterface         *fenceInterface;
   NEXUS_DISPLAYHANDLE           display;
   DisplayType                   displayType;
   int                          *bound;

   struct buffer                 active;
   struct buffer                 pending;
   BKNI_MutexHandle              mutex;

   void                         *vsyncEvent;
   int                           terminating;
   EventContext                 *eventContext;

   NXPL_NativeWindow            *nw;
} display;

static void vsyncCallback(void *context, int param)
{
   display *self = (display *)context;
   UNUSED(param);

   int terminating = __sync_fetch_and_and(&self->terminating, 1);

   platform_dbg_message_add("%s, terminating %s", __FUNCTION__, terminating ? "true" : "false");

   if (!terminating)
   {
      /* Buffers were swapped. The surface that was previously on screen is
       * off screen now. The surface that was waiting after the last call
       * to DisplaySurface is now on screen. The next call DisplaySurface
       * will initiate another buffer swap.
       */
      BKNI_AcquireMutex(self->mutex);

      platform_dbg_message_add("%s %d - surface = %p", __FUNCTION__, __LINE__, self->active.surface);

      if (self->active.surface != NULL)
      {
         NEXUS_GraphicsFramebufferStatus status;
         NEXUS_Display_GetGraphicsFramebufferStatus(self->display, self->active.surface, &status);

         if (status.state == NEXUS_GraphicsFramebufferState_eUnused)
         {
            FenceInterface_Signal(self->fenceInterface, self->active.fence);

            PerfAddEvent(self->eventContext, PERF_EVENT_TRACK_DISPLAY, PERF_EVENT_ON_DISPLAY, self->active.debugId,
                         BCM_EVENT_END, (uintptr_t)self->active.surface);

            self->active.surface = NULL;
            self->active.fence = INVALID_FENCE;
         }
      }

      if (self->pending.surface != NULL)
      {
         NEXUS_GraphicsFramebufferStatus status;
         NEXUS_Display_GetGraphicsFramebufferStatus(self->display, self->pending.surface, &status);

         if (status.state == NEXUS_GraphicsFramebufferState_eDisplayed)
         {
            // Copy all fields from pending to active surface
            self->active = self->pending;

            PerfAddEvent(self->eventContext, PERF_EVENT_TRACK_DISPLAY, PERF_EVENT_ON_DISPLAY, self->pending.debugId,
                         BCM_EVENT_BEGIN, (uintptr_t)self->pending.surface);

            self->pending.surface = NULL;
            self->pending.fence = INVALID_FENCE;
         }
      }

      SetEvent(self->vsyncEvent);

      BKNI_ReleaseMutex(self->mutex);
   }
}

static void SetDisplayComposition(NEXUS_DISPLAYHANDLE display,
      const NEXUS_SurfaceStatus *status, const WindowInfo *windowInfo)
{
   NEXUS_GraphicsSettings  graphicsSettings;

   NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);

   graphicsSettings.enabled = true;
   graphicsSettings.visible = true;

   /* For size calculations use the Nexus surface size, which is the same as
    * the window size was at the time the surface was created. */
   if (!windowInfo->stretch)
   {
      graphicsSettings.position.x = windowInfo->x;
      graphicsSettings.position.y = windowInfo->y;

      graphicsSettings.position.width = status->width;
      graphicsSettings.position.height = status->height;
   }
   else
   {
      float scaledPosX = windowInfo->x * ((float)graphicsSettings.position.width / status->width);
      float scaledPosY = windowInfo->y * ((float)graphicsSettings.position.height / status->height);

      graphicsSettings.position.x = (uint16_t)scaledPosX;
      graphicsSettings.position.y = (uint16_t)scaledPosY;

      graphicsSettings.position.width -= (2 * scaledPosX);
      graphicsSettings.position.height -= (2 * scaledPosY);
   }

   graphicsSettings.clip.width = status->width;
   graphicsSettings.clip.height = status->height;

   NEXUS_Error err = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetGraphicsSettings failed");
}

static DisplayInterfaceResult display_surface(void *context, void *s,
      const WindowInfo *windowInfo,
      int render_fence, bool create_display_fence, int *display_fence)
{
   display *self = (display *)context;
   NXPL_Surface *nxpl_surface = (NXPL_Surface *)s;
   NEXUS_SurfaceHandle surface = nxpl_surface->surface;

   FenceInterface_WaitAndDestroy(
         self->fenceInterface, &render_fence);

   BKNI_AcquireMutex(self->mutex);
   if (self->pending.surface)
   {
      BKNI_ReleaseMutex(self->mutex);
      return eDisplayFailed;
   }

   NEXUS_SurfaceStatus status;
   NEXUS_Surface_GetStatus(surface, &status);

   if (windowInfo != NULL)
   {
      SetDisplayComposition(self->display, &status, windowInfo);
      self->displayType = windowInfo->type;
   }

   NEXUS_GraphicsFramebuffer3D fb3d;
   NEXUS_Graphics_GetDefaultFramebuffer3D(&fb3d);
   fb3d.main = surface;
   fb3d.right = surface;

   if (self->displayType == NXPL_2D)
      fb3d.orientation = NEXUS_VideoOrientation_e2D;
   else if (self->displayType == NXPL_3D_LEFT_RIGHT)
      fb3d.orientation = NEXUS_VideoOrientation_e3D_LeftRight;
   else if (self->displayType == NXPL_3D_OVER_UNDER)
      fb3d.orientation = NEXUS_VideoOrientation_e3D_OverUnder;
   else
   {
      BKNI_ReleaseMutex(self->mutex);
      FATAL_ERROR("Invalid displayType");
      return eDisplayFailed;
   }

   ResetEvent(self->vsyncEvent);

   platform_dbg_message_add("%s %d - surface = %p", __FUNCTION__, __LINE__, surface);

   NEXUS_Error err = NEXUS_Display_SetGraphicsFramebuffer3D(self->display, &fb3d);
   if (err != NEXUS_SUCCESS)
   {
      BKNI_ReleaseMutex(self->mutex);
      FATAL_ERROR("NEXUS_Display_SetGraphicsFramebuffer3D failed");
      return eDisplayFailed;
   }

   self->pending.surface = surface;
   if (create_display_fence)
      FenceInterface_Create(
            self->fenceInterface, &self->pending.fence);
   else
      self->pending.fence = INVALID_FENCE;
   *display_fence = self->pending.fence;

   static uint32_t debugId = 0;
   self->pending.debugId = debugId++;

   BKNI_ReleaseMutex(self->mutex);
   return eDisplayPending;
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

   if (self->display != NULL)
   {
      int res = __sync_sub_and_fetch(self->bound, 1);
      /* don't remove the callbacks until we've counted everything done */
      if (res == 0)
      {
         NEXUS_GraphicsSettings  graphicsSettings;
         NEXUS_Display_GetGraphicsSettings(self->display, &graphicsSettings);

         graphicsSettings.enabled = false;
         graphicsSettings.frameBufferCallback.callback = NULL;

         NEXUS_Error err = NEXUS_Display_SetGraphicsSettings(self->display, &graphicsSettings);
         if (err != NEXUS_SUCCESS)
            FATAL_ERROR("NEXUS_Display_SetGraphicsSettings failed");
      }
   }

   NEXUS_Error err = NEXUS_Display_SetVsyncCallback(self->display, NULL);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetVsyncCallback failed");

   BKNI_DestroyMutex(self->mutex);
   self->active.surface = NULL;
   self->pending.surface = NULL;
   FenceInterface_Signal(self->fenceInterface, self->active.fence);
   FenceInterface_Signal(self->fenceInterface, self->pending.fence);

   DestroyEvent(self->vsyncEvent);
}

static void destroy(void *context)
{
   display *self = (display *)context;
   free(self);
}

static void SetVsyncCallback(NEXUS_DISPLAYHANDLE display,
      NEXUS_Callback callback, void *context)
{
   NEXUS_CallbackDesc vsync;
   NEXUS_CallbackDesc_Init(&vsync);
   /* vsync available, swap interval can be active */
   vsync.callback = callback;
   vsync.context = context;
   NEXUS_Error err = NEXUS_Display_SetVsyncCallback(display, &vsync);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetVsyncCallback failed");
}

bool DisplayInterface_InitNexusExclusive(DisplayInterface *di,
      const FenceInterface *fi,
      const NXPL_NativeWindowInfoEXT *windowInfo,
      NEXUS_DISPLAYHANDLE display_handle, int *bound, EventContext *eventContext)
{
   display *self = calloc(1, sizeof(*self));
   if (self)
   {
      self->fenceInterface = fi;

      self->displayType = NXPL_2D;
      self->bound = bound;
      self->display = display_handle;
      self->eventContext = eventContext;

      /* setup the display & callback */
      self->vsyncEvent = CreateEvent();

      if (BKNI_CreateMutex(&self->mutex) != BERR_SUCCESS)
         FATAL_ERROR("BKNI_CreateMutex failed");
      self->active.surface = NULL;
      self->active.debugId = 0;
      self->pending.surface = NULL;
      self->pending.debugId = 0;
      self->active.fence = INVALID_FENCE;
      self->pending.fence = INVALID_FENCE;

      if (self->display != NULL)
      {
         __sync_fetch_and_add(self->bound, 1);
         SetVsyncCallback(self->display, vsyncCallback, self);
      }

      di->base.context = self;
      di->base.destroy = destroy;

      di->display = display_surface;
      di->wait_sync = wait_sync;
      di->stop = stop;
   }
   return self != NULL;
}
