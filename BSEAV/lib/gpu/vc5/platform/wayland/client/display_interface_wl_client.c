/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "display_interface_wl_client.h"

#include "surface_interface_wl_client.h"
#include "wayland_egl_priv.h"

#include "ring_buffer.h"

typedef struct DisplayInterfaceWlClient
{
   WaylandClient *client;
   const FenceInterface *fence_interface;
   struct wl_egl_window *window;

   struct ring_buffer fences;

   struct wl_callback * frame_callback;
   struct wl_callback * display_sync_callback;
} DisplayInterfaceWlClient;

static void frame_callback(void *data, struct wl_callback *callback, uint32_t time)
{
   DisplayInterfaceWlClient *self =
         (DisplayInterfaceWlClient *)data;

   self->frame_callback = NULL;
   wl_callback_destroy(callback);
}

static bool set_frame_callback(DisplayInterfaceWlClient *self)
{
   if (!self->frame_callback)
   {
      static const struct wl_callback_listener frame_listener = {
         frame_callback
      };

      self->frame_callback = wl_surface_frame(self->window->surface);
      wl_callback_add_listener(self->frame_callback, &frame_listener, self);
      wl_proxy_set_queue((struct wl_proxy *) self->frame_callback,
            self->client->events);
      return true;
   }
   else
   {
      return false;
   }
}

static bool wait_frame_callback(DisplayInterfaceWlClient *self)
{
   if (set_frame_callback(self))
   {
      /* re-submit the same surface, otherwise we won't get the frame callback */
      wl_surface_commit(self->window->surface);
      wl_display_flush(self->client->display);
   }

   int ret = 0;
   while (self->frame_callback && ret != -1)
   {
      ret = wl_display_dispatch_queue(self->client->display,
            self->client->events);
   }
   return (ret >= 0);
}

static void display_sync_callback(void *data, struct wl_callback *callback, uint32_t time)
{
   DisplayInterfaceWlClient *self =
         (DisplayInterfaceWlClient *)data;

   self->display_sync_callback = NULL;
   wl_callback_destroy(callback);
}

static bool set_display_sync_callback(DisplayInterfaceWlClient *self)
{
   if (!self->display_sync_callback)
   {
      static const struct wl_callback_listener display_sync_listener = {
         display_sync_callback
      };

      self->display_sync_callback = wl_display_sync(self->client->display);
      wl_callback_add_listener(self->display_sync_callback, &display_sync_listener, self);
      wl_proxy_set_queue((struct wl_proxy *) self->display_sync_callback,
            self->client->events);
      return true;
   }
   else
   {
      return false;
   }
}

static bool wait_display_sync(DisplayInterfaceWlClient *self)
{
   int ret = 0;
   while (self->display_sync_callback && ret != -1)
   {
      ret = wl_display_dispatch_queue(self->client->display,
            self->client->events);
   }
   return (ret >= 0);
}

void buffer_release_callback(void *data, struct wl_buffer *buffer)
{
   DisplayInterfaceWlClient *self =
         (DisplayInterfaceWlClient *)data;
   int display_fence;
   if (ring_buffer_read(&self->fences, &display_fence, /*block=*/false))
   {
      FenceInterface_Signal(self->fence_interface, display_fence);
   }
}

static void set_buffer_release_callback(DisplayInterfaceWlClient *self,
      WaylandClientBuffer *cb)
{
   if (!cb->buffer_release_callback)
   {
      static const struct wl_buffer_listener listener =
      {
         .release = buffer_release_callback,
      };
      if (wl_buffer_add_listener(cb->buffer, &listener, self) == 0)
         cb->buffer_release_callback = buffer_release_callback;
   }
}

static void skip(void *context, void *s)
{
   DisplayInterfaceWlClient *self =
         (DisplayInterfaceWlClient *)context;

   /* at least wait for display sync */
   set_display_sync_callback(self);
   wl_display_flush(self->client->display);
   wait_display_sync(self);
}

static DisplayInterfaceResult display(void *context, void *surface,
      int render_fence, bool create_display_fence, int *display_fence)
{
   DisplayInterfaceWlClient *self =
         (DisplayInterfaceWlClient *)context;
   WaylandClientBuffer *cb = (WaylandClientBuffer *)surface;

   /* process any pending events from the last frame */
   wl_display_dispatch_queue_pending(self->client->display, self->client->events);

   set_buffer_release_callback(self, cb);

   /* prepare the frame callback prior to any actions */
   if (set_frame_callback(self))
   {
      FenceInterface_WaitAndDestroy(self->fence_interface, &render_fence);

      pthread_mutex_lock(&self->window->mutex);

      if (create_display_fence)
         FenceInterface_Create(self->fence_interface, display_fence);
      else
         *display_fence = self->fence_interface->invalid_fence;

      ring_buffer_write(&self->fences, display_fence, /*block=*/true);

      /* attach the new buffer to the surface - after resize dx and dx might not be 0*/
      wl_surface_attach(self->window->surface, cb->buffer,
            self->window->dx, self->window->dy);

      self->window->dx = 0;
      self->window->dy = 0;
      self->window->attached_width = cb->info.width;
      self->window->attached_height = cb->info.height;

      pthread_mutex_unlock(&self->window->mutex);

      wl_surface_damage(self->window->surface, 0, 0,
            cb->info.width, cb->info.height);

      /* send the buffer accross to the server */
      wl_surface_commit(self->window->surface);

      /* flush the fd */
      wl_display_flush(self->client->display);

      return eDisplayPending;
   }
   else
   {
      /* couldn't display but still own the render fence */
      FenceInterface_Destroy(self->fence_interface, &render_fence);
      skip(context, surface);
      return eDisplayFailed;
   }
}

static bool wait_sync(void *context)
{
   DisplayInterfaceWlClient *self =
         (DisplayInterfaceWlClient *)context;
   return wait_frame_callback(self);
}

static void destroy(void *context)
{
   DisplayInterfaceWlClient *self =
         (DisplayInterfaceWlClient *)context;
   if (self)
   {
       ring_buffer_destroy(&self->fences);
       free(self);
   }
}

static void stop(void *context)
{
   DisplayInterfaceWlClient *self =
         (DisplayInterfaceWlClient *)context;
   if (self)
   {
      /* finish any pending operations */
      skip(self, NULL);

      /* signal any remaining fences */
      int fence;
      while (ring_buffer_read(&self->fences, &fence, /*block=*/false))
      {
        FenceInterface_Signal(self->fence_interface, fence);
      }
   }
}

bool DisplayInterface_InitWlClient(DisplayInterface *di,
      WaylandClient *client, const FenceInterface *fi,
      struct wl_egl_window *window, int buffers)
{
   DisplayInterfaceWlClient *self = calloc(1, sizeof(*self));
   if (self)
   {
      self->client = client;
      self->fence_interface = fi;
      self->window = window;

      ring_buffer_init(&self->fences, buffers, sizeof(int));

      di->base.context = self;
      di->base.destroy = destroy;
      di->display = display;
      di->wait_sync = wait_sync;
      di->stop = stop;
   }
   return self != NULL;
}
