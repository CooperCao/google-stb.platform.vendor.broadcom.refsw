/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "display_nexus_exclusive.h"

#include "event.h"
#include "fatal_error.h"
#include "platform_common.h"
#include "../nexus/private_nexus.h" /* NXPL_Surface */

struct buffer
{
   NEXUS_SurfaceHandle surface;
   int                 fence;
};

typedef struct display
{
   const FenceInterface         *fenceInterface;
   NEXUS_DISPLAYHANDLE           display;
   NXPL_DisplayType              displayType;
   int                          *bound;

   struct buffer                 active;
   struct buffer                 pending;
   BKNI_MutexHandle              mutex;

   void                         *vsyncEvent;
   bool                          terminating;
} display;

static void vsyncCallback(void *context, int param)
{
   display *self = (display *)context;

   int terminating = __sync_fetch_and_and(&self->terminating, 1);
   if (!terminating)
   {
      BKNI_AcquireMutex(self->mutex);
      if (!self->pending.surface)
      {
         /* This is the 2nd and subsequent vsync after DisplaySurface().
          * We have to skip the 1st one because frameBufferCallback may arrive
          * after vsyncCallback and we use the event to first wait until the
          * buffers are swapped and then to wait for subsequent vsyncs.
          *
          * The 1st vsync is signalled from frameBufferCallback().
          */
         SetEvent(self->vsyncEvent);
      }
      BKNI_ReleaseMutex(self->mutex);
   }
}

static void frameBufferCallback(void *context, int param)
{
   display *self = (display *)context;
   UNUSED(param);

   int terminating = __sync_fetch_and_and(&self->terminating, 1);
   if (!terminating)
   {
      /* Buffers were swapped. The surface that was previously on screen is
       * off screen now. The surface that was waiting after the last call
       * to DisplaySurface is now on screen. The next call DisplaySurface
       * will initiate another buffer swap.
       */
      BKNI_AcquireMutex(self->mutex);

      FenceInterface_Signal(self->fenceInterface, self->active.fence);

      self->active.surface = self->pending.surface;
      self->active.fence = self->pending.fence;

      self->pending.surface = NULL;
      self->pending.fence = self->fenceInterface->invalid_fence;

      /* that's the 1st "vsync" after DisplaySurface() */
      SetEvent(self->vsyncEvent);

      BKNI_ReleaseMutex(self->mutex);
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

   BKNI_AcquireMutex(self->mutex);
   if (self->pending.surface)
   {
      BKNI_ReleaseMutex(self->mutex);
      return eDisplayFailed;
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

   NEXUS_Error err = NEXUS_Display_SetGraphicsFramebuffer3D(self->display, &fb3d);
   if (err != NEXUS_SUCCESS)
   {
      BKNI_ReleaseMutex(self->mutex);
      FATAL_ERROR("NEXUS_Display_SetGraphicsFramebuffer3D failed");
      return eDisplayFailed;
   }

   self->pending.surface = surface;
   FenceInterface_Create(
         self->fenceInterface, &self->pending.fence);
   *display_fence = self->pending.fence;

   BKNI_ReleaseMutex(self->mutex);
   return eDisplayPending;
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

static void SetDisplayComposition(NEXUS_DISPLAYHANDLE display,
      const NXPL_NativeWindowInfoEXT *windowInfo)
{
   NEXUS_GraphicsSettings  graphicsSettings;

   NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);

   graphicsSettings.enabled = true;
   graphicsSettings.position.x = windowInfo->x;
   graphicsSettings.position.y = windowInfo->y;
   if (!windowInfo->stretch)
   {
      graphicsSettings.position.width = windowInfo->width;
      graphicsSettings.position.height = windowInfo->height;
   }
   graphicsSettings.clip.width = windowInfo->width;
   graphicsSettings.clip.height = windowInfo->height;

   NEXUS_Error err = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetGraphicsSettings failed");
}

static void SetVsyncCallback(NEXUS_DISPLAYHANDLE display,
      NEXUS_Callback callback, void *context)
{
   NEXUS_CallbackDesc vsync;
   /* vsync available, swap interval can be active */
   vsync.callback = callback;
   vsync.context = context;
   NEXUS_Error err = NEXUS_Display_SetVsyncCallback(display, &vsync);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("NEXUS_Display_SetVsyncCallback failed");
}

static void SetFrameBufferCallback(NEXUS_DISPLAYHANDLE display,
      NEXUS_Callback callback, void *context)
{
   NEXUS_GraphicsSettings  graphicsSettings;
   NEXUS_Display_GetGraphicsSettings(display, &graphicsSettings);
   graphicsSettings.frameBufferCallback.callback = callback;
   graphicsSettings.frameBufferCallback.context = context;
   NEXUS_Error err = NEXUS_Display_SetGraphicsSettings(display, &graphicsSettings);
   if (err != NEXUS_SUCCESS)
      FATAL_ERROR("SetDisplayFrameBufferCallback failed");
}

bool DisplayInterface_InitNexusExclusive(DisplayInterface *di,
      const FenceInterface *fi,
      const NXPL_NativeWindowInfoEXT *windowInfo, NXPL_DisplayType displayType,
      NEXUS_DISPLAYHANDLE display_handle, int *bound)
{
   display *self = calloc(1, sizeof(*self));
   if (self)
   {
      self->fenceInterface = fi;

      self->displayType = displayType;
      self->bound = bound;
      self->display = display_handle;

      /* setup the display & callback */
      self->vsyncEvent = CreateEvent();

      if (BKNI_CreateMutex(&self->mutex) != BERR_SUCCESS)
         FATAL_ERROR("BKNI_CreateMutex failed");
      self->active.surface = NULL;
      self->pending.surface = NULL;
      self->active.fence = self->fenceInterface->invalid_fence;
      self->pending.fence = self->fenceInterface->invalid_fence;

      if (self->display != NULL)
      {
         __sync_fetch_and_add(self->bound, 1);
         SetDisplayComposition(self->display, windowInfo);
         SetVsyncCallback(self->display, vsyncCallback, self);
         SetFrameBufferCallback(self->display, frameBufferCallback, self);
      }

      di->base.context = self;
      di->base.destroy = destroy;

      di->display = display_surface;
      di->wait_sync = wait_sync;
      di->stop = stop;
   }
   return self != NULL;
}
