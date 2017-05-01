/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
/*
  Overview
  --------
  Each "client" (i.e. process) has a set of buffers that are shared by all active contexts.
  A frame context has its own instance of a "pool" created with khrn_fmem_pool_init()".
  Buffers are:
   - allocated from the "client" and associated with a "pool" by khrn_fmem_pool_alloc()"
   - populated by the CPU
   - passed to the hardware with khrn_fmem_pool_mark_as_submitted()
   - recycled to the "client" for further CPU use with khrn_fmem_pool_deinit()
*/
#ifndef KHRN_FMEM_POOL_4_H
#define KHRN_FMEM_POOL_4_H

#include "khrn_types.h"
#include "libs/core/v3d/v3d_barrier.h"
#include "libs/platform/gmem.h"
#include "libs/util/assert_helpers.h"

/* buffers are allocated on demand up to this limit */
#define KHRN_FMEM_MAX_BLOCKS 256

/* The maximum alignment of anything allocated from an fmem buffer */
#define KHRN_FMEM_ALIGN_MAX V3D_OCCLUSION_QUERY_COUNTER_FIRST_CORE_CACHE_LINE_ALIGN
#define KHRN_FMEM_BUFFER_SIZE (64 * 1024)
#define KHRN_FMEM_USABLE_BUFFER_SIZE (KHRN_FMEM_BUFFER_SIZE - V3D_CL_BRANCH_SIZE - GFX_MAX(V3D_MAX_CLE_READAHEAD, V3D_MAX_QPU_UNIFS_READAHEAD) - 1)

/* when the number of free fmems and number of submitted buffers in the client
 * pool goes below this threshold, start flushing other render states except the
 * current one, till we get over this threshold or there are no other render
 * states to flush */
#define KHRN_FMEM_THRESHOLD_FLUSH_OTHER_RS 4
static_assrt(KHRN_FMEM_THRESHOLD_FLUSH_OTHER_RS <= (KHRN_FMEM_MAX_BLOCKS/2));

/* when a rs is using  KHRN_FMEM_MAX_PER_RS fmems, if the rs gets installed as
 * current, we are going to flush this rs before installing it */
#define KHRN_FMEM_MAX_PER_RS (KHRN_FMEM_MAX_BLOCKS - 4)
static_assrt(KHRN_FMEM_MAX_PER_RS > 0);

#if KHRN_DEBUG
typedef struct khrn_memaccess khrn_memaccess;
#endif

typedef struct khrn_fmem_buffer
{
   bool in_use;

   gmem_handle_t handle;
   void *cpu_address;
   v3d_addr_t hw_address;
   uint32_t bytes_used;

} khrn_fmem_buffer;

/* each fmem state has one of these which tracks the buffers associated with a
 * frame; it is assumed that any operations on this structure is protected by the gl lock */
typedef struct khrn_fmem_pool
{
   khrn_fmem_buffer *buffer[KHRN_FMEM_MAX_BLOCKS];
   unsigned n_buffers;
   bool buffers_submitted;
} khrn_fmem_pool;

extern bool khrn_fmem_client_pool_init(void);
extern void khrn_fmem_client_pool_deinit(void);

/* Returns the number of free fmems  and the number of buffers that were
 * submitted (the submitted buffers will become free once we wait for all the
 * jobs in this client */
extern unsigned khrn_fmem_client_pool_get_num_free_and_submitted(void);

/* Used at frame creation/discarding/completion time */
extern void khrn_fmem_pool_init(khrn_fmem_pool *pool,
                                khrn_render_state *render_state);

extern void khrn_fmem_pool_deinit(khrn_fmem_pool *pool);

extern void *khrn_fmem_pool_alloc(khrn_fmem_pool *pool);

extern void khrn_fmem_pool_post_cpu_write(khrn_fmem_pool* pool);

extern void khrn_fmem_pool_submit(
   khrn_fmem_pool *pool,
#if KHRN_DEBUG
   khrn_memaccess* memacces,
#endif
   v3d_barrier_flags* bin_rw_flags,
   v3d_barrier_flags* render_rw_flags);

/* Convert a client-side CPU address into a V3D address */
extern v3d_addr_t khrn_fmem_pool_hw_address(khrn_fmem_pool *pool, void *address);

/* Return true if this render state is using too many fmem buffers.
 * Caller should flush. */
static inline bool khrn_fmem_pool_should_flush(khrn_fmem_pool *pool)
{
   return pool->n_buffers >= KHRN_FMEM_MAX_PER_RS;
}

/* Mark the end of a block allocated from khrn_fmem_pool_alloc. */
extern void khrn_fmem_pool_finalise_end(khrn_fmem_pool *pool, void *address);

#endif
