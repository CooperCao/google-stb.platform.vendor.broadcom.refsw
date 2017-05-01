/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "khrn_timeline.h"
#include "khrn_render_state.h"


static void flush_to_point(khrn_timeline *tl, uint64_t pt_val);

void khrn_timeline_init(khrn_timeline *tl)
{
   tl->next_id = 1;
   for (unsigned i = 0; i <= LAST_STATE; ++i)
       tl->head[i] = NULL;
   tl->end = NULL;
}

static khrn_timeline_pt* point_create(uint64_t id)
{
   khrn_timeline_pt *pt;

   /* create a new point */
   pt = calloc(1, sizeof(khrn_timeline_pt));
   if (!pt)
      return NULL;

   pt->fence = khrn_fence_create();
   if (!pt->fence)
   {
      free(pt);
      return NULL;
   }

   pt->id = id;
   return pt;
}

static void point_destroy(khrn_timeline_pt *pt)
{
   if (!pt)
      return;

   khrn_fence_refdec(pt->fence);
   free(pt);
}

void khrn_timeline_deinit(khrn_timeline *tl)
{
   if (!tl->head[LAST_STATE])
      return;

   assert(tl->end);

   while (tl->head[LAST_STATE] != NULL)
   {
      khrn_timeline_pt *pt;
      pt = tl->head[LAST_STATE];
      tl->head[LAST_STATE] = tl->head[LAST_STATE]->next;
      point_destroy(pt);
   }

   tl->end = NULL;
   for (unsigned i =0; i < LAST_STATE; ++i)
       tl->head[i] = NULL;
}

static khrn_timeline_pt* find_point(const khrn_timeline *tl,
      const khrn_render_state *rs)
{
   khrn_timeline_pt *pt;

   /* start at the first non-completed, as all completed points will have no
    * users */
   pt = tl->head[V3D_SCHED_DEPS_COMPLETED];
   while (pt != NULL)
   {
      if (khrn_fence_has_user(pt->fence, rs))
      {
         for (unsigned i = 0; i < LAST_STATE; ++i)
            assert(tl->head[i]->id <= pt->id);
         return pt;
      }

      pt = pt->next;
   }
   return NULL;
}

bool khrn_timeline_record(khrn_timeline *tl, khrn_render_state *rs)
{
   khrn_timeline_pt *pt;
   bool ok = false;

   /* see if we already have a point with this render state */
   pt = find_point(tl, rs);
   if (pt != NULL)
      return true;

   pt = point_create(tl->next_id);
   if (!pt)
      return false;

   ok = khrn_render_state_record_fence_to_signal(rs, pt->fence);
   if (!ok)
      goto end;

   if (!tl->head[LAST_STATE])
   {
      assert(tl->end == NULL);
      for (unsigned i = 0; i < LAST_STATE; ++i)
         assert(tl->head[i] == NULL);
      for (unsigned i = 0; i <= LAST_STATE; ++i)
         tl->head[i] =  pt;
      tl->end = pt;
   }
   else
   {
      for (unsigned i = 0; i < LAST_STATE; ++i)
         if (tl->head[i] == NULL)
            tl->head[i] = pt;
      tl->end->next = pt;
      tl->end = pt;
   }
   tl->next_id++;
   ok = true;

end:
   if (!ok)
      point_destroy(pt);

   return ok;
}

uint64_t khrn_timeline_get_current(const khrn_timeline *tl)
{
   return tl->next_id - 1;
}

static void remove_head(khrn_timeline *tl, v3d_sched_deps_state deps_state)
{
   khrn_timeline_pt *pt;

   pt = tl->head[deps_state];
   if (!pt)
      return;

   if(pt == tl->end)
   {
      for (int i = 0; i <= deps_state; ++i)
         tl->head[i] = NULL;

      if (deps_state == LAST_STATE)
      {
         point_destroy(pt);
         tl->end = NULL;
      }
   }
   else
   {
      tl->head[deps_state] = tl->head[deps_state]->next;
      khrn_timeline_pt *new_pt = tl->head[deps_state];

      for (int i = 0; i < deps_state; ++i)
      {
         if (tl->head[i] == pt)
            tl->head[i] = new_pt;
      }

      if (deps_state == LAST_STATE)
         point_destroy(pt);
   }
}

static bool point_passed_for_state(khrn_timeline *tl, uint64_t pt_val,
      v3d_sched_deps_state deps_state)
{
   khrn_timeline_pt *pt = tl->head[deps_state];

   while (pt && pt->id <= pt_val)
   {
      if (!khrn_fence_reached_state(pt->fence, deps_state))
         return false;

      remove_head(tl, deps_state);
      pt = tl->head[deps_state];
   }

   return true;
}

static void flush_to_point(khrn_timeline *tl, uint64_t pt_val)
{
   khrn_timeline_pt *pt;

   assert(pt_val < tl->next_id);

   /* start at the first non-completed, as all completed points would have
    * been flushed */
   pt = tl->head[V3D_SCHED_DEPS_COMPLETED];
   while(pt && pt->id <= pt_val)
   {
      khrn_fence_flush(pt->fence);
      pt = pt->next;
   }
}

bool khrn_timeline_check(khrn_timeline *tl, uint64_t pt_val,
      v3d_sched_deps_state deps_state)
{

   if (point_passed_for_state(tl, pt_val, deps_state))
      return true;
   flush_to_point(tl, pt_val);

   return false;
}

void khrn_timeline_wait(khrn_timeline *tl, uint64_t pt_val,
      v3d_sched_deps_state deps_state)
{

   if (point_passed_for_state(tl, pt_val, deps_state))
      return;
   flush_to_point(tl, pt_val);

   khrn_timeline_pt *pt;

   pt = tl->head[deps_state];
   while(pt && pt->id <= pt_val)
   {
      khrn_fence_wait(pt->fence, deps_state);

      remove_head(tl, deps_state);
      pt = tl->head[deps_state];
   }
}