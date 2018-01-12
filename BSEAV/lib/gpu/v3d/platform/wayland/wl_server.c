/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "wl_server.h"

#include "wayland_nexus_server.h"

#include <wayland-server.h>
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_wayland.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

static WlBufferMemory *CreateWlBufferMemory(NEXUS_HeapHandle heap,
      BEGL_BufferFormat format, uint32_t width, uint32_t height, uint32_t pitch,
      uint32_t alignment, uint32_t size)
{
   WlBufferMemory *memory = calloc(1, sizeof(*memory));
   if (!memory)
      return NULL;

   memory->refs = 1;
   memory->handle = NEXUS_MemoryBlock_Allocate(heap, size, alignment, NULL);
   if (memory->handle)
   {
      NEXUS_Addr devPtr;
      NEXUS_Error err = NEXUS_MemoryBlock_LockOffset(memory->handle, &devPtr);
      if (err == NEXUS_SUCCESS)
         memory->settings.physOffset = (uint64_t)devPtr;
      else
         fprintf(stderr, "%s: unable to lock offset\n", __FUNCTION__);

      void *ptr;
      err = NEXUS_MemoryBlock_Lock(memory->handle, &ptr);
      if (err == NEXUS_SUCCESS)
         memory->settings.cachedAddr = ptr;
      else
         fprintf(stderr, "%s: unable to lock\n", __FUNCTION__);
      memory->settings.format = format;
      memory->settings.width = width;
      memory->settings.height = height;
      memory->settings.pitchBytes = pitch;
      memory->settings.totalByteSize = size;
   }
   return memory;
}

static void DestroyWlBufferMemory(WlBufferMemory *memory)
{
   if (!memory)
      return;

   if (memory->handle)
   {
      if (memory->settings.physOffset)
         NEXUS_MemoryBlock_UnlockOffset(memory->handle);
      if (memory->settings.cachedAddr)
         NEXUS_MemoryBlock_Unlock(memory->handle);
      NEXUS_MemoryBlock_Free(memory->handle);
   }

#ifndef NDEBUG
   memset(memory, 0, sizeof(*memory));
#endif
   free(memory);
}

static void BufferDestroy(struct wl_resource *resource)
{
   WlBufferMemory *memory = wl_resource_get_user_data(resource);
   wl_resource_set_user_data(resource, NULL);
   if (memory)
      WlBufferMemoryRefDec(memory);
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
      uint32_t id, uint32_t format, uint32_t width, uint32_t height,
      uint32_t pitch, uint32_t alignment, uint32_t size, int secure)
{
   WlDisplayBinding *binding = wl_resource_get_user_data(resource);

   NEXUS_HeapHandle heap = secure ? binding->secure_heap : binding->heap;
   if (!heap)
   {
      /* this is fatal and results in client termination */
      wl_resource_post_no_memory(resource);
      goto no_heap;
   }

   struct wl_resource *buffer = wl_resource_create(client, &wl_buffer_interface,
         1, id);
   if (!buffer)
   {
      /* this is fatal and results in client termination */
      wl_resource_post_no_memory(resource);
      goto no_buffer;
   }

   WlBufferMemory *memory = CreateWlBufferMemory(heap,
         (BEGL_BufferFormat)format, width, height, pitch, alignment, size);
   if (!memory)
   {
      /* this is fatal and results in client termination */
      wl_resource_post_no_memory(resource);
      goto no_memory;
   }
   if (!memory->handle || !memory->settings.physOffset
         || !memory->settings.cachedAddr)
   {
      /* this is non-fatal error but created wl_buffer is not usable */
      wl_nexus_send_buffer_out_of_memory(resource, buffer);
      goto no_device_memory;
   }

   wl_resource_set_implementation(buffer, &bufferInterface, memory,
         BufferDestroy);

   /* store 64-bit physical offset in wl_array due to
    * a lack of 64-bit integer type in Wayland protocol */
   struct wl_array array;
   wl_array_init(&array);
   uint64_t *offset = (uint64_t *)wl_array_add(&array, sizeof(*offset));
   *offset = memory->settings.physOffset;

   wl_nexus_send_buffer_created(resource, buffer, &array);

   wl_array_release(&array);
   return;

   no_device_memory: DestroyWlBufferMemory(memory);
   no_memory: wl_resource_destroy(buffer);
   no_buffer: no_heap: return;
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

bool BindWlDisplay(WlDisplayBinding *binding, struct wl_display *wl_display,
      NEXUS_HeapHandle heap, NEXUS_HeapHandle secure_heap)
{
   if (!wl_display || !heap)
      return false;

   memset(binding, 0, sizeof(*binding));
   binding->heap = heap;
   binding->secure_heap = secure_heap;
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

WlBufferMemory *GetWlBufferMemory(struct wl_resource *buffer)
{
   if (!buffer || !wl_resource_get_user_data(buffer))
      return NULL;

   if (!wl_resource_instance_of(buffer, &wl_buffer_interface, &bufferInterface))
      return NULL; /* not one of ours */

   return (WlBufferMemory *)wl_resource_get_user_data(buffer);
}

void WlBufferMemoryRefInc(WlBufferMemory *memory)
{
   int before = __sync_fetch_and_add(&memory->refs, 1);
   assert(before > 0);
}

void WlBufferMemoryRefDec(WlBufferMemory *memory)
{
   int before = __sync_fetch_and_sub(&memory->refs, 1);
   assert(before > 0);
   if (before == 1)
      DestroyWlBufferMemory(memory);
}
