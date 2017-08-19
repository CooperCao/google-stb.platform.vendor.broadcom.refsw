/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef BCM_SCHED_API_H_
#define BCM_SCHED_API_H_
#include "bcm_sched_job.h"
#include "vcos_mutex.h"
#include "vcos_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum bcm_wait_status
{
   BCM_WaitJobDone,
   BCM_WaitJobTimeout
} bcm_wait_status;

/* Job IDs have process scope. Inter-process dependencies must use fences. */

/* A job has an array of jobs on which it depends.
 * Whilst it has dependencies, it is 'conditional'.
 * Once all its dependencies have completed, it becomes
 * 'unconditional', and will be issued to the device driver for its type.
 */
extern void bcm_sched_queue_jobs(
	const struct bcm_sched_job *jobs,
   unsigned num_jobs);

/* Submits a bin and render together */
/* This is only way for the binning memory to be passed to a render */
/* A single bin will have its binning memory freed upon completion */
extern void bcm_sched_queue_bin_render(
	const struct bcm_sched_job *bin,
	const struct bcm_sched_job *render);

/* Query if completed_deps are completed and finalised_deps are finalised */
extern void bcm_sched_query(
   struct bcm_sched_dependencies *completed_deps,
   struct bcm_sched_dependencies *finalised_deps,
   struct bcm_sched_query_response *response);

typedef void (*bcm_update_oldest_nfid_fn) (uint64_t oldest_nfid);
extern void bcm_sched_register_update_oldest_nfid(bcm_update_oldest_nfid_fn update);

/* If force_create is false, may return V3D_PLATFORM_NULL_FENCE indicating that
 * the specified jobs have already completed/finalised, but never fails.
 *
 * If force_create is true, may fail, returning V3D_PLATFORM_NULL_FENCE. */
extern int bcm_sched_create_fence(
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps,
   bool force_create);

/* Wait for a non-finalised job to be finalised
 * Returns true if a job has been waited for
 * Returns false if all job have been finalised */
extern bool bcm_sched_wait_for_any_non_finalised(void);

/* Wait until one of the completed_deps or finalised_deps has reached its
 * corresponding state or the function times out */
extern bcm_wait_status bcm_sched_wait_any_job_timeout(
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps,
   int timeout);

/* Wait until all the completed_deps and finalised_deps have reached their
 * corresponding state. */
extern void bcm_sched_wait_jobs(
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps);

/* Wait until all the completed_deps and finalised_deps have reached either
 * completed or finalised state respectively or the function times out */
extern bcm_wait_status bcm_sched_wait_jobs_timeout(
   const struct bcm_sched_dependencies *completed_deps,
   const struct bcm_sched_dependencies *finalised_deps,
   int timeout);

/* define a return value for invalid scheduler event id */
#define V3D_INVALID_SCHED_EVENT_ID (~(uint64_t)0)

/* Creates a scheduler event and returns its id */
extern bcm_sched_event_id bcm_sched_new_event(void);

/* Delete a scheduler event using its id */
extern void bcm_sched_delete_event(bcm_sched_event_id event_id);

/* Signal an event from the host using its id */
extern void bcm_sched_set_event(bcm_sched_event_id event_id);

/* Unsignal an event from the host using its id */
extern void bcm_sched_reset_event(bcm_sched_event_id event_id);

/* Return true if the event is signalled or false otherwise */
extern bool bcm_sched_query_event(bcm_sched_event_id event_id);

#ifdef __cplusplus
}

#endif

#endif /* BCM_SCHED_API_H_ */
