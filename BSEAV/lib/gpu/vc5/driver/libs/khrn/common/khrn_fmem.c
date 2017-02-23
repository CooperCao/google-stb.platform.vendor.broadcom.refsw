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
#include "khrn_mr_crc.h"
#include "khrn_render_state.h"
#include "khrn_record.h"
#include "khrn_process.h"
#include "khrn_tile_state.h"
#include "../glxx/glxx_query.h"
#include "khrn_fence.h"
#include "../egl/egl_context_gl.h"
#include "khrn_options.h"
#include "libs/platform/gmem.h"

#include "vcos.h"
#include <stddef.h> /* for offsetof */
#include <assert.h>

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
   size_t num_handles = khrn_uintptr_vector_get_size(handles);
   for (size_t i = 0; i != num_handles; ++i)
      gmem_free((gmem_handle_t)khrn_uintptr_vector_item(handles, i));

   khrn_uintptr_vector_destroy(handles);
}

static khrn_fmem_persist* persist_create(KHRN_RENDER_STATE_T *rs)
{
   khrn_fmem_persist* persist = (khrn_fmem_persist*)calloc(1, sizeof(khrn_fmem_persist));
   if (persist == NULL)
      return NULL;

   khrn_fmem_pool_init(&persist->pool, rs);

   khrn_uintptr_vector_init(&persist->client_handles);

#ifdef KHRN_GEOMD
   fmem_debug_info_init(&persist->debug_info);
#endif

   return persist;
}

static void persist_destroy(khrn_fmem_persist* persist)
{
   assert(persist != NULL);

   if (persist->occlusion_query_list)
      glxx_queries_release(persist->occlusion_query_list);
#if V3D_VER_AT_LEAST(4,0,2,0)
   if (persist->prim_counts_query_list)
      glxx_queries_release(persist->prim_counts_query_list);
#endif
   khrn_fmem_pool_deinit(&persist->pool);

   for (unsigned i = 0; i != persist->num_bin_tile_states; ++i)
      gmem_free(persist->bin_tile_state[i]);

   if (persist->bin_shared_tile_state)
      khrn_tile_state_release_shared(persist->bin_shared_tile_state);

   free_client_handles(&persist->client_handles);

#ifdef KHRN_GEOMD
   fmem_debug_info_deinit(&persist->debug_info);
#endif

#if KHRN_DEBUG
   free(persist->gmp_tables[0]);
   free(persist->gmp_tables[1]);
#endif

   free(persist);
}

static void persist_destroy_callback(void *data, uint64_t job_id, v3d_sched_job_error error)
{
   persist_destroy((khrn_fmem_persist*)data);
}

static void render_completion(void *data, uint64_t job_id, v3d_sched_job_error error)
{
   khrn_fmem_persist* persist = (khrn_fmem_persist*)data;

   /* go through the list of queries and update occlusion queries results */
   if (persist->occlusion_query_list)
      glxx_occlusion_queries_update(persist->occlusion_query_list, true);

   if (persist->delete_stage == KHRN_INTERLOCK_STAGE_RENDER)
      persist_destroy(persist);
}

static void bin_completion(void *data, uint64_t job_id, v3d_sched_job_error error)
{
   khrn_fmem_persist* persist = (khrn_fmem_persist*)data;

#if V3D_VER_AT_LEAST(4,0,2,0)
   /* go through the list of queries and update prim counts queries results */
   if (persist->prim_counts_query_list)
      glxx_prim_counts_queries_update(persist->prim_counts_query_list, true);
#endif

   if (persist->delete_stage == KHRN_INTERLOCK_STAGE_BIN)
      persist_destroy(persist);
}

bool khrn_fmem_init(KHRN_FMEM_T *fmem, KHRN_RENDER_STATE_T *render_state)
{
   memset(fmem, 0, sizeof *fmem);
   fmem->render_state = render_state;

   fmem->persist = persist_create(fmem->render_state);
   if (!fmem->persist)
      return false;

#if KHRN_DEBUG
   if (khrn_options.autoclif_enabled || khrn_options.save_crc_enabled || !khrn_options.no_gmp)
      fmem->memaccess = khrn_memaccess_create(render_state);
#endif

   /* the cle and data blocks come from the same allocator */
   block_init(&fmem->cle);
   block_init(&fmem->data);

   // the initialised state is as if khrn_fmem_begin_clist has been called
#ifndef NDEBUG
   fmem->in_begin_clist = true;
#endif

   khrn_uintptr_vector_init(&fmem->res_interlocks);
   khrn_uintptr_vector_init(&fmem->fences_to_signal);
   khrn_uintptr_vector_init(&fmem->fences_to_depend_on);

   return true;
}

static void release_interlocks(KHRN_FMEM_T *fmem)
{
   KHRN_UINTPTR_VECTOR_T *res_interlocks = &fmem->res_interlocks;

   size_t num_interlocks = khrn_uintptr_vector_get_size(res_interlocks);
   for (size_t i = 0; i != num_interlocks; ++i)
   {
      KHRN_RES_INTERLOCK_T* res = (KHRN_RES_INTERLOCK_T*)khrn_uintptr_vector_item(res_interlocks, i);
      khrn_res_interlock_refdec(res);
   }
   khrn_uintptr_vector_destroy(res_interlocks);
}

static void release_fences(KHRN_UINTPTR_VECTOR_T* fences)
{
   size_t num_fences = khrn_uintptr_vector_get_size(fences);
   for (size_t i = 0; i != num_fences ; ++i)
   {
      KHRN_FENCE_T *kfence = (KHRN_FENCE_T*)khrn_uintptr_vector_item(fences, i);
      khrn_fence_refdec(kfence);
   }
   khrn_uintptr_vector_destroy(fences);
}

static void unrecord_interlocks(KHRN_FMEM_T *fmem)
{
   KHRN_UINTPTR_VECTOR_T *res_interlocks = &fmem->res_interlocks;

   for (size_t i = 0; i < khrn_uintptr_vector_get_size(res_interlocks); i++)
   {
      KHRN_RES_INTERLOCK_T *res_i = (KHRN_RES_INTERLOCK_T*)
         khrn_uintptr_vector_item(res_interlocks, i);
      khrn_interlock_remove_rs(&res_i->interlock, fmem->render_state);
   }
}

static void unrecord_fences_to_signal(KHRN_FMEM_T *fmem)
{
   KHRN_UINTPTR_VECTOR_T *fences = &fmem->fences_to_signal;

   for (size_t i = 0; i < khrn_uintptr_vector_get_size(fences); i++)
   {
      KHRN_FENCE_T *fence = (KHRN_FENCE_T*) khrn_uintptr_vector_item(fences, i);
      khrn_fence_remove_user(fence, fmem->render_state);
   }
}

bool khrn_fmem_record_handle(KHRN_FMEM_T *fmem, gmem_handle_t handle)
{
   return khrn_uintptr_vector_push_back(&fmem->persist->client_handles, (uintptr_t)handle);
}

void khrn_fmem_discard(KHRN_FMEM_T *fmem)
{
   log_trace("\nkhrn_fmem_discard() fmem = %p", fmem);

   /* queries expect a number of completion callbacks; if we drop a frame,
    * record things as we would have got a completion callback, but without
    * updating results */
   glxx_occlusion_queries_update(fmem->persist->occlusion_query_list, false);
#if V3D_VER_AT_LEAST(4,0,2,0)
   glxx_prim_counts_queries_update(fmem->persist->prim_counts_query_list, false);
#endif

#if KHRN_DEBUG
   khrn_memaccess_destroy(fmem->memaccess);
#endif

   persist_destroy(fmem->persist);
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
   assert(size <= KHRN_FMEM_USABLE_BUFFER_SIZE);

   /* alignment is relative to the start of the buffer */
   start_offset = (block->current + align - 1) & ~(align - 1);

   if (start_offset + size >= block->end)
   {
      if (!alloc_next(pool, block, rs))
         return NULL;

      start_offset = (block->current  + align - 1) & ~(align - 1);
      assert(start_offset + size < block->end);
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

   uint8_t *result = alloc(&fmem->persist->pool, cle, size, 1, fmem->render_state);

   if (last_block != cle->start)
   {
      /* just allocated a new block */
      assert(result != NULL);

      if (last_block != NULL)
      {
         /* link old block cl to new */
         last_block += last_current;
         v3d_cl_branch(&last_block, khrn_fmem_pool_hw_address(&fmem->persist->pool, result));
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

   return alloc(&fmem->persist->pool, &fmem->data, size, align, fmem->render_state);
}

v3d_addr_t khrn_fmem_begin_clist(KHRN_FMEM_T *fmem)
{
   // ensure not in middle of another clist
   assert(!fmem->in_begin_clist);
#ifndef NDEBUG
   fmem->in_begin_clist = true;
#endif
   uint8_t *p = khrn_fmem_cle(fmem, 0);
   return p ? khrn_fmem_hw_address(fmem, p) : 0;
}

v3d_addr_t khrn_fmem_end_clist(KHRN_FMEM_T *fmem)
{
   assert(fmem->in_begin_clist);
#ifndef NDEBUG
   fmem->in_begin_clist = false;
#endif
   return fmem->cle.start ? khrn_fmem_hw_address(fmem, fmem->cle.start + fmem->cle.current) : 0;
}

#if V3D_VER_AT_LEAST(4,0,2,0)

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

static void get_deps_from_and_release_fences(v3d_scheduler_deps* first_stage_deps, KHRN_FMEM_T* fmem)
{
   KHRN_UINTPTR_VECTOR_T* fences = &fmem->fences_to_depend_on;

   const size_t num_fences = khrn_uintptr_vector_get_size(fences);
   for (size_t i = 0; i != num_fences; ++i)
   {
      KHRN_FENCE_T* fence = (KHRN_FENCE_T*)khrn_uintptr_vector_item(fences, i);
      assert(!khrn_fence_has_user(fence, NULL));
      v3d_scheduler_merge_deps(first_stage_deps, &fence->deps);

      khrn_fence_refdec(fence);
   }
   khrn_uintptr_vector_destroy(fences);
}

/*
 * fmem has a list of "resource interlocks" for resources it reads and buffers
 * it may write (including the one associated with its own render_state). Update
 * the dependencies for each stage from the interlocks.
 */
static void get_deps_from_interlocks(
   v3d_scheduler_deps stage_deps[KHRN_INTERLOCK_NUM_STAGES],
   KHRN_FMEM_T *fmem)
{
   const KHRN_UINTPTR_VECTOR_T *res_interlocks = &fmem->res_interlocks;
   KHRN_RENDER_STATE_T *rs = fmem->render_state;

   size_t num_interlocks = khrn_uintptr_vector_get_size(res_interlocks);
   for (size_t i = 0; i != num_interlocks; ++i)
   {
      KHRN_RES_INTERLOCK_T* res = (KHRN_RES_INTERLOCK_T *)khrn_uintptr_vector_item(res_interlocks, i);
      KHRN_INTERLOCK_T* interlock = &res->interlock;

      if (res->handle == GMEM_HANDLE_INVALID)
      {
         /* no point to transfer dependencies since we don't have storage,
          * just unmark us as the user of that interlock */
         khrn_interlock_remove_rs(interlock, rs);
         continue;
      }

      khrn_interlock_get_deps_for_rs(interlock, rs, stage_deps);
   }
}

static void update_and_release_fences(KHRN_FMEM_T* fmem, job_t last_stage_job_id)
{
   KHRN_UINTPTR_VECTOR_T* fences = &fmem->fences_to_signal;

   const size_t num_fences = khrn_uintptr_vector_get_size(fences);
   for (size_t i = 0; i != num_fences; ++i)
   {
      KHRN_FENCE_T* fence = (KHRN_FENCE_T*)khrn_uintptr_vector_item(fences, i);
      khrn_fence_job_add(fence, last_stage_job_id);
      khrn_fence_remove_user(fence, fmem->render_state);
      khrn_fence_refdec(fence);
   }
   khrn_uintptr_vector_destroy(fences);
}

/*
 * Any resources that this fmem uses can now have their sync objects set up to
 * wait for the jobs we've just submitted to the scheduler.
 */
static void update_interlocks(
   KHRN_FMEM_T *fmem,
   job_t const stage_jobs[KHRN_INTERLOCK_NUM_STAGES])
{
   KHRN_UINTPTR_VECTOR_T *res_interlocks = &fmem->res_interlocks;
   KHRN_RENDER_STATE_T *rs = fmem->render_state;

   size_t num_interlocks = khrn_uintptr_vector_get_size(res_interlocks);
   for (size_t i = 0; i != num_interlocks; ++i)
   {
      KHRN_RES_INTERLOCK_T* res = (KHRN_RES_INTERLOCK_T *)khrn_uintptr_vector_item(res_interlocks, i);
      if (  res->handle != GMEM_HANDLE_INVALID
         && khrn_interlock_update_from_rs(&res->interlock, rs, stage_jobs) )
      {
         khrn_res_interlock_invalidate_synced_range(res);
      }
   }
}

static inline void finalise_block(KHRN_FMEM_POOL_T *pool, KHRN_FMEM_BLOCK_T *block)
{
   if (block->start)
      khrn_fmem_pool_finalise_end(pool, block->start + block->current + GFX_MAX(V3D_MAX_CLE_READAHEAD, V3D_MAX_QPU_UNIFS_READAHEAD));
}

/*
 * Submit the bin and render from br_info. Tell the scheduler to wait for
 * bin and render sync, and get output information about the queued render
 * job in rdr_queued.
 */
static void submit_bin_render(
   KHRN_FMEM_T* fmem,
   v3d_scheduler_deps const stage_deps[KHRN_INTERLOCK_NUM_STAGES],
   job_t stage_jobs[KHRN_INTERLOCK_NUM_STAGES])
{
#ifdef KHRN_GEOMD
   fmem_debug_info_prepare_for_queries(&fmem->persist->debug_info);
#endif

   /* we need to do this *before* we submit, because the submit API reports
    * completion too */
   v3d_barrier_flags bin_rw_flags, render_rw_flags;
   khrn_fmem_pool_submit(
      &fmem->persist->pool,
#if KHRN_DEBUG
      fmem->memaccess,
#endif
      &bin_rw_flags,
      &render_rw_flags);
   fmem->bin_rw_flags |= bin_rw_flags;
   fmem->render_rw_flags |= render_rw_flags;

#if KHRN_DEBUG
   if (khrn_options.autoclif_enabled)
      khrn_record(fmem->memaccess, &fmem->br_info);

   if (!khrn_options.no_gmp)
   {
      khrn_memaccess_build_gmp_tables(fmem->memaccess, fmem->persist->gmp_tables);
      fmem->br_info.bin_gmp_table = fmem->persist->gmp_tables[0];
      fmem->br_info.render_gmp_table = fmem->persist->gmp_tables[1];
   }
#endif

   {
      v3d_barrier_flags bin_flags = fmem->bin_rw_flags;
      v3d_barrier_flags rdr_flags = fmem->render_rw_flags;

      // Pre-bin barrier between memory and bin-readers.
      fmem->br_info.bin_cache_ops = v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_WRITE, bin_flags);
      fmem->br_info.bin_cache_ops &= ~V3D_CACHE_CLEAR_VCD; // VCD cache flushed in CL

      // Post-bin barrier between bin-writers and memory.
      fmem->br_info.bin_cache_ops |= v3d_barrier_cache_cleans(bin_flags, V3D_BARRIER_MEMORY_READ);

      // Pre-render barrier between memory and render-readers.
      fmem->br_info.render_cache_ops = v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_WRITE, rdr_flags);
      fmem->br_info.render_cache_ops &= ~V3D_CACHE_CLEAR_VCD; // VCD cache flushed in CL

      // Post-render barrier between render-writers and memory.
      fmem->br_info.render_cache_ops |= v3d_barrier_cache_cleans(rdr_flags, V3D_BARRIER_MEMORY_READ);
   }

   // If not using fmem->memaccess, the last stage completion callback should delete fmem->persist.
   khrn_fmem_persist* persist = fmem->persist;
#if KHRN_DEBUG
   if (!fmem->memaccess)
#endif
   {
      persist->delete_stage = KHRN_INTERLOCK_STAGE_RENDER;
      fmem->persist = NULL;
   }

   unsigned bin_index = gfx_log2(KHRN_INTERLOCK_STAGE_BIN);
   unsigned render_index = gfx_log2(KHRN_INTERLOCK_STAGE_RENDER);
   v3d_scheduler_submit_bin_render_job(
      &stage_deps[bin_index],
      &stage_deps[render_index],
      &stage_jobs[bin_index],
      &stage_jobs[render_index],
      &fmem->br_info,
      bin_completion, persist,
      render_completion, persist);
}

void khrn_fmem_flush(KHRN_FMEM_T *fmem)
{
   assert(!fmem->in_begin_clist);
   assert(!fmem->in_begin_cle_start);
   assert(!fmem->in_begin_data_start);

   // Finalise fmem blocks, so fmem_pool knows how much data was written.
   finalise_block(&fmem->persist->pool, &fmem->cle);
   finalise_block(&fmem->persist->pool, &fmem->data);

   v3d_scheduler_deps stage_deps[KHRN_INTERLOCK_NUM_STAGES];
   for (unsigned i = 0; i != countof(stage_deps); ++i)
      v3d_scheduler_deps_init(&stage_deps[i]);

   get_deps_from_and_release_fences(&stage_deps[0], fmem);
   get_deps_from_interlocks(stage_deps, fmem);

   // Stage dependencies are transitive.
   for (unsigned s = 1; s != countof(stage_deps); ++s)
      v3d_scheduler_merge_deps(&stage_deps[s], &stage_deps[s-1]);

   job_t stage_jobs[KHRN_INTERLOCK_NUM_STAGES] = { 0, };
   submit_bin_render(fmem, stage_deps, stage_jobs);

   // Find job for last active stage.
   uint64_t last_job = stage_jobs[0];
   for (unsigned s = 1; s != countof(stage_jobs); ++s)
   {
      if (stage_jobs[s])
         last_job = stage_jobs[s];
   }

   update_interlocks(fmem, stage_jobs);
   update_and_release_fences(fmem, last_job);

#if KHRN_DEBUG
   if (khrn_options.save_crc_enabled)
      khrn_save_crc_checksums(fmem->memaccess, &fmem->br_info);

   khrn_memaccess_destroy(fmem->memaccess);
#endif

   // After memaccess is destroyed, it is safe to release references to gmem
   // handles and interlocks used by this render-state.
   release_interlocks(fmem);

   // If we were using khrn_memaccess, then defer delete the persist object.
#if KHRN_DEBUG
   if (fmem->persist)
   {
      v3d_scheduler_deps last_job_deps;
      v3d_scheduler_deps_set(&last_job_deps, last_job);

      v3d_scheduler_submit_null_job(&last_job_deps, persist_destroy_callback, fmem->persist);
      fmem->persist = NULL;
   }
#endif

   // increment frame counter (skip over ~0 "invalid")
   if (++khrn_fmem_frame_i == ~0u)
      khrn_fmem_frame_i = 0;
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
   return khrn_fmem_pool_hw_address(&fmem->persist->pool, p);
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

static bool add_res_interlock(KHRN_FMEM_T *fmem, KHRN_RES_INTERLOCK_T *res_i)
{
   if (!khrn_interlock_get_stages(&res_i->interlock, fmem->render_state))
   {
   #if KHRN_DEBUG
      if (fmem->memaccess)
         khrn_memaccess_register_resource(fmem->memaccess, res_i);
   #endif

      if (!khrn_uintptr_vector_push_back(&fmem->res_interlocks, (uintptr_t)res_i))
         return false;
      khrn_res_interlock_refinc(res_i);
   }
   return true;
}

bool khrn_fmem_record_res_interlock_read(KHRN_FMEM_T *fmem,
      KHRN_RES_INTERLOCK_T *res_i, khrn_interlock_stages_t stages)
{
   if (!add_res_interlock(fmem, res_i))
      return false;
   khrn_interlock_add_reader(&res_i->interlock, stages, fmem->render_state);
   return true;
}

bool khrn_fmem_record_res_interlock_write(KHRN_FMEM_T *fmem,
      KHRN_RES_INTERLOCK_T *res_i, khrn_interlock_stages_t stages,
      khrn_interlock_parts_t parts, bool *invalid_out)
{
   if (!add_res_interlock(fmem, res_i))
      return false;
   bool invalid = khrn_interlock_add_writer(&res_i->interlock, stages, fmem->render_state, parts);
   if (invalid_out)
      *invalid_out = invalid;
   return true;
}

bool khrn_fmem_record_res_interlock_self_read_conflicting_write(KHRN_FMEM_T *fmem,
      KHRN_RES_INTERLOCK_T *res_i, khrn_interlock_stages_t stages,
      khrn_interlock_parts_t parts, bool* requires_flush)
{
   if (!add_res_interlock(fmem, res_i))
   {
      *requires_flush = false;
      return false;
   }
   // On failure could pop res_i if added, but leaving it is harmless.
   bool success = khrn_interlock_add_self_read_conflicting_writer(&res_i->interlock, stages, fmem->render_state, parts);
   *requires_flush = !success;
   return success;
}

/* Allocate memory for the tile state */
v3d_addr_t khrn_fmem_tile_state_alloc(KHRN_FMEM_T *fmem, size_t size)
{
   khrn_fmem_persist* persist = fmem->persist;

   gmem_handle_t handle;
   if (khrn_get_num_cores() == 1)
   {
      // Store shared object to be released on bin completion.
      persist->bin_shared_tile_state = khrn_tile_state_alloc_shared(size, fmem->br_info.secure);
      if (!persist->bin_shared_tile_state)
         return 0;
      handle = persist->bin_shared_tile_state->handle;
   }
   else
   {
      handle = khrn_tile_state_alloc_gmem(size, fmem->br_info.secure);
      if (!handle)
         return 0;

      // Store handle to be freed on bin completion.
      assert(persist->num_bin_tile_states < countof(persist->bin_tile_state));
      persist->bin_tile_state[persist->num_bin_tile_states++] = handle;
   }

   return khrn_fmem_lock_and_sync(fmem, handle, V3D_BARRIER_PTB_TILESTATE_WRITE, 0);
}

v3d_addr_t khrn_fmem_lock_and_sync(KHRN_FMEM_T *fmem, gmem_handle_t handle,
      uint32_t bin_rw_flags, uint32_t render_rw_flags)
{
   assert(bin_rw_flags != 0 || render_rw_flags != 0);

   // Expectations of what bin/render should read/write.
   assert(!(bin_rw_flags & V3D_BARRIER_TMU_DATA_WRITE));
   assert(!(bin_rw_flags & V3D_BARRIER_TLB_IMAGE_READ));
   assert(!(bin_rw_flags & V3D_BARRIER_TLB_IMAGE_WRITE));
   assert(!(bin_rw_flags & V3D_BARRIER_TLB_OQ_READ));
   assert(!(bin_rw_flags & V3D_BARRIER_TLB_OQ_WRITE));
   assert(!(render_rw_flags & V3D_BARRIER_CLE_PRIMIND_READ));
   assert(!(render_rw_flags & V3D_BARRIER_CLE_DRAWREC_READ));
   assert(!(render_rw_flags & V3D_BARRIER_PTB_TF_WRITE));
   assert(!(render_rw_flags & V3D_BARRIER_PTB_TILESTATE_WRITE));
   assert(!(render_rw_flags & V3D_BARRIER_PTB_PCF_READ));
   assert(!(render_rw_flags & V3D_BARRIER_PTB_PCF_WRITE));
   assert(!((bin_rw_flags|render_rw_flags) & V3D_BARRIER_TFU_READ));
   assert(!((bin_rw_flags|render_rw_flags) & V3D_BARRIER_TFU_WRITE));

   fmem->bin_rw_flags |= bin_rw_flags;
   fmem->render_rw_flags |= render_rw_flags;

#if KHRN_DEBUG
   if (fmem->memaccess)
      khrn_memaccess_add_buffer(fmem->memaccess, handle, bin_rw_flags, render_rw_flags);
#endif

   return gmem_get_addr(handle);
}
