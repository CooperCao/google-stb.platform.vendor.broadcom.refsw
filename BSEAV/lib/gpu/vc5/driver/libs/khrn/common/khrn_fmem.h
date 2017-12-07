/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "khrn_fmem_pool.h"
#include "khrn_fmem_tmu_config.h"
#include "khrn_fmem_debug_info.h"
#include "khrn_tile_state.h"
#include "khrn_uintptr_vector.h"
#include "khrn_vector.h"
#include "khrn_resource.h"
#include "khrn_fence.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_barrier.h"
#include "libs/platform/v3d_scheduler.h"
#include "libs/util/common.h"

#if KHRN_DEBUG
#include "khrn_memaccess.h"
#endif

EXTERN_C_BEGIN

#define KHRN_FMEM_RESERVED_DATA_BUFFER_SIZE  V3D_MAX_QPU_UNIFS_READAHEAD
#define KHRN_FMEM_RESERVED_CLE_BUFFER_SIZE   (V3D_MAX_CLE_READAHEAD + V3D_CL_BRANCH_SIZE)
#define KHRN_FMEM_USABLE_DATA_BUFFER_SIZE    (KHRN_FMEM_BUFFER_SIZE - KHRN_FMEM_RESERVED_DATA_BUFFER_SIZE)
#define KHRN_FMEM_USABLE_CLE_BUFFER_SIZE     (KHRN_FMEM_BUFFER_SIZE - KHRN_FMEM_RESERVED_CLE_BUFFER_SIZE)

void khrn_fmem_static_init(void);

typedef uint64_t job_t;

typedef struct khrn_fmem_block
{
   khrn_fmem_buffer* buffer;
   uint8_t*   base_ptr;
   v3d_addr_t base_addr;
   uint32_t   current;
   uint32_t   end;
} khrn_fmem_block;

typedef struct khrn_fmem_buffer_range khrn_fmem_buffer_range;
typedef struct glxx_query_block glxx_query_block;
typedef struct compute_job_mem compute_job_mem;

/* Precious things of the FMEM that need to live longer than the render state
   reside in khrn_fmem_persist, which is cleaned up when the
   completion callbacks for bin/render jobs have bin signalled */
typedef struct khrn_fmem_persist
{
   khrn_fmem_pool pool;                 // memory pool for control lists
   /* Debug info */
#ifdef KHRN_GEOMD
   struct fmem_debug_info_vector debug_info;
#endif
#if KHRN_DEBUG
   uint8_t* gmp_tables;
   khrn_memaccess* memaccess;
   unsigned memaccess_refs;
#endif

   khrn_uintptr_vector client_handles;  // client-allocated handles

   glxx_query_block *occlusion_query_list;
#if V3D_VER_AT_LEAST(4,0,2,0)
   glxx_query_block *prim_counts_query_list;
#endif

#if !V3D_VER_AT_LEAST(4,1,34,0)
   gmem_handle_t bin_tile_state[V3D_MAX_CORES];
   unsigned num_bin_tile_states;
   khrn_shared_tile_state* bin_shared_tile_state;
#endif

   khrn_vector preprocess_buffers;     // of khrn_fmem_buffer_range
   khrn_vector preprocess_ubo_loads;   // of glxx_hw_ubo_load_batch
#if V3D_VER_AT_LEAST(3,3,0,0)
   khrn_vector compute_dispatches;     // of glxx_compute_dispatch
 #if V3D_USE_CSD
   khrn_vector compute_indirect;       // of glxx_compute_indirect
   v3d_compute_subjobs_id compute_subjobs_id;
 #else
   compute_job_mem* compute_job_mem;
 #endif
#endif

   bool *gpu_aborted;                  // This is shared with GLXX_SHARED_T to be able to update
                                       // its value from the bin/render completion callback function.

} khrn_fmem_persist;

typedef struct khrn_fmem
{
   khrn_fmem_block cle;
   khrn_fmem_block data;
   struct khrn_fmem_tmu_cfg_alloc tmu_cfg_alloc;
   khrn_fmem_persist* persist;
   khrn_fmem_buffer_range* last_preprocess_buffer;

   V3D_BIN_RENDER_INFO_T br_info;
   v3d_barrier_flags bin_rw_flags;
   v3d_barrier_flags render_rw_flags;

   khrn_render_state *render_state;

   khrn_uintptr_vector resources;
   khrn_uintptr_vector fences_to_signal;
   khrn_uintptr_vector fences_to_depend_on;

#ifndef NDEBUG
   bool in_begin_clist;
   bool cle_closed;
   uint8_t *in_begin_cle_start;
   uint8_t *in_begin_cle_end;
   uint8_t *in_begin_data_start;
   uint8_t *in_begin_data_end;
#endif
} khrn_fmem;

extern uint32_t khrn_fmem_frame_i;

bool khrn_fmem_init(khrn_fmem *fmem, khrn_render_state *render_state);
void khrn_fmem_discard(khrn_fmem *fmem);

uint8_t *khrn_fmem_cle_new_block(khrn_fmem *fmem, unsigned size);
uint32_t *khrn_fmem_data_new_block(v3d_addr_t* ret_addr, khrn_fmem *fmem, unsigned size, unsigned align);

static inline bool khrn_fmem_should_flush(khrn_fmem *fmem)
{
   return khrn_fmem_pool_should_flush(&fmem->persist->pool);
}

// required to call begin_clist before any calls to khrn_fmem_*_cle
v3d_addr_t khrn_fmem_begin_clist(khrn_fmem *fmem);
v3d_addr_t khrn_fmem_end_clist(khrn_fmem *fmem);


static inline uint8_t *khrn_fmem_cle(khrn_fmem *fmem, unsigned size)
{
   assert(fmem->in_begin_clist);
   assert(!fmem->in_begin_cle_start);
   assert(!fmem->cle_closed);
   assert(size <= KHRN_FMEM_USABLE_CLE_BUFFER_SIZE);

   uint32_t end = fmem->cle.current + size;
   if (end > fmem->cle.end)
      return khrn_fmem_cle_new_block(fmem, size);

   uint8_t* ret = fmem->cle.base_ptr + fmem->cle.current;
   fmem->cle.current = end;
   return ret;
}

static inline uint8_t *khrn_fmem_cle_final(khrn_fmem *fmem, uint32_t size)
{
   // We can make one guaranteed allocation up to V3D_CL_BRANCH_SIZE without allocating a new block.
   assert(size <= V3D_CL_BRANCH_SIZE);
   assert(fmem->in_begin_clist);
   assert(!fmem->in_begin_cle_start);
   assert(!fmem->cle_closed);
   debug_only(fmem->cle_closed = true);
   uint8_t* ret = fmem->cle.base_ptr + fmem->cle.current;
   fmem->cle.current += size;
   if (fmem->cle.current > fmem->cle.end)
      fmem->cle.end = fmem->cle.current;
   return ret;
}

//! Returns the current control list address.
static inline v3d_addr_t khrn_fmem_cle_addr(khrn_fmem* fmem)
{
   assert(fmem->cle.base_addr != 0);
   return fmem->cle.base_addr + fmem->cle.current;
}

//! Return number of bytes left in this fmem buffer.
static inline unsigned khrn_fmem_cle_buffer_remaining(khrn_fmem *fmem)
{
   return fmem->cle.end - fmem->cle.current;
}

//! Begin writing to the cle specifying the maximum number of bytes to write.
static inline uint8_t *khrn_fmem_begin_cle(khrn_fmem *fmem, unsigned max_size)
{
   // khrn_fmem_begin_cle must be closed with khrn_fmem_end_cle
   assert(!fmem->in_begin_cle_start);

   uint8_t *ptr = khrn_fmem_cle(fmem, max_size);
#ifndef NDEBUG
   fmem->in_begin_cle_start = ptr;
   fmem->in_begin_cle_end = fmem->cle.base_ptr + fmem->cle.current;
#endif
   return ptr;
}

//! Close a previous call to khrn_fmem_begin_cle.
static inline void khrn_fmem_end_cle(khrn_fmem *fmem, uint8_t *end)
{
   // khrn_fmem_end_cle must be preceeded by successful khrn_fmem_begin_cle
   // and end pointer must be within the allocation
   assert(end >= fmem->in_begin_cle_start && end <= fmem->in_begin_cle_end);
   assert((fmem->cle.base_ptr + fmem->cle.current) == fmem->in_begin_cle_end);

   fmem->cle.current = end - fmem->cle.base_ptr;
#ifndef NDEBUG
   fmem->in_begin_cle_start = NULL;
   fmem->in_begin_cle_end = NULL;
#endif
}

//! Close a previous call to khrn_fmem_begin_cle exactly.
static inline void khrn_fmem_end_cle_exact(khrn_fmem *fmem, uint8_t *end)
{
   // khrn_fmem_end_cle must be preceeded by successful khrn_fmem_begin_cle
   // and end pointer must match the number of bytes allocated
   assert(end == fmem->in_begin_cle_end);
   assert((fmem->cle.base_ptr + fmem->cle.current) == fmem->in_begin_cle_end);

#ifndef NDEBUG
   fmem->in_begin_cle_start = NULL;
   fmem->in_begin_cle_end = NULL;
#endif
}

static inline uint32_t *khrn_fmem_data(v3d_addr_t* ret_addr, khrn_fmem *fmem, unsigned size, unsigned align)
{
   assert(size <= KHRN_FMEM_USABLE_DATA_BUFFER_SIZE);
   assert(align <= KHRN_FMEM_ALIGN_MAX && gfx_is_power_of_2(align));
   assert(!fmem->in_begin_data_start);

   uint32_t start = (fmem->data.current + (align - 1)) & ~(align - 1);
   uint32_t end = start + size;
   if (end > fmem->data.end)
      return khrn_fmem_data_new_block(ret_addr, fmem, size, align);

   fmem->data.current = end;
   *ret_addr = fmem->data.base_addr + start;
   return (uint32_t*)(fmem->data.base_ptr + start);
}

//! Begin writing to the data specifying the maximum number of bytes to write.
static inline void *khrn_fmem_begin_data(v3d_addr_t* ret_addr, khrn_fmem *fmem, unsigned max_size, unsigned align)
{
   // khrn_fmem_begin_data must be closed with khrn_fmem_end_data
   assert(!fmem->in_begin_data_start);

   void *ptr = khrn_fmem_data(ret_addr, fmem, max_size, align);
#ifndef NDEBUG
   fmem->in_begin_data_start = (uint8_t*)ptr;
   fmem->in_begin_data_end = fmem->data.base_ptr + fmem->data.current;
#endif
   return ptr;
}

//! Close a previous call to khrn_fmem_begin_data.
static inline void khrn_fmem_end_data(khrn_fmem *fmem, void *end)
{
   // khrn_fmem_end_data must be preceeded by successful khrn_fmem_begin_data
   // and end pointer must be within the allocation
   assert((uint8_t*)end >= fmem->in_begin_data_start && (uint8_t*)end <= fmem->in_begin_data_end);
   assert((fmem->data.base_ptr + fmem->data.current) == fmem->in_begin_data_end);

   fmem->data.current = (uint8_t*)end - fmem->data.base_ptr;
#ifndef NDEBUG
   fmem->in_begin_data_start = NULL;
   fmem->in_begin_data_end = NULL;
#endif
}

//! Close a previous call to khrn_fmem_begin_data exactly.
static inline void khrn_fmem_end_data_exact(khrn_fmem *fmem, void *end)
{
   // khrn_fmem_end_data must be preceeded by successful khrn_fmem_begin_data
   // and end pointer must match the number of bytes allocated
   assert(end == fmem->in_begin_data_end);
   assert((fmem->data.base_ptr + fmem->data.current) == fmem->in_begin_data_end);

#ifndef NDEBUG
   fmem->in_begin_data_start = NULL;
   fmem->in_begin_data_end = NULL;
#endif
}

static inline void khrn_fmem_sync(khrn_fmem *fmem, gmem_handle_t handle,
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

static inline v3d_addr_t khrn_fmem_sync_and_get_addr(khrn_fmem *fmem,
      gmem_handle_t handle, v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags)
{
   khrn_fmem_sync(fmem, handle, bin_rw_flags, render_rw_flags);
   return gmem_get_addr(handle);
}

/* khrn_fmem_record_resource_read/write + khrn_fmem_sync */
bool khrn_fmem_sync_res(khrn_fmem *fmem,
   khrn_resource *res, v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags);

#if !V3D_VER_AT_LEAST(4,1,34,0)
v3d_addr_t khrn_fmem_tile_state_alloc(khrn_fmem *fmem, size_t size);
#endif

/* Flush out any binning and/or rendering */
void khrn_fmem_flush(khrn_fmem *fmem);

/*
 * mark the fact that fmem is using this resource for read or write during
 * binning/rendering/etc;
 */
bool khrn_fmem_record_resource_read(khrn_fmem *fmem,
      khrn_resource *res, khrn_stages_t stages);
bool khrn_fmem_record_resource_write(khrn_fmem *fmem,
      khrn_resource *res, khrn_stages_t stages,
      khrn_resource_parts_t parts, bool *invalid);

//! Record a host read to happen in preprocess. The buffer must have already been
// mapped, and the range will be automatically synced at the beginning of preprocess.
bool khrn_fmem_record_preprocess_resource_read(khrn_fmem *fmem,
      khrn_resource* res, v3d_size_t offset, v3d_size_t length);

//! Read a resource now if it wouldn't stall, otherwise record a read in
//! preprocess. Returns a pointer to the host address at offset, or NULL on failure.
void* khrn_fmem_resource_read_now_or_in_preprocess(khrn_fmem *fmem,
   khrn_resource* res, v3d_size_t offset, v3d_size_t length, bool* read_now);

/*
 * Returns false if we're already reading the resource (*requires_flush will
 * be true), or if we run out of memory (*requires_flush will be false).
 */
bool khrn_fmem_record_resource_self_read_conflicting_write(khrn_fmem *fmem,
      khrn_resource *res, khrn_stages_t stages,
      khrn_resource_parts_t parts, bool* requires_flush);

bool khrn_fmem_record_handle(khrn_fmem *fmem, gmem_handle_t handle);

bool khrn_fmem_record_fence_to_signal(khrn_fmem *fmem,
      khrn_fence *fence);
bool khrn_fmem_record_fence_to_depend_on(khrn_fmem *fmem,
      khrn_fence *fence);

static inline bool khrn_fmem_has_queries(khrn_fmem *fmem)
{
   bool res =  fmem->persist->occlusion_query_list != NULL;
#if V3D_VER_AT_LEAST(4,0,2,0)
   res = res || (fmem->persist->prim_counts_query_list != NULL);
#endif
   return res;
}

/* Note:
 * - This only inserts a memory barrier. It does not insert any waits.
 * - The inserted barrier is only between prior ops in the current control list
 *   and later ops in the current control list. If the current control list is
 *   a bin control list, "later ops" does *not* include ops that happen during
 *   render.
 * - Only cache flushes are supported; if cache cleans are required this
 *   function will assert! */
bool khrn_fmem_cle_barrier_flush(khrn_fmem *fmem,
   v3d_barrier_flags src, v3d_barrier_flags dst,
   /* Cache ops already done for src accesses. May be NULL. */
   v3d_cache_ops *done_ops_from_src);

EXTERN_C_END
