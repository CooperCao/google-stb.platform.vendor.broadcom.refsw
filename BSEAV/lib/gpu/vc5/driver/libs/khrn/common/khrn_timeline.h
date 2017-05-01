/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef KHRN_TIMELINE_H
#define KHRN_TIMELINE_H

#include "khrn_fence.h"
#include "khrn_types.h"

typedef struct khrn_timeline_pt
{
   uint64_t id;
   khrn_fence *fence;
   struct khrn_timeline_pt *next;

} khrn_timeline_pt;

#define LAST_STATE V3D_SCHED_DEPS_FINALISED
static_assrt(V3D_SCHED_DEPS_COMPLETED == 0);

typedef struct khrn_timeline
{
   uint64_t next_id;

    /* There is only one list, between head[LAST_STATE] and end;
    * We have one head for each v3d_sched_deps_state;
    * if head[deps_state] != NULL, where deps_state: 0 - LAST_STATE,
    *  - head[deps_state] points to the first element that might not have
    *    specified deps_state
    * head[deps_state], where deps_state < LAST_SATE, points to an element between
    *    head[LAST_STATE] and end;
    * if head[deps_state] == NULL, where deps_state: 0 - LAST_STATE,
    *  - any point on this timeline (between head[LAST_STATE] and end) has at
    *    least deps_state
    */
   khrn_timeline_pt *head[LAST_STATE + 1];
   khrn_timeline_pt *end;

}khrn_timeline;

extern void khrn_timeline_init(khrn_timeline *tl);
extern void khrn_timeline_deinit(khrn_timeline *tl);

extern bool khrn_timeline_record(khrn_timeline *tl, khrn_render_state *rs);
extern uint64_t khrn_timeline_get_current(const khrn_timeline *timeline);

extern bool khrn_timeline_check(khrn_timeline *tl, uint64_t pt_val,
      v3d_sched_deps_state deps_state);
extern void khrn_timeline_wait(khrn_timeline *tl, uint64_t pt_val,
      v3d_sched_deps_state deps_state);

#endif
