/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_FMEM_4_H
#define KHRN_FMEM_4_H

#include "khrn_fmem_pool.h"
#include "khrn_tile_state.h"
#include "khrn_fmem_debug_info.h"
#include "khrn_uintptr_vector.h"
#include "khrn_vector.h"
#include "khrn_resource.h"
#include "khrn_fence.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/core/v3d/v3d_barrier.h"
#include "libs/platform/v3d_scheduler.h"
#include "vcos.h"

#if KHRN_DEBUG
#include "khrn_memaccess.h"
#endif

VCOS_EXTERN_C_BEGIN

typedef uint64_t job_t;

typedef struct khrn_fmem_block
{
   uint8_t *start;
   size_t end;
   size_t current;
} khrn_fmem_block;

typedef struct khrn_fmem_buffer_range khrn_fmem_buffer_range;
typedef struct glxx_query_block glxx_query_block;

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
   void* gmp_tables[2];
   bool no_destroy_after_render;
#endif

   khrn_uintptr_vector client_handles;  // client-allocated handles

   glxx_query_block *occlusion_query_list;
#if V3D_VER_AT_LEAST(4,0,2,0)
   glxx_query_block *prim_counts_query_list;
#endif

#if !V3D_HAS_QTS
   gmem_handle_t bin_tile_state[V3D_MAX_CORES];
   unsigned num_bin_tile_states;
   khrn_shared_tile_state* bin_shared_tile_state;
#endif

   khrn_vector preprocess_buffers;     // of khrn_fmem_buffer_range
   khrn_vector preprocess_ubo_loads;   // of glxx_hw_ubo_load_batch

} khrn_fmem_persist;

struct khrn_fmem_tmu_cfg_alloc
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   void *spare_8; /* 8-byte aligned, 8-byte size. Valid if not NULL. */
   void *spare_16; /* 16-byte aligned, 16-byte size. Valid if not NULL. */
   void *spare_32; /* 32-byte aligned */
   unsigned num_32; /* Number of 32-byte blocks at spare_32 */
#else
   uint8_t* spare;
   unsigned num_spare;
#endif
};

typedef struct khrn_fmem
{
   khrn_fmem_block cle;
   khrn_fmem_block data;
   uint8_t* cle_first;
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

#if KHRN_DEBUG
   khrn_memaccess* memaccess;
#endif
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

extern bool khrn_fmem_init(khrn_fmem *fmem, khrn_render_state *render_state);
extern bool khrn_fmem_reset(khrn_fmem *fmem, khrn_render_state *render_state);
extern void khrn_fmem_discard(khrn_fmem *fmem);
extern uint32_t *khrn_fmem_data(khrn_fmem *fmem, unsigned size, unsigned align);
extern uint8_t *khrn_fmem_cle(khrn_fmem *fmem, unsigned size);

static inline uint8_t *khrn_fmem_cle_final(khrn_fmem *fmem, unsigned size)
{
   // We can make one guaranteed allocation up to V3D_CL_BRANCH_SIZE without allocating a new block.
   assert(size <= V3D_CL_BRANCH_SIZE);
   assert(!fmem->cle_closed);
   debug_only(fmem->cle_closed = true);
   uint8_t* result = fmem->cle.start + fmem->cle.current;
   fmem->cle.current += size;
   if (fmem->cle.current > fmem->cle.end)
      fmem->cle.end = fmem->cle.current;
   return result;
}

static inline bool khrn_fmem_should_flush(khrn_fmem *fmem)
{
   return khrn_fmem_pool_should_flush(&fmem->persist->pool);
}

// required to call begin_clist before any calls to khrn_fmem_*_cle
// begin_clist is called automatically in khrn_fmem_init
extern v3d_addr_t khrn_fmem_begin_clist(khrn_fmem *fmem);
extern v3d_addr_t khrn_fmem_end_clist(khrn_fmem *fmem);

#if V3D_VER_AT_LEAST(4,0,2,0)
extern v3d_addr_t khrn_fmem_add_tmu_tex_state(khrn_fmem *fmem,
   const void *tex_state, bool extended);
extern v3d_addr_t khrn_fmem_add_tmu_sampler(khrn_fmem *fmem,
   const void *sampler, bool extended);
#else
extern v3d_addr_t khrn_fmem_add_tmu_indirect(khrn_fmem *fmem, uint32_t const *tmu_indirect);
#endif

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
   fmem->in_begin_cle_end = fmem->cle.start + fmem->cle.current;
#endif
   return ptr;
}

//! Close a previous call to khrn_fmem_begin_cle.
static inline void khrn_fmem_end_cle(khrn_fmem *fmem, uint8_t *end)
{
   // khrn_fmem_end_cle must be preceeded by successful khrn_fmem_begin_cle
   // and end pointer must be within the allocation
   assert(end >= fmem->in_begin_cle_start && end <= fmem->in_begin_cle_end);
   assert((fmem->cle.start + fmem->cle.current) == fmem->in_begin_cle_end);

   fmem->cle.current = end - fmem->cle.start;
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
   assert((fmem->cle.start + fmem->cle.current) == fmem->in_begin_cle_end);

#ifndef NDEBUG
   fmem->in_begin_cle_start = NULL;
   fmem->in_begin_cle_end = NULL;
#endif
}

//! Begin writing to the data specifying the maximum number of bytes to write.
static inline void *khrn_fmem_begin_data(khrn_fmem *fmem, unsigned max_size, unsigned align)
{
   // khrn_fmem_begin_data must be closed with khrn_fmem_end_data
   assert(!fmem->in_begin_data_start);

   void *ptr = khrn_fmem_data(fmem, max_size, align);
#ifndef NDEBUG
   fmem->in_begin_data_start = (uint8_t*)ptr;
   fmem->in_begin_data_end = fmem->data.start + fmem->data.current;
#endif
   return ptr;
}

//! Close a previous call to khrn_fmem_begin_data.
static inline void khrn_fmem_end_data(khrn_fmem *fmem, void *end)
{
   // khrn_fmem_end_data must be preceeded by successful khrn_fmem_begin_data
   // and end pointer must be within the allocation
   assert((uint8_t*)end >= fmem->in_begin_data_start && (uint8_t*)end <= fmem->in_begin_data_end);
   assert((fmem->data.start + fmem->data.current) == fmem->in_begin_data_end);

   fmem->data.current = (uint8_t*)end - fmem->data.start;
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
   assert((fmem->data.start + fmem->data.current) == fmem->in_begin_data_end);

#ifndef NDEBUG
   fmem->in_begin_data_start = NULL;
   fmem->in_begin_data_end = NULL;
#endif
}

extern void khrn_fmem_sync(khrn_fmem *fmem,
      gmem_handle_t handle, v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags);

static inline v3d_addr_t khrn_fmem_sync_and_get_addr(khrn_fmem *fmem,
      gmem_handle_t handle, v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags)
{
   khrn_fmem_sync(fmem, handle, bin_rw_flags, render_rw_flags);
   return gmem_get_addr(handle);
}

/* khrn_fmem_record_resource_read/write + khrn_fmem_sync */
extern bool khrn_fmem_sync_res(khrn_fmem *fmem,
   khrn_resource *res, v3d_barrier_flags bin_rw_flags, v3d_barrier_flags render_rw_flags);

#if !V3D_HAS_QTS
extern v3d_addr_t khrn_fmem_tile_state_alloc(khrn_fmem *fmem, size_t size);
#endif

/* Flush out any binning and/or rendering */
extern void khrn_fmem_flush(khrn_fmem *fmem);

//extern bool khrn_fmem_special(khrn_fmem *fmem, uint32_t *location, uint32_t special_i, uint32_t offset);
extern bool khrn_fmem_is_here(khrn_fmem *fmem, uint8_t *p);

extern v3d_addr_t khrn_fmem_hw_address(khrn_fmem *fmem, void *p);

/*
 * mark the fact that fmem is using this resource for read or write during
 * binning/rendering/etc;
 */
extern bool khrn_fmem_record_resource_read(khrn_fmem *fmem,
      khrn_resource *res, khrn_stages_t stages);
extern bool khrn_fmem_record_resource_write(khrn_fmem *fmem,
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
extern bool khrn_fmem_record_resource_self_read_conflicting_write(khrn_fmem *fmem,
      khrn_resource *res, khrn_stages_t stages,
      khrn_resource_parts_t parts, bool* requires_flush);

extern bool khrn_fmem_record_handle(khrn_fmem *fmem, gmem_handle_t handle);

extern bool khrn_fmem_record_fence_to_signal(khrn_fmem *fmem,
      khrn_fence *fence);
extern bool khrn_fmem_record_fence_to_depend_on(khrn_fmem *fmem,
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
 * - Due to HW limitations some src/dst combinations are not supported! */
extern bool khrn_fmem_cle_barrier(khrn_fmem *fmem,
   v3d_barrier_flags src, v3d_barrier_flags dst,
   /* Cache ops already done for src accesses. May be NULL. */
   v3d_cache_ops *done_ops_from_src);

VCOS_EXTERN_C_END

#endif
