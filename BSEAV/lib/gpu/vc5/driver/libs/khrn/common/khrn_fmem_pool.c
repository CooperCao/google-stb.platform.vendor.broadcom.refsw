/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
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
   unsigned num_free_buffers;       /* number of fmem free buffers */
   unsigned num_flushed_buffers;    /* number of fmem buffers that will become free after waiting */
   khrn_fmem_buffer* free_buffers;
   khrn_fmem_buffer buffers[KHRN_FMEM_MAX_BLOCKS];

} khrn_fmem_client_pool;

static khrn_fmem_client_pool client_pool;

bool khrn_fmem_client_pool_init(void)
{
   if (vcos_mutex_create(&client_pool.lock, "client_pool.lock") != VCOS_SUCCESS)
      return false;

   memset(client_pool.buffers, 0, sizeof(client_pool.buffers));
   for (unsigned i = 1; i != KHRN_FMEM_MAX_BLOCKS; ++i)
      client_pool.buffers[i-1].next_free = &client_pool.buffers[i];
   client_pool.free_buffers = client_pool.buffers;

   client_pool.num_free_buffers = KHRN_FMEM_MAX_BLOCKS;
   client_pool.num_flushed_buffers = 0;

   return true;
}

void khrn_fmem_client_pool_deinit(void)
{
   assert(client_pool.num_free_buffers == KHRN_FMEM_MAX_BLOCKS);

   for (unsigned i = 0; i < KHRN_FMEM_MAX_BLOCKS; i++)
   {
      khrn_fmem_buffer* buf = &client_pool.buffers[i];
      assert(buf->num_users == 0 && buf->bytes_used == 0 && buf->state == KHRN_FMEM_BUFFER_FREE);
      gmem_free(buf->handle);
   }

   vcos_mutex_delete(&client_pool.lock);
}

// must hold client_pool.lock.
static inline void client_pool_buffer_add_to_free_list(khrn_fmem_buffer* buf)
{
   assert(buf->state == KHRN_FMEM_BUFFER_ASSIGNED || buf->state == KHRN_FMEM_BUFFER_FLUSHED);
   buf->state = KHRN_FMEM_BUFFER_FREE;
   buf->next_free = client_pool.free_buffers;
   client_pool.free_buffers = buf;
   client_pool.num_free_buffers += 1;
   assert(client_pool.num_free_buffers <= KHRN_FMEM_MAX_BLOCKS);
}

void khrn_fmem_pool_init(khrn_fmem_pool *pool, khrn_render_state *render_state)
{
   pool->num_blocks = 0;
   pool->render_state = render_state;
   pool->submitted = false;
   debug_only(pool->begin_allocs = 0);
}

void khrn_fmem_pool_term(khrn_fmem_pool *pool)
{
   assert(pool->begin_allocs == 0);

   vcos_mutex_lock(&client_pool.lock);

   for (unsigned i = 0; i != pool->num_blocks; ++i)
   {
      khrn_fmem_buffer* buf = &client_pool.buffers[pool->blocks[i].buf_index];

      assert(buf->num_users > 0);
      buf->num_users -= 1;

      // Add back to the free list if pool is unsubmitted (buf is assigned to this pool).
      assert(pool->submitted || buf->state == KHRN_FMEM_BUFFER_ASSIGNED);
      bool add_to_free_list = !pool->submitted;

      // Reset buffer if no outstanding users.
      if (!buf->num_users)
      {
         if (buf->state == KHRN_FMEM_BUFFER_FLUSHED)
         {
            assert(client_pool.num_flushed_buffers > 0);
            client_pool.num_flushed_buffers -= 1;
            add_to_free_list = true;
         }
         buf->bytes_used = 0;
      }

      if (add_to_free_list)
         client_pool_buffer_add_to_free_list(buf);
   }

   vcos_mutex_unlock(&client_pool.lock);
}

static khrn_fmem_buffer* wait_for_free_buffer_locked(void)
{
   for (bool try_wait = true; ;)
   {
      khrn_fmem_buffer* buf = client_pool.free_buffers;
      if (buf)
      {
         if (buf->handle)
            return buf;

         buf->handle = gmem_alloc_and_map(KHRN_FMEM_BUFFER_SIZE, KHRN_FMEM_ALIGN_MAX, GMEM_USAGE_V3D_READ, "Fmem buffer");
         if (buf->handle)
            return buf;
      }

      // If no buffers we can wait for.
      if (!client_pool.num_flushed_buffers)
      {
         // Give up if no free buffers now.
         if (!buf)
         {
            log_warn("All fmem buffer slots are in use.");
            return NULL;
         }

         // Give up if there's nothing to wait for.
         if (!try_wait)
         {
            log_warn("All allocated fmem buffers are in use.");
            return NULL;
         }
      }

      // Wait for either one of our buffers or gmem to become free.
      vcos_mutex_unlock(&client_pool.lock);
      try_wait = v3d_scheduler_wait_any();
      vcos_mutex_lock(&client_pool.lock);
   }
}

khrn_fmem_buffer* khrn_fmem_pool_begin_alloc(khrn_fmem_pool *pool, uint32_t min_size, uint32_t align)
{
   assert(!pool->submitted);

   vcos_mutex_lock(&client_pool.lock);

    /* If we are getting low on fmems, flush others render states except
     * ourselves. Flushing a render state increases the number of submitted
     * buffers. Don't do this if we're currently flushing unless we really
     * ran out of buffers. */
   assert(pool->render_state->flush_state != KHRN_RENDER_STATE_FLUSH_ALLOWED);
   unsigned threshold = pool->render_state->flush_state == KHRN_RENDER_STATE_FLUSHING ? 1u : KHRN_FMEM_THRESHOLD_FLUSH_OTHER_RS;
   while (client_pool.num_free_buffers + client_pool.num_flushed_buffers < threshold)
   {
      vcos_mutex_unlock(&client_pool.lock);
      bool flushed = khrn_render_state_flush_oldest_possible();
      vcos_mutex_lock(&client_pool.lock);
      if (!flushed)
         break;
   }

   khrn_fmem_buffer* buf;
   for (; ; )
   {
      buf = wait_for_free_buffer_locked();
      if (!buf)
         break;

      // Remove from free-list.
      assert(client_pool.num_free_buffers > 0);
      client_pool.num_free_buffers -= 1;
      client_pool.free_buffers = buf->next_free;
      buf->next_free = NULL;

      // Check buffer is large enough.
      assert(min_size <= KHRN_FMEM_BUFFER_SIZE);
      assert(align <= KHRN_FMEM_ALIGN_MAX && gfx_is_power_of_2(align));
      uint32_t offset = (buf->bytes_used + (align - 1)) & ~(align - 1);
      if ((offset + min_size) <= KHRN_FMEM_BUFFER_SIZE)
      {
         buf->state = KHRN_FMEM_BUFFER_ASSIGNED;
         buf->num_users += 1;
         debug_only(pool->begin_allocs += 1);
         break;
      }

      // Buffer wasn't large enough, so simply transition to flushed state.
      assert(buf->num_users > 0 && buf->bytes_used > 0);
      client_pool.num_flushed_buffers += 1;
      buf->state = KHRN_FMEM_BUFFER_FLUSHED;
   }

   vcos_mutex_unlock(&client_pool.lock);
   return buf;
}

void khrn_fmem_pool_end_alloc(khrn_fmem_pool *pool, khrn_fmem_buffer* buf, uint32_t bytes_used)
{
   assert(buf->state == KHRN_FMEM_BUFFER_ASSIGNED);
   assert(bytes_used >= buf->bytes_used && bytes_used <= KHRN_FMEM_BUFFER_SIZE);
   assert(pool->begin_allocs > 0);
   debug_only(pool->begin_allocs -= 1);

   // Need to store approximate range of buffer used for cpu flushing.
   // The buffer bytes-used is updated when the pool is submitted.
   assert(pool->num_blocks < countof(pool->blocks));
   khrn_fmem_buffer_block* block = &pool->blocks[pool->num_blocks++];
   block->buf_index = gfx_bits(buf - client_pool.buffers, 8);
   block->start64 = gfx_bits(buf->bytes_used / 64, 12);
   block->end64 = gfx_bits((bytes_used + 63) / 64, 12);
}

void khrn_fmem_pool_cpu_flush(khrn_fmem_pool* pool)
{
   assert(pool->begin_allocs == 0);

   for (unsigned i = 0; i != pool->num_blocks; ++i)
   {
      // OK to access handle without the lock as this is immutable.
      khrn_fmem_buffer_block* block = &pool->blocks[i];
      gmem_handle_t handle = client_pool.buffers[block->buf_index].handle;
      gmem_flush_mapped_range(handle, (uint32_t)block->start64 * 64, (uint32_t)(block->end64 - block->start64) * 64);
   }
}

void khrn_fmem_pool_on_submit(khrn_fmem_pool *pool)
{
   assert(pool->begin_allocs == 0);
   assert(!pool->submitted);
   pool->submitted = true;

   vcos_mutex_lock(&client_pool.lock);

   for (unsigned i = 0; i != pool->num_blocks; ++i)
   {
      khrn_fmem_buffer_block *block = &pool->blocks[i];
      khrn_fmem_buffer *buf = &client_pool.buffers[block->buf_index];

      // Update the number of bytes used from this buffer.
      assert(buf->state == KHRN_FMEM_BUFFER_ASSIGNED);
      buf->bytes_used = (uint32_t)block->end64 * 64;

      // If there's enough space remaining, then add it back to the free list.
      if (buf->bytes_used <= (KHRN_FMEM_BUFFER_SIZE*15)/16)
         client_pool_buffer_add_to_free_list(buf);
      else
      {
         client_pool.num_flushed_buffers += 1;
         buf->state = KHRN_FMEM_BUFFER_FLUSHED;
      }
   }

   vcos_mutex_unlock(&client_pool.lock);
}

#if KHRN_DEBUG
void khrn_fmem_pool_add_to_memaccess(
   khrn_fmem_pool *pool,
   khrn_memaccess* memaccess,
   v3d_barrier_flags bin_rw_flags,
   v3d_barrier_flags render_rw_flags)
{
   assert(pool->begin_allocs == 0);

   for (unsigned i = 0; i != pool->num_blocks; ++i)
   {
      khrn_fmem_buffer_block* block = &pool->blocks[i];
      gmem_handle_t handle = client_pool.buffers[block->buf_index].handle;
      khrn_memaccess_add_buffer(memaccess, handle, bin_rw_flags, render_rw_flags);
   }
}
#endif
