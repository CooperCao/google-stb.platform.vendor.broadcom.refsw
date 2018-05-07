/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <string.h>
#include "bcm_sched_job.h"
#include "bcm_sched_api.h"
#include "libs/core/v3d/v3d_limits.h"
#include "libs/core/v3d/v3d_addr.h"
#include "libs/core/v3d/v3d_tfu.h"
#include "v3d_platform.h"

#ifdef __cplusplus
extern "C" {
#endif

#define V3D_SCHEDULER_COMPUTE_MIN_SHARED_MEM_PER_CORE (32u*1024u)

typedef struct bcm_sched_dependencies v3d_scheduler_deps;
typedef bcm_completion_fn v3d_sched_completion_fn;
typedef bcm_user_fn v3d_sched_user_fn;

#define V3D_SCHED_JOB_SUCCESS  BCM_SCHED_JOB_SUCCESS
#define V3D_SCHED_JOB_OUT_OF_MEMORY  BCM_SCHED_JOB_OUT_OF_MEMORY
#define V3D_SCHED_JOB_ERROR   BCM_SCHED_JOB_ERROR
typedef enum bcm_sched_job_error v3d_sched_job_error;

typedef enum
{
   V3D_SCHED_DEPS_COMPLETED, /* Dependendecies have completed but may not have finalised */
   V3D_SCHED_DEPS_FINALISED  /* Dependendencies have finalised and all their
                                resources have been tidied */
}v3d_sched_deps_state;

/* Terms:
 * A job is considered in state complete when the computational/hardware work is done or some error occured.
 * A completion callback function is called after a job completes.
 * A job is considered in state "finalised" after the completion callback function has run.
 */

/* Submits a TFU conversion job.
 * The job will run after all jobs specified in deps complete;
 * If a completion function is supplied, it will be called after this job
 * completes and deps are finalised.
 */
uint64_t v3d_scheduler_submit_tfu_job(
   const v3d_scheduler_deps *deps,
   v3d_cache_ops cache_ops,
   const V3D_TFU_COMMAND_T *tfu_cmd,
   bool secure,
   v3d_sched_completion_fn completion, void *data);

#if V3D_USE_CSD

/* Submits compute shader job.
 * The job will run after all job specified in deps complete;
 * If a completion function is supplied, it will be called after this job
 * completes and deps are finalised.
 */
uint64_t v3d_scheduler_submit_compute_job(
   const v3d_scheduler_deps *deps,
   v3d_cache_ops cache_ops,
   const v3d_compute_subjob* subjobs,
   unsigned num_subjobs,
   bool secure,
   v3d_sched_completion_fn completion, void *data);

/* Returns a new kernel object that holds compute subjobs. The number of subjobs defaults to 0. */
v3d_compute_subjobs_id v3d_scheduler_new_compute_subjobs(unsigned max_subjobs);

/* Update the subjobs in the kernel object. Must specify num_subjobs > 0. */
void v3d_scheduler_update_compute_subjobs(v3d_compute_subjobs_id subjobs_id, const v3d_compute_subjob* subjobs, unsigned num_subjobs);

/* Submits an indirect compute shader job.
 * The job will run after all job specified in deps complete;
 * If a completion function is supplied, it will be called after this job
 * completes and deps are finalised.
 * The subjobs are stored in a handle created using new_compute_subjobs
 * and updated using update_compute_subjobs.
 * This handle is automatically read and deleted by the indirect compute job
 * once the dependencies for the compute job have been met.
 */
uint64_t v3d_scheduler_submit_indirect_compute_job(
   const v3d_scheduler_deps *deps,
   v3d_cache_ops cache_ops,
   v3d_compute_subjobs_id subjobs_id,
   bool secure,
   v3d_sched_completion_fn completion, void *data);

#endif

typedef struct
{
   v3d_size_t tile_alloc_layer_stride;
   void* bin_gmp_table;
   v3d_cache_ops bin_cache_ops;     // Flushes executed after bin-deps are met, before job execution.
                                    // Cleans executed after job execution, before reported completed.


   void* render_gmp_table;
   v3d_cache_ops render_cache_ops;  // Flushes executed after render-deps are met, before job execution.
                                    // Cleans executed after job execution, before reported completed.
   v3d_empty_tile_mode empty_tile_mode;

#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool bin_workaround_gfxh_1181;
   bool render_workaround_gfxh_1181;
#endif
   bool bin_no_overlap;                   // If true, do not run this bin job concurrently on the same core with other job types.
   bool render_no_overlap;                // If true, do not run this render job concurrently on the same core with other job types.
   bool secure;
   bool render_depends_on_bin;            // Make the render job dependent on the bin job. This is
                                          // required if the bin cache cleans must happen before
                                          // render cache flushes.

   unsigned min_initial_bin_block_size;
#if V3D_VER_AT_LEAST(4,1,34,0)
   unsigned bin_tile_state_size;
#endif
}v3d_bin_render_details;

/* bin_render  cle layout info */
typedef struct
{
   v3d_subjobs_list  bin_subjobs;

   unsigned num_layers;
   v3d_subjobs_list render_subjobs;

   v3d_bin_render_details details;

} V3D_BIN_RENDER_INFO_T;

/* render  cle layout info */
typedef struct
{
   v3d_subjobs_list subjobs_list;

   void* gmp_table;
   v3d_cache_ops cache_ops;
   v3d_empty_tile_mode empty_tile_mode;
#if !V3D_VER_AT_LEAST(3,3,0,0)
   bool workaround_gfxh_1181;
#endif
   bool no_overlap;  // If true, do not run this render job concurrently on the same core with other job types.
   bool secure;
} V3D_RENDER_INFO_T;

/* Submits a render-only job */
uint64_t v3d_scheduler_submit_render_job(
   const v3d_scheduler_deps* deps,
   const V3D_RENDER_INFO_T* info,
   v3d_sched_completion_fn completion, void *data);

/*
 * Submits a bin and render job.
 * The bin job will run after all the jobs specified in bin_deps are complete;
 * The render job will run after the bin job completes and the jobs specified
 * in render_deps complete;
 * If completion functions are supplied, they will run after the
 * corresponding job completes and the corresponding deps are finalised.
 */
void v3d_scheduler_submit_bin_render_job(
   const v3d_scheduler_deps *bin_deps,
   const v3d_scheduler_deps *render_deps,
   uint64_t *bin_job,
   uint64_t *render_job,
   const V3D_BIN_RENDER_INFO_T *br_info,
   v3d_sched_completion_fn bin_completion, void *bin_compl_data,
   v3d_sched_completion_fn render_completion, void *render_compl_data);

/* Submits a user mode job.
 * The user function will be called when jobs specified in deps complete
 */
uint64_t v3d_scheduler_submit_usermode_job(
   const v3d_scheduler_deps *deps,     // completed
   v3d_sched_user_fn user_fn, void *data);
uint64_t v3d_scheduler_submit_usermode_job2(
   const v3d_scheduler_deps *completed_deps,
   const v3d_scheduler_deps *finalised_deps,
   v3d_sched_user_fn user_fn, void *data);

/* Submits a barrier job. */
uint64_t v3d_scheduler_submit_barrier_job(
   const v3d_scheduler_deps *deps,
   v3d_cache_ops cache_ops);

#if !V3D_PLATFORM_SIM
/* Behaves like a fence that has already been signaled. */
#define V3D_PLATFORM_NULL_FENCE (-1)

/* Creates a fence that will be signaled when all the jobs specified in deps
 * achieve the specified state.
 *
 * Once a fence is created, the caller becomes the owner of this fence.
 * The caller can relinquish ownership of the fence by calling
 * v3d_scheduler_submit_wait_fence or using the appropriate platform API.
 *
 * If force_create is false, this function will never fail, though it may block
 * waiting for the jobs to complete if a fence cannot be allocated. If
 * V3D_PLATFORM_NULL_FENCE is returned, that means the jobs specified in deps
 * have achieved the specified state. V3D_PLATFORM_NULL_FENCE behaves like a
 * fence that has already been signaled, so you generally don't need to handle
 * this return value specially. Be aware though that it may not be accepted by
 * code outside of the GL stack, so eg should not be returned from
 * eglDupNativeFenceFDAndroid.
 *
 * If force_create is true, this function may fail, returning
 * V3D_PLATFORM_NULL_FENCE. If it succeeds it will always return a real
 * platform fence that is accepted everywhere (so eg can be returned from
 * eglDupNativeFenceFDAndroid). */
int v3d_scheduler_create_fence(const v3d_scheduler_deps *deps,
      v3d_sched_deps_state deps_state, bool force_create);

/* Submit a job that will wait for the given fence;
 * The scheduler gets ownership of the fence and is responsible for closing the
 * fence. */
uint64_t v3d_scheduler_submit_wait_fence(int fence);
#endif

/* Submits a null job.
 * A null job is a new job that depends on the specified dependencies;
 * It can be used to replaces a set of dependencies with one job id (since
 * v3d_scheduler_deps has a maxium of V3D_SCHED_MAX_DEPENDENCIES] )
 * If a completion function is supplied, it will be called after this job
 * completes and deps are finalised
 */
uint64_t v3d_scheduler_submit_null_job(const v3d_scheduler_deps *deps,
      v3d_sched_completion_fn completion, void *data);

/* free a gmem_handle when all the jobs specified in deps are finalised */
void v3d_scheduler_gmem_deferred_free(v3d_scheduler_deps *deps, gmem_handle_t handle);

/*  Return true if all the jobs specified in deps have reached state deps_state
 *  If call_kernel = false, the returned result is based on the information the
 *  stack has about jobs.
 *  If call_kernel = true, a call into the kernel scheduler will be made to
 *  find out the state of queried dependencies.
 */
bool v3d_scheduler_jobs_reached_state(v3d_scheduler_deps *deps,
      v3d_sched_deps_state deps_state, bool call_kernel);

/*  Wait for jobs specified in deps to have state deps_state */
void v3d_scheduler_wait_jobs(v3d_scheduler_deps *deps, v3d_sched_deps_state deps_state);

/*  Wait for jobs specified in deps to have state deps_state, returns false if timed-out. */
bool v3d_scheduler_wait_jobs_timeout(v3d_scheduler_deps *deps, v3d_sched_deps_state deps_state, int timeout);

/* Wait for an outstanding job to finalise; Return true if a job was found */
bool v3d_scheduler_wait_any(void);

/* Wait for any job specified in deps to have state deps_state, returns false if timed-out. */
bool v3d_scheduler_wait_any_job_timeout(v3d_scheduler_deps *deps, v3d_sched_deps_state deps_state, int timeout);

/* Wait for all outstanding jobs to finalise */
void v3d_scheduler_wait_all(void);

/* Flush any batched jobs. */
void v3d_scheduler_flush(void);

/* Merges two lists of dependencies, creating a null job if required. */
void v3d_scheduler_merge_deps(v3d_scheduler_deps *a, const v3d_scheduler_deps *b);

/* Initialise dependencies. */
static inline void v3d_scheduler_deps_init(v3d_scheduler_deps* deps) { deps->n = 0; }

/* Replace the existing dependencies from deps with a job id. */
void v3d_scheduler_deps_set(v3d_scheduler_deps *deps, uint64_t job);

/* Add a job as a to the list of dependencies creating a null job if required. */
void v3d_scheduler_add_dep(v3d_scheduler_deps *deps, uint64_t job);

/* Copy src dependencies over dst dependencies. */
static inline void v3d_scheduler_copy_deps(v3d_scheduler_deps *dst, const v3d_scheduler_deps *src)
{
   if (src == dst)
      return;

   dst->n = src->n;
   if (src->n != 0)
   {
      memcpy(dst->dependency, src->dependency, sizeof(dst->dependency[0]) * src->n);
   }
}

const V3D_HUB_IDENT_T* v3d_scheduler_get_hub_identity(void);
const V3D_IDENT_T* v3d_scheduler_get_identity(void);
uint32_t v3d_scheduler_get_ddr_map_ver(void);

uint32_t v3d_scheduler_get_soc_quirks(void);

/* Create a new scheduler event that can be set/reset/query from the host */
/* and can be wait on/set/reset from the device (using jobs) */
bcm_sched_event_id v3d_scheduler_new_event(void);
/* An event should only be deleted once all jobs using this event have been completed */
/* as event jobs do not maintain a reference to their event */
void v3d_scheduler_delete_event(bcm_sched_event_id event_id);
void v3d_scheduler_set_event(bcm_sched_event_id event_id);
void v3d_scheduler_reset_event(bcm_sched_event_id event_id);
bool v3d_scheduler_query_event(bcm_sched_event_id event_id);
/* Create a job that will wait before to be executed for all dependencies to be completed
 * and then for the event to be signalled */
uint64_t v3d_scheduler_submit_wait_on_event_job(const v3d_scheduler_deps* deps, bcm_sched_event_id event_id);
/* Create a job that will wait before to be executed for all dependencies to be completed
 * and then will signal the event */
uint64_t v3d_scheduler_submit_set_event_job(const v3d_scheduler_deps* deps, bcm_sched_event_id event_id);
/* Create a job that will wait before to be executed for all dependencies to be completed
 * and then will unsignal the event */
uint64_t v3d_scheduler_submit_reset_event_job(const v3d_scheduler_deps* deps, bcm_sched_event_id event_id);

//! Return amount of shared memory per-core for this process.
uint32_t v3d_scheduler_get_compute_shared_mem_size_per_core(void);

#if !V3D_USE_L2T_LOCAL_MEM
//! Return compute shared memory for this process. If !alloc, will return
//! GMEM_INVALID_HANDLE unless shared memory already allocated.
gmem_handle_t v3d_scheduler_get_compute_shared_mem(bool secure, bool alloc);
#endif

#if V3D_USE_L2T_LOCAL_MEM
//! Return compute shared memory address.
static inline v3d_addr_t v3d_scheduler_get_compute_shared_mem_addr(void)
{
   return gmem_get_l2t_local_mem_addr();
}
#endif

#ifdef __cplusplus
}
#endif
