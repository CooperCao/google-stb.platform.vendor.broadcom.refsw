/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "vcos_atomic.h"
#include "bcm_sched_api.h"
#include "v3d_scheduler.h"
#include "v3d_platform.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/util/gfx_options/gfx_options.h"
#include "libs/core/v3d/v3d_ident.h"
#include "libs/compute/compute.h"
#include "gmem.h"
#include "v3d_driver_api.h"
#include "v3d_scheduler_graph.h"

LOG_DEFAULT_CAT("v3d_scheduler")

#define MAX_BATCHED_JOBS 16

// The static (for now) scheduler instance.
typedef struct v3d_scheduler
{
   V3D_HUB_IDENT_T hub_identity;
   V3D_IDENT_T identity;
   bool wait_after_submit;
   bool dump_node_graph;
   char dump_node_graph_filename[VCOS_PROPERTY_VALUE_MAX];

   volatile uint64_t min_non_finalised_job_id;  // any job-id smaller than this is finalised

   // must hold lock to access following members
   VCOS_MUTEX_T lock;
   uint64_t max_submitted_job_id;               // the highest job id that we've submitted to the scheduler
   struct bcm_sched_job batched_jobs[MAX_BATCHED_JOBS];
   unsigned num_batched_jobs;
   uintptr_t compute_shared_mem[2];
} v3d_scheduler;

static v3d_scheduler scheduler;

static void update_min_finalised_job(uint64_t min_non_finalised_job_id);

void v3d_scheduler_init(void)
{
   vcos_mutex_create(&scheduler.lock, "v3d_scheduler.lock");
   scheduler.min_non_finalised_job_id = 1;

   struct v3d_idents info;
   v3d_get_info(&info);
   v3d_unpack_hub_ident(&scheduler.hub_identity, info.hubIdent);
   v3d_unpack_ident(&scheduler.identity, info.ident);

   unsigned num_cores = gfx_options_uint32("V3D_LIMIT_CORES", 0);
   if (num_cores)
      scheduler.hub_identity.num_cores = gfx_umin(num_cores, scheduler.hub_identity.num_cores);

   if (gfx_options_bool("V3D_NO_TFU", false))
      scheduler.hub_identity.has_tfu = false;

   scheduler.wait_after_submit = gfx_options_bool("V3D_WAIT_AFTER_SUBMIT", false);

   scheduler.dump_node_graph = gfx_options_bool("V3D_DUMP_NODE_GRAPH", false);
   gfx_options_str("V3D_DUMP_NODE_GRAPH_FILENAME", "SchedulerGraph.dot",
      scheduler.dump_node_graph_filename,
      sizeof(scheduler.dump_node_graph_filename));

   if (scheduler.dump_node_graph)
      v3d_sched_graph_init(scheduler.dump_node_graph_filename);

   bcm_sched_register_update_oldest_nfid(update_min_finalised_job);
}

void v3d_scheduler_shutdown(void)
{
   if (scheduler.dump_node_graph)
      v3d_sched_graph_term();

   v3d_scheduler_wait_all();
   bcm_sched_register_update_oldest_nfid(NULL);

   for (unsigned i = 0; i != countof(scheduler.compute_shared_mem); ++i)
      gmem_free((gmem_handle_t)vcos_atomic_load_uintptr(&scheduler.compute_shared_mem[i], VCOS_MEMORY_ORDER_RELAXED));

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

   if (scheduler.dump_node_graph)
      v3d_sched_graph_add_node(job);

   if (flush || scheduler.num_batched_jobs == MAX_BATCHED_JOBS)
      flush_locked();

   vcos_mutex_unlock(&scheduler.lock);

   if (scheduler.wait_after_submit)
      v3d_scheduler_wait_all();

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

uint64_t v3d_scheduler_submit_null_job(const v3d_scheduler_deps *deps,
      v3d_sched_completion_fn completion, void *data)
{
   struct bcm_sched_job job;

   assert(deps->n != 0);

   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_NULL;
   v3d_scheduler_copy_deps(&job.completed_dependencies, deps);

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
   /* Note about YV12 conversions:
    * YV12 2 planes: chroma has CbCr format
    * YV12 3 planes: plane 0: Y; plane 1: chroma is Cr/V and plane 2: uplane is Cb/U */
   tfu_job.input.address = cmd->src_base_addrs[0];
   tfu_job.input.chroma_address = cmd->src_base_addrs[1];
   tfu_job.input.uplane_address = cmd->src_base_addrs[2];
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
   v3d_cache_ops cache_ops,
   const V3D_TFU_COMMAND_T* tfu_cmd,
   bool secure,
   v3d_sched_completion_fn completion, void *data)
{
   struct bcm_sched_job job;

   memset(&job, 0, sizeof(job));
   job.job_type = BCM_SCHED_JOB_TYPE_V3D_TFU;
   v3d_scheduler_copy_deps(&job.completed_dependencies, deps);
   job.driver.tfu = tfu_cmd_to_tfu_job(tfu_cmd);
   job.completion_fn = completion;
   job.completion_data = data;
   job.secure = secure;
   job.cache_ops = cache_ops;

   // Don't expect the driver to deal with anything other than L3C cache ops
   // for TFU jobs.
   assert(!((job.cache_ops) & ~(V3D_CACHE_FLUSH_L3C | V3D_CACHE_CLEAN_L3C)));

   return submit_job(&job, false);
}

uint64_t v3d_scheduler_submit_render_job(
   const v3d_scheduler_deps* deps,
   const V3D_RENDER_INFO_T* info,
   v3d_sched_completion_fn completion, void *data)
{
   struct bcm_sched_job job;
   memset(&job, 0, sizeof(job));
   job.job_type = BCM_SCHED_JOB_TYPE_V3D_RENDER;
   v3d_scheduler_copy_deps(&job.completed_dependencies, deps);

   job.driver.render.subjobs_list = info->subjobs_list;
   job.driver.render.num_layers = 1;
   job.driver.render.tile_alloc_layer_stride = 0;
   job.driver.render.empty_tile_mode = info->empty_tile_mode;
   job.driver.render.gmp_table = info->render_gmp_table;
   job.driver.render.workaround_gfxh_1181 = info->render_workaround_gfxh_1181;
   job.driver.render.no_bin_overlap = info->render_no_bin_overlap;

   job.completion_fn = completion;
   job.completion_data = data;
   job.secure = info->secure;
   job.cache_ops |= info->render_cache_ops;

   return submit_job(&job, true);
}

void v3d_scheduler_submit_bin_render_job(
   const v3d_scheduler_deps* bin_deps,
   const v3d_scheduler_deps* render_deps,
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
   v3d_scheduler_copy_deps(&jobs[0].completed_dependencies, bin_deps);
   jobs[0].driver.bin.subjobs_list = br_info->bin_subjobs;
   jobs[0].driver.bin.workaround_gfxh_1181 = br_info->details.bin_workaround_gfxh_1181;
   jobs[0].driver.bin.no_render_overlap = br_info->details.bin_no_render_overlap;
   jobs[0].driver.bin.minInitialBinBlockSize = br_info->details.min_initial_bin_block_size;
   jobs[0].secure = br_info->details.secure;

   jobs[0].driver.bin.gmp_table = br_info->details.bin_gmp_table;
#if V3D_HAS_QTS
   jobs[0].driver.bin.tile_state_size = br_info->details.bin_tile_state_size;
#endif
   jobs[0].completion_fn = bin_completion;
   jobs[0].completion_data = bin_compl_data;
   jobs[0].cache_ops |= br_info->details.bin_cache_ops;

   /* And the render job */
   jobs[1].job_type = BCM_SCHED_JOB_TYPE_V3D_RENDER;
   v3d_scheduler_copy_deps(&jobs[1].completed_dependencies, render_deps);
   jobs[1].driver.render.subjobs_list = br_info->render_subjobs;
   jobs[1].driver.render.num_layers = br_info->num_layers;
   jobs[1].driver.render.tile_alloc_layer_stride = br_info->details.tile_alloc_layer_stride;
   jobs[1].driver.render.workaround_gfxh_1181 = br_info->details.render_workaround_gfxh_1181;
   jobs[1].driver.render.no_bin_overlap = br_info->details.render_no_bin_overlap;
   jobs[1].secure = br_info->details.secure;

   jobs[1].driver.render.gmp_table = br_info->details.render_gmp_table;
   jobs[1].driver.render.empty_tile_mode = br_info->details.empty_tile_mode;
   jobs[1].completion_fn = render_completion;
   jobs[1].completion_data = render_compl_data;
   jobs[1].cache_ops |= br_info->details.render_cache_ops;

   // Ensure space for render to bin dependency outside of lock.
   if (br_info->details.render_depends_on_bin)
      ensure_space_for_dep(&jobs[1].completed_dependencies);

   vcos_mutex_lock(&scheduler.lock);

   flush_locked();

   /* Submit the job */
   jobs[0].job_id = ++scheduler.max_submitted_job_id;
   jobs[1].job_id = ++scheduler.max_submitted_job_id;
   if (br_info->details.render_depends_on_bin)
   {
      v3d_scheduler_deps* deps = &jobs[1].completed_dependencies;
      assert(deps->n < BCM_SCHED_MAX_DEPENDENCIES);
      deps->dependency[deps->n] = jobs[0].job_id;
      deps->n++;
   }
   bcm_sched_queue_bin_render(&jobs[0], &jobs[1]);

   if (scheduler.dump_node_graph)
   {
      v3d_sched_graph_add_node(&jobs[0]);
      v3d_sched_graph_add_node(&jobs[1]);
      v3d_sched_graph_add_bin_render_dep(jobs[0].job_id, jobs[1].job_id);
   }

   vcos_mutex_unlock(&scheduler.lock);

   /* and record the job ids. */
   *bin_job = jobs[0].job_id;
   *render_job = jobs[1].job_id;

   if (scheduler.wait_after_submit)
      v3d_scheduler_wait_all();
}

uint64_t v3d_scheduler_submit_usermode_job(const v3d_scheduler_deps* deps,
      v3d_sched_user_fn user_fn, void *data)
{
   struct bcm_sched_job job;
   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_USERMODE;
   v3d_scheduler_copy_deps(&job.completed_dependencies, deps);
   job.driver.usermode.user_fn = user_fn;
   job.driver.usermode.data = data;
   return submit_job(&job, true);
}

uint64_t v3d_scheduler_submit_usermode_job2(
   const v3d_scheduler_deps* completed_deps,
   const v3d_scheduler_deps* finalised_deps,
   v3d_sched_user_fn user_fn, void *data)
{
   struct bcm_sched_job job;
   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_USERMODE;
   v3d_scheduler_copy_deps(&job.completed_dependencies, completed_deps);
   v3d_scheduler_copy_deps(&job.finalised_dependencies, finalised_deps);
   job.driver.usermode.user_fn = user_fn;
   job.driver.usermode.data = data;
   return submit_job(&job, true);
}

uint64_t v3d_scheduler_submit_wait_on_event_job(const v3d_scheduler_deps* deps, bcm_sched_event_id event_id)
{
   struct bcm_sched_job job;

   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_WAIT_ON_EVENT;
   v3d_scheduler_copy_deps(&job.completed_dependencies, deps);
   job.driver.event.id = event_id;

   return submit_job(&job, false);
}

uint64_t v3d_scheduler_submit_set_event_job(const v3d_scheduler_deps* deps, bcm_sched_event_id event_id)
{
   struct bcm_sched_job job;
   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_SET_EVENT;
   v3d_scheduler_copy_deps(&job.completed_dependencies, deps);
   job.driver.event.id = event_id;

   return submit_job(&job, true);
}

uint64_t v3d_scheduler_submit_reset_event_job(const v3d_scheduler_deps* deps, bcm_sched_event_id event_id)
{
   struct bcm_sched_job job;
   memset(&job, 0, sizeof job);
   job.job_type = BCM_SCHED_JOB_TYPE_RESET_EVENT;
   v3d_scheduler_copy_deps(&job.completed_dependencies, deps);
   job.driver.event.id = event_id;

   return submit_job(&job, false);
}

uint64_t v3d_scheduler_submit_barrier_job(
   const v3d_scheduler_deps *deps,
   v3d_cache_ops cache_ops)
{
   // No scheduler support for clearing VCD cache, perform
   // this in the control list.
   assert(!(cache_ops & V3D_CACHE_CLEAR_VCD));

   struct bcm_sched_job job;
   memset(&job, 0, sizeof(job));
   job.job_type = BCM_SCHED_JOB_TYPE_V3D_BARRIER;
   v3d_scheduler_copy_deps(&job.completed_dependencies, deps);
   job.cache_ops = cache_ops;

   return submit_job(&job, false);
}

static void fill_deps(const v3d_scheduler_deps **completed_deps,
      const v3d_scheduler_deps **finalised_deps,
      const v3d_scheduler_deps *deps, v3d_sched_deps_state deps_state)
{
   static v3d_scheduler_deps empty_deps;

   switch (deps_state)
   {
   case V3D_SCHED_DEPS_COMPLETED:
      *completed_deps = deps;
      *finalised_deps = &empty_deps;
      break;
   case V3D_SCHED_DEPS_FINALISED:
      *finalised_deps = deps;
      *completed_deps = &empty_deps;
      break;
   default:
      unreachable();
   }
}

int v3d_scheduler_create_fence(const v3d_scheduler_deps *deps,
      v3d_sched_deps_state deps_state, bool force_create)
{
   v3d_scheduler_flush();

   const struct bcm_sched_dependencies *completed_deps, *finalised_deps;
   fill_deps(&completed_deps, &finalised_deps, deps, deps_state);
   int fence = bcm_sched_create_fence(completed_deps, finalised_deps, force_create);

   if (scheduler.dump_node_graph)
      v3d_sched_graph_add_fence(fence, completed_deps, finalised_deps);

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
         if (response.state_achieved == 1)
            deps->n = 0;
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

   v3d_scheduler_flush();

   const struct bcm_sched_dependencies *completed_deps, *finalised_deps;
   fill_deps(&completed_deps, &finalised_deps, deps, deps_state);
   bcm_sched_wait_jobs(completed_deps, finalised_deps);

   if (deps_state == V3D_SCHED_DEPS_FINALISED)
      deps->n = 0;
}

bool v3d_scheduler_wait_jobs_timeout(v3d_scheduler_deps *deps, v3d_sched_deps_state deps_state, int timeout)
{
   if (v3d_scheduler_jobs_reached_state(deps, deps_state, false))
      return true;

   const struct bcm_sched_dependencies *completed_deps, *finalised_deps;
   fill_deps(&completed_deps, &finalised_deps, deps, deps_state);
   bcm_wait_status status = bcm_sched_wait_jobs_timeout(completed_deps, finalised_deps, timeout);

   if (status == BCM_WaitJobDone)
   {
      if (deps_state == V3D_SCHED_DEPS_FINALISED)
         deps->n = 0;
      return true;
   }
   return false;
}

bool v3d_scheduler_wait_any_job_timeout(v3d_scheduler_deps *deps, v3d_sched_deps_state deps_state, int timeout)
{
   if (v3d_scheduler_jobs_reached_state(deps, deps_state, false))
      return true;

   const struct bcm_sched_dependencies *completed_deps, *finalised_deps;
   fill_deps(&completed_deps, &finalised_deps, deps, deps_state);
   bcm_wait_status status = bcm_sched_wait_any_job_timeout(completed_deps, finalised_deps, timeout);

   return status == BCM_WaitJobDone;
}

void v3d_scheduler_wait_all(void)
{
   while (v3d_scheduler_wait_any()) {}
}

bool v3d_scheduler_wait_any(void)
{
   v3d_scheduler_flush();

   bool waited = bcm_sched_wait_for_any_non_finalised();
   return waited;
}

void v3d_scheduler_flush(void)
{
   vcos_mutex_lock(&scheduler.lock);
   flush_locked();
   vcos_mutex_unlock(&scheduler.lock);
}

static void deferred_free_callback(void *data, uint64_t job_id, v3d_sched_job_error error)
{
   gmem_free((gmem_handle_t)data);
}

void v3d_scheduler_gmem_deferred_free(v3d_scheduler_deps *deps, gmem_handle_t handle)
{
   if (v3d_scheduler_jobs_reached_state(deps, V3D_SCHED_DEPS_FINALISED, false))
   {
      gmem_free(handle);
   }
   else
   {
      v3d_scheduler_submit_null_job(deps, deferred_free_callback, handle);
   }
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

bcm_sched_event_id v3d_scheduler_new_event(void)
{
   return bcm_sched_new_event();
}

void v3d_scheduler_delete_event(bcm_sched_event_id event_id)
{
   bcm_sched_delete_event(event_id);
}

void v3d_scheduler_set_event(bcm_sched_event_id event_id)
{
   bcm_sched_set_event(event_id);
}

void v3d_scheduler_reset_event(bcm_sched_event_id event_id)
{
   bcm_sched_reset_event(event_id);
}

bool v3d_scheduler_query_event(bcm_sched_event_id event_id)
{
   // Ensure any batched jobs are flushed.
   v3d_scheduler_flush();

   return bcm_sched_query_event(event_id);
}

uint32_t v3d_scheduler_get_compute_shared_mem_size_per_core(void)
{
#if V3D_VER_AT_LEAST(4,0,2,0)
   uint32_t l2t_size = (V3D_L2T_CACHE_LINE_SIZE << scheduler.identity.l2t_way_depth) * scheduler.identity.l2t_ways;
#else
   uint32_t l2t_size_in_kb = V3D_VER_AT_LEAST(3,3,0,0) ? 256 : 128;

   // The L2T size is not stored in the ident, but small configurations of
   // V3D like 7250 and 7260 had a cut down L2T.
   if (scheduler.identity.num_slices == 1)
      l2t_size_in_kb = V3D_VER_AT_LEAST(3,3,0,0) ? 32 : 16;

   uint32_t l2t_size = l2t_size_in_kb * 1024;
#endif

   // Use half the L2T size but at least COMPUTE_MIN_SHARED_MEM_PER_CORE.
   return gfx_umax(l2t_size / 2, V3D_SCHEDULER_COMPUTE_MIN_SHARED_MEM_PER_CORE);
}

gmem_handle_t v3d_scheduler_get_compute_shared_mem(bool secure, bool alloc)
{
   // Allocate shared memory on demand.
   uintptr_t* shared_mem_ptr = &scheduler.compute_shared_mem[secure];
   gmem_handle_t handle = (gmem_handle_t)vcos_atomic_load_uintptr(shared_mem_ptr, VCOS_MEMORY_ORDER_ACQUIRE);
   if (!handle && alloc)
   {
      vcos_mutex_lock(&scheduler.lock);
      handle = (gmem_handle_t)vcos_atomic_load_uintptr(shared_mem_ptr, VCOS_MEMORY_ORDER_RELAXED);
      if (!handle)
      {
         handle = gmem_alloc(
            v3d_scheduler_get_compute_shared_mem_size_per_core() * scheduler.hub_identity.num_cores,
            V3D_MAX_CACHE_LINE_SIZE,
            GMEM_USAGE_V3D_RW | (secure ? GMEM_USAGE_SECURE : GMEM_USAGE_NONE),
            "compute_shared_mem");
         vcos_atomic_store_uintptr(shared_mem_ptr, (uintptr_t)handle, VCOS_MEMORY_ORDER_RELEASE);
      }
      vcos_mutex_unlock(&scheduler.lock);
   }
   return handle;
}
