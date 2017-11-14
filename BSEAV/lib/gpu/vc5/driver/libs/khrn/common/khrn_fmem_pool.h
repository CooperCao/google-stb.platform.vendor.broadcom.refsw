/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

/*
  Each "client" (i.e. process) has a set of buffers that are shared by all active contexts.
  A frame context has its own instance of a "pool" created with khrn_fmem_pool_init()".
*/

#include "khrn_types.h"
#include "libs/core/v3d/v3d_barrier.h"
#include "libs/platform/gmem.h"
#include "libs/util/assert_helpers.h"

/* buffers are allocated on demand up to this limit */
#define KHRN_FMEM_MAX_BLOCKS 256

/* The maximum alignment of anything allocated from an fmem buffer */
#define KHRN_FMEM_ALIGN_MAX V3D_MAX_ALIGN
#define KHRN_FMEM_BUFFER_SIZE (64 * 1024)

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

typedef enum khrn_fmem_buffer_state
{
   KHRN_FMEM_BUFFER_FREE,        // Available for further allocation.      (not waitable, on free list)
   KHRN_FMEM_BUFFER_ASSIGNED,    // Assigned to an unflushed fmem.         (not waitable, not on free list)
   KHRN_FMEM_BUFFER_FLUSHED,     // Filled up, used only by flushed jobs.  (waitable, not on free list)
} khrn_fmem_buffer_state;

typedef struct khrn_fmem_buffer
{
   struct khrn_fmem_buffer* next_free;    // next buffer on the free list.
   gmem_handle_t            handle;
   khrn_fmem_buffer_state   state;
   uint32_t                 bytes_used;   // bytes used by users of this buffer.
   uint32_t                 num_users;    // number of users of this buffer.
} khrn_fmem_buffer;

bool khrn_fmem_client_pool_init(void);
void khrn_fmem_client_pool_deinit(void);

typedef struct khrn_fmem_buffer_block
{
   uint32_t buf_index   : 8;
   uint32_t start64     : 12;  // start / 64.
   uint32_t end64       : 12;  // (end + 63) / 64.
} khrn_fmem_buffer_block;

/* each fmem state has one of these which tracks the buffers associated with a
 * frame; it is assumed that any operations on this structure is protected by the gl lock */
typedef struct khrn_fmem_pool
{
   khrn_render_state* render_state;
   unsigned num_blocks;
   bool submitted;
#ifndef NDEBUG
   unsigned begin_allocs;
#endif
   khrn_fmem_buffer_block blocks[KHRN_FMEM_MAX_BLOCKS];  // This feels a bit wasteful...
} khrn_fmem_pool;

/* Used at frame creation/discarding/completion time */
void khrn_fmem_pool_init(khrn_fmem_pool *pool, khrn_render_state *render_state);

void khrn_fmem_pool_term(khrn_fmem_pool *pool);

/* Begin allocating from a pooled buffer. The buffer may already be partially used.
 * It is necessary to call end_alloc with the buffer before calling on_submit or term
 * or it will leak. The buffer returned will satisfy an allocation of min_size at given
 * alignment. */
khrn_fmem_buffer* khrn_fmem_pool_begin_alloc(khrn_fmem_pool *pool, uint32_t min_size, uint32_t align);

/* End allocating from the buffer specifying the end point. */
void khrn_fmem_pool_end_alloc(khrn_fmem_pool *pool, khrn_fmem_buffer* buf, uint32_t bytes_used);

/* Flush all CPU writes to memory */
void khrn_fmem_pool_cpu_flush(khrn_fmem_pool* pool);

/* Mark all the buffers used by this pool as being submitted to the scheduler. */
void khrn_fmem_pool_on_submit(khrn_fmem_pool *pool);

#if KHRN_DEBUG
void khrn_fmem_pool_add_to_memaccess(
   khrn_fmem_pool *pool,
   khrn_memaccess* memaccess,
   v3d_barrier_flags bin_rw_flags,
   v3d_barrier_flags render_rw_flags);
#endif

/* Return true if this render state is using too many fmem buffers.
 * Caller should flush. */
static inline bool khrn_fmem_pool_should_flush(khrn_fmem_pool *pool)
{
   return pool->num_blocks >= KHRN_FMEM_MAX_PER_RS;
}
