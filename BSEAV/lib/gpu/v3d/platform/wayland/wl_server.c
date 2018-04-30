/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "wl_server.h"

#include "wayland_nexus_server.h"
#include "nexus_base_mmap.h"
#include "nexus_platform.h"

#include <wayland-server.h>
#include <EGL/begl_platform.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

typedef struct WlBufferMemory
{
   BEGL_BufferSettings settings;
   NEXUS_MemoryBlockHandle handle;
} WlBufferMemory;

static void BufferDestroy(struct wl_resource *resource)
{
   WlBufferMemory *memory = wl_resource_get_user_data(resource);
   wl_resource_set_user_data(resource, NULL);
   if (!memory)
      return;

   ReleaseWlBufferMemory(memory->handle);

#ifndef NDEBUG
   memset(memory, 0, sizeof(*memory));
#endif
   free(memory);
}

static void BufferInterface_Delete(struct wl_client *client,
      struct wl_resource *resource)
{
   BSTD_UNUSED(client);
   wl_resource_destroy(resource); /*this will call BufferDestroy() */
}

static const struct wl_buffer_interface bufferInterface =
{ BufferInterface_Delete };

static void BufferCreate(struct wl_client *client, struct wl_resource *resource,
      uint32_t id, uint32_t format, uint32_t width, uint32_t height, int secure,
      uint32_t pitch, uint32_t size, struct wl_array *wrapped_token)
{
   WlDisplayBinding *binding = wl_resource_get_user_data(resource);

   /* unwrap memory block token */
   uint64_t value = *(const uint64_t *)wrapped_token->data;
   NEXUS_MemoryBlockTokenHandle token;
   assert(sizeof(token) <= sizeof(value));
   token = (NEXUS_MemoryBlockTokenHandle)(uintptr_t)value;

   struct wl_resource *buffer = wl_resource_create(client, &wl_buffer_interface,
         1, id);
   if (!buffer)
      goto no_buffer;

   WlBufferMemory *memory = calloc(1, sizeof(*memory));
   if (!memory)
      goto no_memory;

   memory->handle = NEXUS_MemoryBlock_Clone(token);
   if (!memory->handle)
      goto no_handle;

   /* the token is no longer valid after clone */
   token = NULL;

   NEXUS_Addr devPtr;
   NEXUS_Error err = NEXUS_MemoryBlock_LockOffset(memory->handle, &devPtr);
   if (err != NEXUS_SUCCESS)
      goto no_offset;

   memory->settings.cachedAddr = secure ? NULL : NEXUS_OffsetToCachedAddr(devPtr);
   memory->settings.physOffset = (uint32_t)devPtr;

   memory->settings.width = width;
   memory->settings.height = height;
   memory->settings.pitchBytes = pitch;
   memory->settings.totalByteSize = size;
   memory->settings.secure = secure;
   memory->settings.format = (BEGL_BufferFormat)format;

   wl_resource_set_implementation(buffer, &bufferInterface, memory,
         BufferDestroy);
   goto end;

no_offset:
   NEXUS_MemoryBlock_Free(memory->handle);
no_handle:
no_memory:
   wl_resource_destroy(buffer);
no_buffer:
   if (token)
      NEXUS_MemoryBlock_DestroyToken(token);
   /* this is fatal and results in client termination */
   wl_resource_post_no_memory(resource);
end:
   return;
}

static void BindNexus(struct wl_client *client, void *data, uint32_t version,
      uint32_t id)
{
   WlDisplayBinding *binding = (WlDisplayBinding *)data;

   struct wl_resource *wl_nexus = wl_resource_create(client,
         &wl_nexus_interface, version, id);
   if (wl_nexus == NULL)
   {
      wl_client_post_no_memory(client);
      return;
   }

   const static struct wl_nexus_interface nexusInterface =
   { .buffer_create = BufferCreate, };
   wl_resource_set_implementation(wl_nexus, &nexusInterface, binding, NULL);

   /* The wl_nexus binding will be destroyed with the client that created it. */
}

bool BindWlDisplay(WlDisplayBinding *binding, struct wl_display *wl_display)
{
   if (!wl_display)
      return false;

   memset(binding, 0, sizeof(*binding));
   binding->wl_display = wl_display;

   binding->wl_global = wl_global_create(wl_display, &wl_nexus_interface, 1,
         binding, BindNexus);
   if (!binding->wl_global)
      return false;

   return true;
}

bool UnbindWlDisplay(WlDisplayBinding *binding, struct wl_display *wl_display)
{
   if (!wl_display || wl_display != binding->wl_display)
      return false;

   wl_global_destroy(binding->wl_global);
   memset(binding, 0, sizeof(*binding));
   return true;
}

static WlBufferMemory *GetData(struct wl_resource *buffer)
{
   if (!buffer || !wl_resource_get_user_data(buffer))
      return false;

   if (!wl_resource_instance_of(buffer, &wl_buffer_interface, &bufferInterface))
      return false; /* not one of ours */

   return (WlBufferMemory *)wl_resource_get_user_data(buffer);
}

bool GetWlBufferSettings(struct wl_resource *buffer,
      BEGL_BufferSettings *settings)
{
   WlBufferMemory *memory = GetData(buffer);
   if (!memory)
      return false;

   *settings = memory->settings;
   return true;
}

NEXUS_MemoryBlockHandle AcquireWlBufferMemory(struct wl_resource *buffer)
{
   NEXUS_MemoryBlockHandle clone = NULL;
   WlBufferMemory *memory = GetData(buffer);
   if (!memory)
      return NULL;

   NEXUS_MemoryBlockTokenHandle token = NEXUS_MemoryBlock_CreateToken(
         memory->handle);
   clone = NEXUS_MemoryBlock_Clone(token);
   if (!clone)
      return NULL;

   NEXUS_Addr devPtr;
   NEXUS_Error err = NEXUS_MemoryBlock_LockOffset(clone, &devPtr);
   if (err != NEXUS_SUCCESS)
   {
      NEXUS_MemoryBlock_Free(clone);
      clone = NULL;
   }

   return clone;
}

void ReleaseWlBufferMemory(NEXUS_MemoryBlockHandle handle)
{
   if (!handle)
      return;

   NEXUS_MemoryBlock_UnlockOffset(handle);
   NEXUS_MemoryBlock_Free(handle);
}
