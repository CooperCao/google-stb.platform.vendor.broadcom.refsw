/*=============================================================================
Copyright (c) 2008 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos
Module   :  Interlock system

FILE DESCRIPTION
Interlock system to ensure correct ordering of reads/writes.
=============================================================================*/

#include "khrn_interlock.h"
#include "khrn_process.h"
#include "libs/platform/v3d_scheduler.h"

static inline void khrn_interlock_write_now_unsynchronised(
   KHRN_INTERLOCK_T *interlock,
   khrn_interlock_parts_t parts)
{
   assert(!interlock->in_begin_flags);
   interlock->invalid_parts &= ~parts;
}

static inline bool khrn_interlock_has_reader(KHRN_INTERLOCK_T const* interlock)
{
   return interlock->readers != 0;
}

static inline bool khrn_interlock_has_writer(KHRN_INTERLOCK_T const* interlock)
{
   return interlock->writer != 0;
}

static inline bool khrn_interlock_has_reader_or_writer(KHRN_INTERLOCK_T const* interlock)
{
   return interlock->readers != 0 || interlock->writer != 0;
}

static inline bool khrn_interlock_read_now_would_stall(KHRN_INTERLOCK_T* interlock)
{
   return khrn_interlock_has_writer(interlock)
      || !v3d_scheduler_jobs_reached_state(&interlock->pre_read,
            V3D_SCHED_DEPS_COMPLETED, false);
}

static inline bool khrn_interlock_write_now_would_stall(KHRN_INTERLOCK_T* interlock)
{
   /* Workaround to avoid needless renames due to stack side of scheduler
    * not knowing that deps have completed until they are finalised. */
   bool const call_kernel = true;

   return khrn_interlock_has_reader_or_writer(interlock)
      || !v3d_scheduler_jobs_reached_state(&interlock->pre_write,
            V3D_SCHED_DEPS_COMPLETED, call_kernel);
}
