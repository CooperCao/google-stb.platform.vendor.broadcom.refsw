/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "wl_client.h"

#include "wayland_egl/wayland_egl_priv.h"
#include "nexus_base_mmap.h"

#include <wayland-client.h>
#include <stdint.h>
#include <memory.h>
#include <assert.h>
#include <stdio.h>

#define UNUSED(x) (void)(x)

static void BufferCreated(void *data, struct wl_nexus *wl_nexus,
      struct wl_buffer *wl_buffer, struct wl_array *physical)
{
   UNUSED(data);
   UNUSED(wl_nexus);
   WlSharedBuffer *buffer = wl_buffer_get_user_data(wl_buffer);

   buffer->settings.physicalOffset = *(const uint64_t *)physical->data;
   buffer->settings.cachedAddr = NEXUS_OffsetToCachedAddr(
         buffer->settings.physicalOffset);
}

static void BufferOutOfMemory(void *data, struct wl_nexus *wl_nexus,
      struct wl_buffer *wl_buffer)
{
   UNUSED(data);
   UNUSED(wl_nexus);
   WlSharedBuffer *buffer = wl_buffer_get_user_data(wl_buffer);

   buffer->settings.physicalOffset = 0;
   buffer->settings.cachedAddr = NULL;
}

bool CreateWlSharedBuffer(WlSharedBuffer *buffer, WlClient *client,
      const BEGL_SurfaceInfo *settings, bool secure,
      void (*release)(void *data, struct wl_buffer *wl_buffer), void *data)
{
   static const unsigned int alignment = 4096;
   bool result = false;

   assert(buffer != NULL);

   memset(buffer, 0, sizeof(*buffer));

   buffer->buffer = wl_nexus_buffer_create(client->nexus, settings->format,
         settings->width, settings->height, settings->pitchBytes, alignment,
         settings->byteSize, secure);
   if (!buffer->buffer)
      goto end;

   buffer->settings = *settings;
   buffer->settings.cachedAddr = NULL;
   buffer->settings.physicalOffset = 0;

   /* The "user_data" pointer is shared between several mechanisms and gets
    * overwritten, for example, when a listener is added. This is temporary.
    */
   wl_buffer_set_user_data(buffer->buffer, buffer);

   /* trigger the event callback now */
   wl_proxy_set_queue((struct wl_proxy *)buffer->buffer, client->buffer_events);
   wl_display_roundtrip_queue(client->display, client->buffer_events);

   if (!buffer->settings.physicalOffset)
      goto no_physical;

   /* Attaching a listener overwrites the user_data pointer set above in the
    * wl_buffer_set_user_data(). We're past the callback so it's safe now.
    */
   if (release)
   {
      buffer->listener.release = release;
      wl_buffer_add_listener(buffer->buffer, &buffer->listener, data);
   }

   result = true;
   goto end;

   no_physical: DestroyWlSharedBuffer(buffer);
   end: return result;
}

void DestroyWlSharedBuffer(WlSharedBuffer *buffer)
{
   assert(buffer != NULL);

   if (buffer->buffer)
      wl_buffer_destroy(buffer->buffer);

#ifndef NDEBUG
   memset(buffer, 0, sizeof(*buffer));
#endif
   /*the memory block isnt't ours to free */
}

static void RegistryGlobalAdd(void *data, struct wl_registry *registry,
      uint32_t name, const char *interface, uint32_t version)
{
   if (strcmp(interface, "wl_nexus") == 0)
   {
      WlClient *client = (WlClient *)data;
      client->nexus = (struct wl_nexus *)wl_registry_bind(registry, name,
            &wl_nexus_interface, version);
      wl_proxy_set_queue((struct wl_proxy *)client->nexus,
            client->buffer_events);
   }
}

static void RegistryGlobalRemove(void *data, struct wl_registry *registry,
      uint32_t name)
{
   UNUSED(data);
   UNUSED(registry);
   UNUSED(name);
   /* nothing to do here */
}

void DestroyWlClient(WlClient *client)
{
   assert(client != NULL);

   if (client->nexus) /* uses buffer_events */
   {
      wl_nexus_destroy(client->nexus);
      client->nexus = NULL;
   }

   if (client->registry) /* uses buffer_events */
   {
      wl_registry_destroy(client->registry);
      client->registry = NULL;
   }

   if (client->buffer_events)
   {
      wl_event_queue_destroy(client->buffer_events);
      client->buffer_events = NULL;
   }

   if (client->display)
   {
      if (client->disconnect)
         wl_display_disconnect(client->display);
      client->disconnect = false;
      client->display = NULL;
   }
}

bool CreateWlClient(WlClient *client, struct wl_display *display)
{
   assert(client != NULL);

   memset(client, 0, sizeof(*client));

   if (display)
   {
      client->display = display;
      client->disconnect = false; /* display isn't ours to disconnect */
   }
   else /* no Wayland display provided, try default display, if available */
   {
      client->display = wl_display_connect(NULL);
      client->disconnect = true;

      if (!client->display)
         fprintf(stderr, "failed to connect to the default Wayland display\n");
   }
   if (!client->display)
      return false;

   client->registry = wl_display_get_registry(client->display);
   if (!client->registry)
      goto error;

   client->buffer_events = wl_display_create_queue(client->display);
   if (!client->buffer_events)
      goto error;

   static const struct wl_registry_listener registryGlobalListener =
   { RegistryGlobalAdd, RegistryGlobalRemove };

   wl_registry_add_listener(client->registry, &registryGlobalListener, client);
   wl_proxy_set_queue((struct wl_proxy *)client->registry,
         client->buffer_events);
   wl_display_roundtrip_queue(client->display, client->buffer_events);
   if (client->nexus == NULL)
   {
      fprintf(stderr,
            "ERROR: Wayland server doesn't support the wl_nexus protocol\n");
      goto error;
   }

   static const struct wl_nexus_listener nexusListener =
   {
         .buffer_created = BufferCreated,
         .buffer_out_of_memory = BufferOutOfMemory,
   };
   wl_nexus_add_listener(client->nexus, &nexusListener, NULL);

   return true;

error:
   DestroyWlClient(client);
   return false;
}

bool CreateWlWindow(WlWindow *window, WlClient *client,
      struct wl_egl_window *wl_egl_window)
{
   assert(window != NULL);

   if (!client || !client->display || !wl_egl_window)
      goto error;

   memset(window, 0, sizeof(*window));

   window->client = client;
   window->window = wl_egl_window;
   window->frame_events = wl_display_create_queue(client->display);
   if (!window->frame_events)
      goto error;
   window->release_events = wl_display_create_queue(client->display);
   if (!window->release_events)
      goto error;
   return true;

error:
   DestroyWlWindow(window);
   return false;
}

void DestroyWlWindow(WlWindow *window)
{
   assert(window != NULL);

   if (window->frame_events)
      wl_event_queue_destroy(window->frame_events);
   if (window->release_events)
      wl_event_queue_destroy(window->release_events);

#ifndef NDEBUG
   memset(window, 0, sizeof(*window));
#endif
}

static void FrameCb(void *data, struct wl_callback *callback, uint32_t time)
{
   WlWindow *window = (WlWindow *)data;
   UNUSED(time);

   assert(callback == window->frame_callback);
   window->frame_callback = NULL;
   wl_callback_destroy(callback);
}

static bool SetFrameCb(WlWindow *window)
{
   if (window->frame_callback)
      return false; /* already set */

   window->frame_callback = wl_surface_frame(window->window->surface);
   /* TODO: possible race:
    * something might have dispatched event before we set the right queue
    */
   static const struct wl_callback_listener frame_listener =
   { FrameCb };
   wl_proxy_set_queue((struct wl_proxy *)window->frame_callback,
         window->frame_events);
   wl_callback_add_listener(window->frame_callback, &frame_listener, window);
   return true;
}

bool DisplayWlSharedBuffer(WlWindow *window, WlSharedBuffer *buffer,
      int dx, int dy)
{
   /* process any pending events from the last frame */
   wl_display_dispatch_queue_pending(window->client->display,
         window->frame_events);

   /* prepare the frame callback prior to any actions */
   SetFrameCb(window);

   wl_proxy_set_queue((struct wl_proxy *)buffer->buffer,
         window->release_events);

   /* attach the new buffer to the surface */
   wl_surface_attach(window->window->surface, buffer->buffer, dx, dy);

#if 0
   /* This isn't thread safe in our implementation, where attach/damage/commit
    * happens asynchronously, not in eglSwapBuffers()
    *
    * The Wayland devs are going to include Mesa implementation
    * of libwayland-eglin Wayland tree as the one and only implementation.
    * Mesa doesn't have a thread safe way of setting the attached size because
    * in Mesa it's done synchronously in eglSwapBuffers() and implicit
    * cross-process synchronisation takes care of the buffer access.
    *
    * In our case it's done asynchronously in a display thread because we
    * dont't support implicit cross-process synchronisation.
    *
    * Once Wayland has explicit fence support and we've got fences that
    * Wayland compositors can use, this code should be re-enabled and
    * the attached size callback in wayland-egl will no longer be needed.
    *
    * Hopefully this happens before Wayland adopts Mesa implementation
    * of libwayland-egl.
    */
   window->window->attached_width = buffer->settings.width;
   window->window->attached_height = buffer->settings.height;
#endif

   wl_surface_damage(window->window->surface, 0, 0,
         buffer->settings.width, buffer->settings.height);

   /* send the buffer accross to the server */
   wl_surface_commit(window->window->surface);

   /* flush the fd */
   return wl_display_flush(window->client->display) != -1;
}

bool WaitWlFrame(WlWindow *window)
{
   if (SetFrameCb(window))
   {
      /* re-submit the surface, otherwise we won't get another frame callback */
      wl_surface_commit(window->window->surface);
      wl_display_flush(window->client->display);
   }

   int ret = 0;
   while (window->frame_callback && ret != -1)
      ret = wl_display_dispatch_queue(window->client->display,
            window->frame_events);
   return (ret >= 0);
}

int WaitWlRelease(WlWindow *window, bool block)
{
   int ret;
   if (block)
      ret = wl_display_dispatch_queue(window->client->display,
            window->release_events);
   else
      ret = wl_display_dispatch_queue_pending(window->client->display,
            window->release_events);
   return ret;
}
