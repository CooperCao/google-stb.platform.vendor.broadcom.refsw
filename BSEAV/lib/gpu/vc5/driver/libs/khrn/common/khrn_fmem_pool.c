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
   if (vcos_mutex_create(&client_pool.lock, "client_pool.lock") != VCOS_SUCCESS)
      return false;

   for (unsigned i = 0; i < KHRN_FMEM_MAX_BLOCKS; i++)
   {
      KHRN_FMEM_BUFFER* buffer = &client_pool.buffer[i];
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
   for (unsigned i = 0; i < KHRN_FMEM_MAX_BLOCKS; i++)
   {
      KHRN_FMEM_BUFFER* buffer = &client_pool.buffer[i];
      assert(!buffer->in_use);
      if (buffer->handle != GMEM_HANDLE_INVALID)
      {
         assert(buffer->cpu_address != NULL);
         gmem_free(buffer->handle);
      }
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
   vcos_mutex_lock(&client_pool.lock);
   unsigned count = client_pool.n_submitted_buffers + client_pool.n_free_buffers;
   vcos_mutex_unlock(&client_pool.lock);
   return count;
}

void khrn_fmem_pool_init(KHRN_FMEM_POOL_T *pool, KHRN_RENDER_STATE_T *render_state)
{
   pool->n_buffers = 0;
   pool->buffers_submitted = false;
}

void khrn_fmem_pool_deinit(KHRN_FMEM_POOL_T *pool)
{
   for (unsigned i = 0; i < pool->n_buffers; i++)
   {
      client_pool_release_buffer(pool->buffer[i], pool->buffers_submitted);
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
            GMEM_USAGE_V3D_READ, "Fmem buffer");
      if (buffer->handle == GMEM_HANDLE_INVALID)
      {
         log_error("[%s] gmem_alloc failed", VCOS_FUNCTION);
         goto fail;
      }

      assert(buffer->cpu_address == NULL);
      buffer->cpu_address = gmem_map_and_get_ptr(buffer->handle);
      if (!buffer->cpu_address)
      {
         gmem_free(buffer->handle);
         buffer->handle = GMEM_HANDLE_INVALID;
         goto fail;
      }
   }

   buffer->hw_address = gmem_get_addr(buffer->handle);

   pool->buffer[pool->n_buffers++] = buffer;
   buffer->bytes_used = ~0u;
   return buffer->cpu_address;

fail:
   client_pool_release_buffer(buffer, false);
   return NULL;
}

void khrn_fmem_pool_submit(
   KHRN_FMEM_POOL_T *pool,
#if KHRN_DEBUG
   khrn_memaccess* memaccess,
#endif
   v3d_barrier_flags* bin_rw_flags,
   v3d_barrier_flags* render_rw_flags)
{
   // Be conservative with read flags since all sorts of data can live in fmem.
   v3d_barrier_flags common_rw_flags =
         V3D_BARRIER_CLE_CL_READ
      |  V3D_BARRIER_CLE_SHADREC_READ
      |  V3D_BARRIER_VCD_READ
      |  V3D_BARRIER_QPU_INSTR_READ
      |  V3D_BARRIER_QPU_UNIF_READ
      |  V3D_BARRIER_TMU_CONFIG_READ
      |  V3D_BARRIER_TMU_DATA_READ;

   *bin_rw_flags =
         common_rw_flags
      |  V3D_BARRIER_CLE_PRIMIND_READ
      |  V3D_BARRIER_CLE_DRAWREC_READ;

   *render_rw_flags =
         common_rw_flags
       | V3D_BARRIER_TLB_IMAGE_READ;

   for (unsigned i=0; i < pool->n_buffers; i++)
   {
      KHRN_FMEM_BUFFER *buffer = pool->buffer[i];
      assert(buffer->bytes_used != ~0u);
      gmem_flush_mapped_range(buffer->handle, 0, buffer->bytes_used);

   #if KHRN_DEBUG
      if (memaccess)
         khrn_memaccess_add_buffer(memaccess, buffer->handle, *bin_rw_flags, *render_rw_flags);
   #endif
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

void khrn_fmem_pool_finalise_end(KHRN_FMEM_POOL_T *pool, void *address)
{
   KHRN_FMEM_BUFFER *buffer = get_buffer_containing(pool, address);
   assert(buffer->bytes_used == ~0u);
   buffer->bytes_used = (char*)address - (char*)buffer->cpu_address;
}
