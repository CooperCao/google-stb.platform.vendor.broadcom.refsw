/*=============================================================================
Broadcom Proprietary and Confidential. (c)2014 Broadcom.
All rights reserved.

Project  :  v3d_scheduler

FILE DESCRIPTION
=============================================================================*/
#ifndef V3D_SCHEDULER_H
#define V3D_SCHEDULER_H

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

struct gmem_v3d_sync_list;

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
   const struct gmem_v3d_sync_list *sync_list,
   const V3D_TFU_COMMAND_T *tfu_cmd,
   bool secure,
   v3d_sched_completion_fn completion, void *data);

/* bin_render  cle layout info */
typedef struct
{
   unsigned int num_bins;
   v3d_addr_t bin_begins[V3D_MAX_BIN_SUBJOBS];
   v3d_addr_t bin_ends[V3D_MAX_BIN_SUBJOBS];

   unsigned int num_renders;
   v3d_addr_t render_begins[V3D_MAX_RENDER_SUBJOBS];
   v3d_addr_t render_ends[V3D_MAX_RENDER_SUBJOBS];
   v3d_empty_tile_mode empty_tile_mode;

   bool bin_workaround_gfxh_1181;
   bool render_workaround_gfxh_1181;
   bool secure;

   unsigned int min_initial_bin_block_size;
} V3D_BIN_RENDER_INFO_T;

/* render  cle layout info */
typedef struct
{
   unsigned int num_renders;
   v3d_addr_t render_begins[V3D_MAX_CORES];
   v3d_addr_t render_ends[V3D_MAX_CORES];
   v3d_empty_tile_mode empty_tile_mode;
   bool secure;
} V3D_RENDER_INFO_T;

/* Submits a render-only job */
uint64_t v3d_scheduler_submit_render_job(
   const v3d_scheduler_deps* deps,
   const struct gmem_v3d_sync_list* sync_list,
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
   const struct gmem_v3d_sync_list *bin_sync_list,
   const struct gmem_v3d_sync_list *render_sync_list,
   uint64_t *bin_job,
   uint64_t *render_job,
   const V3D_BIN_RENDER_INFO_T *br_info,
   v3d_sched_completion_fn bin_completion, void *bin_compl_data,
   v3d_sched_completion_fn render_completion, void *render_compl_data);

/* Submits a user mode job.
 * The user function will be called when jobs specified in deps complete
 */
uint64_t v3d_scheduler_submit_usermode_job(
   const v3d_scheduler_deps *deps,
   v3d_sched_user_fn user_fn, void *data);

/* Creates a fence that will be signaled when all the jobs specified in deps
 * achieve the specified state.
 *
 * If force_create is false, this function will never fail, though it may block
 * waiting for the jobs to complete if a fence cannot be allocated. If
 * V3D_PLATFORM_NULL_FENCE is returned, that means the jobs specified in deps
 * have achieved the specified state. V3D_PLATFORM_NULL_FENCE can be used with
 * the v3d_platform_fence_* functions and behaves like a fence that has already
 * been signaled, so you generally don't need to handle this return value
 * specially. Be aware though that it may not be accepted by code outside of
 * the GL stack, so eg should not be returned from eglDupNativeFenceFDAndroid.
 * If the fence is to be passed outside of the GL stack, you should specify
 * force_create=true...
 *
 * If force_create is true, this function may fail, returning
 * V3D_PLATFORM_NULL_FENCE. If it succeeds it will always return a real
 * platform fence that is accepted everywhere (so eg can be returned from
 * eglDupNativeFenceFDAndroid). */
int v3d_scheduler_create_fence(const v3d_scheduler_deps *deps,
      v3d_sched_deps_state deps_state, bool force_create);

/* Submit a job that will wait for the given fence. */
uint64_t v3d_scheduler_submit_wait_fence(int fence);

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

/* Wait for an outstanding job to finalise; Return true if a job was found */
bool v3d_scheduler_wait_any(void);

/* Wait for all outstanding jobs to finalise */
void v3d_scheduler_wait_all(void);

/* Flush any batched jobs. */
void v3d_scheduler_flush(void);

/* Merges two lists of dependencies, creating a null job if required. */
void v3d_scheduler_merge_deps(v3d_scheduler_deps *a, const v3d_scheduler_deps *b);

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

/* helper functions/ structs */
typedef struct v3d_lock_sync
{
   struct gmem_lock_list lock_list;
   struct gmem_v3d_sync_list sync_list;
} v3d_lock_sync;

v3d_lock_sync* v3d_lock_sync_create(void);
void v3d_lock_sync_destroy(void *data);
void v3d_lock_sync_completion_and_destroy(void *data, uint64_t job_id,
      v3d_sched_job_error error);

#ifdef __cplusplus
}
#endif

#endif /* V3D_SCHEDULER_H */
