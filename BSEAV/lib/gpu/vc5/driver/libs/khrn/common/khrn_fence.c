/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_fence.h"
#include "khrn_render_state.h"
#include "libs/platform/v3d_scheduler.h"
#include "khrn_process.h"
#include "libs/platform/v3d_platform.h"

LOG_DEFAULT_CAT("khrn_fence")

khrn_fence* khrn_fence_create(void)
{
   khrn_fence *fence = NULL;

   fence = calloc(1, sizeof(khrn_fence));
   if (fence)
   {
      fence->ref_count = 1;
      /* empty fence --> can be considered finalised */
      fence->known_state = KHRN_FENCE_STATE_FINALISED;
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
   fence->known_state = KHRN_FENCE_STATE_NONE;
   return khrn_render_state_set_add(&fence->users, rs);
}

void khrn_fence_remove_user(khrn_fence *fence,
      const khrn_render_state *rs)
{
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
}

static khrn_fence_state deps_state_to_fence_state(v3d_sched_deps_state deps_state)
{
   khrn_fence_state fence_state = KHRN_FENCE_STATE_NONE;
   switch (deps_state)
   {
      case V3D_SCHED_DEPS_COMPLETED:
         fence_state = KHRN_FENCE_STATE_COMPLETED;
         break;
      case V3D_SCHED_DEPS_FINALISED:
         fence_state = KHRN_FENCE_STATE_FINALISED;
         break;
      default:
         unreachable();
         break;
   }
   return fence_state;
}

void khrn_fence_set_known_state(khrn_fence *fence, v3d_sched_deps_state deps_state)
{
   khrn_fence_state fence_state = deps_state_to_fence_state(deps_state);
   if (fence_state > fence->known_state)
      fence->known_state = fence_state;
}

void khrn_fence_wait(khrn_fence *fence, v3d_sched_deps_state deps_state)
{
   khrn_fence_flush(fence);

   if (deps_state_to_fence_state(deps_state) <= fence->known_state)
      return;

   v3d_scheduler_wait_jobs(&fence->deps, deps_state);
   khrn_fence_set_known_state(fence, deps_state);
}

bool khrn_fence_reached_state(khrn_fence *fence, v3d_sched_deps_state deps_state)
{
   if (khrn_fence_has_user(fence, NULL))
      return false;

   if (deps_state_to_fence_state(deps_state) <= fence->known_state)
      return true;

   bool res = v3d_scheduler_jobs_reached_state(&fence->deps, deps_state, true);
   if (res)
      khrn_fence_set_known_state(fence, deps_state);
   return res;
}

void khrn_fence_job_add(khrn_fence *fence, uint64_t job_id)
{
   fence->known_state = KHRN_FENCE_STATE_NONE;
   v3d_scheduler_add_dep(&fence->deps, job_id);
}

void khrn_fence_deps_add(khrn_fence *fence, v3d_scheduler_deps const* deps)
{
   fence->known_state = KHRN_FENCE_STATE_NONE;
   v3d_scheduler_merge_deps(&fence->deps, deps);
}

int khrn_fence_get_platform_fence(khrn_fence *fence,
      v3d_sched_deps_state deps_state, bool force_create)
{
   khrn_fence_flush(fence);
   return v3d_scheduler_create_fence(&fence->deps, deps_state, force_create);
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
   new_fence->known_state = fence->known_state;
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
   fence_1->known_state = GFX_MIN(fence_1->known_state, fence_2->known_state);
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
