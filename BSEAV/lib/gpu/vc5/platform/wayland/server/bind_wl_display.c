/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bind_wl_display.h"

#include "wayland_nexus_server.h"
#include "display_surface.h"

#include <wayland-server.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_wayland.h>

#define CLIENT_BUFFER_MAGIC 0x574c4342 /* "WLCB" */

typedef struct ClientHandle
{
   struct wl_client *client;
   NEXUS_ClientHandle handle;
   struct wl_list link;
} ClientHandle;

static void ClientBufferRefInc(ClientBuffer *cb)
{
   int before = __sync_fetch_and_add(&cb->refcount, 1);
   assert(before > 0);
}

static void ClientBufferRefDec(ClientBuffer *cb)
{
   int before = __sync_fetch_and_sub(&cb->refcount, 1);
   assert(before > 0);
   if (before == 1)
   {
      if (cb->surface.physicalOffset)
         DestroySurface(&cb->surface);
      free(cb);
   }
}

static void ClientBufferDelete(struct wl_resource *resource)
{
   ClientBuffer *cb = (ClientBuffer *)resource->data;
   ClientBufferRefDec(cb);
   resource->data = NULL;
}

static void BufferInterface_Delete(struct wl_client *client,
      struct wl_resource *resource)
{
   BSTD_UNUSED(client);
   wl_resource_destroy(resource); /*this will call ClientBufferDelete() */
}

static const struct wl_buffer_interface bufferInterface = {
      BufferInterface_Delete
};

static void BufferCreate(struct wl_client *client,
      struct wl_resource *resource, uint32_t id, uint32_t width,
      uint32_t height, uint32_t format, int secure)
{
   WLServer_DisplayContext *wl_context =
         (WLServer_DisplayContext *) resource->data;

   ClientBuffer *cb = calloc(1, sizeof(*cb));
   if (!cb)
      goto no_cb;

   cb->magic = CLIENT_BUFFER_MAGIC;
   cb->refcount = 1;

   struct wl_resource *buffer = wl_resource_create(client,
         &wl_buffer_interface, 1, id);
   if (!buffer)
      goto no_buffer;

   wl_resource_set_implementation(buffer, &bufferInterface, cb,
         ClientBufferDelete);

   if (!CreateSurface(&cb->surface, (BEGL_BufferFormat)format, width, height,
         secure, "Wayland client buffer"))
      goto no_surface;

   assert(cb->surface.physicalOffset != 0);

   /* store 64-bit physical offset in wl_array due to
    * a lack of 64-bit integer type in Wayland protocol */
   struct wl_array array;
   wl_array_init(&array);
   uint64_t *offset = (uint64_t *)wl_array_add(&array, sizeof(*offset));
   *offset = cb->surface.physicalOffset;

   /* get line pitch from Nexus */
   NEXUS_SurfaceStatus surfStatus;
   NEXUS_Surface_GetStatus(cb->surface.surface, &surfStatus);

   wl_nexus_send_buffer_created(resource, buffer, &array, surfStatus.pitch);

   wl_array_release(&array);
   return;

no_surface:
   /* this is non-fatal error but created wl_buffer is not usable */
   wl_nexus_send_buffer_out_of_memory(resource, buffer);
   return;

   /* the error below are fatal and result in client termination */
no_buffer:
   free(cb);
no_cb:
   wl_resource_post_no_memory(resource);
   return;
}

static void BindNexus(struct wl_client *client, void *data, uint32_t version,
      uint32_t id)
{
   WLServer_DisplayContext *wl_context =
         (WLServer_DisplayContext *) data;

   wl_context->nexus = wl_resource_create(client, &wl_nexus_interface,
         version, id);
   if (wl_context->nexus == NULL)
   {
      wl_client_post_no_memory(client);
      return;
   }

   const static struct wl_nexus_interface nexusInterface =
   {
         .buffer_create = BufferCreate,
   };
   wl_resource_set_implementation(wl_context->nexus, &nexusInterface,
         wl_context, NULL);
}

bool BindWlDisplay(WLServer_DisplayContext *wl_context,
      struct wl_display *wl_display)
{
   if (!wl_display || wl_context->display)
      return false;

   wl_context->global = wl_global_create(wl_display, &wl_nexus_interface, 1,
         wl_context, BindNexus);
   if (!wl_context->global)
      return false;

   wl_context->display = wl_display;

   return true;
}

bool UnbindWlDisplay(WLServer_DisplayContext *wl_context,
      struct wl_display *wl_display)
{
   if (!wl_display || wl_display != wl_context->display)
      return false;

   wl_global_destroy(wl_context->global);
   wl_context->global = NULL;
   wl_context->display = NULL;
   return true;
}


bool IsClientBuffer(const void *buffer)
{
   ClientBuffer *cb = (ClientBuffer *)buffer;
   return cb && (cb->magic == CLIENT_BUFFER_MAGIC);
}

ClientBuffer *GetClientBuffer(struct wl_resource *resource)
{
   if (!resource || !resource->data)
      return NULL;

   if (!wl_resource_instance_of(resource, &wl_buffer_interface,
         &bufferInterface))
      return NULL; /* not one of ours */

   return IsClientBuffer(resource->data) ? resource->data : NULL;
}

BEGL_Error ChangeWlBufferRefCount(ClientBuffer *cb,
      BEGL_RefCountMode incOrDec)
{
   if (!cb)
      return BEGL_Fail;

   if (incOrDec == BEGL_Increment)
      ClientBufferRefInc(cb);
   else
      ClientBufferRefDec(cb);
   return BEGL_Success;
}
