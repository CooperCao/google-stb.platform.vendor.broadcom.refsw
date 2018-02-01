/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_fence.h"
#include "khrn_render_state.h"
#include "libs/platform/v3d_scheduler.h"
#include "khrn_process.h"
#include "libs/platform/v3d_platform.h"

/* These values are important;
 * We want to store a new state only if it is higher than the existing state
 * and to achieve this we use vcos_atomic_fetch_or (see
 * khrn_fence_set_known_deps_state)
 */
#define KHRN_FENCE_DEPS_STATE_NONE  0
#define KHRN_FENCE_DEPS_STATE_COMPLETED (1 | KHRN_FENCE_DEPS_STATE_NONE)
#define KHRN_FENCE_DEPS_STATE_FINALISED (2 | KHRN_FENCE_DEPS_STATE_COMPLETED)

typedef struct khrn_fence
{
   khrn_render_state_set_t users;
   v3d_scheduler_deps deps;
   volatile int ref_count;
   uint8_t known_deps_state;
#ifndef NDEBUG
   bool allow_change;      /* when this is set to false, users and deps cannot be modified anymore */
#endif
} khrn_fence;

LOG_DEFAULT_CAT("khrn_fence")

khrn_fence *khrn_fence_create(void)
{
   khrn_fence *fence = NULL;

   fence = calloc(1, sizeof(khrn_fence));
   if (fence)
   {
      fence->ref_count = 1;
      /* empty fence --> can be considered finalised */
      fence->known_deps_state = KHRN_FENCE_DEPS_STATE_FINALISED;
      debug_only(fence->allow_change = true);
   }

   return fence;
}

void khrn_fence_refinc(khrn_fence *fence)
{
   vcos_atomic_inc(&fence->ref_count);
}

void khrn_fence_refdec(khrn_fence *fence)
{
   int before_count = 0;
   if (!fence)
      return;

   before_count = vcos_atomic_dec(&fence->ref_count);
   if (before_count == 1)
   {
      assert(fence->users == 0);
      free(fence);
   }
}

bool khrn_fence_add_user(khrn_fence *fence,
      const khrn_render_state *rs)
{
   assert(fence->allow_change);

   fence->known_deps_state = KHRN_FENCE_DEPS_STATE_NONE;
   return khrn_render_state_set_add(&fence->users, rs);
}

void khrn_fence_remove_user(khrn_fence *fence,
      const khrn_render_state *rs)
{
   assert(fence->allow_change);
   khrn_render_state_set_remove(&fence->users, rs);
}

bool khrn_fence_has_user(const khrn_fence *fence,
      const khrn_render_state *rs)
{
   if (!rs)
      return (fence->users != 0);

   return khrn_render_state_set_contains(fence->users, rs);
}

void khrn_fence_flush(khrn_fence *fence)
{
   for (khrn_render_state *rs; (rs = khrn_render_state_set_pop(&fence->users)) != NULL; )
   {
      khrn_render_state_flush(rs);
   }

   if (fence->deps.n != 0)
      v3d_scheduler_flush();

   debug_only(fence->allow_change = false);
}

static uint8_t deps_state_to_fence_state(v3d_sched_deps_state deps_state)
{
   uint8_t fence_state = KHRN_FENCE_DEPS_STATE_NONE;
   switch (deps_state)
   {
      case V3D_SCHED_DEPS_COMPLETED:
         fence_state = KHRN_FENCE_DEPS_STATE_COMPLETED;
         break;
      case V3D_SCHED_DEPS_FINALISED:
         fence_state = KHRN_FENCE_DEPS_STATE_FINALISED;
         break;
      default:
         unreachable();
         break;
   }
   return fence_state;
}

void khrn_fence_set_known_deps_state(khrn_fence *fence, v3d_sched_deps_state deps_state)
{
   uint8_t fence_state = deps_state_to_fence_state(deps_state);
   // set know state to the highest of existing and latest we know about
   vcos_atomic_fetch_or_uint8(&fence->known_deps_state, fence_state, VCOS_MEMORY_ORDER_RELEASE);
}

bool khrn_fence_reached_state(khrn_fence *fence, v3d_sched_deps_state deps_state)
{
   khrn_fence_flush(fence);

   uint8_t existing_state = vcos_atomic_load_uint8(&fence->known_deps_state,
         VCOS_MEMORY_ORDER_ACQUIRE);
   if (deps_state_to_fence_state(deps_state) <= existing_state)
      return true;

   bool res = v3d_scheduler_jobs_reached_state(&fence->deps, deps_state, true);
   if (res)
      khrn_fence_set_known_deps_state(fence, deps_state);
   return res;
}

void khrn_fence_job_add(khrn_fence *fence, uint64_t job_id)
{
   assert(fence->allow_change);

   fence->known_deps_state = KHRN_FENCE_DEPS_STATE_NONE;
   v3d_scheduler_add_dep(&fence->deps, job_id);
}

void khrn_fence_deps_add(khrn_fence *fence, v3d_scheduler_deps const* deps)
{
   assert(fence->allow_change);

   fence->known_deps_state = KHRN_FENCE_DEPS_STATE_NONE;
   v3d_scheduler_merge_deps(&fence->deps, deps);
}

const v3d_scheduler_deps *khrn_fence_get_deps(khrn_fence *fence)
{
   khrn_fence_flush(fence);
   return &fence->deps;
}

static bool record_fence_to_signal(khrn_render_state *rs, void *param)
{
   khrn_fence *fence = (khrn_fence*) param;
   return khrn_render_state_record_fence_to_signal(rs, fence);
}

khrn_fence* khrn_fence_dup(const khrn_fence *fence)
{
   bool res;
   khrn_fence* new_fence = NULL;

   new_fence = khrn_fence_create();
   if (!new_fence)
      return NULL;

   /* iterate through all the renders states from the existing fence and on
    * each of them add this new fence as a fence to be signaled */
   res = khrn_render_state_set_iterate_cb(&fence->users, record_fence_to_signal,
         new_fence);
   if (!res)
      goto end;

   assert(new_fence->users == fence->users);
   new_fence->deps = fence->deps;
   new_fence->known_deps_state = vcos_atomic_load_uint8(&fence->known_deps_state,
         VCOS_MEMORY_ORDER_ACQUIRE);
   res = true;

end:
   if (!res)
      KHRN_MEM_ASSIGN(new_fence, NULL);

   return new_fence;
}

bool khrn_fence_merge(khrn_fence *fence_1, const khrn_fence *fence_2)
{
   bool res = false;

   /* iterate through all the renders states from fence_2 and on
    * each of them add fence_1 as a fence to be signaled */
   res = khrn_render_state_set_iterate_cb(&fence_2->users, record_fence_to_signal,
         fence_1);
   if (!res)
      goto end;

   assert(khrn_render_state_set_is_subset(&fence_1->users,
            &fence_2->users));

   v3d_scheduler_merge_deps(&fence_1->deps, &fence_2->deps);

   //set the known state to the smallest of the two fences
   uint8_t fence2_state = vcos_atomic_load_uint8(&fence_2->known_deps_state, VCOS_MEMORY_ORDER_ACQUIRE);
   fence_1->known_deps_state = GFX_MIN(fence_1->known_deps_state, fence2_state);
   res = true;

end:
   return res;
}

void khrn_fence_print(const khrn_fence *fence)
{
   unsigned i;

   log_error("%s fence=0x%p users=0x%x deps.n=%d", __FUNCTION__, fence,
         fence->users, fence->deps.n);
   for (i = 0; i < fence->deps.n; i++)
   {
      log_error(" dependency[%d]=%" PRIu64 " ", i, fence->deps.dependency[i]);
   }
}

#ifndef NDEBUG
void khrn_fence_allow_change(khrn_fence *fence)
{
   assert(fence->users == 0);
   fence->allow_change = true;
}
#endif

static bool fence_wait(khrn_fence *fence, v3d_sched_deps_state deps_state, bool with_timeout,
      int timeout_ms)
{
   assert(fence->users == 0);

   uint8_t existing_state = vcos_atomic_load_uint8(&fence->known_deps_state,
         VCOS_MEMORY_ORDER_ACQUIRE);
   if (deps_state_to_fence_state(deps_state) <= existing_state)
      return true;

   bool res = true;
   if (with_timeout)
      res = v3d_scheduler_wait_jobs_timeout(&fence->deps, deps_state, timeout_ms);
   else
      v3d_scheduler_wait_jobs(&fence->deps, deps_state);

   if (res)
      khrn_fence_set_known_deps_state(fence, deps_state);
   return res;
}

void khrn_fence_wait(khrn_fence *fence, v3d_sched_deps_state deps_state)
{
   bool res = fence_wait(fence, deps_state, false, 0);
   assert(res);
}

bool khrn_fence_wait_timeout(khrn_fence *fence, v3d_sched_deps_state deps_state, int timeout_ms)
{
   return fence_wait(fence, deps_state, true, timeout_ms);
}
