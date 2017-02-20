/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "../egl/egl_platform.h"
#include "khrn_int_common.h"
#include "khrn_interlock.h"
#include "khrn_process.h"
#include "khrn_render_state.h"

#include "libs/platform/v3d_scheduler.h"

static inline uint64_t get_render_state_mask(unsigned rs_index)
{
   return gfx_mask64(KHRN_INTERLOCK_BITS_PER_RS) << rs_index*KHRN_INTERLOCK_BITS_PER_RS;
}

static inline unsigned get_next_render_state_index(uint64_t rs_stages)
{
   return gfx_msb64(rs_stages) / KHRN_INTERLOCK_BITS_PER_RS;
}

static inline uint64_t encode_stages(khrn_interlock_stages_t stages, unsigned rs_index)
{
   return (uint64_t)stages << rs_index*KHRN_INTERLOCK_BITS_PER_RS;
}

static inline khrn_interlock_stages_t decode_stages(uint64_t rs_stages, unsigned rs_index)
{
   return (khrn_interlock_stages_t)((uint32_t)(rs_stages >> rs_index*KHRN_INTERLOCK_BITS_PER_RS) & gfx_mask(KHRN_INTERLOCK_BITS_PER_RS));
}

static inline void verify_interlock(KHRN_INTERLOCK_T* interlock)
{
#ifndef NDEBUG
   // If writer..
   if (interlock->writer)
   {
      // then can be only reader (and writer).
      unsigned writer_index = get_next_render_state_index(interlock->writer);
      uint64_t other_mask = ~get_render_state_mask(writer_index);
      assert(!(interlock->readers & other_mask));
      assert(!(interlock->writer & other_mask));
   }
#endif
}

static void flush_render_states(KHRN_INTERLOCK_T* interlock, uint64_t rs_stages)
{
   assert(((interlock->readers | interlock->writer) & rs_stages) == rs_stages);

   do
   {
      unsigned rs_index = get_next_render_state_index(rs_stages);
      khrn_render_state_flush(&render_states[rs_index]);
      assert( !((interlock->readers | interlock->writer) & get_render_state_mask(rs_index)) );

      // Reload from interlock in case additional render-states have been flushed.
      rs_stages &= interlock->readers | interlock->writer;
   }
   while (rs_stages);
}

void khrn_interlock_init(KHRN_INTERLOCK_T *interlock)
{
   memset(interlock, 0, sizeof *interlock);
}

void khrn_interlock_flush(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   uint64_t users = interlock->readers | interlock->writer;
   if (users)
      flush_render_states(interlock, users);

   assert(!interlock->writer && !interlock->readers);
}

void khrn_interlock_flush_writer(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   if (interlock->writer)
   {
      flush_render_states(interlock, interlock->writer);
      assert(!interlock->writer && !interlock->readers);
   }
}

void khrn_interlock_add_reader(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_stages_t stages, KHRN_RENDER_STATE_T *rs)
{
   assert(!interlock->in_begin_flags);
   assert(stages != 0);

   unsigned rs_index = khrn_render_state_get_index(rs);

   uint64_t other_writer = interlock->writer & ~get_render_state_mask(rs_index);
   if (other_writer)
      flush_render_states(interlock, other_writer);

   interlock->readers |= encode_stages(stages, rs_index);
   verify_interlock(interlock);
}

bool khrn_interlock_add_writer(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_stages_t stages, KHRN_RENDER_STATE_T *rs,
      khrn_interlock_parts_t parts)
{
   assert(!interlock->in_begin_flags);
   assert(stages != 0);

   unsigned rs_index = khrn_render_state_get_index(rs);

   // Flush any other render-states using this interlock.
   uint64_t other_users = (interlock->readers | interlock->writer) & ~get_render_state_mask(rs_index);
   if (other_users)
      flush_render_states(interlock, other_users);

   interlock->writer |= encode_stages(stages, rs_index);
   verify_interlock(interlock);

   bool all_invalid = (interlock->invalid_parts & parts) == parts;
   interlock->invalid_parts &= ~parts;
   return all_invalid;
}

bool khrn_interlock_add_self_read_conflicting_writer(KHRN_INTERLOCK_T *interlock,
      khrn_interlock_stages_t stages, KHRN_RENDER_STATE_T *rs,
      khrn_interlock_parts_t parts)
{
   assert(!interlock->in_begin_flags);
   assert(rs != NULL);
   assert(stages != 0);

   unsigned rs_index = khrn_render_state_get_index(rs);

   /* If writing only during render, no need to conflict with bin reads -- they
    * will be done before we even start writing */
   khrn_interlock_stages_t conflicting_read_stages = ~(gfx_lowest_bit(stages) - 1);
   if (decode_stages(interlock->readers, rs_index) & conflicting_read_stages)
      return false;

   khrn_interlock_add_writer(interlock, stages, rs, parts);
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
      bool success, v3d_scheduler_deps const* deps,
      khrn_interlock_parts_t parts)
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
   if (success)
      interlock->invalid_parts &= ~parts;
}

static inline void khrn_interlock_remove_rs_by_index(KHRN_INTERLOCK_T *interlock, unsigned rs_index)
{
   uint64_t rs_mask = ~get_render_state_mask(rs_index);
   interlock->readers &= rs_mask;
   interlock->writer &= rs_mask;
}

void khrn_interlock_remove_rs(KHRN_INTERLOCK_T *interlock,
      const KHRN_RENDER_STATE_T *rs)
{
   assert(!interlock->in_begin_flags);
   khrn_interlock_remove_rs_by_index(interlock, khrn_render_state_get_index(rs));
}

void khrn_interlock_get_deps_for_rs(KHRN_INTERLOCK_T *interlock,
      const KHRN_RENDER_STATE_T *rs, v3d_scheduler_deps stage_deps[KHRN_INTERLOCK_NUM_STAGES])
{
   unsigned rs_index = khrn_render_state_get_index(rs);

   // Transfer read deps from interlock to deps for first reader stage.
   khrn_interlock_stages_t read_stages = decode_stages(interlock->readers, rs_index);
   if (read_stages)
      v3d_scheduler_merge_deps(&stage_deps[gfx_msb(gfx_lowest_bit(read_stages))], &interlock->pre_read);

   // Transfer write deps from interlock to deps for first writer stage.
   khrn_interlock_stages_t write_stages = decode_stages(interlock->writer, rs_index);
   if (write_stages)
      v3d_scheduler_merge_deps(&stage_deps[gfx_msb(gfx_lowest_bit(write_stages))], &interlock->pre_write);
}

bool khrn_interlock_update_from_rs(KHRN_INTERLOCK_T *interlock,
      const KHRN_RENDER_STATE_T *rs, job_t const stage_jobs[KHRN_INTERLOCK_NUM_STAGES])
{
   bool v3d_writer = false;
   unsigned rs_index = khrn_render_state_get_index(rs);
   khrn_interlock_stages_t write_stages = decode_stages(interlock->writer, rs_index);
   if (write_stages)
   {
      // Replace the pre-read and pre-write dependencies with the job for the last writer stage.
      uint64_t last_write_job = stage_jobs[gfx_msb(write_stages)];
      assert(last_write_job != 0);
      v3d_scheduler_deps_set(&interlock->pre_write, last_write_job);
      v3d_scheduler_deps_set(&interlock->pre_read, last_write_job);
      v3d_writer = write_stages & (KHRN_INTERLOCK_STAGE_BIN | KHRN_INTERLOCK_STAGE_RENDER);
   }
   else
   {
      // Add the job for the last reader stage to the pre-write dependencies.
      khrn_interlock_stages_t read_stages = decode_stages(interlock->readers, rs_index);
      if (read_stages)
      {
         unsigned last_read_stage = gfx_msb(read_stages);
         uint64_t last_read_job = stage_jobs[last_read_stage];

         // It's possible for preprocess to not have a job ID if it ran immediately.
         assert((1 << last_read_stage) == KHRN_INTERLOCK_STAGE_PREPROCESS || last_read_job != 0);

         if (last_read_job != 0)
            v3d_scheduler_add_dep(&interlock->pre_write, last_read_job);
      }
   }

   // Remove the render-state.
   khrn_interlock_remove_rs_by_index(interlock, rs_index);
   return v3d_writer;
}

khrn_interlock_stages_t khrn_interlock_get_stages(
      const KHRN_INTERLOCK_T *interlock, KHRN_RENDER_STATE_T const* rs)
{
   assert(!interlock->in_begin_flags);
   unsigned rs_index = khrn_render_state_get_index(rs);
   return decode_stages(interlock->readers | interlock->writer, rs_index);
}

khrn_interlock_stages_t khrn_interlock_get_read_stages(
      KHRN_INTERLOCK_T const* interlock, KHRN_RENDER_STATE_T const* rs)
{
   assert(!interlock->in_begin_flags);
   unsigned rs_index = khrn_render_state_get_index(rs);
   return decode_stages(interlock->readers, rs_index);
}

khrn_interlock_stages_t khrn_interlock_get_write_stages(
      KHRN_INTERLOCK_T const* interlock, KHRN_RENDER_STATE_T const* rs)
{
   assert(!interlock->in_begin_flags);
   unsigned rs_index = khrn_render_state_get_index(rs);
   return decode_stages(interlock->writer, rs_index);
}

void khrn_interlock_invalidate(KHRN_INTERLOCK_T *interlock, khrn_interlock_parts_t parts)
{
   assert(!interlock->in_begin_flags);

   khrn_interlock_flush_writer(interlock);
   interlock->invalid_parts |= parts;
}

void khrn_interlock_read_now(KHRN_INTERLOCK_T *interlock)
{
   assert(!interlock->in_begin_flags);

   khrn_interlock_flush_writer(interlock);
   v3d_scheduler_wait_jobs(&interlock->pre_read, V3D_SCHED_DEPS_COMPLETED);
}

void khrn_interlock_write_now(KHRN_INTERLOCK_T *interlock, khrn_interlock_parts_t parts)
{
   assert(!interlock->in_begin_flags);

   khrn_interlock_flush(interlock);
   interlock->invalid_parts &= ~parts;
   v3d_scheduler_wait_jobs(&interlock->pre_write, V3D_SCHED_DEPS_COMPLETED);
}


bool khrn_interlock_is_reader(KHRN_INTERLOCK_T const* interlock, KHRN_RENDER_STATE_T const* rs)
{
   unsigned rs_index = khrn_render_state_get_index(rs);
   return (interlock->readers & get_render_state_mask(rs_index)) != 0;
}

bool khrn_interlock_is_writer(KHRN_INTERLOCK_T const* interlock, KHRN_RENDER_STATE_T const* rs)
{
   unsigned rs_index = khrn_render_state_get_index(rs);
   return (interlock->writer & get_render_state_mask(rs_index)) != 0;
}

void khrn_interlock_job_replace(KHRN_INTERLOCK_T *interlock, uint64_t job_id)
{
   assert(!interlock->readers && !interlock->writer);
   v3d_scheduler_deps_set(&interlock->pre_write, job_id);
   v3d_scheduler_deps_set(&interlock->pre_read, job_id);
}
