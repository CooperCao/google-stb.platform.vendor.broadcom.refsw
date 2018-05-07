/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BVC5_TRACE_JOBS_H__
#define BVC5_TRACE_JOBS_H__

#include "bvc5.h"

/* we start to track jobs in wait state, since we create jobs and
 * immediately move them to wait queue
 */
typedef enum BVC5_JobPhase
{
   BVC5_JobPhase_Runnable,
   BVC5_JobPhase_HwQueued,
   BVC5_JobPhase_HwDone,
   BVC5_JobPhase_Completed,
   BVC5_JobPhase_Finalised,
   BVC5_JobPhase_eNumJobPhase
} BVC5_JobPhase;

/* #define BVC5_HAS_TRACE */
#ifdef BVC5_HAS_TRACE
extern void internal_trace_job_start(const BVC5_JobBase *job, uint32_t client_id);
extern void internal_trace_job_new_phase(const BVC5_JobBase *job, uint32_t client_id, BVC5_JobPhase job_phase);
extern void internal_trace_job_bin_oom(const BVC5_JobBase *job, uint32_t client_id);

extern void internal_trace_job_fence_created(const void *psFence,
      const void *signal_data, uint32_t client_id,
      const BVC5_SchedDependencies *completed, const BVC5_SchedDependencies *finalised);
extern void internal_trace_job_fence_signalled(const void *psFence,
      const void *signal_data, uint32_t client_id);
#else
static __inline__ void internal_trace_job_start(const BVC5_JobBase *job, uint32_t client_id)
{
   BSTD_UNUSED(job);
   BSTD_UNUSED(client_id);
}
static __inline__ void internal_trace_job_new_phase(const BVC5_JobBase *job, uint32_t client_id, BVC5_JobPhase job_phase)
{
   BSTD_UNUSED(job);
   BSTD_UNUSED(client_id);
   BSTD_UNUSED(job_phase);
}

static __inline__ void internal_trace_job_bin_oom(const BVC5_JobBase *job, uint32_t client_id)
{
   BSTD_UNUSED(job);
   BSTD_UNUSED(client_id);
}

static __inline__ void internal_trace_job_fence_created(const void *psFence,
      const void *signal_data, uint32_t client_id,
      const BVC5_SchedDependencies *completed, const BVC5_SchedDependencies *finalised)
{
   BSTD_UNUSED(psFence);
   BSTD_UNUSED(signal_data);
   BSTD_UNUSED(client_id);
   BSTD_UNUSED(completed);
   BSTD_UNUSED(finalised);
}

static __inline__ void internal_trace_job_fence_signalled(const void *psFence, const void *signal_data, uint32_t client_id)
{
   BSTD_UNUSED(psFence);
   BSTD_UNUSED(signal_data);
   BSTD_UNUSED(client_id);
}
#endif
#endif
