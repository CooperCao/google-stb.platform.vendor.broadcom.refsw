/*=============================================================================
Broadcom Proprietary and Confidential. (c)2012 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Per-frame memory pool

FILE DESCRIPTION
Handles allocation and uploading of memory for control lists and
associated data that will be generated each frame as HW input.
=============================================================================*/
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
#include "khrn_synclist.h"
#include "libs/platform/gmem.h"

/* buffers are allocated on demand up to this limit */
#define KHRN_FMEM_MAX_BLOCKS 128

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

typedef struct
{
   /* If this is set, it means V3D may write to this buffer and it may be read
    * on the CPU after the frame has completed. eg This should be set if the
    * buffer contains occlusion query counters. */
   bool is_render_output;
   bool in_use;

   gmem_handle_t handle;
   void *cpu_address;
   v3d_addr_t hw_address;
   uint32_t bytes_used;

} KHRN_FMEM_BUFFER;

/* each fmem state has one of these which tracks the buffers associated with a
 * frame; it is assumed that any operations on this structure is protected by the gl lock */
typedef struct
{
   KHRN_FMEM_BUFFER *buffer[KHRN_FMEM_MAX_BLOCKS];
   unsigned n_buffers;
   bool buffers_submitted;

   /* the frame's lock set */
   struct gmem_lock_list lock_list;
} KHRN_FMEM_POOL_T;

extern bool khrn_fmem_client_pool_init(void);
extern void khrn_fmem_client_pool_deinit(void);

/* Returns the number of free fmems  and the number of buffers that were
 * submitted (the submitted buffers will become free once we wait for all the
 * jobs in this client */
extern unsigned khrn_fmem_client_pool_get_num_free_and_submitted(void);

/* Used at frame creation/discarding/completion time */
extern void khrn_fmem_pool_init(KHRN_FMEM_POOL_T *pool,
                                KHRN_RENDER_STATE_T *render_state);

extern void khrn_fmem_pool_deinit(KHRN_FMEM_POOL_T *pool);

extern void *khrn_fmem_pool_alloc(KHRN_FMEM_POOL_T *pool);

extern void khrn_fmem_pool_submit(KHRN_FMEM_POOL_T *pool,
   struct khrn_synclist *bin, struct khrn_synclist *render);

/* Convert a client-side CPU address into a V3D address */
extern v3d_addr_t khrn_fmem_pool_hw_address(KHRN_FMEM_POOL_T *pool, void *address);

/* Return true if this render state is using too many fmem buffers.
 * Caller should flush. */
static inline bool khrn_fmem_pool_should_flush(KHRN_FMEM_POOL_T *pool)
{
   return pool->n_buffers >= KHRN_FMEM_MAX_PER_RS;
}

extern void khrn_fmem_pool_mark_as_render_output(KHRN_FMEM_POOL_T *pool, void *address);

/* Mark the end of a block allocated from khrn_fmem_pool_alloc. */
extern void khrn_fmem_pool_finalise_end(KHRN_FMEM_POOL_T *pool, void *address);

extern void khrn_fmem_pool_pre_cpu_read_outputs(const KHRN_FMEM_POOL_T *pool);

#endif
