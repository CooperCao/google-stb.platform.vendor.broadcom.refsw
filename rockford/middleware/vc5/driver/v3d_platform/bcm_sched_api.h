/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
Job scheduler API
=============================================================================*/
#ifndef BCM_SCHED_API_H_
#define BCM_SCHED_API_H_
#include "bcm_sched_job.h"
#include "vcos_mutex.h"
#include "vcos_thread.h"

#ifdef __cplusplus
extern "C" {
#endif

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

#ifdef __cplusplus
}

#endif

#endif /* BCM_SCHED_API_H_ */
