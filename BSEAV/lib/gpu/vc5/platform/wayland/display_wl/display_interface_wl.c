/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 ******************************************************************************/
#include "display_interface_wl.h"

#include "surface_interface_wl.h"
#include <pthread.h>
#include <assert.h>

typedef struct DisplayItem
{
   struct wl_buffer *buffer;
   int fence;
} DisplayItem;

typedef struct WlDisplayInterface
{
   WlClient *client;
   const FenceInterface *fence_interface;
   WlWindow window;

   pthread_t thread;
   bool stop;
   pthread_mutex_t mutex;
   size_t buffers;
   DisplayItem displayed[0]; /* variable size array */
} WlDisplayInterface;

static bool add_display_item(WlDisplayInterface *self,
      struct wl_buffer *buffer, int fence)
{
   bool added = false;
   pthread_mutex_lock(&self->mutex);
   for (unsigned i = 0; i < self->buffers; i++)
   {
      if (!self->displayed[i].buffer)
      {
         self->displayed[i].buffer = buffer;
         self->displayed[i].fence = fence;
         added = true;
         break;
      }
   }
   pthread_mutex_unlock(&self->mutex);
   return added;
}

static int remove_display_item(WlDisplayInterface *self,
      struct wl_buffer *wl_buffer)
{
   int fence = self->fence_interface->invalid_fence;
   pthread_mutex_lock(&self->mutex);
   for (unsigned i = 0; i < self->buffers; i++)
   {
      if (self->displayed[i].buffer && self->displayed[i].buffer == wl_buffer)
      {
         fence = self->displayed[i].fence;
         self->displayed[i].buffer = NULL;
         self->displayed[i].fence = self->fence_interface->invalid_fence;
         break;
      }
   }
   pthread_mutex_unlock(&self->mutex);
   return fence;
}

static DisplayInterfaceResult display(void *context, void *surface,
      int render_fence, bool create_display_fence, int *display_fence)
{
   WlDisplayInterface *self = (WlDisplayInterface *)context;
   WlWindowBuffer *buffer = (WlWindowBuffer *)surface;

   FenceInterface_WaitAndDestroy(self->fence_interface, &render_fence);

   if (create_display_fence)
   {
      FenceInterface_Create(self->fence_interface, display_fence);
      struct wl_buffer *wl_buffer = buffer->buffer.buffer;
      bool added = add_display_item(self, wl_buffer, *display_fence);
      assert(added);
   }
   else
      *display_fence = self->fence_interface->invalid_fence;

   if (DisplayWlSharedBuffer(&self->window, &buffer->buffer, buffer->dx,
         buffer->dy))
   {
      buffer->dx = buffer->dy = 0;
      WaitWlRelease(&self->window, /*block=*/false);
      return eDisplayPending;
   }
   else
      return eDisplayFailed;
}

static bool wait_sync(void *context)
{
   WlDisplayInterface *self = (WlDisplayInterface *)context;
   return WaitWlFrame(&self->window);
}

static void stop(void *context)
{
   WlDisplayInterface *self = (WlDisplayInterface *)context;

   /* stop the buffer release thread */
   self->stop = true;
   wl_display_roundtrip_queue(self->window.client->display,
         self->window.release_events);
   pthread_join(self->thread, NULL);

   /* signal any remaining fences */
   pthread_mutex_lock(&self->mutex);
   for (unsigned i = 0; i < self->buffers; i++)
   {
      FenceInterface_Signal(self->fence_interface, self->displayed[i].fence);
      self->displayed[i].fence = self->fence_interface->invalid_fence;
   }
   pthread_mutex_unlock(&self->mutex);
}

static void destroy(void *context)
{
   WlDisplayInterface *self = (WlDisplayInterface *)context;
   if (self)
   {
      stop(self);
      pthread_mutex_destroy(&self->mutex);
      DestroyWlWindow(&self->window);
      free(self);
   }
}

static void *release_thread_func(void *p)
{
   WlDisplayInterface *self = (WlDisplayInterface *)p;
   while(!self->stop && WaitWlRelease(&self->window, /*block=*/true) >= 0)
      continue;
   return NULL;
}

bool DisplayInterface_InitWayland(DisplayInterface *di, WlClient *client,
      const FenceInterface *fi, struct wl_egl_window *window, unsigned buffers)
{
   WlDisplayInterface *self;
   size_t size = sizeof(*self) + buffers * sizeof(self->displayed[0]);
   self = calloc(1, size);
   if (!self)
      goto end;

   if (!CreateWlWindow(&self->window, client, window))
      goto cleanup_self;

   if (pthread_mutex_init(&self->mutex, NULL) != 0)
      goto cleanup_window;

   self->stop = false;
   if (pthread_create(&self->thread, NULL, release_thread_func, self) != 0)
      goto cleanup_mutex;

   self->client = client;
   self->fence_interface = fi;
   self->buffers = buffers;
   for (unsigned i = 0; i < buffers; i++)
      self->displayed[i].fence = fi->invalid_fence;

   di->base.context = self;
   di->base.destroy = destroy;
   di->display = display;
   di->wait_sync = wait_sync;
   di->stop = stop;
   goto end;

cleanup_mutex:
   pthread_mutex_destroy(&self->mutex);

cleanup_window:
   DestroyWlWindow(&self->window);

cleanup_self:
   free(self);
   self = NULL;

end:
   return self != NULL;
}

void DisplayInterface_WlBufferRelease(DisplayInterface *di,
      struct wl_buffer *wl_buffer)
{
   WlDisplayInterface *self = (WlDisplayInterface *)di->base.context;
   int fence = remove_display_item(self, wl_buffer);
   FenceInterface_Signal(self->fence_interface, fence);
}
