/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Interlock system

FILE DESCRIPTION
Interlock system to ensure correct ordering of reads/writes.
=============================================================================*/
#ifndef KHRN_INTERLOCK_H
#define KHRN_INTERLOCK_H

#include <stdbool.h>
#include "middleware/khronos/common/khrn_types.h"
#include "gmem.h"
#include "v3d_scheduler.h"

/*
 * Terminology: A "reader" is a render state that reads the resource, a
 * "writer" one that writes it. A "user" is a reader or a writer.
 *
 * A "user" uses the resource during one or more of the user's "actions" which
 * are things like binning and rendering. This allows us to be more
 * fine-grained about synchronization-- if you want to wait for a user to have
 * finished with a resource then you only have to wait for his specific
 * actions on the resource to have completed.
 */
typedef enum
{
   ACTION_NONE     = 0,
   ACTION_BIN      = 1U << 0,
   ACTION_RENDER   = 1U << 1,
   ACTION_BOTH     = ACTION_BIN | ACTION_RENDER,
   ACTION_TFU      = 1U << 2,
}
khrn_interlock_action_t;

struct khrn_interlock
{
   union
   {
      /*
       * Readers are represented by a series of sets, one for each action. So
       * if a reader is in the set, then it reads during that action. We do it
       * like this because the small number of readers in the system (<32)
       * means we can implement sets efficiently and easily with bitfields.
       */
      struct
      {
         /* Render states that are reading the resource during binning */
         khrn_render_state_set_t bin;

         /*
          * Render states that are reading the resource during rendering
          * (often the same render state will appear in both bin and render)
          */
         khrn_render_state_set_t render;

         /* We will probably need to add another set for tfu */
      }
      readers;

      struct
      {
         /* There is only ever one writer (who might also be a reader) */
         KHRN_RENDER_STATE_T *rs;

         /*
          * Whether the writer is writing (or reading) during binning and/or
          * rendering etc.
          */
         khrn_interlock_action_t actions;

         /* The one writer might also be reading, so record that. */
         bool also_reading;
      }
      writer;
   }
   users;
   bool is_writer;

   /*
    * If you add a writer, everything is always flushed.
    *
    * If you add a reader, then any existing writer is flushed. But if the
    * interlock currently has readers instead of a writer, you just join the
    * list of readers, and don't need to flush it yet.
    */

   /* Resource contents are invalid */
   bool invalid;

#ifndef NDEBUG
   union
   {
      struct
      {
         uint8_t in_begin_submit_reader_jobs : 1;
         uint8_t in_begin_submit_writer_jobs : 1;
      };
      uint8_t in_begin_flags;
   };
#endif

   /*
    * When a writer or reader is flushed, any resulting jobs will be added to
    * these deps so we can wait for for them to complete/be finalised.
    *
    * If you are writing the buffer, you need to wait for the jobs in pre_write
    * to complete.
    * If you are reading the buffer, you need to wait for the jobs in pre_read
    * to complete.
    * If you are freeing the buffer, you need to wait for the jobs in pre_write
    * to be finalised.
    *
    * Note that:
    * - pre_write is always a superset of pre_read (ie after waiting for all
    *   the jobs in pre_write to complete/be finalised, it is guaranteed that
    *   all the jobs in pre_read will have completed/been finalised)
    * - jobs may be removed from pre_read as soon as they complete, but jobs
    *   may not be removed from pre_write until they have been finalised (or
    *   replaced with a dependent job)
    */
   v3d_scheduler_deps pre_write;
   v3d_scheduler_deps pre_read;
};

extern void khrn_interlock_init(KHRN_INTERLOCK_T *interlock);

/*
 * Flush whatever users the interlock has (whether it's a writer or a set of
 * readers).
 */
extern void khrn_interlock_flush(KHRN_INTERLOCK_T *interlock);

/*
 * Flush the writer of an interlock if it has one.
 * Returns true if there was a writer.
 */
extern bool khrn_interlock_flush_writer(KHRN_INTERLOCK_T *interlock);

/*
 * Add rs as a reader of interlock unless it is already the writer, in which
 * case, do nothing except add in the actions.
 *
 * If there is a writer (that isn't rs), it is flushed.
 */
extern void khrn_interlock_add_reader(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_action_t actions, KHRN_RENDER_STATE_T *rs);

/*
 * Prepare the resource protected by the interlock for instant reading. This
 * means flush out any existing writers, and actually waits.
 */
extern void khrn_interlock_read_now(KHRN_INTERLOCK_T *interlock);

/*
 * Add a writer, flushing any existing different writer or readers. Say
 * whether you're writing in the bin or render action or both (if in doubt,
 * both is safe and slow).
 *
 * If rs was already the reader, other readers are flushed, and rs still
 * becomes the writer, but rs is not flushed.
 */
extern void khrn_interlock_add_writer(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_action_t actions, KHRN_RENDER_STATE_T *rs);

/*
 * Add a transform feedback writer. Returns true for success.
 * Failure means it is necessary to flush the render state and try again with a fresh one.
 */
extern bool khrn_interlock_add_writer_tf(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_action_t actions, KHRN_RENDER_STATE_T *rs);

/* Begin submitting jobs that read data guarded by this interlock.
   The next call must be to khrn_interlock_end_submit_reader_jobs().
   Returns the dependencies for the job to be submitted. */
extern v3d_scheduler_deps const* khrn_interlock_begin_submit_reader_jobs(KHRN_INTERLOCK_T *interlock);

/* End submitting jobs that read data guarded by this interlock by providing
 * the job-ids of those jobs;
 * call with success = false and desp->n = 0, if we've failed submitting and reader jobs
 * call deps->n != 0, if we've submitted any asynchonrous reader jobs
 * call with success = true and deps->n = 0 if we've done synchronous reads from the data
 *                                          protected by this interlock
 */
extern void khrn_interlock_end_submit_reader_jobs(KHRN_INTERLOCK_T *interlock,
      bool success, v3d_scheduler_deps const* deps);

/* Begin submitting jobs that write data guarded by this interlock.
   The next call must be to khrn_interlock_end_submit_writer_jobs().
   Returns the dependencies for the job to be submitted. */
extern v3d_scheduler_deps const* khrn_interlock_begin_submit_writer_jobs(KHRN_INTERLOCK_T *interlock);

/* End submitting jobs that write data guarded by this interlock by providing the
 * job-ids of those jobs.
 * call with success = false and desp->n = 0, if we've failed submitting and writer jobs
 * call deps->n != 0, if we've submitted any asynchonrous writer jobs
 * call with success = true and deps->n = 0 if we've done synchronous writes to the data
 *                                           protected by this interlock
 */
extern void khrn_interlock_end_submit_writer_jobs(KHRN_INTERLOCK_T *interlock,
      bool success, v3d_scheduler_deps const* deps);

/*
 * Prepare the resource protected by the interlock for instant writing. This
 * means flush out any existing writers or readers, and actually wait.
 */
extern void khrn_interlock_write_now(KHRN_INTERLOCK_T *interlock);

/* Get the writer if there is one */
extern KHRN_RENDER_STATE_T *khrn_interlock_get_writer(
      const KHRN_INTERLOCK_T *interlock);

/* If rs is the writer or one of the readers, remove it, and return true. */
extern bool khrn_interlock_remove_user(KHRN_INTERLOCK_T *interlock,
      const KHRN_RENDER_STATE_T *rs);

/*
 * Returns the actions that rs has on interlock (i.e. whether rs is not a user
 * at all, or uses the resource for binning, rendering, etc.).
 *
 * If rs is NULL, returns the union of actions by all users.
 */
extern khrn_interlock_action_t khrn_interlock_get_actions(
      const KHRN_INTERLOCK_T *interlock, const KHRN_RENDER_STATE_T *rs);

/* Call this when the underlying resource's contents have become invalid */
extern void khrn_interlock_invalidate(KHRN_INTERLOCK_T *interlock);

extern bool khrn_interlock_is_valid(const KHRN_INTERLOCK_T *interlock);

/*
 * Return a sync to wait for work queued on the interlock's resource to
 * complete. Set for_write to true if you are waiting to write (i.e. wait for
 * all prior readers and writers). If for_write is false, you will get a fence
 * that only waits for prior writers, which is OK if you only want to read.
 */
extern v3d_scheduler_deps *khrn_interlock_get_sync(
      KHRN_INTERLOCK_T *interlock, bool for_write);

/*
 * Adds a scheduler job to the list of dependencies the interlock waits for.
 * If the job to wait for is writing to the buffer is_write_job must be true.
 */
void khrn_interlock_job_add(KHRN_INTERLOCK_T *interlock, uint64_t job_id,
      bool is_write_job);

/*
 * Replaces all dependencies with a single job. Only use this when you know
 * the job depends on everything in the interlock. This will normally only
 * be the case when the job is a writer.
 */
void khrn_interlock_job_replace(KHRN_INTERLOCK_T *interlock, uint64_t job_id);

//! Returns true if khrn_interlock_read_now() would flush something.
static inline bool khrn_interlock_read_now_would_flush(KHRN_INTERLOCK_T* interlock);

//! Returns true if khrn_interlock_write_now() would flush something.
static inline bool khrn_interlock_write_now_would_flush(KHRN_INTERLOCK_T* interlock);

//! Returns true if khrn_interlock_read_now() would stall.
static inline bool khrn_interlock_read_now_would_stall(KHRN_INTERLOCK_T* interlock);

//! Returns true if khrn_interlock_write_now() would stall.
static inline bool khrn_interlock_write_now_would_stall(KHRN_INTERLOCK_T* interlock);

#include "khrn_interlock.inl"

#endif
