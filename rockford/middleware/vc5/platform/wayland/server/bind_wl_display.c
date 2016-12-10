/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include "bind_wl_display.h"

#include "private_nexus.h"
#include "wayland_nexus_server.h"
#include "display_helpers.h"
#include "display_surface.h"
#include "nexus_platform.h"
#include "nexus_base_mmap.h"
#include "nxserverlib.h"

#include <wayland-server.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_wayland.h>

typedef struct ClientHandle
{
   struct wl_client *client;
   NEXUS_ClientHandle handle;
   struct wl_list link;
} ClientHandle;

typedef struct ClientBuffer
{
   BEGL_SurfaceInfo info;
} ClientBuffer;

typedef struct ClientDestroyListener
{
   WLServer_DisplayContext *context;
   struct wl_listener listener;
} ClientDestroyListener;

static ClientHandle *ClientFind(const struct wl_list *client_list,
      const struct wl_client *search)
{
   ClientHandle *client;
   wl_list_for_each(client, client_list, link)
   {
      if (client->client == search)
      {
         return client;
      }
   }
   return NULL;
}

static void ClientDelete(struct wl_list *client_list, struct wl_client *client)
{
   ClientHandle *found = ClientFind(client_list, client);
   if (found)
   {
      NEXUS_Platform_UnregisterClient(found->handle);
      wl_list_remove(&found->link);
      free(found);
   }
}

static void ClientDeleteAll(struct wl_list *client_list)
{
   while (!wl_list_empty(client_list))
   {
      ClientHandle *client = wl_container_of(client_list->next, client,
            link);
      wl_list_remove(&client->link);
      free(client);
   }
}

static void ClientDeleted(struct wl_listener *listener, void *data)
{
   ClientDestroyListener *cdl = wl_container_of(listener, cdl, listener);
   struct wl_client *client = (struct wl_client *) data;
   ClientDelete(&cdl->context->clients, client);
   free(cdl);
}

static ClientDestroyListener *CreateClientDestroyListener(
      WLServer_DisplayContext *wl_context, struct wl_client *client)
{
   ClientDestroyListener *cdl = calloc(1, sizeof(*cdl));
   if (cdl)
   {
      cdl->context = wl_context;
      cdl->listener.notify = ClientDeleted;
      wl_client_add_destroy_listener(client, &cdl->listener);
   }
   return cdl;
}

static NEXUS_Certificate CreateClientCertificate(struct wl_client *client)
{
   NEXUS_Certificate Certificate;
   BKNI_Memset(&Certificate, 0, sizeof(Certificate));
   Certificate.length = BKNI_Snprintf((char *) &Certificate.data,
         sizeof(Certificate.data), "%p,%08x%08x,nexus-host-signature", client,
         lrand48(), lrand48());
   assert(Certificate.length <= sizeof(Certificate.data));
   return Certificate;
}

static void Request_ClientJoin(struct wl_client *client,
      struct wl_resource *resource)
{
   WLServer_DisplayContext *wl_context =
         (WLServer_DisplayContext *) resource->data;

   NEXUS_PlatformConfiguration platformConfiguration;
   NEXUS_Platform_GetConfiguration(&platformConfiguration);

   NEXUS_ClientSettings clientSettings;
   NEXUS_Platform_GetDefaultClientSettings(&clientSettings);
   clientSettings.authentication.certificate = CreateClientCertificate(client);
   clientSettings.configuration.mode = NEXUS_ClientMode_eVerified;

   struct nxserver_settings settings;
   nxserverlib_get_settings(nxserverlib_get_singleton(), &settings);

   unsigned int i;
   for (i = 0; i < NEXUS_MAX_HEAPS; ++i)
   {
      if (settings.client.heap[i] != NULL)
      {
         break;
      }
   }
   if (i == NEXUS_MAX_HEAPS)
   {
      goto error;
   }

   unsigned empty_slot = i;

   for (i = 0; i < NEXUS_MAX_HEAPS; ++i)
   if (platformConfiguration.heap[i] != NULL)
   {
      unsigned int j;
      for (j = 0; j < NEXUS_MAX_HEAPS; j++)
      if (settings.client.heap[j] == platformConfiguration.heap[i])
      break;

      if (j == NEXUS_MAX_HEAPS)
      {
         /* not found */
         BDBG_ASSERT(empty_slot != NEXUS_MAX_HEAPS);
         settings.client.heap[empty_slot] = platformConfiguration.heap[i];
         ++empty_slot;
      }
   }

   BKNI_Memcpy(clientSettings.configuration.heap, settings.client.heap,
         sizeof(clientSettings.configuration.heap));

   ClientHandle *clientHandle = calloc(1, sizeof(*clientHandle));
   if (!clientHandle)
   {
      goto error;
   }
   clientHandle->client = client;
   clientHandle->handle = NEXUS_Platform_RegisterClient(&clientSettings);
   if (clientHandle->handle == NULL)
   {
      goto error;
   }

   wl_list_insert(&wl_context->clients, &clientHandle->link);
   CreateClientDestroyListener(wl_context, clientHandle->client);

   wl_nexus_send_client_certificate(resource,
         (const char *) clientSettings.authentication.certificate.data,
         clientSettings.authentication.certificate.length);

   return;

   error: free(clientHandle);
   wl_resource_post_no_memory(resource);
   return;
}

static void BufferInterface_Delete(struct wl_client *client,
      struct wl_resource *resource)
{
   BSTD_UNUSED(client);
   wl_resource_destroy(resource);
}

static const struct wl_buffer_interface bufferInterface = {
      BufferInterface_Delete
};

static void BufferDelete(struct wl_resource *resource)
{
   free(resource->data);
   resource->data = NULL;
}

static void Request_BufferCreate(struct wl_client *client,
      struct wl_resource *resource, uint32_t id, uint32_t width, uint32_t pitch,
      uint32_t height, uint32_t format, struct wl_array *physical)
{
   WLServer_DisplayContext *wl_context =
         (WLServer_DisplayContext *) resource->data;

   ClientBuffer *cb = calloc(1, sizeof(*cb));
   if (!cb)
      goto error;

   struct wl_resource *buffer = wl_resource_create(client,
         &wl_buffer_interface, 1, id);
   if (!buffer)
      goto error;

   cb->info.width = width;
   cb->info.height = height;
   cb->info.pitchBytes = pitch;
   cb->info.physicalOffset = *(const uint64_t *) physical->data;
   cb->info.cachedAddr = NEXUS_OffsetToCachedAddr(cb->info.physicalOffset);
   cb->info.byteSize = pitch * height;
   cb->info.format = (BEGL_BufferFormat)format;

   wl_resource_set_implementation(buffer, &bufferInterface, cb,
         BufferDelete);
   return;

error:
   free(cb);
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
         .client_join = Request_ClientJoin,
         .buffer_create = Request_BufferCreate,
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

   wl_list_init(&wl_context->clients);

   return true;
}

bool UnbindWlDisplay(WLServer_DisplayContext *wl_context,
      struct wl_display *wl_display)
{
   if (!wl_display || wl_display != wl_context->display)
      return false;

   ClientDeleteAll(&wl_context->clients);

   wl_global_destroy(wl_context->global);
   wl_context->global = NULL;
   wl_context->display = NULL;
   return true;
}

static ClientBuffer *GetClientBuffer(
      WLServer_DisplayContext *wl_context,
      struct wl_resource *bufferResource)
{
   if (!wl_context->display || !bufferResource || !bufferResource->data)
      return NULL;

   if (!wl_resource_instance_of(bufferResource, &wl_buffer_interface,
         &bufferInterface))
      return NULL; /* not one of ours */

   return (ClientBuffer *)bufferResource->data;
}

bool GetWlSurfaceInfo(WLServer_DisplayContext *wl_context,
      struct wl_resource *bufferResource, BEGL_SurfaceInfo *info)
{
   ClientBuffer *cb = GetClientBuffer(wl_context, bufferResource);
   if (cb && info)
   {
      *info = cb->info;
      return true;
   }
   return false;
}

bool QueryWlBuffer(WLServer_DisplayContext *wl_context,
      struct wl_resource *bufferResource, int32_t attribute, int32_t *value)
{
   BEGL_SurfaceInfo info;
   if (!GetWlSurfaceInfo(wl_context, bufferResource, &info))
   {
      return false;
   }

   switch (attribute)
   {
   case EGL_WIDTH:
      *value = info.width;
      break;

   case EGL_HEIGHT:
      *value = info.height;
      break;

   case EGL_TEXTURE_FORMAT:
   {
      NEXUS_PixelFormat result = NEXUS_PixelFormat_eUnknown;
      BeglToNexusFormat(&result, info.format);
      switch (result)
      {
      case NEXUS_PixelFormat_eA8_B8_G8_R8:
         *value = EGL_TEXTURE_RGBA;
         break;

      case NEXUS_PixelFormat_eR5_G6_B5:
         *value = EGL_TEXTURE_RGB;
         break;

      default:
         return false;
      }
      break;
   }
   case EGL_WAYLAND_Y_INVERTED_WL:
      *value = EGL_TRUE;
      break;

   default:
      return false;
   }
   return true;
}
