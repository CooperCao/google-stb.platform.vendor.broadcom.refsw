/*=============================================================================
Broadcom Proprietary and Confidential. (c)2010 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Fixed-size non-relocatable block allocator

FILE DESCRIPTION
Used primarily for per-frame allocations:
- control lists, uniforms, shader records and the like (via fmem)
- tessellated vertices for vg
- binning memory
As such, total memory usage depends heavily on the number of frames in the
pipeline. We attempt to limit memory usage by waiting before certain
allocations. We split allocations into two categories: "throttling" (fmems) and
non-throttling (everything else). We have a separate limit for throttling
allocations that is adjusted dynamically: we try to maximise the limit while
avoiding non-throttling allocation waits. If non-throttling allocations are
being forced to wait (or exceed the desired limit, or fail), the limit is
adjusted downwards. Otherwise, if the high water mark is low, the limit is
adjusted upwards. This separate (lower) limit is an attempt to avoid the backend
of the pipeline being choked by allocations occurring in the front of the
pipeline.
=============================================================================*/

#ifndef KHRN_NMEM_4_H
#define KHRN_NMEM_4_H

#include "middleware/khronos/common/khrn_hw.h"

#define KHRN_NMEM_BLOCK_SIZE (64 * 1024)

extern bool khrn_nmem_init(void);
extern void khrn_nmem_term(void);

/* callback registration: callbacks are called when blocks are freed / something
 * exits the pipeline (see below) */
typedef void (*KHRN_NMEM_CALLBACK_T)(void *);
extern void khrn_nmem_register(KHRN_NMEM_CALLBACK_T callback, void *p);
extern void khrn_nmem_unregister(KHRN_NMEM_CALLBACK_T callback, void *p);

/* provided for the vg tessellation stuff. at least 1 qpu is blocked when
 * performing a tessellation allocation. to prevent wasted qpu time and
 * potential deadlocks (if all qpus are blocked, binning/rendering of frames
 * further down the pipeline won't be able to progress), we don't allow
 * tessellation allocations to wait. instead, we wait prior to adding shaders to
 * the user shader fifo. khrn_nmem_reserve lowers the effective allocation
 * limit, allowing any user shaders that are outstanding when we hit the
 * effective limit a couple of blocks with which to complete */
extern void khrn_nmem_reserve(int32_t adjustment); /* only call from the master task */

/* in order to avoid deadlocks, we need to be careful about waiting: we should
 * only consider waiting when there is something ahead of us in the pipeline
 * that will drain even if we wait (normally the case)
 *
 * we keep a simple model of the pipeline to help us decide if this is the case
 *
 * we notify potential waiters when blocks are freed, but also when something
 * exits the pipeline (it's possible for an entry moving through the pipeline to
 * have no associated blocks, but for someone to wait for it anyway) */
extern uint32_t khrn_nmem_enter(void); /* only call from the master task */
extern uint32_t khrn_nmem_get_enter_pos(void);
extern void khrn_nmem_exit(uint32_t pos);

/* throttle determines if this is a throttling or non-throttling allocation (see
 * above)
 *
 * if wait is NULL, we will not consider waiting: the allocation must be done
 * immediately or not at all. if wait is not NULL, and NULL is returned, *wait
 * determines the action that should be taken: true if the caller should wait
 * and try again later, false if the caller should give up and report out of
 * memory
 *
 * pos must be provided if wait is not NULL. waiting will only be advised if
 * there is something ahead of pos in the pipeline
 *
 * khrn_nmem_should_wait() just does the wait-checking bit of khrn_nmem_alloc() */
extern bool khrn_nmem_should_wait(bool throttle, uint32_t pos);
extern void *khrn_nmem_alloc(bool throttle, bool *wait, uint32_t pos, MEM_LOCK_T *lbh); /* returned pointer is very aligned. probably ~4k aligned */
extern void khrn_nmem_free(void *p, bool throttle);

#define KHRN_NMEM_GROUP_BLOCK_SIZE (KHRN_NMEM_BLOCK_SIZE - sizeof(KHRN_NMEM_GROUP_BLOCK_T *))

struct KHRN_NMEM_GROUP_BLOCK_S;
typedef struct KHRN_NMEM_GROUP_BLOCK_S KHRN_NMEM_GROUP_BLOCK_T;
struct KHRN_NMEM_GROUP_BLOCK_S {
   uint8_t data[KHRN_NMEM_GROUP_BLOCK_SIZE];
   KHRN_NMEM_GROUP_BLOCK_T *next;
};

vcos_static_assert(sizeof(KHRN_NMEM_GROUP_BLOCK_T) == KHRN_NMEM_BLOCK_SIZE);

typedef struct {
   KHRN_NMEM_GROUP_BLOCK_T *head;
   bool throttle;
   uint32_t n; /* how many blocks? */
} KHRN_NMEM_GROUP_T;

extern void khrn_nmem_group_init(KHRN_NMEM_GROUP_T *group, bool throttle);
extern void *khrn_nmem_group_alloc(KHRN_NMEM_GROUP_T *group, bool *wait, uint32_t pos, MEM_LOCK_T *lbh); /* returned pointer is very aligned. probably ~4k aligned */
extern void khrn_nmem_group_term(KHRN_NMEM_GROUP_T *group);
extern void khrn_nmem_group_term_and_exit(KHRN_NMEM_GROUP_T *group, uint32_t pos);

/*
   allocations on the master can wait for anything in the pipeline (so we pass
   khrn_nmem_get_enter_pos() as pos)
*/

static INLINE void *khrn_nmem_alloc_master(bool throttle, MEM_LOCK_T *lbh)
{
   void *p;
   bool wait;
   for (;;) {
      p = khrn_nmem_alloc(throttle, &wait, khrn_nmem_get_enter_pos(), lbh);
      if (p || !wait) {
         break;
      }
   }
   return p;
}

static INLINE void *khrn_nmem_group_alloc_master(KHRN_NMEM_GROUP_T *group, MEM_LOCK_T *lbh)
{
   void *p;
   bool wait;
   for (;;) {
      p = khrn_nmem_group_alloc(group, &wait, khrn_nmem_get_enter_pos(), lbh);
      if (p || !wait) {
         break;
      }
   }
   return p;
}

#endif
