/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "surface_interface_wl.h"

#include "display_helpers.h"

#ifndef NDEBUG
#include <string.h>
#endif

typedef struct WlSurfaceInterface
{
   WlClient *client;
   void (*release)(void *data, struct wl_buffer *wl_buffer);
   void *data;
} WlSurfaceInterface;

static void destroy_surface(void *context, void *surface)
{
   WlWindowBuffer *buffer = (WlWindowBuffer *)surface;
   DestroyWlSharedBuffer(&buffer->buffer);

#ifndef NDEBUG
   memset(buffer, 0, sizeof(*buffer));
#endif
   /*the memory block isnt't ours to free */
}

static bool create_surface(void *context, void *surface, uint32_t width,
      uint32_t height, BEGL_BufferFormat format, bool secure)
{
   WlSurfaceInterface *self = (WlSurfaceInterface *)context;
   WlWindowBuffer *buffer = (WlWindowBuffer *)surface;

   memset(buffer, 0, sizeof(*buffer));
   return CreateWlSharedBuffer(&buffer->buffer, self->client, width, height,
         (BEGL_BufferFormat)format, secure, self->release, self->data);
}

static void destroy(void *context)
{
   WlSurfaceInterface *self = (WlSurfaceInterface *)context;
#ifndef NDEBUG
   memset(self, 0, sizeof(*self));
#endif
   free(self);
}

bool SurfaceInterface_InitWayland(SurfaceInterface *si, WlClient *client,
      void (*release)(void *data, struct wl_buffer *wl_buffer), void *data)
{
   WlSurfaceInterface *self = calloc(1, sizeof(*self));
   if (!self)
      return false;

   self->client = client;
   self->release = release;
   self->data = data;

   si->base.context = self;
   si->base.destroy = destroy;
   si->sizeof_surface = sizeof(WlWindowBuffer);
   si->create = create_surface;
   si->destroy = destroy_surface;

   return true;
}
