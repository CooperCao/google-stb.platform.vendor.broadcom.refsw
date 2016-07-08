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

static inline bool khrn_interlock_read_now_would_flush(KHRN_INTERLOCK_T* interlock)
{
   return interlock->is_writer;
}

static inline bool khrn_interlock_write_now_would_flush(KHRN_INTERLOCK_T* interlock)
{
   return interlock->is_writer
      ||  (interlock->users.readers.bin | interlock->users.readers.render) != 0;
}

static inline bool khrn_interlock_read_now_would_stall(KHRN_INTERLOCK_T* interlock)
{
   return khrn_interlock_read_now_would_flush(interlock)
      || !v3d_scheduler_jobs_reached_state(&interlock->pre_read,
            V3D_SCHED_DEPS_COMPLETED, false);
}

static inline bool khrn_interlock_write_now_would_stall(KHRN_INTERLOCK_T* interlock)
{
   return khrn_interlock_write_now_would_flush(interlock)
      || !v3d_scheduler_jobs_reached_state(&interlock->pre_write,
            V3D_SCHED_DEPS_COMPLETED, false);
}
