/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_fmem.h"
#include "khrn_mr_crc.h"
#include "khrn_render_state.h"
#include "khrn_record.h"
#include "khrn_process.h"
#include "khrn_tile_state.h"
#include "../glxx/glxx_query.h"
#include "../glxx/glxx_shader.h"
#include "khrn_fence.h"
#include "../egl/egl_context_gl.h"
#include "khrn_options.h"
#include "libs/platform/gmem.h"
#if KHRN_GLES31_DRIVER
#include "libs/compute/compute.h"
#endif

#include "vcos.h"
#include <stddef.h> /* for offsetof */
#include <assert.h>

#include "libs/core/v3d/v3d_cl.h"

#include "libs/platform/v3d_driver_api.h"
#include "libs/platform/bcm_sched_api.h"

LOG_DEFAULT_CAT("khrn_fmem")

#if KHRN_GLES31_DRIVER
static uint64_t khrn_fmem_compute_jobs[2];
#endif

void khrn_fmem_static_init(void)
{
 #if KHRN_GLES31_DRIVER
   memset(khrn_fmem_compute_jobs, 0, sizeof(khrn_fmem_compute_jobs));
 #endif
}

typedef struct khrn_fmem_buffer_range
{
   gmem_handle_t handle;
   v3d_size_t start;
   v3d_size_t end;
} khrn_fmem_buffer_range;

static void block_init(khrn_fmem_block *block);
static bool alloc_next(khrn_fmem_pool *pool, khrn_fmem_block *block,
      khrn_render_state *rs);

uint32_t khrn_fmem_frame_i = 0;

static void free_client_handles(khrn_uintptr_vector *handles)
{
   size_t num_handles = khrn_uintptr_vector_get_size(handles);
   for (size_t i = 0; i != num_handles; ++i)
      gmem_free((gmem_handle_t)khrn_uintptr_vector_item(handles, i));

   khrn_uintptr_vector_destroy(handles);
}

static khrn_fmem_persist* persist_create(khrn_render_state *rs)
{
   khrn_fmem_persist* persist = (khrn_fmem_persist*)calloc(1, sizeof(khrn_fmem_persist));
   if (persist == NULL)
      return NULL;

   khrn_fmem_pool_init(&persist->pool, rs);

#if KHRN_DEBUG
   if (!khrn_options.no_gmp)
      persist->gmp_tables = calloc(2, V3D_GMP_TABLE_SIZE);

   persist->memaccess_refs = !!khrn_options.autoclif_enabled
                           + !!khrn_options.save_crc_enabled
                           + !khrn_options.no_gmp;
   if (persist->memaccess_refs != 0)
      persist->memaccess = khrn_memaccess_create(rs);
#endif

#ifdef KHRN_GEOMD
   fmem_debug_info_init(&persist->debug_info);
#endif

   persist->gpu_aborted = NULL;

   return persist;
}

#if KHRN_DEBUG
static void persist_release_memaccess(khrn_fmem_persist* persist)
{
   assert(persist->memaccess_refs > 0 && persist->memaccess != NULL);
   if (!--persist->memaccess_refs)
   {
      khrn_memaccess_destroy(persist->memaccess);
      persist->memaccess = NULL;
   }
}
#endif

static void persist_destroy(khrn_fmem_persist* persist)
{
   assert(persist != NULL);

#if KHRN_DEBUG
   khrn_memaccess_destroy(persist->memaccess);
   free(persist->gmp_tables);
#endif

   KHRN_MEM_ASSIGN(persist->gpu_aborted, NULL);

   if (persist->occlusion_query_list)
      glxx_queries_release(persist->occlusion_query_list);
#if V3D_VER_AT_LEAST(4,0,2,0)
   if (persist->prim_counts_query_list)
      glxx_queries_release(persist->prim_counts_query_list);
#endif
   khrn_fmem_pool_deinit(&persist->pool);

#if !V3D_HAS_QTS
   for (unsigned i = 0; i != persist->num_bin_tile_states; ++i)
      gmem_free(persist->bin_tile_state[i]);

   if (persist->bin_shared_tile_state)
      khrn_tile_state_release_shared(persist->bin_shared_tile_state);
#endif

   free_client_handles(&persist->client_handles);

   for (unsigned b = 0; b != persist->preprocess_ubo_loads.size; ++b)
      khrn_mem_release(khrn_vector_data(glxx_hw_ubo_load_batch, &persist->preprocess_ubo_loads)[b].uniform_map);

   free(persist->preprocess_buffers.data);
   free(persist->preprocess_ubo_loads.data);
 #if KHRN_GLES31_DRIVER
   free(persist->compute_dispatches.data);
   compute_job_mem_delete(persist->compute_job_mem);
 #endif

#ifdef KHRN_GEOMD
   fmem_debug_info_deinit(&persist->debug_info);
#endif

   free(persist);
}

static void render_completion(void *data, uint64_t job_id, v3d_sched_job_error error)
{
   khrn_fmem_persist* persist = (khrn_fmem_persist*)data;

   if (error == V3D_SCHED_JOB_ERROR)
   {
      assert(persist->gpu_aborted != NULL);
      vcos_atomic_store_bool(persist->gpu_aborted, true, VCOS_MEMORY_ORDER_RELAXED);
   }

   /* go through the list of queries and update occlusion queries results */
   if (persist->occlusion_query_list)
      glxx_occlusion_queries_update(persist->occlusion_query_list, error == V3D_SCHED_JOB_SUCCESS);

 #if KHRN_GLES31_DRIVER
   compute_job_mem_delete(persist->compute_job_mem);
   persist->compute_job_mem = NULL;
 #endif

#if KHRN_DEBUG
   if (!khrn_options.save_crc_enabled)
#endif
   {
      persist_destroy(persist);
   }
}

static void bin_completion(void *data, uint64_t job_id, v3d_sched_job_error error)
{
   // If adding anything else to this function, then ensure it is actually
   // registered against the bin job.

   khrn_fmem_persist* persist = (khrn_fmem_persist*)data;

   if (error == V3D_SCHED_JOB_ERROR)
   {
      assert(persist->gpu_aborted);
      vcos_atomic_store_bool(persist->gpu_aborted, true, VCOS_MEMORY_ORDER_RELAXED);
   }

#if V3D_VER_AT_LEAST(4,0,2,0)
   /* go through the list of queries and update prim counts queries results */
   if (persist->prim_counts_query_list != NULL)
      glxx_prim_counts_queries_update(persist->prim_counts_query_list, error == V3D_SCHED_JOB_SUCCESS);
#endif
}

bool khrn_fmem_init(khrn_fmem *fmem, khrn_render_state *render_state)
{
   memset(fmem, 0, sizeof *fmem);
   fmem->render_state = render_state;

   fmem->persist = persist_create(fmem->render_state);
   if (!fmem->persist)
      return false;

   /* the cle and data blocks come from the same allocator */
   block_init(&fmem->cle);
   block_init(&fmem->data);

   // the initialised state is as if khrn_fmem_begin_clist has been called
#ifndef NDEBUG
   fmem->in_begin_clist = true;
#endif

   khrn_uintptr_vector_init(&fmem->resources);
   khrn_uintptr_vector_init(&fmem->fences_to_signal);
   khrn_uintptr_vector_init(&fmem->fences_to_depend_on);
   return true;
}

static void release_resources(khrn_fmem *fmem)
{
   khrn_uintptr_vector *resources = &fmem->resources;

   size_t num_resources = khrn_uintptr_vector_get_size(resources);
   for (size_t i = 0; i != num_resources; ++i)
   {
      khrn_resource* res = (khrn_resource*)khrn_uintptr_vector_item(resources, i);
      khrn_resource_refdec(res);
   }
   khrn_uintptr_vector_destroy(resources);
}

static void release_fences(khrn_uintptr_vector* fences)
{
   size_t num_fences = khrn_uintptr_vector_get_size(fences);
   for (size_t i = 0; i != num_fences ; ++i)
   {
      khrn_fence *kfence = (khrn_fence*)khrn_uintptr_vector_item(fences, i);
      khrn_fence_refdec(kfence);
   }
   khrn_uintptr_vector_destroy(fences);
}

static void unrecord_resources(khrn_fmem *fmem)
{
   khrn_uintptr_vector *resources = &fmem->resources;

   for (size_t i = 0; i < khrn_uintptr_vector_get_size(resources); i++)
   {
      khrn_resource *res = (khrn_resource*)
         khrn_uintptr_vector_item(resources, i);
      khrn_resource_remove_rs(res, fmem->render_state);
   }
}

static void unrecord_fences_to_signal(khrn_fmem *fmem)
{
   khrn_uintptr_vector *fences = &fmem->fences_to_signal;

   for (size_t i = 0; i < khrn_uintptr_vector_get_size(fences); i++)
   {
      khrn_fence *fence = (khrn_fence*) khrn_uintptr_vector_item(fences, i);
      khrn_fence_remove_user(fence, fmem->render_state);
   }
}

bool khrn_fmem_record_handle(khrn_fmem *fmem, gmem_handle_t handle)
{
   return khrn_uintptr_vector_push_back(&fmem->persist->client_handles, (uintptr_t)handle);
}

void khrn_fmem_discard(khrn_fmem *fmem)
{
   log_trace("\nkhrn_fmem_discard() fmem = %p", fmem);

   /* queries expect a number of completion callbacks; if we drop a frame,
    * record things as we would have got a completion callback, but without
    * updating results */
   glxx_occlusion_queries_update(fmem->persist->occlusion_query_list, false);
#if V3D_VER_AT_LEAST(4,0,2,0)
   glxx_prim_counts_queries_update(fmem->persist->prim_counts_query_list, false);
#endif

   persist_destroy(fmem->persist);
   unrecord_resources(fmem);
   release_resources(fmem);

   unrecord_fences_to_signal(fmem);
   release_fences(&fmem->fences_to_signal);
   release_fences(&fmem->fences_to_depend_on);
}

bool khrn_fmem_reset(khrn_fmem *fmem, khrn_render_state *render_state)
{
   khrn_fmem_discard(fmem);
   return (khrn_fmem_init(fmem, render_state));
}

static void *alloc(khrn_fmem_pool *pool, khrn_fmem_block *block,
   unsigned int size, unsigned int align, khrn_render_state *rs)
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

uint8_t *khrn_fmem_cle(khrn_fmem *fmem, unsigned size)
{
   assert(fmem->in_begin_clist);
   assert(!fmem->in_begin_cle_start);
   assert(!fmem->cle_closed);
   khrn_fmem_block *cle  = &fmem->cle;
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


uint32_t *khrn_fmem_data(khrn_fmem *fmem, unsigned size, unsigned align)
{
   assert(!fmem->in_begin_data_start);

   return alloc(&fmem->persist->pool, &fmem->data, size, align, fmem->render_state);
}

v3d_addr_t khrn_fmem_begin_clist(khrn_fmem *fmem)
{
   // ensure not in middle of another clist
   assert(!fmem->in_begin_clist);
#ifndef NDEBUG
   fmem->in_begin_clist = true;
#endif
   uint8_t *p = khrn_fmem_cle(fmem, 0);
   return p ? khrn_fmem_hw_address(fmem, p) : 0;
}

v3d_addr_t khrn_fmem_end_clist(khrn_fmem *fmem)
{
   assert(fmem->in_begin_clist);
#ifndef NDEBUG
   fmem->in_begin_clist = false;
#endif
   v3d_addr_t ret = fmem->cle.start ? khrn_fmem_hw_address(fmem, fmem->cle.start + fmem->cle.current) : 0;
   /* we need to avoid end address being in the same 32 bytes as the next start
    * address (GFXH-1285) */
   fmem->cle.current = gfx_zmin( fmem->cle.current + V3D_CLE_MIN_DIST_FROM_END_TO_AVOID_FALSE_HITS,
         fmem->cle.end);
   return ret;
}

#if V3D_VER_AT_LEAST(4,0,2,0)

static_assrt(V3D_TMU_CONFIG_CACHE_LINE_SIZE >= 32);

static void *tmu_cfg_alloc_32(khrn_fmem *fmem)
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

v3d_addr_t khrn_fmem_add_tmu_tex_state(khrn_fmem *fmem,
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

v3d_addr_t khrn_fmem_add_tmu_sampler(khrn_fmem *fmem,
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

v3d_addr_t khrn_fmem_add_tmu_indirect(khrn_fmem *fmem, uint32_t const *tmu_indirect)
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

static void get_deps_from_and_release_fences(v3d_scheduler_deps* first_stage_deps, khrn_fmem* fmem)
{
   khrn_uintptr_vector* fences = &fmem->fences_to_depend_on;

   const size_t num_fences = khrn_uintptr_vector_get_size(fences);
   for (size_t i = 0; i != num_fences; ++i)
   {
      khrn_fence* fence = (khrn_fence*)khrn_uintptr_vector_item(fences, i);
      assert(!khrn_fence_has_user(fence, NULL));
      v3d_scheduler_merge_deps(first_stage_deps, &fence->deps);

      khrn_fence_refdec(fence);
   }
   khrn_uintptr_vector_destroy(fences);
}

/*
 * fmem has a list of resources that it reads and writes (including the one
 * associated with its own render_state). Update the dependencies for each
 * stage from the resource list.
 */
static void get_deps_from_resources(
   v3d_scheduler_deps stage_deps[KHRN_RESOURCE_NUM_STAGES],
   khrn_fmem *fmem)
{
   const khrn_uintptr_vector *resources = &fmem->resources;
   khrn_render_state *rs = fmem->render_state;

   size_t num_resources = khrn_uintptr_vector_get_size(resources);
   for (size_t i = 0; i != num_resources; ++i)
   {
      khrn_resource* res = (khrn_resource *)khrn_uintptr_vector_item(resources, i);

      if (res->handle == GMEM_HANDLE_INVALID)
      {
         /* no point to transfer dependencies since we don't have storage,
          * just unmark us as the user of that resource */
         khrn_resource_remove_rs(res, rs);
         continue;
      }

      khrn_resource_get_deps_for_rs(res, rs, stage_deps);
   }
}

static void update_and_release_fences(khrn_fmem* fmem, job_t last_stage_job_id)
{
   khrn_uintptr_vector* fences = &fmem->fences_to_signal;

   const size_t num_fences = khrn_uintptr_vector_get_size(fences);
   for (size_t i = 0; i != num_fences; ++i)
   {
      khrn_fence* fence = (khrn_fence*)khrn_uintptr_vector_item(fences, i);
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
static void update_resources(
   khrn_fmem *fmem,
   job_t const stage_jobs[KHRN_RESOURCE_NUM_STAGES])
{
   khrn_uintptr_vector *resources = &fmem->resources;
   khrn_render_state *rs = fmem->render_state;

   size_t num_resources = khrn_uintptr_vector_get_size(resources);
   for (size_t i = 0; i != num_resources; ++i)
   {
      khrn_resource* res = (khrn_resource *)khrn_uintptr_vector_item(resources, i);
      if (res->handle != GMEM_HANDLE_INVALID)
         khrn_resource_update_from_rs(res, rs, stage_jobs);
   }
}

static inline void finalise_block(khrn_fmem_pool *pool, khrn_fmem_block *block)
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
   khrn_fmem* fmem,
   v3d_scheduler_deps stage_deps[KHRN_RESOURCE_NUM_STAGES],
   job_t stage_jobs[KHRN_RESOURCE_NUM_STAGES])
{
#ifdef KHRN_GEOMD
   fmem_debug_info_prepare_for_queries(&fmem->persist->debug_info);
#endif

#if KHRN_DEBUG
   if (khrn_options.autoclif_enabled)
   {
      // Wait for preprocess before using memaccess.
      v3d_scheduler_wait_jobs(&stage_deps[KHRN_STAGE_PREPROCESS], V3D_SCHED_DEPS_COMPLETED);
      khrn_record(fmem->persist->memaccess, &fmem->br_info);
      persist_release_memaccess(fmem->persist);
   }
#endif

   {
      bool has_l3c = v3d_scheduler_get_hub_identity()->has_l3c;

      v3d_barrier_flags bin_flags = fmem->bin_rw_flags;
      v3d_barrier_flags rdr_flags = fmem->render_rw_flags;

      // Pre-bin barrier between memory and bin-readers.
      fmem->br_info.details.bin_cache_ops = v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_WRITE, bin_flags, false, has_l3c);
      fmem->br_info.details.bin_cache_ops &= ~V3D_CACHE_CLEAR_VCD; // VCD cache flushed in CL

      // Post-bin barrier between bin-writers and memory.
      fmem->br_info.details.bin_cache_ops |= v3d_barrier_cache_cleans(bin_flags, V3D_BARRIER_MEMORY_READ, false, has_l3c);

      // Pre-render barrier between memory and render-readers.
      fmem->br_info.details.render_cache_ops = v3d_barrier_cache_flushes(V3D_BARRIER_MEMORY_WRITE, rdr_flags, false, has_l3c);
      fmem->br_info.details.render_cache_ops &= ~V3D_CACHE_CLEAR_VCD; // VCD cache flushed in CL

      // Post-render barrier between render-writers and memory.
      fmem->br_info.details.render_cache_ops |= v3d_barrier_cache_cleans(rdr_flags, V3D_BARRIER_MEMORY_READ, false, has_l3c);
   }

   unsigned bin_index = gfx_log2(KHRN_STAGE_BIN);
   unsigned render_index = gfx_log2(KHRN_STAGE_RENDER);
   v3d_scheduler_submit_bin_render_job(
      &stage_deps[bin_index],
      &stage_deps[render_index],
      &stage_jobs[bin_index],
      &stage_jobs[render_index],
      &fmem->br_info,
      bin_completion, fmem->persist,
      render_completion, fmem->persist);
}

#if KHRN_DEBUG && KHRN_GLES31_DRIVER
static void memaccess_add_buffer(void* memaccess, gmem_handle_t buf, v3d_barrier_flags rw_flags)
{
   khrn_memaccess_add_buffer((khrn_memaccess*)memaccess, buf, V3D_BARRIER_NO_ACCESS, rw_flags);
}
#endif

static void khrn_fmem_preprocess_job(void* data)
{
   khrn_fmem_persist* persist = (khrn_fmem_persist*)data;

   // Sync all buffers needed for preprocess.
   khrn_fmem_buffer_range* buffers = persist->preprocess_buffers.data;
   for (unsigned b = 0; b != persist->preprocess_buffers.size; ++b)
   {
      gmem_invalidate_mapped_range(
         buffers[b].handle,
         buffers[b].start,
         buffers[b].end - buffers[b].start);
   }
   free(persist->preprocess_buffers.data);
   persist->preprocess_buffers.data = NULL;

   // Handle preprocess uniform loads.
   for (unsigned b = 0; b != persist->preprocess_ubo_loads.size; ++b)
   {
      glxx_hw_process_ubo_load_batch(khrn_vector_data(glxx_hw_ubo_load_batch, &persist->preprocess_ubo_loads) + b, true);
   }
   free(persist->preprocess_ubo_loads.data);
   persist->preprocess_ubo_loads.data = NULL;
   persist->preprocess_ubo_loads.size = 0;

   // Handle compute dispatches.
 #if KHRN_GLES31_DRIVER
   if (persist->compute_dispatches.size != 0)
   {
      compute_job_mem* compute_mem = glxx_compute_process_dispatches(
         persist->compute_dispatches.data,
         persist->compute_dispatches.size);

    #if KHRN_DEBUG
      if (compute_mem)
      {
         if (persist->memaccess)
            compute_job_mem_enumerate_accesses(compute_mem, memaccess_add_buffer, persist->memaccess);

         if (!khrn_options.no_gmp)
            compute_job_mem_patch_gmp_table(compute_mem, persist->gmp_tables + V3D_GMP_TABLE_SIZE*1);
      }
    #endif

      persist->compute_job_mem = compute_mem;
   }
   free(persist->compute_dispatches.data);
   persist->compute_dispatches.data = NULL;
 #endif

   // Now perform gmem_sync_cpu_post_write for all the fmem blocks.
   khrn_fmem_pool_post_cpu_write(&persist->pool);
}

void khrn_fmem_flush(khrn_fmem *fmem)
{
   assert(!fmem->in_begin_clist);
   assert(!fmem->in_begin_cle_start);
   assert(!fmem->in_begin_data_start);

   // Finalise fmem blocks, so fmem_pool knows how much data was written.
   finalise_block(&fmem->persist->pool, &fmem->cle);
   finalise_block(&fmem->persist->pool, &fmem->data);

   /* We need to do this *before* we submit, because the submit API reports
    * completion too. This also needs to happen before the call to
    * khrn_memaccess_build_gmp_tables. */
   v3d_barrier_flags bin_rw_flags, render_rw_flags;
   khrn_fmem_pool_submit(
      &fmem->persist->pool,
 #if KHRN_DEBUG
      fmem->persist->memaccess,
 #endif
      &bin_rw_flags,
      &render_rw_flags);
   fmem->bin_rw_flags |= bin_rw_flags;
   fmem->render_rw_flags |= render_rw_flags;

 #if KHRN_DEBUG
   if (!khrn_options.no_gmp)
   {
      khrn_memaccess_build_gmp_tables(fmem->persist->memaccess, fmem->persist->gmp_tables);
      fmem->br_info.details.bin_gmp_table = fmem->persist->gmp_tables + V3D_GMP_TABLE_SIZE*0;
      fmem->br_info.details.render_gmp_table = fmem->persist->gmp_tables + V3D_GMP_TABLE_SIZE*1;
      persist_release_memaccess(fmem->persist);
   }
 #endif

   v3d_scheduler_deps stage_deps[KHRN_RESOURCE_NUM_STAGES];
   for (unsigned i = 0; i != countof(stage_deps); ++i)
      v3d_scheduler_deps_init(&stage_deps[i]);

   get_deps_from_and_release_fences(&stage_deps[0], fmem);
   get_deps_from_resources(stage_deps, fmem);

 #if KHRN_GLES31_DRIVER
   bool has_compute = fmem->persist->compute_dispatches.size;
 #else
   bool has_compute = false;
 #endif

   job_t stage_jobs[KHRN_RESOURCE_NUM_STAGES] = { 0, };
   if (fmem->persist->preprocess_ubo_loads.size || has_compute)
   {
      // Preprocess better be the first stage.
      static_assrt(KHRN_STAGE_PREPROCESS == 1);

      // Limited the number of preprocessed, but not executed compute jobs.
      v3d_scheduler_deps finalised_deps;
      v3d_scheduler_deps_init(&finalised_deps);
    #if KHRN_GLES31_DRIVER
      if (has_compute && khrn_fmem_compute_jobs[0] != 0)
         v3d_scheduler_add_dep(&finalised_deps, khrn_fmem_compute_jobs[0]);
    #endif

      // Issue job to complete uniform-streams, process compute dispatches
      // and flush CPU cache.
      // Note that this triggers potentially concurrent use of fmem->persist by
      // the remainder of khrn_fmem_flush and khrn_fmem_preprocess_job.
      // Specifically any further attempt to use fmem->persist->memaccess must
      // wait for preprocess.
      stage_jobs[0] = v3d_scheduler_submit_usermode_job2(
         &stage_deps[0],
         &finalised_deps,
         khrn_fmem_preprocess_job,
         fmem->persist);

      // Following stages are dependent on preprocess.
      v3d_scheduler_add_dep(&stage_deps[1], stage_jobs[0]);
   }
   else
   {
      // Flush CPU cache now if we don't have any uniform-streams to complete later.
      khrn_fmem_pool_post_cpu_write(&fmem->persist->pool);
   }

   // Stage dependencies are transitive.
   for (unsigned s = 1; s != countof(stage_deps); ++s)
      v3d_scheduler_merge_deps(&stage_deps[s], &stage_deps[s-1]);

   submit_bin_render(fmem, stage_deps, stage_jobs);

   // Find job for last active stage.
   uint64_t last_job = stage_jobs[0];
   for (unsigned s = 1; s != countof(stage_jobs); ++s)
   {
      if (stage_jobs[s])
         last_job = stage_jobs[s];
   }

 #if KHRN_GLES31_DRIVER
   // Store compute jobs for throttling purposes.
   if (has_compute)
   {
      for (unsigned i = 1; i != countof(khrn_fmem_compute_jobs); ++i)
         khrn_fmem_compute_jobs[i-1] = khrn_fmem_compute_jobs[i];
      khrn_fmem_compute_jobs[countof(khrn_fmem_compute_jobs)-1] = last_job;
   }
 #endif

   update_resources(fmem, stage_jobs);
   update_and_release_fences(fmem, last_job);

#if KHRN_DEBUG
   if (khrn_options.save_crc_enabled)
   {
      // Wait for preprocess before using memaccess.
      v3d_scheduler_wait_jobs(&stage_deps[KHRN_STAGE_PREPROCESS], V3D_SCHED_DEPS_COMPLETED);
      khrn_save_crc_checksums(fmem->persist->memaccess, &fmem->br_info);
      persist_destroy(fmem->persist);
   }
#endif

   // After memaccess is destroyed, it is safe to release references to gmem
   // handles and resources used by this render-state.
   release_resources(fmem);

   // increment frame counter (skip over ~0 "invalid")
   if (++khrn_fmem_frame_i == ~0u)
      khrn_fmem_frame_i = 0;
}

static bool alloc_next(khrn_fmem_pool *pool, khrn_fmem_block *block,
      khrn_render_state *rs)
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

static void block_init(khrn_fmem_block *block)
{
   block->start = NULL;
   block->end = 0;
   block->current = block->end;
}

bool khrn_fmem_is_here(khrn_fmem *fmem, uint8_t *p)
{
   return fmem->cle.start + fmem->cle.current == p;
}

v3d_addr_t khrn_fmem_hw_address(khrn_fmem *fmem, const void *p)
{
   return khrn_fmem_pool_hw_address(&fmem->persist->pool, p);
}

bool khrn_fmem_record_fence_to_signal(khrn_fmem *fmem,
      khrn_fence *fence)
{
   bool already_user = false;
   khrn_render_state *rs = fmem->render_state;
   khrn_uintptr_vector *fences;

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

bool khrn_fmem_record_fence_to_depend_on(khrn_fmem *fmem,
      khrn_fence *fence)
{
   khrn_uintptr_vector *fences;

   /* the fence should have been flushed before getting to this point
    * ( before choosing this fmem/render state) */
   assert(!khrn_fence_has_user(fence, NULL));

   fences = &fmem->fences_to_depend_on;
   if (!khrn_uintptr_vector_push_back(fences, (uintptr_t)fence))
      return false;

   khrn_fence_refinc(fence);
   return true;
}

static bool add_resource(khrn_fmem *fmem, khrn_resource *res)
{
   if (!khrn_resource_get_stages(res, fmem->render_state))
   {
   #if KHRN_DEBUG
      if (fmem->persist->memaccess)
         khrn_memaccess_register_resource(fmem->persist->memaccess, res);
   #endif

      if (!khrn_uintptr_vector_push_back(&fmem->resources, (uintptr_t)res))
         return false;
      khrn_resource_refinc(res);
   }
   return true;
}

bool khrn_fmem_record_resource_read(khrn_fmem *fmem,
      khrn_resource *res, khrn_stages_t stages)
{
   if (!add_resource(fmem, res))
      return false;
   khrn_resource_add_reader(res, stages, fmem->render_state);
   return true;
}

bool khrn_fmem_record_resource_write(khrn_fmem *fmem,
      khrn_resource *res, khrn_stages_t stages,
      khrn_resource_parts_t parts, bool *invalid_out)
{
   if (!add_resource(fmem, res))
      return false;
   bool invalid = khrn_resource_add_writer(res, stages, fmem->render_state, parts);
   if (invalid_out)
      *invalid_out = invalid;
   return true;
}

bool khrn_fmem_record_resource_self_read_conflicting_write(khrn_fmem *fmem,
      khrn_resource *res, khrn_stages_t stages,
      khrn_resource_parts_t parts, bool* requires_flush)
{
   if (!add_resource(fmem, res))
   {
      *requires_flush = false;
      return false;
   }
   // On failure could pop res if added, but leaving it is harmless.
   bool success = khrn_resource_add_self_read_conflicting_writer(res, stages, fmem->render_state, parts);
   *requires_flush = !success;
   return success;
}

bool khrn_fmem_record_preprocess_resource_read(khrn_fmem *fmem,
      khrn_resource* res, v3d_size_t offset, v3d_size_t length)
{
   if (!khrn_fmem_record_resource_read(fmem, res, KHRN_STAGE_PREPROCESS))
      return false;

   khrn_fmem_buffer_range* buf = fmem->last_preprocess_buffer;
   if (buf != NULL)
   {
      // Check last used first.
      if (buf->handle != res->handle)
      {
         // Reverse linear search.
         khrn_fmem_buffer_range* buffers = khrn_vector_data(khrn_fmem_buffer_range, &fmem->persist->preprocess_buffers);
         unsigned b = fmem->persist->preprocess_buffers.size - 1;
         for (; buffers[b].handle != res->handle; --b)
         {
            if (!b)
               goto no_match;
         }
         buf = &buffers[b];
         fmem->last_preprocess_buffer = buf;
      }

      // Expand existing sync-range.
      buf->start = gfx_umin(buf->start, offset);
      buf->end = gfx_umax(buf->end, offset + length);
      return true;
   }

no_match:
   // Create a new entry.
   buf = khrn_vector_emplace_back(khrn_fmem_buffer_range, &fmem->persist->preprocess_buffers);
   if (!buf)
      return false;
   buf->handle = res->handle;
   buf->start = offset;
   buf->end = offset + length;
   fmem->last_preprocess_buffer = buf;
   return true;
}

void* khrn_fmem_resource_read_now_or_in_preprocess(khrn_fmem *fmem,
   khrn_resource* res, v3d_size_t offset, v3d_size_t length, bool* read_now)
{
   void* ptr = khrn_resource_try_read_now(res, offset, length, read_now);
   if (!ptr)
      return NULL;

   if (!*read_now && !khrn_fmem_record_preprocess_resource_read(fmem, res, offset, length))
      return NULL;

   return ptr;
}

#if !V3D_HAS_QTS
/* Allocate memory for the tile state */
v3d_addr_t khrn_fmem_tile_state_alloc(khrn_fmem *fmem, size_t size)
{
   khrn_fmem_persist* persist = fmem->persist;

   gmem_handle_t handle;
   if (khrn_get_num_cores() == 1)
   {
      // Store shared object to be released on bin completion.
      persist->bin_shared_tile_state = khrn_tile_state_alloc_shared(size, fmem->br_info.details.secure);
      if (!persist->bin_shared_tile_state)
         return 0;
      handle = persist->bin_shared_tile_state->handle;
   }
   else
   {
      handle = khrn_tile_state_alloc_gmem(size, fmem->br_info.details.secure);
      if (!handle)
         return 0;

      // Store handle to be freed on bin completion.
      assert(persist->num_bin_tile_states < countof(persist->bin_tile_state));
      persist->bin_tile_state[persist->num_bin_tile_states++] = handle;
   }

   return khrn_fmem_sync_and_get_addr(fmem, handle, V3D_BARRIER_PTB_TILESTATE_WRITE, 0);
}
#endif

void khrn_fmem_sync(khrn_fmem *fmem, gmem_handle_t handle,
      v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags)
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
   if (fmem->persist->memaccess)
      khrn_memaccess_add_buffer(fmem->persist->memaccess, handle, bin_rw_flags, render_rw_flags);
#endif
}

bool khrn_fmem_sync_res(khrn_fmem *fmem,
   khrn_resource *res, v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags)
{
   khrn_stages_t stages =
      (bin_rw_flags ? KHRN_STAGE_BIN : 0) |
      (render_rw_flags ? KHRN_STAGE_RENDER : 0);

   if ((bin_rw_flags | render_rw_flags) & V3D_BARRIER_V3D_WRITE)
   {
      if (!khrn_fmem_record_resource_write(fmem, res, stages, KHRN_RESOURCE_PARTS_ALL, NULL))
         return false;
   }
   else
   {
      if (!khrn_fmem_record_resource_read(fmem, res, stages))
         return false;
   }

   khrn_fmem_sync(fmem, res->handle, bin_rw_flags, render_rw_flags);
   return true;
}

bool khrn_fmem_cle_barrier_flush(khrn_fmem *fmem,
   v3d_barrier_flags src, v3d_barrier_flags dst,
   v3d_cache_ops *done_ops_from_src)
{
   v3d_cache_ops ops = v3d_barrier_cache_flushes_within_core(src, dst);
   assert(!(v3d_barrier_cache_cleans_within_core(src, dst) &
      ~(done_ops_from_src ? *done_ops_from_src : 0)));
   if (done_ops_from_src)
   {
      ops &= ~*done_ops_from_src;
      *done_ops_from_src |= ops;
   }

   if (!ops)
      return true;

   uint8_t *instr = khrn_fmem_begin_cle(fmem,
      V3D_CL_FLUSH_L2T_SIZE +
#if !V3D_VER_AT_LEAST(3,3,0,0)
      V3D_CL_CLEAR_L2C_SIZE +
#endif
      V3D_CL_CLEAR_VCD_CACHE_SIZE +
      V3D_CL_CLEAR_SLICE_CACHES_SIZE);
   if (!instr)
      return false;

   if (ops & V3D_CACHE_FLUSH_L2T)
   {
      v3d_cl_flush_l2t(&instr, 0, ~0, V3D_L2T_FLUSH_MODE_FLUSH, /*deferred=*/false);
      ops &= ~V3D_CACHE_FLUSH_L2T;
   }

#if !V3D_VER_AT_LEAST(3,3,0,0)
   if (ops & V3D_CACHE_CLEAR_L2C)
   {
      v3d_cl_clear_l2c(&instr);
      ops &= ~V3D_CACHE_CLEAR_L2C;
   }
#endif

   if (ops & V3D_CACHE_CLEAR_VCD)
   {
      v3d_cl_clear_vcd_cache(&instr);
      ops &= ~V3D_CACHE_CLEAR_VCD;
   }

   if (ops & (V3D_CACHE_CLEAR_SIC | V3D_CACHE_CLEAR_SUC | V3D_CACHE_CLEAR_L1TD | V3D_CACHE_CLEAR_L1TC))
   {
      v3d_cl_clear_slice_caches(&instr,
         (ops & V3D_CACHE_CLEAR_SIC) ? 0xf : 0,
         (ops & V3D_CACHE_CLEAR_SUC) ? 0xf : 0,
         (ops & V3D_CACHE_CLEAR_L1TD) ? 0xf : 0,
         (ops & V3D_CACHE_CLEAR_L1TC) ? 0xf : 0);
      ops &= ~(V3D_CACHE_CLEAR_SIC | V3D_CACHE_CLEAR_SUC | V3D_CACHE_CLEAR_L1TD | V3D_CACHE_CLEAR_L1TC);
   }

   assert(!ops);

   khrn_fmem_end_cle(fmem, instr);
   return true;
}
