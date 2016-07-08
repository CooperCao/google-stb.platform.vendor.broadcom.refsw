/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Fmem pool allocator

FILE DESCRIPTION
Implementation of fmem pool allocator.
=============================================================================*/
#include "khrn_fmem_pool.h"
#include "khrn_int_common.h"
#include "khrn_render_state.h"
#include "khrn_process.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/platform/v3d_scheduler.h"
#include "libs/platform/gmem.h"
#include "vcos.h"

LOG_DEFAULT_CAT("khrn_fmem_pool")

/* Each client process has one of these; the buffers are shared between all
 * contexts;*/
typedef struct khrn_fmem_client_pool
{
   VCOS_MUTEX_T lock;
   KHRN_FMEM_BUFFER buffer[KHRN_FMEM_MAX_BLOCKS];
   unsigned n_free_buffers;   /* number of fmem free buffers */
   unsigned n_submitted_buffers; /* number of fmem buffers that were submitted;
                                  waiting on all the completion callbacks in this client,
                                  will free these buffers back to the pool */

} khrn_fmem_client_pool;

static khrn_fmem_client_pool client_pool;

bool khrn_fmem_client_pool_init(void)
{
   unsigned int i;
   KHRN_FMEM_BUFFER *buffer;

   if (vcos_mutex_create(&client_pool.lock, "client_pool.lock") !=
         VCOS_SUCCESS)
      return false;

   for (i = 0; i < KHRN_FMEM_MAX_BLOCKS; i++)
   {
      buffer = &client_pool.buffer[i];
      buffer->in_use = false;
      buffer->bytes_used = ~0u;
      buffer->cpu_address = NULL;
      buffer->handle = GMEM_HANDLE_INVALID;
   }

   client_pool.n_free_buffers = KHRN_FMEM_MAX_BLOCKS;
   client_pool.n_submitted_buffers = 0;

   return true;
}

void khrn_fmem_client_pool_deinit(void)
{
   unsigned int i;
   KHRN_FMEM_BUFFER *buffer;

   for (i = 0; i < KHRN_FMEM_MAX_BLOCKS; i++)
   {
      buffer = &client_pool.buffer[i];
      assert(!buffer->in_use);
      if (buffer->handle != GMEM_HANDLE_INVALID)
         gmem_free(buffer->handle);
   }
   vcos_mutex_delete(&client_pool.lock);
}

/* get a free buffer from the client_pool if possible, return NULL otherwise */
static KHRN_FMEM_BUFFER* client_pool_get_buffer(void)
{
   KHRN_FMEM_BUFFER *buffer = NULL;

   vcos_mutex_lock(&client_pool.lock);

   if (client_pool.n_free_buffers == 0)
      goto end;

   for (unsigned i = 0; i < KHRN_FMEM_MAX_BLOCKS; i++)
   {
      if (!client_pool.buffer[i].in_use)
      {
         client_pool.buffer[i].in_use = true;
         buffer = &client_pool.buffer[i];
         client_pool.n_free_buffers--;
         break;
      }
   }

end:
   vcos_mutex_unlock(&client_pool.lock);
   return buffer;
}

/*
   Release a buffer  back to the client fmem pool,
   was_submitted is true if this buffer became free after it was flushed, false
   if we've not submitted this buffer;
*/
static void client_pool_release_buffer(KHRN_FMEM_BUFFER *buffer, bool was_submitted)
{
   vcos_mutex_lock(&client_pool.lock);
   /* we should have unmapped the handle by now and set the cpu_addr to NULL */
   assert(buffer->cpu_address == NULL);
   buffer->in_use = false;
   client_pool.n_free_buffers++;
   if (was_submitted)
      client_pool.n_submitted_buffers--;
   vcos_mutex_unlock(&client_pool.lock);
}

static void client_pool_add_submitted_buffers(unsigned count)
{
   vcos_mutex_lock(&client_pool.lock);
   assert(client_pool.n_submitted_buffers + client_pool.n_free_buffers +
         count <= KHRN_FMEM_MAX_BLOCKS);
   client_pool.n_submitted_buffers += count;
   vcos_mutex_unlock(&client_pool.lock);
}

unsigned khrn_fmem_client_pool_get_num_free_and_submitted()
{
   unsigned count;
   vcos_mutex_lock(&client_pool.lock);
   count = client_pool.n_submitted_buffers + client_pool.n_free_buffers;
   vcos_mutex_unlock(&client_pool.lock);
   return count;
}

void khrn_fmem_pool_init(KHRN_FMEM_POOL_T *pool, KHRN_RENDER_STATE_T *render_state)
{
   gmem_lock_list_init(&pool->lock_list);
   pool->n_buffers = 0;
   pool->buffers_submitted = false;
}

void khrn_fmem_pool_deinit(KHRN_FMEM_POOL_T *pool)
{
   gmem_lock_list_unlock_and_destroy(&pool->lock_list);

   for (unsigned i = 0; i < pool->n_buffers; i++)
   {
      KHRN_FMEM_BUFFER *buffer = pool->buffer[i];
      if (buffer->cpu_address)
      {
         gmem_unmap(buffer->handle);
         buffer->cpu_address = NULL;
      }
      client_pool_release_buffer(buffer, pool->buffers_submitted);
   }
}

void* khrn_fmem_pool_alloc(KHRN_FMEM_POOL_T *pool)
{
   assert(!pool->buffers_submitted);

   KHRN_FMEM_BUFFER *buffer = client_pool_get_buffer();
   if (buffer == NULL)
   {
      /* no free buffers, see if we have some submitted ones that we can wait
       * for to become free */
      if (khrn_fmem_client_pool_get_num_free_and_submitted() != 0)
      {
         do
         {
            bool all_finalised = !v3d_scheduler_wait_any();
            buffer = client_pool_get_buffer();
            assert(buffer || !all_finalised);
         }while(buffer == NULL);
      }
      else
      {
         log_warn("%s: all fmems are in use",
               VCOS_FUNCTION);
         return NULL;
      }
   }

   if (buffer->handle == GMEM_HANDLE_INVALID)
   {
      buffer->handle = gmem_alloc(KHRN_FMEM_BUFFER_SIZE, KHRN_FMEM_ALIGN_MAX,
            GMEM_USAGE_ALL, "Fmem buffer");
      if (buffer->handle == GMEM_HANDLE_INVALID)
      {
         log_error("[%s] gmem_alloc failed", VCOS_FUNCTION);
         goto fail;
      }
  }

   assert(buffer->cpu_address == NULL);
   buffer->cpu_address = gmem_map(buffer->handle);
   if (!buffer->cpu_address)
      goto fail;

   buffer->hw_address = gmem_lock(&pool->lock_list, buffer->handle);
   if (buffer->hw_address == 0)
      goto fail;

   gmem_sync_pre_cpu_access(buffer->handle, GMEM_SYNC_CPU_WRITE | GMEM_SYNC_DISCARD);
   pool->buffer[pool->n_buffers++] = buffer;
   buffer->is_render_output = false;
   buffer->bytes_used = ~0u;
   return buffer->cpu_address;

fail:
   if (buffer->cpu_address)
   {
      gmem_unmap(buffer->handle);
      buffer->cpu_address = NULL;
   }
   client_pool_release_buffer(buffer, false);
   return NULL;
}

void khrn_fmem_pool_submit(KHRN_FMEM_POOL_T *pool,
   struct khrn_synclist *bin, struct khrn_synclist *render)
{
   for (unsigned i=0; i < pool->n_buffers; i++)
   {
      KHRN_FMEM_BUFFER *buffer = pool->buffer[i];

      gmem_sync_post_cpu_write_range(buffer->handle, 0, buffer->bytes_used, GMEM_SYNC_CPU_WRITE | GMEM_SYNC_RELAXED);

      if (!buffer->is_render_output)
      {
         gmem_unmap(buffer->handle);
         buffer->cpu_address = NULL;
      } /* else: keep output buffers mapped so we can read them after the frame completes */

      // Be conservative with read flags since all sorts of data can live in fmem.
      uint32_t bin_sync_flags = (GMEM_SYNC_V3D_READ & ~GMEM_SYNC_TFU_READ)
                              | GMEM_SYNC_RELAXED;
      uint32_t render_sync_flags = bin_sync_flags
                                 | (buffer->is_render_output ? GMEM_SYNC_CORE_WRITE : 0u);
      assert(buffer->bytes_used != ~0u);
      khrn_synclist_add_range(bin, buffer->handle, 0, buffer->bytes_used, bin_sync_flags);
      khrn_synclist_add_range(render, buffer->handle, 0, buffer->bytes_used, render_sync_flags);
   }

   assert(pool->buffers_submitted == false);
   pool->buffers_submitted = true;
   client_pool_add_submitted_buffers(pool->n_buffers);
}

static bool buffer_contains(const KHRN_FMEM_BUFFER *buffer, void *address)
{
   uintptr_t offset = (uintptr_t)address - (uintptr_t)buffer->cpu_address;
   return offset < KHRN_FMEM_BUFFER_SIZE;
}

static KHRN_FMEM_BUFFER *get_buffer_containing(const KHRN_FMEM_POOL_T *pool, void *address)
{
   static unsigned lastMatchedBufIndx = 0;

   if (lastMatchedBufIndx >= pool->n_buffers)
      lastMatchedBufIndx = 0;

   /* high probability that the match is the previous, so start here and scan to end */
   for (unsigned i = lastMatchedBufIndx; i != pool->n_buffers; i++)
   {
      if (buffer_contains(pool->buffer[i], address))
      {
         lastMatchedBufIndx = i;
         return pool->buffer[i];
      }
   }

   /* scan from start to lastMatchedBufIndx */
   for (unsigned i = 0; i != lastMatchedBufIndx; i++)
   {
      if (buffer_contains(pool->buffer[i], address))
      {
         lastMatchedBufIndx = i;
         return pool->buffer[i];
      }
   }
   unreachable();
   return NULL;
}

v3d_addr_t khrn_fmem_pool_hw_address(KHRN_FMEM_POOL_T *pool, void *address)
{
   KHRN_FMEM_BUFFER *buffer = get_buffer_containing(pool, address);
   return buffer->hw_address + ((char *)address - (char *)buffer->cpu_address);
}

void khrn_fmem_pool_mark_as_render_output(KHRN_FMEM_POOL_T *pool, void *address)
{
   KHRN_FMEM_BUFFER *buffer = get_buffer_containing(pool, address);
   buffer->is_render_output = true;
}

void khrn_fmem_pool_finalise_end(KHRN_FMEM_POOL_T *pool, void *address)
{
   KHRN_FMEM_BUFFER *buffer = get_buffer_containing(pool, address);
   assert(buffer->bytes_used == ~0u);
   buffer->bytes_used = (char*)address - (char*)buffer->cpu_address;
}

void khrn_fmem_pool_pre_cpu_read_outputs(const KHRN_FMEM_POOL_T *pool)
{
   for (unsigned i = 0; i != pool->n_buffers; ++i)
   {
      const KHRN_FMEM_BUFFER *buffer = pool->buffer[i];
      if (buffer->is_render_output)
         gmem_sync_pre_cpu_access(buffer->handle, GMEM_SYNC_CPU_READ);
   }
}
