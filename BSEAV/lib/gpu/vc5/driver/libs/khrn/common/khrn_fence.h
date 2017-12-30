/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_FENCE_H
#define KHRN_FENCE_H

#include <stdbool.h>
#include "khrn_int_common.h"
#include "khrn_types.h"
#include "libs/platform/v3d_scheduler.h"

typedef enum
{
   KHRN_FENCE_STATE_NONE,
   KHRN_FENCE_STATE_COMPLETED,
   KHRN_FENCE_STATE_FINALISED,
}khrn_fence_state;

typedef struct khrn_fence
{
   khrn_render_state_set_t users;
   v3d_scheduler_deps deps;
   volatile int ref_count;
   khrn_fence_state known_state;
} khrn_fence;

extern khrn_fence* khrn_fence_create(void);

/* return false if rs is already the user of that fence;
 * You should not call this function directly, instead call
 * khrn_render_state_record_fence_* ;
 */
extern bool khrn_fence_add_user(khrn_fence *fence,
      const khrn_render_state *rs);

/* If rs is the user of that fence, remove it.
 * You should not call this directly. This will be called after we
 * flush the render state */
extern void khrn_fence_remove_user(khrn_fence *fence,
      const khrn_render_state *rs);

/* returns true if the rs is a user of this fence; if rs is NULL return true if
 * there are any users on this fence */
extern bool khrn_fence_has_user(const khrn_fence *fence,
      const khrn_render_state *rs);

extern void khrn_fence_job_add(khrn_fence *fence, uint64_t job_id);

extern void khrn_fence_deps_add(khrn_fence *fence, v3d_scheduler_deps const* deps);

/* flush all the users of this fence */
extern void khrn_fence_flush(khrn_fence *fence);

extern void khrn_fence_wait(khrn_fence *fence,
      v3d_sched_deps_state deps_state);

/* returns true if the fence was flushed and the dependencies in fence
 *  reached the specified deps_state */
extern bool khrn_fence_reached_state(khrn_fence *fence,
      v3d_sched_deps_state deps_state);

extern void khrn_fence_refinc(khrn_fence *fence);
extern void khrn_fence_refdec(khrn_fence *fence);

/* Flush all the users of this fence and return the dependencies. */
extern const v3d_scheduler_deps* khrn_fence_get_deps(khrn_fence *fence);

/* when we wait for a deps_state for platform fence, we might one to set the
 * known state of the fence to the one we waited for */
extern void khrn_fence_set_known_state(khrn_fence *fence,
      v3d_sched_deps_state deps_state);

/* Creates a new fence and adds it to all the render states(fmems) used by the
 * existing fence; copies dependenciess from existing fence the new fence; This
 * way we get a snapshot of a fence at one point; waiting for the new fence is
 * the same as waiting for the fence at the point when this snapshot was taken
 * */
extern khrn_fence* khrn_fence_dup(const khrn_fence *fence);

/* merge fence_1 into fence_2; fence_2 will be added to all the render states
 * specified in fence_1; fence_2, as a result of this, will have as users a
 * union of the render states in fence_1 and fence_2 and the dependencies from
 * both fences; if the sum of dependencies exceeds 8, a null job will be
 * created */
extern bool khrn_fence_merge(khrn_fence *fence_1, const khrn_fence *fence_2);

/* print a khrn_fence */
extern void khrn_fence_print(const khrn_fence *fence);
#endif
