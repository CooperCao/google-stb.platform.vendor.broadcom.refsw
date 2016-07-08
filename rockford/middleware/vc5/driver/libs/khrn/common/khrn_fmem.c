/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Control per-frame memory allocator

FILE DESCRIPTION
Handles allocation of memory for control lists and associated data that will be
generated each frame as HW input.
=============================================================================*/

#include "khrn_fmem.h"
#include "khrn_control_list.h"
#include "khrn_mr_crc.h"
#include "khrn_render_state.h"
#include "khrn_record.h"
#include "khrn_synclist_validate.h"
#include "khrn_process.h"
#include "khrn_process.h"
#include "../glxx/glxx_query.h"
#include "khrn_mem.h"
#include "khrn_fence.h"
#include "../egl/egl_context_gl.h"
#include "khrn_options.h"
#include "libs/platform/gmem.h"

#include "vcos.h"
#include <stddef.h> /* for offsetof */
#include <assert.h>

#ifdef KHRN_VALIDATE_SYNCLIST
#include "libs/util/demand.h"
#endif
#include "libs/core/v3d/v3d_cl.h"

#include "libs/platform/v3d_driver_api.h"
#include "libs/platform/bcm_sched_api.h"

LOG_DEFAULT_CAT("khrn_fmem")

static void block_init(KHRN_FMEM_BLOCK_T *block);
static bool alloc_next(KHRN_FMEM_POOL_T *pool, KHRN_FMEM_BLOCK_T *block,
      KHRN_RENDER_STATE_T *rs);

uint32_t khrn_fmem_frame_i = 0;

static void free_client_handles(KHRN_UINTPTR_VECTOR_T *handles)
{
   unsigned int i;
   for (i = 0; i < khrn_uintptr_vector_get_size(handles); i++)
   {
      gmem_handle_t handle = (gmem_handle_t) khrn_uintptr_vector_item(
            handles, i);
      assert(handle != GMEM_HANDLE_INVALID);
      gmem_free(handle);
   }
   khrn_uintptr_vector_destroy(handles);
}

static void release_queries(KHRN_QUERY_BLOCK_T *query_list)
{
   KHRN_QUERY_BLOCK_T *b;
   unsigned i;

   b = query_list;
   while (b)
   {
      for (i = 0; i < b->count; ++i)
         KHRN_MEM_ASSIGN(b->query[i].obj, NULL);
      b = b->prev;
   }
}

static void stage_persist_init(khrn_fmem_stage_persist *stage_per,
      khrn_fmem_common_persist *common)
{
   memset(stage_per, 0, sizeof *stage_per);

   gmem_lock_list_init(&stage_per->lock_list);
   khrn_synclist_init(&stage_per->sync_list);
   KHRN_MEM_ASSIGN(stage_per->common, common);
}

static void stage_persist_term(void *p, size_t size)
{
   khrn_fmem_stage_persist *stage_per = (khrn_fmem_stage_persist *)p;
   UNUSED(size);

   gmem_lock_list_unlock_and_destroy(&stage_per->lock_list);
   khrn_synclist_destroy(&stage_per->sync_list);

   KHRN_MEM_ASSIGN(stage_per->common, NULL);
}

static khrn_fmem_stage_persist* stage_persist_create(khrn_fmem_common_persist *common)
{
   khrn_fmem_stage_persist *per = KHRN_MEM_ALLOC_STRUCT(khrn_fmem_stage_persist);
   if (per == NULL)
      return NULL;

   stage_persist_init(per, common);
   khrn_mem_set_term(per, stage_persist_term);
   return per;
}

static bool common_persist_init(khrn_fmem_common_persist *per,
      KHRN_RENDER_STATE_T *rs)
{
   memset(per, 0, sizeof *per);

   khrn_fmem_pool_init(&per->pool, rs);

   if (!khrn_uintptr_vector_init(&per->client_handles, 0, 16))
   {
      khrn_fmem_pool_deinit(&per->pool);
      return false;
   }

#ifdef KHRN_GEOMD
   fmem_debug_info_init(&per->debug_info);
#endif
   return true;
}

static void common_persist_term(void *v, size_t size)
{
   khrn_fmem_common_persist *per = (khrn_fmem_common_persist *)v;
   UNUSED(size);

   release_queries(per->query_list);
   khrn_fmem_pool_deinit(&per->pool);

   for (unsigned i = 0; i != per->num_bin_tile_states; ++i)
      gmem_free(per->bin_tile_state[i]);

   free_client_handles(&per->client_handles);

#ifdef KHRN_GEOMD
   fmem_debug_info_deinit(&per->debug_info);
#endif
}

static khrn_fmem_common_persist* common_persist_create(KHRN_RENDER_STATE_T *rs)
{
   khrn_fmem_common_persist *per = KHRN_MEM_ALLOC_STRUCT(khrn_fmem_common_persist);
   if (per == NULL)
      return NULL;

   if (!common_persist_init(per, rs))
   {
      KHRN_MEM_ASSIGN(per, NULL);
      return NULL;
   }
   khrn_mem_set_term(per, common_persist_term);
   return per;
}

static void render_completion(void *data, uint64_t job_id,
      v3d_sched_job_error error)
{
   khrn_fmem_stage_persist *stage_per = (khrn_fmem_stage_persist*)data;

   /* go through the list of queries and update the queries results */
   khrn_fmem_pool_pre_cpu_read_outputs(&stage_per->common->pool);
   glxx_queries_update(stage_per->common->query_list, true);

   KHRN_MEM_ASSIGN(stage_per, NULL);
}

static void bin_completion(void *data, uint64_t job_id,
      v3d_sched_job_error error)
{
   khrn_fmem_stage_persist *stage_per = (khrn_fmem_stage_persist*)data;
   KHRN_MEM_ASSIGN(stage_per, NULL);
}

bool khrn_fmem_init(KHRN_FMEM_T *fmem, KHRN_RENDER_STATE_T *render_state)
{
   memset(fmem, 0, sizeof *fmem);
   khrn_fmem_common_persist *common_per = NULL;

   /* the cle and data blocks come from the same allocator */
   block_init(&fmem->cle);
   block_init(&fmem->data);

   // the initialised state is as if khrn_fmem_begin_clist has been called
#ifndef NDEBUG
   fmem->in_begin_clist = true;
#endif

   fmem->render_state = render_state;

   // ok to destroy these if just zero initialised
   if (!khrn_uintptr_vector_init(&fmem->res_interlocks, 64, 64))
      goto fail;
   if (!khrn_uintptr_vector_init(&fmem->fences_to_signal, 0, 16))
      goto fail;
   if (!khrn_uintptr_vector_init(&fmem->fences_to_depend_on, 0, 16))
      goto fail;

   common_per = common_persist_create(fmem->render_state);
   if (common_per == NULL)
      goto fail;

   for (unsigned stage =0; stage < COUNT_STAGES; stage++)
   {
      fmem->persist[stage] = stage_persist_create(common_per);
      if (fmem->persist[stage] == NULL)
         goto fail;
   }
   KHRN_MEM_ASSIGN(common_per, NULL);
   return true;

fail:
   KHRN_MEM_ASSIGN(common_per, NULL);
   for (unsigned stage = 0; stage < COUNT_STAGES; stage++)
      KHRN_MEM_ASSIGN(fmem->persist[stage], NULL);

   khrn_uintptr_vector_destroy(&fmem->res_interlocks);
   khrn_uintptr_vector_destroy(&fmem->fences_to_signal);
   khrn_uintptr_vector_destroy(&fmem->fences_to_depend_on);
   return false;
}

static void release_interlocks(KHRN_FMEM_T *fmem)
{
   KHRN_RES_INTERLOCK_T *res_i;
   KHRN_UINTPTR_VECTOR_T *res_interlocks = &fmem->res_interlocks;
   size_t i;

   for (i = 0; i < khrn_uintptr_vector_get_size(res_interlocks); i++)
   {
      res_i = (KHRN_RES_INTERLOCK_T*)khrn_uintptr_vector_item(res_interlocks, i);
      khrn_res_interlock_refdec(res_i);
   }
   khrn_uintptr_vector_destroy(res_interlocks);
}

static void release_fences(KHRN_UINTPTR_VECTOR_T *vec_fences)
{
   KHRN_FENCE_T *kfence;
   size_t i;

   for (i = 0; i < khrn_uintptr_vector_get_size(vec_fences); i++)
   {
      kfence = (KHRN_FENCE_T*)khrn_uintptr_vector_item(vec_fences, i);
      khrn_fence_refdec(kfence);
   }
   khrn_uintptr_vector_destroy(vec_fences);
}

static void unrecord_interlocks(KHRN_FMEM_T *fmem)
{
   KHRN_RES_INTERLOCK_T *res_i;
   KHRN_UINTPTR_VECTOR_T *res_interlocks = &fmem->res_interlocks;
   size_t i;

   for (i = 0; i < khrn_uintptr_vector_get_size(res_interlocks); i++)
   {
      res_i = (KHRN_RES_INTERLOCK_T*)
         khrn_uintptr_vector_item(res_interlocks, i);
      khrn_interlock_remove_user(&res_i->interlock, fmem->render_state);
   }
}

static void unrecord_fences_to_signal(KHRN_FMEM_T *fmem)
{
   KHRN_FENCE_T *fence;
   KHRN_UINTPTR_VECTOR_T *fences = &fmem->fences_to_signal;
   size_t i;

   for (i = 0; i < khrn_uintptr_vector_get_size(fences); i++)
   {
      fence = (KHRN_FENCE_T*) khrn_uintptr_vector_item(fences, i);
      khrn_fence_remove_user(fence, fmem->render_state);
   }
}

bool khrn_fmem_record_handle(KHRN_FMEM_T *fmem, gmem_handle_t handle)
{
   khrn_fmem_common_persist *fmem_common = khrn_fmem_get_common_persist(fmem);
   return khrn_uintptr_vector_push_back(&fmem_common->client_handles, (uintptr_t)handle);
}

void khrn_fmem_discard(KHRN_FMEM_T *fmem)
{
   log_trace("\nkhrn_fmem_discard() fmem = %p", fmem);

   /* queries expect a number of completion callbacks; if we drop a frame,
    * record things as we would have got a completion callback, but without
    * updating results */
   khrn_fmem_common_persist *fmem_common = khrn_fmem_get_common_persist(fmem);
   glxx_queries_update(fmem_common->query_list, false);


   for (unsigned stage = 0; stage < COUNT_STAGES; stage++)
      KHRN_MEM_ASSIGN(fmem->persist[stage], NULL);

   unrecord_interlocks(fmem);
   release_interlocks(fmem);

   unrecord_fences_to_signal(fmem);
   release_fences(&fmem->fences_to_signal);
   release_fences(&fmem->fences_to_depend_on);
}

bool khrn_fmem_reset(KHRN_FMEM_T *fmem, KHRN_RENDER_STATE_T *render_state)
{
   khrn_fmem_discard(fmem);
   return (khrn_fmem_init(fmem, render_state));
}

static void *alloc(KHRN_FMEM_POOL_T *pool, KHRN_FMEM_BLOCK_T *block,
   unsigned int size, unsigned int align, KHRN_RENDER_STATE_T *rs)
{
   /* The caller is expected to put the render-state into a non-flushable state
      before attempting to allocate memory. */
   assert(rs->flush_state != KHRN_RENDER_STATE_FLUSH_ALLOWED);

   void *result;
   size_t start_offset;

   assert(align <= KHRN_FMEM_ALIGN_MAX);
   assert(size <= KHRN_FMEM_BUFFER_SIZE);

   /* alignment is relative to the start of the buffer */
   start_offset = (block->current + align - 1) & ~(align - 1);

   if (start_offset + size >= block->end)
   {
      if (!alloc_next(pool, block, rs))
         return NULL;

      start_offset = (block->current  + align - 1) & ~(align - 1);
      if (start_offset + size >= block->end)
      {
         assert(0);
         return NULL;
      }
   }

   result = block->start + start_offset;
   block->current = start_offset + size;

   return result;
}

uint8_t *khrn_fmem_cle(KHRN_FMEM_T *fmem, unsigned size)
{
   assert(fmem->in_begin_clist);
   assert(!fmem->in_begin_cle_start);
   KHRN_FMEM_BLOCK_T *cle  = &fmem->cle;
   size_t last_current = cle->current;
   uint8_t *last_block = cle->start;

   khrn_fmem_common_persist *fmem_common = khrn_fmem_get_common_persist(fmem);
   uint8_t *result = alloc(&fmem_common->pool, cle, size, 1, fmem->render_state);

   if (last_block != cle->start)
   {
      /* just allocated a new block */
      assert(result != NULL);

      if (last_block != NULL)
      {
         /* link old block cl to new */
         last_block += last_current;
         add_byte(&last_block, V3D_CL_BRANCH);
         put_word(last_block, khrn_fmem_pool_hw_address(&fmem_common->pool, result));
      }
      else
      {
         // note first block
         assert(fmem->cle_first == NULL);
         fmem->cle_first = cle->start;
      }
   }
   return result;
}


uint32_t *khrn_fmem_data(KHRN_FMEM_T *fmem, unsigned size, unsigned align)
{
   assert(!fmem->in_begin_data_start);

   khrn_fmem_common_persist *fmem_common = khrn_fmem_get_common_persist(fmem);
   void * p = alloc(&fmem_common->pool, &fmem->data, size, align, fmem->render_state);

   return p;
}

void khrn_fmem_new_query_entry(KHRN_FMEM_T *fmem,
      KHRN_QUERY_BLOCK_T **p_block, unsigned *index)
{
   khrn_fmem_common_persist *fmem_common = khrn_fmem_get_common_persist(fmem);

   KHRN_QUERY_BLOCK_T *block = fmem_common->query_list;

   if (block == NULL || block->count == KHRN_CLE_QUERY_COUNT)
   {
      KHRN_QUERY_BLOCK_T *new_block = (KHRN_QUERY_BLOCK_T *)khrn_fmem_data(fmem,
            sizeof(KHRN_QUERY_BLOCK_T), KHRN_FMEM_ALIGN_MAX);
      if (new_block != NULL)
      {
         khrn_fmem_pool_mark_as_render_output(&fmem_common->pool, new_block);

         /* Clear values here to indicate unused counters. We will initialise the
          * values correctly if/when we actually start using them. */
         /* Clear the counters for cores 1+ to 0, so we get the same sums for unused
          * counters no matter how many cores we have. This is a useful for
          * verification. */
         memset(new_block->query_values, 0, sizeof(new_block->query_values));
         memset(new_block->query_values, 0x55, V3D_QUERY_COUNTER_SINGLE_CORE_CACHE_LINE_SIZE);
         new_block->count = 0;
         new_block->prev = block;
         fmem_common->query_list = new_block;
      }
      block = new_block;
   }

   *p_block = block;
   if (block)
   {
      *index = block->count;
      block->count++;
   }

   return;
}

uint8_t *khrn_fmem_begin_clist(KHRN_FMEM_T *fmem)
{
   // ensure not in middle of another clist
   assert(!fmem->in_begin_clist);
#ifndef NDEBUG
   fmem->in_begin_clist = true;
#endif
   return khrn_fmem_cle(fmem, 0);
}

uint8_t *khrn_fmem_end_clist(KHRN_FMEM_T *fmem)
{
   assert(fmem->in_begin_clist);
#ifndef NDEBUG
   fmem->in_begin_clist = false;
#endif
   return fmem->cle.start + fmem->cle.current;
}

#if V3D_HAS_NEW_TMU_CFG

static_assrt(V3D_TMU_CONFIG_CACHE_LINE_SIZE >= 32);

static void *tmu_cfg_alloc_32(KHRN_FMEM_T *fmem)
{
   struct khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;

   if (!a->num_32)
   {
      const unsigned size = 2048;
      assert((size % 32) == 0);
      a->spare_32 = khrn_fmem_data(fmem, size, V3D_TMU_CONFIG_CACHE_LINE_SIZE);
      if (!a->spare_32)
         return NULL;
      a->num_32 = size / 32;
   }

   void *p = a->spare_32;
   a->spare_32 = ((char *)a->spare_32) + 32;
   --a->num_32;
   return p;
}

static_assrt(V3D_TMU_TEX_STATE_PACKED_SIZE == 16);
static_assrt(V3D_TMU_TEX_STATE_ALIGN == 16);
static_assrt((V3D_TMU_TEX_STATE_PACKED_SIZE + V3D_TMU_TEX_EXTENSION_PACKED_SIZE) == 24);
static_assrt(V3D_TMU_EXTENDED_TEX_STATE_ALIGN == 32);

v3d_addr_t khrn_fmem_add_tmu_tex_state(KHRN_FMEM_T *fmem,
   const void *tex_state, bool extended)
{
   struct khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;

   void *p;
   if (extended)
   {
      p = tmu_cfg_alloc_32(fmem);
      if (!p)
         return 0;
      a->spare_8 = (char *)p + 24;
   }
   else if (a->spare_16)
   {
      p = a->spare_16;
      a->spare_16 = NULL;
   }
   else
   {
      p = tmu_cfg_alloc_32(fmem);
      if (!p)
         return 0;
      a->spare_16 = (char *)p + 16;
   }

   memcpy(p, tex_state,
      V3D_TMU_TEX_STATE_PACKED_SIZE + (extended ? V3D_TMU_TEX_EXTENSION_PACKED_SIZE : 0));
   return khrn_fmem_hw_address(fmem, p);
}

static_assrt(V3D_TMU_SAMPLER_PACKED_SIZE == 8);
static_assrt(V3D_TMU_SAMPLER_ALIGN == 8);
static_assrt(V3D_TMU_EXTENDED_SAMPLER_ALIGN == 32);

v3d_addr_t khrn_fmem_add_tmu_sampler(KHRN_FMEM_T *fmem,
   const void *sampler, bool extended)
{
   struct khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;

   void *p;
   if (extended)
   {
      p = tmu_cfg_alloc_32(fmem);
      if (!p)
         return 0;
      a->spare_8 = (char *)p + 24;
   }
   else if (a->spare_8)
   {
      p = a->spare_8;
      a->spare_8 = NULL;
   }
   else if (a->spare_16)
   {
      p = a->spare_16;
      a->spare_8 = (char *)p + 8;
      a->spare_16 = NULL;
   }
   else
   {
      p = tmu_cfg_alloc_32(fmem);
      if (!p)
         return 0;
      a->spare_8 = (char *)p + 8;
      a->spare_16 = (char *)p + 16;
   }

   memcpy(p, sampler, V3D_TMU_SAMPLER_PACKED_SIZE + (extended ? 16 : 0));
   return khrn_fmem_hw_address(fmem, p);
}

#else

v3d_addr_t khrn_fmem_add_tmu_indirect(KHRN_FMEM_T *fmem, uint32_t const *tmu_indirect)
{
   struct khrn_fmem_tmu_cfg_alloc *a = &fmem->tmu_cfg_alloc;

   // allocate tmu records in blocks of 64
   const unsigned c_cache_size = 64;
   if (!a->num_spare)
   {
      a->spare = (uint8_t*)khrn_fmem_data(fmem,
         V3D_TMU_INDIRECT_PACKED_SIZE*c_cache_size,
         V3D_TMU_CONFIG_CACHE_LINE_SIZE
         );
      if (!a->spare)
      {
         return 0;
      }
      a->num_spare = c_cache_size;
   }

   v3d_addr_t ret = khrn_fmem_hw_address(fmem, a->spare);
   memcpy(a->spare, tmu_indirect, V3D_TMU_INDIRECT_PACKED_SIZE);
   a->spare += V3D_TMU_INDIRECT_PACKED_SIZE;
   a->num_spare -= 1;
   return ret;
}

#endif

static void sync_interlock(KHRN_INTERLOCK_T *interlock,
      v3d_scheduler_deps *bin_sync,
      v3d_scheduler_deps *render_sync, KHRN_RENDER_STATE_T *rs)
{
   khrn_interlock_action_t actions;
   const v3d_scheduler_deps *sync;

   /* Look up the way in which we use this interlock */
   actions = khrn_interlock_get_actions(interlock, rs);

   sync = khrn_interlock_get_sync(interlock, interlock->is_writer);

   /* We use the resource during binning */
   if (actions & ACTION_BIN)
      v3d_scheduler_merge_deps(bin_sync, sync);

   /* We use the resource during rendering */
   if (actions & ACTION_RENDER)
      v3d_scheduler_merge_deps(render_sync, sync);

}

static void sync_fences_to_depend_on(v3d_scheduler_deps *sync,
      const KHRN_FMEM_T *fmem)
{
   const KHRN_UINTPTR_VECTOR_T *fences = &fmem->fences_to_depend_on;
   const unsigned num_fences = khrn_uintptr_vector_get_size(fences);
   for (unsigned i = 0; i < num_fences; i++)
   {
      KHRN_FENCE_T *fence = (KHRN_FENCE_T*) khrn_uintptr_vector_item(fences, i);
      assert(!khrn_fence_has_user(fence, NULL));
      v3d_scheduler_merge_deps(sync, &fence->deps);
   }
}

/*
 * fmem has a list of "resource interlocks" for resources it reads and buffers
 * it may write (including the one associated with its own render_state). Set
 * up bin_sync and render_sync to wait for all those resources to be ready.
 */
static void sync_interlocks(v3d_scheduler_deps *bin_sync,
      v3d_scheduler_deps *render_sync, const KHRN_FMEM_T *fmem)
{
   const KHRN_UINTPTR_VECTOR_T *res_interlocks = &fmem->res_interlocks;
   KHRN_RENDER_STATE_T *rs = fmem->render_state;
   KHRN_INTERLOCK_T *interlock;
   KHRN_RES_INTERLOCK_T *res_i;
   size_t i;

   for (i = 0; i < khrn_uintptr_vector_get_size(res_interlocks); i++)
   {
      res_i = (KHRN_RES_INTERLOCK_T *)
         khrn_uintptr_vector_item(res_interlocks, i);
      interlock = &res_i->interlock;

      if (res_i->handle == GMEM_HANDLE_INVALID)
      {
         /* no point to transfer dependencies since we don't have storage,
          * just unmark us as the user of that interlock */
         khrn_interlock_remove_user(interlock, rs);
         continue;
      }

      sync_interlock(interlock, bin_sync, render_sync, rs);
   }
}

static void update_interlock(KHRN_INTERLOCK_T *interlock,
      job_t bin_job, job_t render_job, KHRN_RENDER_STATE_T *rs)
{
   khrn_interlock_action_t actions;
   job_t job = 0;

   actions = khrn_interlock_get_actions(interlock, rs);
   assert(actions);

   /* render job depends on a bin job, so if the action performed on this
    * interlock happend in the render_stage, use the render job id */
   if (actions & ACTION_RENDER)
      job = render_job;
   else
   {
      assert(actions & ACTION_BIN);
      job = bin_job;
   }

   /*
    * The next writer has to wait for us regardless of whether we're
    * reading or writing. But the next reader only has to wait for us if we
    * are a writer.
    */
   if (interlock->is_writer)
   {
      /* if we've written to this interlock, then "job" was created to
       * depend on interlock->pre_write( which is a superset of
       * interlock->pre_read), so we can just replace all the dependencies
       * with this job */
      khrn_interlock_job_replace(interlock, job);
   }
   else
      khrn_interlock_job_add(interlock, job, false);

   khrn_interlock_remove_user(interlock, rs);
}

/*
 * Any resources that this fmem uses can now have their sync objects set up to
 * wait for the jobs we've just submitted to the scheduler.
 */
static void update_interlocks(const KHRN_FMEM_T *fmem,
      job_t bin_job, job_t render_job)
{
   const KHRN_UINTPTR_VECTOR_T *res_interlocks = &fmem->res_interlocks;
   KHRN_RENDER_STATE_T *rs = fmem->render_state;
   KHRN_RES_INTERLOCK_T *res_i;
   KHRN_INTERLOCK_T *interlock;
   khrn_interlock_action_t actions;
   size_t i;

   for (i = 0; i < khrn_uintptr_vector_get_size(res_interlocks); i++)
   {
      res_i = (KHRN_RES_INTERLOCK_T *)
         khrn_uintptr_vector_item(res_interlocks, i);
      interlock = &res_i->interlock;

      actions = khrn_interlock_get_actions(interlock, rs);
      if (actions == 0)
      {
         assert(res_i->handle == GMEM_HANDLE_INVALID);
         continue;
      }
      update_interlock(interlock, bin_job, render_job, rs);
   }
}

static void update_fences_to_signal(const KHRN_FMEM_T *fmem, uint64_t render_job)
{
   const KHRN_UINTPTR_VECTOR_T *fences = &fmem->fences_to_signal;
   KHRN_FENCE_T *fence;
   size_t i;

   for (i = 0; i < khrn_uintptr_vector_get_size(fences); i++)
   {
      fence = (KHRN_FENCE_T*) khrn_uintptr_vector_item(fences, i);
      khrn_fence_job_add(fence, render_job);
      khrn_fence_remove_user(fence, fmem->render_state);
   }
}

static inline void finalise_block(KHRN_FMEM_POOL_T *pool, KHRN_FMEM_BLOCK_T *block)
{
   if (block->start)
      khrn_fmem_pool_finalise_end(pool, block->start + block->current + GFX_MAX(V3D_MAX_CLE_READAHEAD, V3D_MAX_QPU_UNIFS_READAHEAD));
}

static void pre_submit_debug(
   const V3D_BIN_RENDER_INFO_T *br_info,
   const struct khrn_synclist *bin_synclist,
   const struct khrn_synclist *render_synclist)
{
#if defined(KHRN_AUTOCLIF) || defined(KHRN_VALIDATE_SYNCLIST)
   v3d_scheduler_wait_all();
   gmem_sync_all(/*to_cpu=*/true, /*to_v3d=*/false);
#endif

#ifdef KHRN_AUTOCLIF
   if (br_info->num_bins) // workaround for broken compute shader recording.
      khrn_record(br_info);
#endif

#ifdef KHRN_VALIDATE_SYNCLIST
   if (!khrn_synclist_validate(br_info, &bin_synclist->handles, &render_synclist->handles))
      demand_msg(0, "%s: missing entries from sync-list", VCOS_FUNCTION);
#endif
}

static void post_submit_debug(
   const V3D_BIN_RENDER_INFO_T *br_info)
{
#ifdef KHRN_SAVE_CRC_CHECKSUMS
   v3d_scheduler_wait_all();
   gmem_sync_all(/*to_cpu=*/true, /*to_v3d=*/false);
   khrn_save_crc_checksums(br_info);
#endif

   // increment frame counter (skip over ~0 "invalid")
   if (++khrn_fmem_frame_i == ~0u)
      khrn_fmem_frame_i = 0;
}

/*
 * Submit the bin and render from br_info. Tell the scheduler to wait for
 * bin and render sync, and get output information about the queued render
 * job in rdr_queued.
 */
static void submit_bin_render(KHRN_FMEM_T *fmem,
      const v3d_scheduler_deps *bin_sync,
      const v3d_scheduler_deps *render_sync,
      job_t *bin_job, job_t *render_job)
{
   khrn_fmem_common_persist *fmem_common = khrn_fmem_get_common_persist(fmem);

#ifdef KHRN_GEOMD
   fmem_debug_info_prepare_for_queries(&fmem_common->debug_info);
#endif

   // Finalise fmem blocks, so fmem_pool knows how much data was written.
   finalise_block(&fmem_common->pool, &fmem->cle);
   finalise_block(&fmem_common->pool, &fmem->data);

   /* we need to do this *before* we submit, because the submit API reports
    * completion too */
   khrn_fmem_pool_submit(&fmem_common->pool,
      &fmem->persist[BIN_STAGE]->sync_list,
      &fmem->persist[RENDER_STAGE]->sync_list);

   pre_submit_debug(&fmem->br_info,
      &fmem->persist[BIN_STAGE]->sync_list,
      &fmem->persist[RENDER_STAGE]->sync_list);

   /* we need to keep fmem persist data around till callbacks finish;*/
   khrn_mem_acquire(fmem->persist[BIN_STAGE]);
   khrn_mem_acquire(fmem->persist[RENDER_STAGE]);
   v3d_scheduler_submit_bin_render_job(bin_sync, render_sync,
      &fmem->persist[BIN_STAGE]->sync_list.sync_list,
      &fmem->persist[RENDER_STAGE]->sync_list.sync_list,
      bin_job, render_job, &fmem->br_info,
      bin_completion, fmem->persist[BIN_STAGE],
      render_completion, fmem->persist[RENDER_STAGE]);

   post_submit_debug(&fmem->br_info);

   /* fmem doesn't have anymore ownership of persist data;
    * fmem persist data will be freed by bin/render callbacks */
   for (unsigned stage = 0; stage < COUNT_STAGES; stage++)
      KHRN_MEM_ASSIGN(fmem->persist[stage], NULL);
}

static void khrn_fmem_queue_bin_render(KHRN_FMEM_T *fmem)
{
   v3d_scheduler_deps bin_sync, render_sync;
   memset(&bin_sync, 0, sizeof bin_sync);
   memset(&render_sync, 0, sizeof render_sync);
   sync_interlocks(&bin_sync, &render_sync, fmem);
   sync_fences_to_depend_on(fmem->br_info.num_bins ? &bin_sync : &render_sync, fmem);

   job_t bin_job = 0, render_job = 0;
   submit_bin_render(fmem, &bin_sync, &render_sync, &bin_job, &render_job);

   update_interlocks(fmem, bin_job, render_job);
   update_fences_to_signal(fmem, render_job);
   release_interlocks(fmem);
   release_fences(&fmem->fences_to_signal);
   release_fences(&fmem->fences_to_depend_on);
}

void khrn_fmem_flush(KHRN_FMEM_T *fmem)
{
   bool ok = false;
   khrn_fmem_common_persist *fmem_common = khrn_fmem_get_common_persist(fmem);

   assert(!fmem->in_begin_clist);
   assert(!fmem->in_begin_cle_start);
   assert(!fmem->in_begin_data_start);

   if (gmem_lock_list_is_bad(&fmem_common->pool.lock_list))
      goto end;

   for (unsigned stage = 0; stage < COUNT_STAGES; stage++)
   {
      if (gmem_lock_list_is_bad(&fmem->persist[stage]->lock_list))
         goto end;
   }

   khrn_fmem_queue_bin_render(fmem);

   ok = true;
end:
   if (!ok)
      khrn_fmem_discard(fmem);
}

static bool alloc_next(KHRN_FMEM_POOL_T *pool, KHRN_FMEM_BLOCK_T *block,
      KHRN_RENDER_STATE_T *rs)
{
   /* If we are getting low on fmems, flush others render states except
    * ourselves. Flushing a render state increases the number of submitted
    * buffers. Don't do this if we're currently flushing unless we really
    * ran out of buffers. */
   unsigned threshold = rs->flush_state == KHRN_RENDER_STATE_FLUSHING ? 1u : KHRN_FMEM_THRESHOLD_FLUSH_OTHER_RS;
   while (khrn_fmem_client_pool_get_num_free_and_submitted() < threshold)
   {
      assert(rs->flush_state != KHRN_RENDER_STATE_FLUSH_ALLOWED);
      if (!khrn_render_state_flush_oldest_possible())
         break;
   }

   uint8_t *buffer = khrn_fmem_pool_alloc(pool);
   if (!buffer)
      return false;

   finalise_block(pool, block);
   block->start = buffer;
   block->end = KHRN_FMEM_USABLE_BUFFER_SIZE;
   block->current = 0;
   return true;
}

static void block_init(KHRN_FMEM_BLOCK_T *block)
{
   block->start = NULL;
   block->end = 0;
   block->current = block->end;
}

bool khrn_fmem_is_here(KHRN_FMEM_T *fmem, uint8_t *p)
{
   return fmem->cle.start + fmem->cle.current == p;
}

v3d_addr_t khrn_fmem_hw_address(KHRN_FMEM_T *fmem, void *p)
{
   khrn_fmem_common_persist *fmem_common = khrn_fmem_get_common_persist(fmem);
   return khrn_fmem_pool_hw_address(&fmem_common->pool, p);
}

bool khrn_fmem_record_fence_to_signal(KHRN_FMEM_T *fmem,
      KHRN_FENCE_T *fence)
{
   bool already_user = false;
   KHRN_RENDER_STATE_T *rs = fmem->render_state;
   KHRN_UINTPTR_VECTOR_T *fences;

   already_user = !khrn_fence_add_user(fence, rs);

   fences = &fmem->fences_to_signal;

   if (!already_user)
   {
      if (!khrn_uintptr_vector_push_back(fences, (uintptr_t)fence))
      {
         khrn_fence_remove_user(fence, rs);
         return false;
      }
      khrn_fence_refinc(fence);
   }

   return true;
}

bool khrn_fmem_record_fence_to_depend_on(KHRN_FMEM_T *fmem,
      KHRN_FENCE_T *fence)
{
   KHRN_UINTPTR_VECTOR_T *fences;

   /* the fence should have been flushed before getting to this point
    * ( before choosing this fmem/render state) */
   assert(!khrn_fence_has_user(fence, NULL));

   fences = &fmem->fences_to_depend_on;
   if (!khrn_uintptr_vector_push_back(fences, (uintptr_t)fence))
      return false;

   khrn_fence_refinc(fence);
   return true;
}

bool khrn_fmem_record_res_interlock(KHRN_FMEM_T *fmem,
      KHRN_RES_INTERLOCK_T *res_i, bool write, khrn_interlock_action_t actions)
{
   KHRN_RENDER_STATE_T *rs = fmem->render_state;
   KHRN_INTERLOCK_T *interlock = &res_i->interlock;

   bool already_user = khrn_interlock_get_actions(interlock, rs) != ACTION_NONE;
   if (!already_user)
   {
      if (!khrn_uintptr_vector_push_back(&fmem->res_interlocks, (uintptr_t)res_i))
         return false;
      khrn_res_interlock_refinc(res_i);
   }

   if (write)
   {
      khrn_interlock_add_writer(interlock, actions, rs);
   }
   else
   {
      khrn_interlock_add_reader(interlock, actions, rs);
   }

   return true;
}

bool khrn_fmem_record_res_interlock_tf_write(KHRN_FMEM_T *fmem,
      KHRN_RES_INTERLOCK_T *res_i, bool* requires_flush)
{
   KHRN_RENDER_STATE_T *rs = fmem->render_state;
   KHRN_INTERLOCK_T *interlock = &res_i->interlock;

   bool already_user = khrn_interlock_get_actions(interlock, rs) != ACTION_NONE;
   if (!already_user)
   {
      *requires_flush = false;
      if (!khrn_uintptr_vector_push_back(&fmem->res_interlocks, (uintptr_t)res_i))
         return false;
      khrn_res_interlock_refinc(res_i);
   }

   // Could pop res_i if added, but leaving it is harmless.
   bool success = khrn_interlock_add_writer_tf(interlock, ACTION_BIN, rs);
   *requires_flush = !success;
   return success;
}

/* Allocate memory for the tile state */
v3d_addr_t khrn_fmem_tile_state_alloc(KHRN_FMEM_T *fmem, size_t size)
{
   gmem_usage_flags_t   flags = GMEM_USAGE_V3D | GMEM_USAGE_HINT_DYNAMIC;

   if (fmem->br_info.secure)
      flags = flags | GMEM_USAGE_SECURE;

   gmem_handle_t handle = gmem_alloc(size, V3D_TILE_STATE_ALIGN, flags, "tile state");
   if (handle == GMEM_HANDLE_INVALID)
      return 0;

   // store gmem handle
   khrn_fmem_common_persist *fmem_common = khrn_fmem_get_common_persist(fmem);
   assert(fmem_common->num_bin_tile_states < countof(fmem_common->bin_tile_state));
   fmem_common->bin_tile_state[fmem_common->num_bin_tile_states++] = handle;

   return khrn_fmem_lock_and_sync(fmem, handle,
         GMEM_SYNC_CORE_READ | GMEM_SYNC_CORE_WRITE, 0);
}

v3d_addr_t khrn_fmem_lock_and_sync(KHRN_FMEM_T *fmem, gmem_handle_t handle,
      uint32_t bin_rw_flags, uint32_t render_rw_flags)
{
   v3d_addr_t result = 0;

   assert(bin_rw_flags != 0 || render_rw_flags != 0);

   if (render_rw_flags != 0)
      result = gmem_lock(&fmem->persist[RENDER_STAGE]->lock_list, handle);
   else
      result = gmem_lock(&fmem->persist[BIN_STAGE]->lock_list, handle);


   if (bin_rw_flags != 0)
   {
      khrn_synclist_add(&fmem->persist[BIN_STAGE]->sync_list,
            handle, bin_rw_flags);
   }

   if (render_rw_flags != 0)
   {
      khrn_synclist_add(&fmem->persist[RENDER_STAGE]->sync_list,
            handle, render_rw_flags);
   }

   return result;
}
