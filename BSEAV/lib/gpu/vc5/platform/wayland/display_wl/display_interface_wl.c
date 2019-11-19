/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
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
   int fence = INVALID_FENCE;
   pthread_mutex_lock(&self->mutex);
   for (unsigned i = 0; i < self->buffers; i++)
   {
      if (self->displayed[i].buffer && self->displayed[i].buffer == wl_buffer)
      {
         fence = self->displayed[i].fence;
         self->displayed[i].buffer = NULL;
         self->displayed[i].fence = INVALID_FENCE;
         break;
      }
   }
   pthread_mutex_unlock(&self->mutex);
   return fence;
}

static DisplayInterfaceResult display(void *context, void *surface,
      const WindowInfo *windowInfo,
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
      *display_fence = INVALID_FENCE;

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

static void destroy(void *context)
{
   WlDisplayInterface *self = (WlDisplayInterface *)context;
   if (self)
   {
      DestroyWlWindow(&self->window);
      free(self);
   }
}

static void release(void *context)
{
   WlDisplayInterface *self = (WlDisplayInterface *)context;
   WaitWlRelease(&self->window, /*block=*/false);
}

static void stop(void *context)
{
   WlDisplayInterface *self = (WlDisplayInterface *)context;
   /* signal any remaining fences */
   for (unsigned i = 0; i < self->buffers; i++)
   {
      FenceInterface_Signal(self->fence_interface, self->displayed[i].fence);
      self->displayed[i].fence = INVALID_FENCE;
   }
}

bool DisplayInterface_InitWayland(DisplayInterface *di, WlClient *client,
      const FenceInterface *fi, struct wl_egl_window *window, unsigned buffers)
{
   WlDisplayInterface *self;
   size_t size = sizeof(*self) + buffers * sizeof(self->displayed[0]);
   self = calloc(1, size);
   if (self && CreateWlWindow(&self->window, client, window))
   {
      self->client = client;
      self->fence_interface = fi;

      pthread_mutex_init(&self->mutex, NULL);
      self->buffers = buffers;
      for (unsigned i = 0; i < buffers; i++)
         self->displayed[i].fence = INVALID_FENCE;

      di->base.context = self;
      di->base.destroy = destroy;
      di->display = display;
      di->wait_sync = wait_sync;
      di->release = release;
      di->stop = stop;
   }
   else
   {
      free(self);
      self = NULL;
   }
   return self != NULL;
}

void DisplayInterface_WlBufferRelease(DisplayInterface *di,
      struct wl_buffer *wl_buffer)
{
   WlDisplayInterface *self = (WlDisplayInterface *)di->base.context;
   int fence = remove_display_item(self, wl_buffer);
   FenceInterface_Signal(self->fence_interface, fence);
}
