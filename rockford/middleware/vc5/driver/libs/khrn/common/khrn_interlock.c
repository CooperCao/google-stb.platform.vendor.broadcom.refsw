/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Interlock system

FILE DESCRIPTION
Interlock system to ensure correct ordering of reads/writes.
=============================================================================*/

#include "../egl/egl_platform.h"
#include "khrn_int_common.h"
#include "khrn_interlock.h"
#include "khrn_process.h"
#include "khrn_render_state.h"

#include "libs/platform/v3d_scheduler.h"

void khrn_interlock_init(KHRN_INTERLOCK_T *interlock)
{
   memset(interlock, 0, sizeof *interlock);
   memset(&interlock->pre_read, 0, sizeof(v3d_scheduler_deps));
   memset(&interlock->pre_write, 0, sizeof(v3d_scheduler_deps));
}

static void reset_interlock(KHRN_INTERLOCK_T *interlock)
{
   interlock->is_writer = false;
   memset(&interlock->users, 0, sizeof interlock->users);
}

bool khrn_interlock_flush_writer(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   if (!interlock->is_writer)
      return false;

   khrn_render_state_flush(interlock->users.writer.rs);
   reset_interlock(interlock);
   return true;
}

/*
 * Flush any readers except except (which can be NULL). Remove all readers
 * regardless.
 */
static void flush_readers(KHRN_INTERLOCK_T *interlock,
      const KHRN_RENDER_STATE_T *except)
{
#define N (2)
   khrn_render_state_set_t readers[N];
   KHRN_RENDER_STATE_T *rs;
   int i, j;

   if (interlock->is_writer)
      return;

   readers[0] = interlock->users.readers.bin;
   readers[1] = interlock->users.readers.render;

   for (i = 0; i < N; i++)
   {
      while (1)
      {
         rs = khrn_render_state_set_pop(readers + i);
         if (!rs) break;

         if (rs != except)
            khrn_render_state_flush(rs);

         /* Only flush each render state once */
         for (j = i + 1; j < N; j++)
            khrn_render_state_set_remove(readers + j, rs);
      }
   }

   reset_interlock(interlock);
#undef N
}

static void flush_except_reader(KHRN_INTERLOCK_T *interlock,
      const KHRN_RENDER_STATE_T *except)
{
   if (!khrn_interlock_flush_writer(interlock))
      flush_readers(interlock, except);
}

void khrn_interlock_flush(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   flush_except_reader(interlock, NULL);
}

void khrn_interlock_add_reader(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_action_t actions, KHRN_RENDER_STATE_T *rs)
{
   assert(!interlock->in_begin_flags);

   if (interlock->is_writer && interlock->users.writer.rs == rs)
   {
      /* Already a writer, but keep the actions */
      interlock->users.writer.actions |= actions;
      interlock->users.writer.also_reading = true;
      return;
   }

   khrn_interlock_flush_writer(interlock);

   if (actions & ACTION_BIN)
      khrn_render_state_set_add(&interlock->users.readers.bin, rs);

   if (actions & ACTION_RENDER)
      khrn_render_state_set_add(&interlock->users.readers.render, rs);
}

void khrn_interlock_add_writer(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_action_t actions, KHRN_RENDER_STATE_T *rs)
{
   assert(!interlock->in_begin_flags);
   assert(rs != NULL);

   bool already_there = interlock->is_writer && interlock->users.writer.rs == rs;
   if (!already_there)
   {
      /* Whatever actions we were doing as a reader, we're still doing */
      actions |= khrn_interlock_get_actions(interlock, rs);
      flush_except_reader(interlock, rs);

      interlock->users.writer.rs = rs;
      interlock->is_writer = true;
      interlock->invalid = false;
   }
   interlock->users.writer.actions |= actions;
}

bool khrn_interlock_add_writer_tf(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_action_t actions, KHRN_RENDER_STATE_T *rs)
{
   assert(!interlock->in_begin_flags);
   assert(rs != NULL);

   if (interlock->is_writer)
   {
      // This writing render state is also a reader.
      if (interlock->users.writer.rs == rs && interlock->users.writer.also_reading)
         return false;
   }
   else
   {
      // This render state is one of the readers.
      if (khrn_interlock_get_actions(interlock, rs))
         return false;
   }

   khrn_interlock_add_writer(interlock, actions, rs);
   return true;
}

v3d_scheduler_deps const* khrn_interlock_begin_submit_reader_jobs(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);
   khrn_interlock_flush_writer(interlock);
   debug_only(interlock->in_begin_submit_reader_jobs = 1);
   return &interlock->pre_read;
}

void khrn_interlock_end_submit_reader_jobs(KHRN_INTERLOCK_T *interlock,
      bool success, v3d_scheduler_deps const* deps)
{
   assert(interlock->in_begin_submit_reader_jobs);
   debug_only(interlock->in_begin_submit_reader_jobs = 0);
   if (deps->n == 0)
   {
      if (!success)
         return;

      /* we've waited for all the pre_read jobs already */
      v3d_scheduler_copy_deps(&interlock->pre_read, deps);
   }
   else
      v3d_scheduler_merge_deps(&interlock->pre_write, deps);
}

v3d_scheduler_deps const* khrn_interlock_begin_submit_writer_jobs(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);
   khrn_interlock_flush(interlock);
   debug_only(interlock->in_begin_submit_writer_jobs = 1);
   return &interlock->pre_write;
}

void khrn_interlock_end_submit_writer_jobs(KHRN_INTERLOCK_T *interlock,
      bool success, v3d_scheduler_deps const* deps)
{
   assert(interlock->in_begin_submit_writer_jobs);
   debug_only(interlock->in_begin_submit_writer_jobs = 0);
   if (deps->n == 0 && !success)
         return;
   /* If deps->n != 0, we assume the jobs in deps are dependent on the jobs in
    * pre_write, so it is fine to replace pre_read/write with deps.
    * If deps->n == 0, we know that all the jobs in pre_write have completed,
    * but they may not have been finalised yet. It is ok to remove jobs from
    * pre_read as soon as they complete, but jobs may only be removed from
    * pre_write if they have been finalised or are replaced by a dependent job.
    * So we leave pre_write alone in the deps->n == 0 case. */
   v3d_scheduler_copy_deps(&interlock->pre_read, deps);
   if (deps->n != 0)
      v3d_scheduler_copy_deps(&interlock->pre_write, deps);
   interlock->invalid = false;
}

KHRN_RENDER_STATE_T *khrn_interlock_get_writer(
      const KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   if (!interlock->is_writer)
      return NULL;

   return interlock->users.writer.rs;
}

bool khrn_interlock_remove_user(KHRN_INTERLOCK_T *interlock,
      const KHRN_RENDER_STATE_T *rs)
{
   assert(!interlock->in_begin_flags);

   bool bin = false, render = false;

   if (interlock->is_writer && rs == interlock->users.writer.rs)
   {
      reset_interlock(interlock);
      return true;
   }

   bin = khrn_render_state_set_remove(&interlock->users.readers.bin, rs);
   render = khrn_render_state_set_remove(&interlock->users.readers.render, rs);

   return bin || render;
}

static khrn_interlock_action_t get_all_actions(
      const KHRN_INTERLOCK_T *interlock)
{
   khrn_interlock_action_t actions = ACTION_NONE;

   if (interlock->is_writer)
      return interlock->users.writer.actions;

   if (interlock->users.readers.bin)
      actions |= ACTION_BIN;

   if (interlock->users.readers.render)
      actions |= ACTION_RENDER;

   return actions;
}

static khrn_interlock_action_t get_actions_for_user(
      const KHRN_INTERLOCK_T *interlock, const KHRN_RENDER_STATE_T *rs)
{
   khrn_interlock_action_t actions = ACTION_NONE;

   if (interlock->is_writer)
   {
      if (interlock->users.writer.rs == rs)
         return interlock->users.writer.actions;
      else
         return ACTION_NONE;
   }

   if (khrn_render_state_set_contains(interlock->users.readers.bin, rs))
      actions |= ACTION_BIN;

   if (khrn_render_state_set_contains(interlock->users.readers.render, rs))
      actions |= ACTION_RENDER;

   return actions;
}

khrn_interlock_action_t khrn_interlock_get_actions(
      const KHRN_INTERLOCK_T *interlock, const KHRN_RENDER_STATE_T *rs)
{
   assert(!interlock->in_begin_flags);

   if (rs)
      return get_actions_for_user(interlock, rs);
   else
      return get_all_actions(interlock);
}

void khrn_interlock_invalidate(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   khrn_interlock_flush(interlock);
   interlock->invalid = true;

   /* the content of this resource is invalid, so reading from it doesn't need
    * to wait for the previous readers. */
   memset(&interlock->pre_read, 0, sizeof(v3d_scheduler_deps));

   /* do not set interlock->pre_write to 0; though the content is invalid, any
    * future writes will still depend on the pre_write; This is especially
    * important for the case when we dequeue a buffer with a fence */
}

bool khrn_interlock_is_valid(const KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   return !interlock->invalid;
}

static void wait_sync(KHRN_INTERLOCK_T *interlock, bool for_write)
{
   v3d_scheduler_deps *sync = khrn_interlock_get_sync(interlock, for_write);
   v3d_scheduler_wait_jobs(sync, V3D_SCHED_DEPS_COMPLETED);
}

void khrn_interlock_read_now(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   khrn_interlock_flush_writer(interlock);
   wait_sync(interlock, false);
}

void khrn_interlock_write_now(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   khrn_interlock_flush(interlock);
   interlock->invalid = false;
   wait_sync(interlock, true);
}

v3d_scheduler_deps *khrn_interlock_get_sync(KHRN_INTERLOCK_T *interlock, bool for_write)
{
   return for_write ? &interlock->pre_write : &interlock->pre_read;
}

void khrn_interlock_job_add(KHRN_INTERLOCK_T *interlock, uint64_t job_id,
      bool is_write_job)
{
   /* All jobs must be complete before a write job starts */
   v3d_scheduler_add_dep(&interlock->pre_write, job_id);

   /* Writes must complete before we can read from a buffer */
   if (is_write_job)
      v3d_scheduler_add_dep(&interlock->pre_read, job_id);
}

void khrn_interlock_job_replace(KHRN_INTERLOCK_T *interlock, uint64_t job_id)
{
   v3d_scheduler_deps_set(&interlock->pre_write, job_id);
   v3d_scheduler_deps_set(&interlock->pre_read, job_id);
}
