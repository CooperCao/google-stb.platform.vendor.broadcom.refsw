/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "wl_client.h"

#include "wayland_egl/wayland_egl_priv.h"
#include "nexus_base_mmap.h"
#include "nexus_platform.h"

#include <wayland-client.h>
#include <stdint.h>
#include <memory.h>
#include <assert.h>
#include <stdio.h>

#define UNUSED(x) (void)(x)

static NEXUS_PixelFormat GetNativeFormat(BEGL_BufferFormat beglFormat)
{
   NEXUS_PixelFormat format;
   switch (beglFormat)
   {
   case BEGL_BufferFormat_eA8B8G8R8: format = NEXUS_PixelFormat_eA8_B8_G8_R8; break;
   case BEGL_BufferFormat_eX8B8G8R8: format = NEXUS_PixelFormat_eX8_B8_G8_R8; break;
   case BEGL_BufferFormat_eR5G6B5:   format = NEXUS_PixelFormat_eR5_G6_B5; break;
   default: format = NEXUS_PixelFormat_eUnknown; break;
   }
   return format;
}

bool CreateWlSharedBuffer(WlSharedBuffer *buffer, WlClient *client,
      uint32_t width, uint32_t height, BEGL_BufferFormat format, bool secure,
      void (*release)(void *data, struct wl_buffer *wl_buffer), void *data)
{
   assert(buffer != NULL);

   memset(buffer, 0, sizeof(*buffer));

   NEXUS_SurfaceCreateSettings surfSettings;
   NEXUS_Surface_GetDefaultCreateSettings(&surfSettings);
   surfSettings.pixelFormat = GetNativeFormat(format);
   surfSettings.width = width;
   surfSettings.height = height;
   surfSettings.compatibility.graphicsv3d = true;

   if (secure)
   {
      surfSettings.heap = NEXUS_Platform_GetFramebufferHeap(
            NEXUS_OFFSCREEN_SECURE_GRAPHICS_SURFACE);
      if (!surfSettings.heap)
         goto failed;
   }

   buffer->handle = NEXUS_Surface_Create(&surfSettings);
   if (!buffer->handle)
   {
      fprintf(stderr, "%s: unable to create Nexus surface\n", __FUNCTION__);
      goto failed;
   }

   NEXUS_Addr devPtr;
   NEXUS_SurfaceMemoryProperties memProperties;
   NEXUS_Surface_GetMemoryProperties(buffer->handle, &memProperties);
   NEXUS_Error err = NEXUS_MemoryBlock_LockOffset(memProperties.pixelMemory, &devPtr);
   if (err != NEXUS_SUCCESS)
   {
      printf("%s: unable to lock physical offset\n", __FUNCTION__);
      goto failed;
   }

   NEXUS_SurfaceMemory surfMemory;
   err = NEXUS_Surface_GetMemory(buffer->handle, &surfMemory);
   if (err != NEXUS_SUCCESS)
   {
      printf("%s: unable to get surface memory\n", __FUNCTION__);
      goto failed;
   }

   NEXUS_SurfaceStatus surfStatus;
   NEXUS_Surface_GetStatus(buffer->handle, &surfStatus);

   NEXUS_Surface_Flush(buffer->handle);

   NEXUS_MemoryBlockTokenHandle token = NEXUS_MemoryBlock_CreateToken(
         memProperties.pixelMemory);

   /* store a possibly 64-bit memory token in wl_array due to
    * a lack of 64-bit integer type in Wayland protocol */
   struct wl_array wrapped_token;
   wl_array_init(&wrapped_token);
   uint64_t *value = (uint64_t *)wl_array_add(&wrapped_token, sizeof(*value));
   assert(sizeof(token) <= sizeof(value));
   *value = (uint64_t)(uintptr_t)token;

   buffer->buffer = wl_nexus_buffer_create(client->nexus, format, width, height,
         secure, surfStatus.pitch, surfMemory.bufferSize, &wrapped_token);
   if (!buffer->buffer)
      goto failed;

   if (release)
   {
      buffer->listener.release = release;
      wl_buffer_add_listener(buffer->buffer, &buffer->listener, data);
   }

   buffer->settings.cachedAddr = surfMemory.buffer;
   buffer->settings.physicalOffset = (uint32_t)devPtr;

   buffer->settings.width = surfStatus.width;
   buffer->settings.height = surfStatus.height;
   buffer->settings.pitchBytes = surfStatus.pitch;
   buffer->settings.byteSize = surfMemory.bufferSize;
   buffer->settings.secure = surfMemory.buffer == NULL;
   buffer->settings.format = format;
   buffer->settings.miplevels = 1;
   buffer->settings.colorimetry = BEGL_Colorimetry_RGB;
   buffer->settings.contiguous = true;

   return true;

failed:
   if (buffer->handle)
      NEXUS_Surface_Destroy(buffer->handle);
   buffer->handle = NULL;
   return false;
}

void DestroyWlSharedBuffer(WlSharedBuffer *buffer)
{
   assert(buffer != NULL);

   wl_buffer_destroy(buffer->buffer);

   NEXUS_SurfaceMemoryProperties memProperties;
   NEXUS_Surface_GetMemoryProperties(buffer->handle, &memProperties);
   NEXUS_MemoryBlock_UnlockOffset(memProperties.pixelMemory);

   NEXUS_Surface_Destroy(buffer->handle);

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
