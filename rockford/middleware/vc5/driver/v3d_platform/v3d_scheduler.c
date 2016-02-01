/*=============================================================================
  Copyright (c) 2014 Broadcom Europe Limited.
  All rights reserved.

Project  :  khronos
Module   :  Job submission handler

FILE DESCRIPTION
Submission of jobs to the scheduler
=============================================================================*/

#include "vcos.h"
#include "vcos_atomic.h"
#include "bcm_sched_api.h"
#include "v3d_scheduler.h"
#include "v3d_platform.h"
#include "helpers/gfx/gfx_util.h"
#include "gmem.h"
#include "v3d_driver_api.h"

#define MAX_BATCHED_JOBS 16

// The static (for now) scheduler instance.
typedef struct v3d_scheduler
{
   V3D_HUB_IDENT_T hub_identity;
   V3D_IDENT_T identity;

   volatile uint64_t min_non_finalised_job_id;  // any job-id smaller than this is finalised

   // must hold lock to access following members
   VCOS_MUTEX_T lock;
   uint64_t max_submitted_job_id;               // the highest job id that we've submitted to the scheduler
   struct bcm_sched_job batched_jobs[MAX_BATCHED_JOBS];
   unsigned num_batched_jobs;
} v3d_scheduler;

static v3d_scheduler scheduler;

static void update_min_finalised_job(uint64_t min_non_finalised_job_id);

void v3d_scheduler_init(void)
{
   vcos_mutex_create(&scheduler.lock, "v3d_scheduler.lock");
   scheduler.min_non_finalised_job_id = 1;

   struct v3d_idents info;
   v3d_get_info(&info);
   v3d_unpack_hub_ident(&scheduler.hub_identity, info.hubIdent[0],
         info.hubIdent[1], info.hubIdent[2], info.hubIdent[3]);
   v3d_unpack_ident(&scheduler.identity, info.ident[0], info.ident[1],
         info.ident[2], info.ident[3]);

   bcm_sched_register_update_oldest_nfid(update_min_finalised_job);
}

void v3d_scheduler_shutdown(void)
{
   v3d_scheduler_wait_all();
   bcm_sched_register_update_oldest_nfid(NULL);
   vcos_mutex_delete(&scheduler.lock);
   memset(&scheduler, 0, sizeof(scheduler));
}

// Remove indicated dep, and shuffle down to keep the list compact.
static void deps_remove(v3d_scheduler_deps* deps, unsigned int index)
{
   assert (index < deps->n);
   deps->dependency[index] = deps->dependency[--deps->n];
   deps->dependency[deps->n] = 0;
}

static bool remove_finalised_deps(v3d_scheduler_deps* deps)
{
   uint64_t min_non_finalised_job_id = vcos_atomic_load_uint64(&scheduler.min_non_finalised_job_id, VCOS_MEMORY_ORDER_ACQUIRE);
   for (unsigned i = 0; i < deps->n; i++)
   {
      if (deps->dependency[i] < min_non_finalised_job_id)
         deps_remove(deps, i--); /* adjust position to adjust for removal */
   }
   return deps->n == 0;
}

// Make space in deps for an additional dependency. Submits a null job if necessary.
static void make_space_for_dep(v3d_scheduler_deps* deps)
{
   // only call me if full
   assert(deps->n == BCM_SCHED_MAX_DEPENDENCIES);
   remove_finalised_deps(deps);
   if (deps->n < BCM_SCHED_MAX_DEPENDENCIES)
      return;

   /* create a null job */
   uint64_t job_id = v3d_scheduler_submit_null_job(deps, NULL, NULL);
   deps->n = 1;
   deps->dependency[0] = job_id;
}

// Ensures space in deps for an additional dependency. Submits a null job if necessary.
static inline void ensure_space_for_dep(v3d_scheduler_deps* deps)
{
   if (deps->n == BCM_SCHED_MAX_DEPENDENCIES)
      make_space_for_dep(deps);
}

static void flush_locked(void)
{
   assert(vcos_mutex_is_locked(&scheduler.lock));

   struct bcm_sched_job* jobs = scheduler.batched_jobs;
   unsigned num_jobs = scheduler.num_batched_jobs;
   if (num_jobs)
   {
      bcm_sched_queue_jobs(jobs, num_jobs);
      scheduler.num_batched_jobs = 0;
   }
}

static uint64_t submit_job(struct bcm_sched_job *job, bool flush)
{
   vcos_mutex_lock(&scheduler.lock);

   assert(job->job_id == 0);
   job->job_id = ++scheduler.max_submitted_job_id;

   assert(scheduler.num_batched_jobs < MAX_BATCHED_JOBS);
   scheduler.batched_jobs[scheduler.num_batched_jobs] = *job;
   scheduler.num_batched_jobs += 1;

   if (flush || scheduler.num_batched_jobs == MAX_BATCHED_JOBS)
      flush_locked();

   vcos_mutex_unlock(&scheduler.lock);

   return job->job_id;
}

static bool deps_contains_id(const v3d_scheduler_deps* deps, uint64_t id)
{
   for (unsigned i = 0 ; i < deps->n ; i++)
   {
      if (deps->dependency[i] == id)
         return true;
   }
   return false;
}

void v3d_scheduler_deps_set(v3d_scheduler_deps* deps, uint64_t job)
{
   deps->n = 1;
   deps->dependency[0] = job;
}

void v3d_scheduler_add_dep(v3d_scheduler_deps* deps, uint64_t job)
{
   if (!deps_contains_id(deps, job))
   {
      ensure_space_for_dep(deps);
      deps->dependency[deps->n] = job;
      deps->n++;
   }
}

void v3d_scheduler_merge_deps(v3d_scheduler_deps* a, const v3d_scheduler_deps* b)
{
   assert(a->n <= BCM_SCHED_MAX_DEPENDENCIES);
   assert(b->n <= BCM_SCHED_MAX_DEPENDENCIES);

   // early out (this happens!)
   if (b->n == 0)
      return;

   if (a->n == 0)
   {
      v3d_scheduler_copy_deps(a, b);
      return;
   }

   // Copy original deps since ensure_space_for_dep might replace with null job id.
   v3d_scheduler_deps old;
   v3d_scheduler_copy_deps(&old, a);

   for (unsigned i = 0; i != b->n; ++i)
   {
      uint64_t dep = b->dependency[i];
      if (!deps_contains_id(&old, dep))
      {
         ensure_space_for_dep(a);
         a->dependency[a->n] = dep;
         a->n++;
      }
   }
}

static void fill_sched_job_deps(struct bcm_sched_job *job, const v3d_scheduler_deps *deps,
      v3d_sched_deps_state deps_state)
{
   job->completed_dependencies.n = 0;
   job->finalised_dependencies.n = 0;
   switch (deps_state)
   {
      case V3D_SCHED_DEPS_COMPLETED:
         job->completed_dependencies = *deps;
         break;
      case V3D_SCHED_DEPS_FINALISED:
         job->finalised_dependencies = *deps;
         break;
      default:
         assert(0);
   }
}

uint64_t v3d_scheduler_submit_null_job(const v3d_scheduler_deps *deps,
      v3d_sched_completion_fn completion, void *data)
{
   struct bcm_sched_job job;

   assert(deps->n != 0);

   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_NULL;
   fill_sched_job_deps(&job, deps, V3D_SCHED_DEPS_COMPLETED);

   job.completion_fn = completion;
   job.completion_data = data;

   return submit_job(&job, false);
}

typedef struct bcm_tfu_job v3d_tfu_job;

static v3d_tfu_job tfu_cmd_to_tfu_job(const V3D_TFU_COMMAND_T *cmd)
{
   v3d_tfu_job tfu_job = { {0}, };

   assert(cmd->src_base_addrs[0] != 0);

   tfu_job.input.texture_type = (enum tfu_input_type)cmd->src_ttype;
   tfu_job.input.byte_format = (enum tfu_input_byte_format)cmd->src_memory_format;
   tfu_job.input.endianness = TFU_BIGEND_NOREORDER; // not in cmd-> obsolete?
   tfu_job.input.component_order = (enum tfu_rgbord)cmd->src_channel_order;
   tfu_job.input.raster_stride = cmd->src_strides[0];
   tfu_job.input.chroma_stride = cmd->src_strides[1];
   tfu_job.input.address = cmd->src_base_addrs[0];
   tfu_job.input.chroma_address = cmd->src_base_addrs[1];
   tfu_job.input.flags = (cmd->flip_y ? TFU_INPUT_FLIPY : 0) | (cmd->srgb ? TFU_INPUT_SRGB : 0);
   assert(cmd->num_mip_levels >=1);
   /* output.mipmap_count is the same as  nummmm in V3D_TFUICFG register, so 0 means no
    * mipmap generation */
   tfu_job.output.mipmap_count = cmd->num_mip_levels - 1;
   tfu_job.output.vertical_padding = cmd->dst_pad_in_uif_blocks;
   tfu_job.output.width = cmd->width;
   tfu_job.output.height = cmd->height;
   tfu_job.output.endianness = TFU_BIGEND_NOREORDER; // not in cmd-> obsolete?
   tfu_job.output.byte_format = (enum tfu_output_byte_format)cmd->dst_memory_format;
   tfu_job.output.address = cmd->dst_base_addr;
   tfu_job.output.flags = (cmd->disable_main_texture_write ? TFU_OUTPUT_DISABLE_MAIN_TEXTURE : 0);

   return tfu_job;
}

uint64_t v3d_scheduler_submit_tfu_job(
   const v3d_scheduler_deps* deps,
   const struct gmem_v3d_sync_list* sync_list,
   const V3D_TFU_COMMAND_T* tfu_cmd,
   v3d_sched_completion_fn completion, void *data)
{
   struct bcm_sched_job job;

   memset(&job, 0, sizeof(job));
   job.job_type = BCM_SCHED_JOB_TYPE_V3D_TFU;
   fill_sched_job_deps(&job, deps, V3D_SCHED_DEPS_COMPLETED);
   job.driver.tfu = tfu_cmd_to_tfu_job(tfu_cmd);
   job.sync_list = *sync_list;
   job.completion_fn = completion;
   job.completion_data = data;

   return submit_job(&job, false);
}

uint64_t v3d_scheduler_submit_render_job(
   const v3d_scheduler_deps* deps,
   const struct gmem_v3d_sync_list* sync_list,
   const V3D_RENDER_INFO_T* info,
   v3d_sched_completion_fn completion, void *data)
{
   struct bcm_sched_job job;
   unsigned int i;

   memset(&job, 0, sizeof(job));
   job.job_type = BCM_SCHED_JOB_TYPE_V3D_RENDER;
   fill_sched_job_deps(&job, deps, V3D_SCHED_DEPS_COMPLETED);

   job.driver.render.n = info->num_renders;
   for (i = 0; i < job.driver.render.n; i++)
   {
      job.driver.render.start[i] = info->render_begins[i];
      job.driver.render.end[i] = info->render_ends[i];
   }

   job.sync_list = *sync_list;
   job.completion_fn = completion;
   job.completion_data = data;

   return submit_job(&job, true);
}

void v3d_scheduler_submit_bin_render_job(
   const v3d_scheduler_deps* bin_deps,
   const v3d_scheduler_deps* render_deps,
   const struct gmem_v3d_sync_list* bin_sync_list,
   const struct gmem_v3d_sync_list* render_sync_list,
   uint64_t* bin_job,
   uint64_t* render_job,
   const V3D_BIN_RENDER_INFO_T* br_info,
   v3d_sched_completion_fn bin_completion, void *bin_compl_data,
   v3d_sched_completion_fn render_completion, void *render_compl_data)
{
   struct bcm_sched_job jobs[2];

   memset(jobs, 0, sizeof(jobs));

   /* Create the bin job */
   jobs[0].job_type = BCM_SCHED_JOB_TYPE_V3D_BIN;
   fill_sched_job_deps(&jobs[0], bin_deps, V3D_SCHED_DEPS_COMPLETED);
   jobs[0].sync_list = *bin_sync_list;
   jobs[0].driver.bin.n = br_info->num_bins;
   jobs[0].driver.bin.offset = br_info->bin_offset;
   jobs[0].driver.bin.workaround_gfxh_1181 = br_info->bin_workaround_gfxh_1181;
   jobs[0].driver.bin.minInitialBinBlockSize = br_info->min_initial_bin_block_size;

   for (unsigned i = 0; i < jobs[0].driver.bin.n; i++)
   {
      jobs[0].driver.bin.start[i] = br_info->bin_begins[i];
      jobs[0].driver.bin.end[i] = br_info->bin_ends[i];
   }
   jobs[0].completion_fn = bin_completion;
   jobs[0].completion_data = bin_compl_data;

   /* And the render job */
   jobs[1].job_type = BCM_SCHED_JOB_TYPE_V3D_RENDER;
   fill_sched_job_deps(&jobs[1], render_deps, V3D_SCHED_DEPS_COMPLETED);

   jobs[1].sync_list = *render_sync_list;
   jobs[1].driver.render.n = br_info->num_renders;
   jobs[1].driver.render.workaround_gfxh_1181 = br_info->render_workaround_gfxh_1181;
   for (unsigned i = 0; i < jobs[1].driver.render.n; i++)
   {
      jobs[1].driver.render.start[i] = br_info->render_begins[i];
      jobs[1].driver.render.end[i] = br_info->render_ends[i];
   }
   jobs[1].completion_fn = render_completion;
   jobs[1].completion_data = render_compl_data;

   // bcm_sched falls over if the render dependencies are full...
   if (V3D_PLATFORM_SIM)
      ensure_space_for_dep(&jobs[1].completed_dependencies);

   vcos_mutex_lock(&scheduler.lock);

   flush_locked();

   /* Submit the job */
   jobs[0].job_id = ++scheduler.max_submitted_job_id;
   jobs[1].job_id = ++scheduler.max_submitted_job_id;
   bcm_sched_queue_bin_render(&jobs[0], &jobs[1]);

   vcos_mutex_unlock(&scheduler.lock);

   /* and record the job ids. */
   *bin_job = jobs[0].job_id;
   *render_job = jobs[1].job_id;
}

void v3d_scheduler_submit_usermode_job(v3d_scheduler_deps* deps,
      v3d_sched_user_fn user_fn, void *data)
{
   remove_finalised_deps(deps);
   if (deps->n == 0)
   {
      if (user_fn)
         user_fn(data);
      return;
   }

   struct bcm_sched_job job;
   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_USERMODE;
   fill_sched_job_deps(&job, deps, V3D_SCHED_DEPS_COMPLETED);
   job.driver.usermode.user_fn = user_fn;
   job.driver.usermode.data = data;

   submit_job(&job, true);
}

int v3d_scheduler_create_fence(const v3d_scheduler_deps *deps,
      v3d_sched_deps_state deps_state)
{
   int fence;
   struct bcm_sched_job job;

   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_FENCE_SIGNAL;
   job.driver.fence_signal.fence = &fence;
   fill_sched_job_deps(&job, deps, deps_state);
   submit_job(&job, true);

   return fence;
}

uint64_t v3d_scheduler_submit_wait_fence(int fence)
{
   struct bcm_sched_job job;

   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_FENCE_WAIT;
   job.driver.fence_wait.fence = fence;
   /* The only dependency is the fence we are waiting on */
   job.completed_dependencies.n = 0;
   job.finalised_dependencies.n = 0;

   return submit_job(&job, true);
}

bool v3d_scheduler_jobs_reached_state(v3d_scheduler_deps *deps,
      v3d_sched_deps_state deps_state, bool call_kernel)
{
   /* stack know only about finalised jobs */
   remove_finalised_deps(deps);
   if (deps->n == 0)
      return true;

   if (!call_kernel)
      return false;

   v3d_scheduler_flush();

   struct bcm_sched_query_response response;
   memset(&response, 0, sizeof response);

   switch (deps_state)
   {
      case V3D_SCHED_DEPS_COMPLETED:
         bcm_sched_query(deps, NULL, &response);
         break;
      case V3D_SCHED_DEPS_FINALISED:
         bcm_sched_query(NULL, deps, &response);
         break;
      default:
         assert(0);
   }
   return (response.state_achieved == 1);
}

void v3d_scheduler_wait_jobs(v3d_scheduler_deps *deps, v3d_sched_deps_state deps_state)
{
   if (v3d_scheduler_jobs_reached_state(deps, deps_state, false))
      return;

   int fence = v3d_scheduler_create_fence(deps, deps_state);
   v3d_platform_fence_wait(fence);
   v3d_platform_fence_close(fence);

   memset(deps,  0, sizeof(v3d_scheduler_deps));
}

void v3d_scheduler_wait_all(void)
{
   while (v3d_scheduler_wait_any()) {}
}

bool v3d_scheduler_wait_any(void)
{
   vcos_mutex_lock(&scheduler.lock);

   flush_locked();

   uint64_t min_non_finalised_job_id = vcos_atomic_load_uint64(&scheduler.min_non_finalised_job_id, VCOS_MEMORY_ORDER_ACQUIRE);
   if (min_non_finalised_job_id < scheduler.max_submitted_job_id)
   {
      v3d_scheduler_deps deps;
      deps.dependency[0] = min_non_finalised_job_id;
      deps.n = 1;
      vcos_mutex_unlock(&scheduler.lock);
      v3d_scheduler_wait_jobs(&deps, V3D_SCHED_DEPS_FINALISED);
      return true;
   }
   else if (min_non_finalised_job_id == scheduler.max_submitted_job_id)
   {
      v3d_scheduler_deps deps;
      deps.dependency[0] = min_non_finalised_job_id;
      deps.n = 1;
      vcos_mutex_unlock(&scheduler.lock);
      struct bcm_sched_query_response response;
      bcm_sched_query(NULL, &deps, &response);
      return response.state_achieved == 0;
   }

   vcos_mutex_unlock(&scheduler.lock);
   return false;
}

void v3d_scheduler_flush(void)
{
   vcos_mutex_lock(&scheduler.lock);
   flush_locked();
   vcos_mutex_unlock(&scheduler.lock);
}

void v3d_scheduler_gmem_deferred_free(v3d_scheduler_deps *deps, gmem_handle_t handle)
{
   /* not implemened yet */
   assert(0);
}

/* the platform layer will call this function to update the mimimum non finalised job id;
 * any job_id smaller than is finalised */
static void update_min_finalised_job(uint64_t min_non_finalised_job_id)
{
   assert(min_non_finalised_job_id >= vcos_atomic_load_uint64(&scheduler.min_non_finalised_job_id, VCOS_MEMORY_ORDER_RELAXED));
   vcos_atomic_store_uint64(&scheduler.min_non_finalised_job_id, min_non_finalised_job_id, VCOS_MEMORY_ORDER_RELEASE);
}

const V3D_HUB_IDENT_T* v3d_scheduler_get_hub_identity(void)
{
   return &scheduler.hub_identity;
}

const V3D_IDENT_T* v3d_scheduler_get_identity(void)
{
   return &scheduler.identity;
}

int v3d_scheduler_get_v3d_ver(void)
{
   return V3D_MAKE_VER(scheduler.hub_identity.v3d_tech_version,
      scheduler.hub_identity.v3d_revision);
}

v3d_lock_sync* v3d_lock_sync_create(void)
{
   v3d_lock_sync *lock_sync = malloc(sizeof(v3d_lock_sync));
   if (lock_sync)
   {
      gmem_lock_list_init(&lock_sync->lock_list);
      gmem_v3d_sync_list_init(&lock_sync->sync_list);
   }
   return lock_sync;
}

void v3d_lock_sync_destroy(void *data)
{
   v3d_lock_sync *lock_sync = data;
   gmem_lock_list_unlock_and_destroy(&lock_sync->lock_list);
   gmem_v3d_sync_list_destroy(&lock_sync->sync_list);
   free(lock_sync);
}

void v3d_lock_sync_completion_and_destroy(void *data, uint64_t job_id,
      v3d_sched_job_error job_error)
{
   v3d_lock_sync_destroy(data);
}
