/*=============================================================================
Copyright (c) 2010 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
khrn timeline API - used for recording operations on different render states,
                    getting a "current point" on the timeline and
                    being able to wait till the operations recorded up to a
                    point have reached a certain state (completed. finalised)
============================================================================*/
#ifndef KHRN_TIMELINE_H
#define KHRN_TIMELINE_H

#include "khrn_fence.h"
#include "khrn_types.h"

typedef struct khrn_timeline_pt_t
{
   uint64_t id;
   KHRN_FENCE_T *fence;
   struct khrn_timeline_pt_t *next;

}KHRN_TIMELINE_PT_T;

#define LAST_STATE V3D_SCHED_DEPS_FINALISED
static_assrt(V3D_SCHED_DEPS_COMPLETED == 0);

typedef struct khrn_timeline_t
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
   KHRN_TIMELINE_PT_T *head[LAST_STATE + 1];
   KHRN_TIMELINE_PT_T *end;

}KHRN_TIMELINE_T;

extern void khrn_timeline_init(KHRN_TIMELINE_T *tl);
extern void khrn_timeline_deinit(KHRN_TIMELINE_T *tl);

extern bool khrn_timeline_record(KHRN_TIMELINE_T *tl, KHRN_RENDER_STATE_T *rs);
extern uint64_t khrn_timeline_get_current(const KHRN_TIMELINE_T *timeline);

extern bool khrn_timeline_check(KHRN_TIMELINE_T *tl, uint64_t pt_val,
      v3d_sched_deps_state deps_state);
extern void khrn_timeline_wait(KHRN_TIMELINE_T *tl, uint64_t pt_val,
      v3d_sched_deps_state deps_state);

#endif
