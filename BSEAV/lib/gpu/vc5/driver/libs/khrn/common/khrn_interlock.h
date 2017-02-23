/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Interlock system to ensure correct ordering of reads/writes.
=============================================================================*/
#pragma once

#include <stdbool.h>
#include "khrn_types.h"
#include "libs/platform/gmem.h"
#include "libs/platform/v3d_scheduler.h"

#ifdef __cplusplus
extern "C" {
#endif

/*
 * Terminology: A "reader" is a render state that reads the resource, a
 * "writer" one that writes it.
 *
 * A render-state uses the resource during one or more "stages" which
 * are things like binning and rendering. This allows us to be more
 * fine-grained about synchronization-- if you want to wait for a
 * render-state to have finished with a resource then you only have
 * to wait for his specific stages on the resource to have completed.
 */
typedef enum khrn_interlock_stages_t
{
   KHRN_INTERLOCK_STAGE_PREPROCESS = 1 << 0,
   KHRN_INTERLOCK_STAGE_BIN        = 1 << 1,
   KHRN_INTERLOCK_STAGE_RENDER     = 1 << 2,
}
khrn_interlock_stages_t;
#define KHRN_INTERLOCK_STAGES_BIN_RENDER (KHRN_INTERLOCK_STAGE_BIN | KHRN_INTERLOCK_STAGE_RENDER)
#define KHRN_INTERLOCK_NUM_STAGES 3u
#define KHRN_INTERLOCK_BITS_PER_RS 4u

/* Resources are divided up into parts. Exactly how they are divided up is
 * defined on a per-resource basis.
 *
 * The first KHRN_INTERLOCK_PARTS_BITS-1 bits of a khrn_interlock_parts_t
 * correspond to the first KHRN_INTERLOCK_PARTS_BITS-1 parts of a resource. The
 * last bit (KHRN_INTERLOCK_PARTS_REST) corresponds to the rest of the parts. */
typedef uint32_t khrn_interlock_parts_t;
#define KHRN_INTERLOCK_PARTS_ALL ((khrn_interlock_parts_t)-1)
#define KHRN_INTERLOCK_PARTS_BITS (sizeof(khrn_interlock_parts_t) * 8)
#define KHRN_INTERLOCK_PARTS_REST ((khrn_interlock_parts_t)1 << (KHRN_INTERLOCK_PARTS_BITS - 1))

/* Make a khrn_interlock_parts_t covering parts i through i+num-1. Note that
 * the returned value may cover more parts than requested -- if any of the
 * requested parts does not directly correspond to a bit in
 * khrn_interlock_parts_t, KHRN_INTERLOCK_PARTS_REST will be set in the
 * returned value. */
static inline khrn_interlock_parts_t khrn_interlock_parts(unsigned i, unsigned num)
{
   assert(num);
   if (i >= KHRN_INTERLOCK_PARTS_BITS)
      return KHRN_INTERLOCK_PARTS_REST;
   khrn_interlock_parts_t parts = (num < KHRN_INTERLOCK_PARTS_BITS) ?
      (KHRN_INTERLOCK_PARTS_ALL >> (KHRN_INTERLOCK_PARTS_BITS - num)) :
      KHRN_INTERLOCK_PARTS_ALL;
   return parts << i;
}

struct khrn_interlock
{
   // KHRN_INTERLOCK_BITS_PER_RS bits per render-state to indicate which
   // stages are reading this interlock.
   uint64_t readers;

   // KHRN_INTERLOCK_BITS_PER_RS bits per render-state to indicate which
   // stages are writing this interlock.
   // There can be only one render-state writing an interlock.
   uint64_t writer;

   /*
    * If you add a writer, everything is always flushed.
    *
    * If you add a reader, then any existing writer is flushed. But if the
    * interlock currently has readers instead of a writer, you just join the
    * list of readers, and don't need to flush it yet.
    */

   /* Which parts of the resource will have invalid contents after all writers
    * have been flushed and waited for? This is conservative -- flushing may
    * cause more parts to be marked as invalid and invalid parts need not be
    * marked as such. */
   khrn_interlock_parts_t invalid_parts;

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

void khrn_interlock_init(KHRN_INTERLOCK_T *interlock);

/*
 * Flush whatever render-states the interlock has (whether it's a writer or a set of readers).
 */
void khrn_interlock_flush(KHRN_INTERLOCK_T *interlock);

/*
 * Flush the writer of an interlock if it has one.
 */
void khrn_interlock_flush_writer(KHRN_INTERLOCK_T *interlock);

/*
 * Add rs as a reader of interlock for the specified stages.
 *
 * If there is a writer (that isn't rs), it is flushed.
 */
void khrn_interlock_add_reader(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_stages_t stages, KHRN_RENDER_STATE_T *rs);

/*
 * Prepare the resource protected by the interlock for instant reading. This
 * means flush out any existing writers, and actually waits.
 */
void khrn_interlock_read_now(KHRN_INTERLOCK_T *interlock);

/*
 * Add a writer for the specified stages, flushing any existing
 * different writer or readers.
 *
 * If rs was already the reader, other readers are flushed, and rs still
 * becomes the writer, but rs is not flushed.
 *
 * parts should cover all the parts which might be written. true is returned
 * iff all the parts which might be written were marked as invalid.
 */
bool khrn_interlock_add_writer(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_stages_t stages, KHRN_RENDER_STATE_T *rs,
      khrn_interlock_parts_t parts);

/*
 * If interlock is already marked as being read by rs, returns false immediately.
 * Otherwise, calls khrn_interlock_add_writer() and returns true.
 * parts should cover all the parts which might be written.
 */
bool khrn_interlock_add_self_read_conflicting_writer(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_stages_t stages, KHRN_RENDER_STATE_T *rs,
      khrn_interlock_parts_t parts);

/* Begin submitting jobs that read data guarded by this interlock.
   The next call must be to khrn_interlock_end_submit_reader_jobs().
   Returns the dependencies for the job to be submitted. */
v3d_scheduler_deps const* khrn_interlock_begin_submit_reader_jobs(KHRN_INTERLOCK_T *interlock);

/* End submitting jobs that read data guarded by this interlock by providing
 * the job-ids of those jobs;
 * call with success = false and deps->n = 0, if we've failed submitting and reader jobs
 * call deps->n != 0, if we've submitted any asynchonrous reader jobs
 * call with success = true and deps->n = 0 if we've done synchronous reads from the data
 *                                          protected by this interlock
 */
void khrn_interlock_end_submit_reader_jobs(KHRN_INTERLOCK_T *interlock,
      bool success, v3d_scheduler_deps const* deps);

/* Begin submitting jobs that write data guarded by this interlock.
   The next call must be to khrn_interlock_end_submit_writer_jobs().
   Returns the dependencies for the job to be submitted. */
v3d_scheduler_deps const* khrn_interlock_begin_submit_writer_jobs(KHRN_INTERLOCK_T *interlock);

/* End submitting jobs that write data guarded by this interlock by providing the
 * job-ids of those jobs.
 * call with success = false and deps->n = 0, if we've failed submitting and writer jobs
 * call deps->n != 0, if we've submitted any asynchonrous writer jobs
 * call with success = true and deps->n = 0 if we've done synchronous writes to the data
 *                                           protected by this interlock
 * parts should cover all the parts which might be/have been written by the
 * jobs.
 */
void khrn_interlock_end_submit_writer_jobs(KHRN_INTERLOCK_T *interlock,
      bool success, v3d_scheduler_deps const* deps,
      khrn_interlock_parts_t parts);

/*
 * Prepare the resource protected by the interlock for instant writing. This
 * means flush out any existing writers or readers, and actually wait.
 * parts should cover all the parts which might be written.
 */
void khrn_interlock_write_now(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_parts_t parts);

/*
 * Notify the interlock of an unsynchronised write now.
 * parts should cover all the parts which might be written.
 */
static inline void khrn_interlock_write_now_unsynchronised(
   KHRN_INTERLOCK_T *interlock,
   khrn_interlock_parts_t parts);

/*
 * Replaces all dependencies with a single job. Only use this when you know
 * the job depends on everything in the interlock. This will normally only
 * be the case when the job is a writer.
 */
void khrn_interlock_job_replace(KHRN_INTERLOCK_T *interlock, uint64_t job_id);

//! If rs is the writer or one of the readers, remove it.
void khrn_interlock_remove_rs(KHRN_INTERLOCK_T *interlock,
      const KHRN_RENDER_STATE_T *rs);

//! Get dependencies for render-state stages from this interlock.
//! Dependencies are expected to be made transitive, so only the first stage is updated.
void khrn_interlock_get_deps_for_rs(KHRN_INTERLOCK_T *interlock,
   const KHRN_RENDER_STATE_T *rs, v3d_scheduler_deps stage_deps[KHRN_INTERLOCK_NUM_STAGES]);

//! Update the interlock with job-ids from the render-state and remove it.
//! Returns true if V3D is writing interlock.
bool khrn_interlock_update_from_rs(KHRN_INTERLOCK_T *interlock,
      const KHRN_RENDER_STATE_T *rs, uint64_t const stage_jobs[KHRN_INTERLOCK_NUM_STAGES]);

//! Returns the stages that rs has on interlock for reading or writing.
khrn_interlock_stages_t khrn_interlock_get_stages(
      const KHRN_INTERLOCK_T *interlock, KHRN_RENDER_STATE_T const* rs);

//! Returns the stages that rs has on interlock for reading.
khrn_interlock_stages_t khrn_interlock_get_read_stages(
      const KHRN_INTERLOCK_T* interlock, KHRN_RENDER_STATE_T const* rs);

//! Returns the stages that rs has on interlock for writing.
khrn_interlock_stages_t khrn_interlock_get_write_stages(
      const KHRN_INTERLOCK_T* interlock, KHRN_RENDER_STATE_T const* rs);

/* Call this when the underlying resource's contents have become invalid. Note
 * this will flush the writer if there is one. */
void khrn_interlock_invalidate(KHRN_INTERLOCK_T *interlock, khrn_interlock_parts_t parts);

//! Returns true if interlock has this render-state as a reader.
bool khrn_interlock_is_reader(KHRN_INTERLOCK_T const* interlock, KHRN_RENDER_STATE_T const* rs);

//! Returns true if interlock has this render-state as a writer.
bool khrn_interlock_is_writer(KHRN_INTERLOCK_T const* interlock, KHRN_RENDER_STATE_T const* rs);

//! Returns true if interlock has a reader.
static inline bool khrn_interlock_has_reader(KHRN_INTERLOCK_T const* interlock);

//! Returns true if interlock has a writer.
static inline bool khrn_interlock_has_writer(KHRN_INTERLOCK_T const* interlock);

//! Returns true if interlock has a reader or writer.
static inline bool khrn_interlock_has_reader_or_writer(KHRN_INTERLOCK_T const* interlock);

//! Returns true if khrn_interlock_read_now() would stall.
static inline bool khrn_interlock_read_now_would_stall(KHRN_INTERLOCK_T * interlock);

//! Returns true if khrn_interlock_write_now() would stall.
static inline bool khrn_interlock_write_now_would_stall(KHRN_INTERLOCK_T* interlock);

#ifdef __cplusplus
}
#endif

#include "khrn_interlock.inl"
