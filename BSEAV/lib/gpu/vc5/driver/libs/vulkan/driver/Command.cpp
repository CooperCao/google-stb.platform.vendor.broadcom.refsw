/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "Command.h"
#include "Options.h"
#include "ClifRecorder.h"

#include "libs/platform/v3d_scheduler.h"
#include "libs/core/v3d/v3d_align.h"
#include "libs/util/log/log.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::Command");

////////////////////////////////////////////////////////////////////////////
// BinRenderJob
////////////////////////////////////////////////////////////////////////////

// The internal counter of all submitted bin/render jobs
// Used as a 'frame' counter for clif recording and supertile isolation.
uint32_t CmdBinRenderJobObj::Task::m_brJobCounter = 0;

CmdBinRenderJobObj::CmdBinRenderJobObj(const CmdBinRenderJobObj &rhs) :
   m_binSubjobs(rhs.m_binSubjobs)
   , m_layers(rhs.m_layers)
   , m_renderSubjobs(rhs.m_renderSubjobs)
   , m_brDetails(rhs.m_brDetails)
#if !V3D_VER_AT_LEAST(4,1,34,0)
   , m_binConfig(rhs.m_binConfig)
   , m_tileStateManager(rhs.m_tileStateManager)
#endif
{
}

// Task destructor - called when the job has completed
CmdBinRenderJobObj::Task::~Task()
{
#if !V3D_VER_AT_LEAST(4,1,34,0)
   // Clean up the stuff we made for this task scope
   m_cmd->m_tileStateManager.Release(m_tileStateAddr);
   gmem_free(m_binCfgHandle);
#endif
}

#if !V3D_VER_AT_LEAST(4,1,34,0)
// Makes a small block of control-list memory from transient gmem that will
// just hold the binner config item and a branch to the master control list.
void CmdBinRenderJobObj::Task::CreateBinConfigCL()
{
   // We will need a piece of transient device memory for the bin config instruction
   // and a branch.
   m_binCfgHandle = gmem_alloc_and_map(
      V3D_CL_TILE_BINNING_MODE_CFG_SIZE + V3D_CL_BRANCH_SIZE + V3D_MAX_CLE_READAHEAD,
      V3D_CL_ALIGN, GMEM_USAGE_V3D_READ | GMEM_USAGE_HINT_DYNAMIC, "Binner config");
   if (m_binCfgHandle == GMEM_HANDLE_INVALID)
      throw bvk::bad_device_alloc();

   // Get CPU addressable memory so we can fill it in
   uint8_t *ptr = static_cast<uint8_t*>(gmem_get_ptr(m_binCfgHandle));

   // Clone the cfg data and update to use the correct address
   V3D_CL_TILE_BINNING_MODE_CFG_T binCfg = m_cmd->m_binConfig;
   binCfg.u.part1.tile_state_addr = m_tileStateAddr;

   // Write the config control list item
   v3d_cl_tile_binning_mode_cfg_indirect(&ptr, &binCfg);

   // Add a branch to the start of the main master bin control list
   assert(m_cmd->m_binSubjobs.size() == 1);
   v3d_cl_branch(&ptr, m_cmd->m_binSubjobs[0].start);

   // We're done writing the buffer.
   gmem_flush_mapped_buffer(m_binCfgHandle);
}
#endif

JobID CmdBinRenderJobObj::Task::ScheduleTask(const SchedDependencies &deps)
{
   V3D_BIN_RENDER_INFO_T info;

   info.details = m_cmd->m_brDetails;

   info.bin_subjobs.num_subjobs = m_cmd->m_binSubjobs.size();
   size_t size = sizeof(v3d_subjob) * m_cmd->m_binSubjobs.size();
   info.bin_subjobs.subjobs = (v3d_subjob*)alloca(size);
   memcpy(info.bin_subjobs.subjobs, m_cmd->m_binSubjobs.data(), size);

   info.num_layers = m_cmd->m_layers;
   info.render_subjobs.num_subjobs = m_cmd->m_renderSubjobs.size();
   size = sizeof(v3d_subjob) * m_cmd->m_renderSubjobs.size();
   info.render_subjobs.subjobs = (v3d_subjob*)alloca(size);
   memcpy(info.render_subjobs.subjobs, m_cmd->m_renderSubjobs.data(), size);

#if !V3D_VER_AT_LEAST(4,1,34,0)
   // Allocate some tile-state memory for the job now.
   size_t tileStateBytes = m_cmd->m_binConfig.u.part1.w_in_tiles *
                           m_cmd->m_binConfig.u.part1.h_in_tiles * V3D_TILE_STATE_SIZE;

   m_tileStateAddr = m_cmd->m_tileStateManager.Acquire(tileStateBytes);

   // Now we can create a bin config record which references the tile-state memory.
   // We deferred doing this during the creation of the master control lists so we don't
   // take up tile-state memory for control-lists that aren't executing.
   CreateBinConfigCL();

   assert(info.bin_subjobs.num_subjobs == 1);
   info.bin_subjobs.subjobs[0].start = gmem_get_addr(m_binCfgHandle);
#endif

#if KHRN_DEBUG
   // Dump the control-lists to clif if wanted
   if (Options::autoclifEnabled)
   {
      ClifRecorder   rec(info);
      rec.Record();
   }
#endif

   // Now submit the job
   SchedDependencies noDeps;
   JobID binJob = 0, renderJob = 0;
   v3d_scheduler_submit_bin_render_job(&deps, &noDeps,
                                       &binJob, &renderJob, &info,
                                       Task::BinTaskHandler, this,
                                       Task::RenderTaskHandler, this);

   ++m_brJobCounter;    // Increment the 'global' frame counter

   return renderJob;
}


////////////////////////////////////////////////////////////////////////////
// ComputeJob
////////////////////////////////////////////////////////////////////////////

const uint32_t* CmdComputeIndirectJobObj::PatchAndGetNumWorkGroups() const noexcept
{
   // Get num-work-groups from indirect buffer if required.
   gmem_invalidate_mapped_range(m_bufferHandle, m_bufferOffset, sizeof(uint32_t) * 3);
   const uint32_t* numWorkGroups = (uint32_t*)((char*)gmem_get_ptr(m_bufferHandle) + m_bufferOffset);

   // Early out...
   if (!numWorkGroups[0] || !numWorkGroups[1] || !numWorkGroups[2])
      return nullptr;

   // Patch num-work-group uniforms into uniform stream. It isn't really ideal to flush these
   // buffers from the CPU cache a second time, but the existing structure doesn't make this easily
   // avoidable. Hopefully this will be addressed when support is added for UBO_LOAD uniforms.
   if (m_patchInfo != nullptr)
   {
      assert(m_uniformsHandle != nullptr);
      uint32_t* uniforms = (uint32_t*)((char*)gmem_get_ptr(m_uniformsHandle) + m_uniformsOffset);
      for (const auto& u: m_patchInfo->numWorkGroups)
      {
         uniforms[u.offset] = numWorkGroups[u.index];
      }
      gmem_flush_mapped_range(
         m_uniformsHandle,
         m_uniformsOffset + m_patchInfo->rangeOffset,
         m_patchInfo->rangeSize);
   }
   return numWorkGroups;
};

#if V3D_USE_CSD

JobID CmdComputeImmediateJobObj::Schedule(Queue &queue, const SchedDependencies &deps) const
{
   return v3d_scheduler_submit_compute_job(
      &deps,
      queue.GetDevice().ComputeCacheOps(),
      &m_subjob,
      1,
      false /* TODO: secure_context */,
      nullptr, nullptr);
}

JobID CmdComputeIndirectJobObj::Schedule(Queue &queue, const SchedDependencies &deps) const
{
   struct PreprocessContext
   {
      const CmdComputeIndirectJobObj& job;
      v3d_compute_subjobs_id subjobsID;
   };

   std::unique_ptr<PreprocessContext> preprocessContext(new PreprocessContext{*this, 0});
   v3d_compute_subjobs_id subjobsID = v3d_scheduler_new_compute_subjobs(1);
   if (!subjobsID)
      throw std::bad_alloc();
   preprocessContext->subjobsID = subjobsID;

   static const auto preprocess = [](void *ctx) noexcept
   {
      // Grab pointers from context and delete it.
      PreprocessContext* context = (PreprocessContext*)ctx;
      const CmdComputeIndirectJobObj& job = context->job;
      v3d_compute_subjobs_id subjobsID = context->subjobsID;
      delete context;

      // Early out if no work-groups.
      const uint32_t* numWorkGroups = job.PatchAndGetNumWorkGroups();
      if (!numWorkGroups)
         return;

      v3d_compute_subjob subjob = job.m_subjob;
      subjob.num_wgs_x = numWorkGroups[0];
      subjob.num_wgs_y = numWorkGroups[1];
      subjob.num_wgs_z = numWorkGroups[2];
      v3d_scheduler_update_compute_subjobs(subjobsID, &subjob, 1);
   };

   // Wait for previous job submission to complete before patching uniforms again.
   SchedDependencies preprocessDeps = deps;
   if (m_last)
   {
      m_last->mutex.lock();
      preprocessDeps += m_last->jobID;
   }

   uint64_t preprocessJob = v3d_scheduler_submit_usermode_job(
      &preprocessDeps,
      preprocess,
      preprocessContext.release());

   SchedDependencies computeDeps(preprocessJob);
   JobID computeJob = v3d_scheduler_submit_indirect_compute_job(
      &computeDeps,
      queue.GetDevice().ComputeCacheOps(),
      subjobsID,
      false /* TODO: secure_context */,
      nullptr, nullptr);

   if (m_last)
   {
      m_last->jobID = computeJob;
      m_last->mutex.unlock();
   }

   return computeJob;
}

#else

JobID CmdComputeJobObj::Schedule(Queue &queue, const SchedDependencies &deps) const
{
   struct PreprocessContext
   {
      const CmdComputeJobObj& job;
      compute_job_mem* computeMem;
   };

   std::unique_ptr<PreprocessContext> preprocessContext(new PreprocessContext{*this, nullptr});
   compute_job_mem* computeMem = compute_job_mem_new();
   if (!computeMem)
      throw std::bad_alloc();
   preprocessContext->computeMem = computeMem;

   static const auto preprocess = [](void *ctx) noexcept
   {
      // Grab pointers from context and delete it.
      PreprocessContext* context = (PreprocessContext*)ctx;
      const CmdComputeJobObj& job = context->job;
      compute_job_mem* computeMem = context->computeMem;
      delete context;

      uint8_t* dispatchInPrimary = (uint8_t*)gmem_get_ptr(job.m_dispatchClMem) + job.m_dispatchClOffset;

      const uint32_t* numWorkGroups = job.PatchAndGetNumWorkGroups();
      bool ok = numWorkGroups && compute_build_dispatch(computeMem, dispatchInPrimary,
            &job.m_computeShader->program, job.m_unifsAddr, numWorkGroups);
      if (ok)
         compute_job_mem_flush(computeMem);
      else
         compute_clear_dispatch(dispatchInPrimary);   // Clear dispatch from previous execution.

      // Flush CPU cache before returning.
      gmem_flush_mapped_range(job.m_dispatchClMem, job.m_dispatchClOffset, compute_cl_dispatch_size());
   };

   // Throttle the number of outstanding compute jobs in flight per queue
   // to avoid excessive memory consumption the compute runtime.
   SchedDependencies preprocessCompletedDeps = deps;
   SchedDependencies preprocessFinalisedDeps = queue.PenultimateComputeJob();

   // Wait for previous job submission to complete before patching control-list and uniforms again.
   if (m_last)
   {
      m_last->mutex.lock();
      preprocessCompletedDeps += m_last->jobID;
   }

   JobID preprocessJob = v3d_scheduler_submit_usermode_job2(
      &preprocessCompletedDeps,
      &preprocessFinalisedDeps,
      preprocess,
      preprocessContext.release());

   V3D_RENDER_INFO_T info{};
   info.subjobs_list.subjobs = const_cast<v3d_subjob*>(&m_subjob);
   info.subjobs_list.num_subjobs = 1;
   info.no_overlap = !m_computeShader->allowConcurrentJobs;
   info.cache_ops = queue.GetDevice().ComputeCacheOps();

   auto finalise = [](void* ctx, uint64_t jobId, enum bcm_sched_job_error error) noexcept
   {
      compute_job_mem_delete((compute_job_mem*)ctx);
   };

   SchedDependencies renderDeps(preprocessJob);
   JobID computeJob = v3d_scheduler_submit_render_job(
      &renderDeps,
      &info,
      finalise,
      computeMem);

   if (m_last)
   {
      m_last->jobID = computeJob;
      m_last->mutex.unlock();
   }

   queue.RecordComputeJob(computeJob);
   return computeJob;
}

#endif

////////////////////////////////////////////////////////////////////////////
// PipelineBarrier
////////////////////////////////////////////////////////////////////////////

JobID CmdPipelineBarrierObj::Schedule(Queue &queue, const SchedDependencies &deps) const
{
   // TODO : memory barriers, finer grained barrier using stages?

   // Insert an execution dependency between everything before this point and
   // everything after.
   SchedDependencies const &allPrev = queue.AllPreviousJobs();
   if (allPrev.n > 0)
   {
      JobID job = v3d_scheduler_submit_null_job(&allPrev, nullptr, nullptr);
      queue.RecordBarrierJob(job);
      return job;
   }

   return 0;
}

////////////////////////////////////////////////////////////////////////////
// Secondary Command Buffer Execution (outside render pass)
////////////////////////////////////////////////////////////////////////////
JobID CmdSecondaryExecuteObj::Schedule(Queue &queue, const SchedDependencies &deps) const
{
   SchedDependencies retDeps;

   for (const Command *cmd : m_secondaryBuffer->CommandList())
   {
      log_trace("Scheduling secondary cmd %s", cmd->CommandName());
      JobID jobID = cmd->Schedule(queue, deps);
      queue.RecordPrevJob(jobID);
      retDeps += jobID;
   }

   return retDeps.Amalgamate();
}

////////////////////////////////////////////////////////////////////////////
// WaitEvents (inside or outside render pass)
//
// When inside a render pass, the CmdWaitEventsObj will be automatically
// pulled in front of the CmdBinRenderJobObj (which will still be under
// construction until the renderPass or subpass is ended).
//
// Since the CmdSetEvent and CmdResetEvent aren't allowed inside a
// renderPass, we think this will give the desired behviour.
////////////////////////////////////////////////////////////////////////////
JobID CmdWaitEventsObj::Schedule(Queue &queue, const SchedDependencies &deps) const
{
   // TODO : memory barriers, finer grained barrier using stages?
   SchedDependencies ourDeps;

   for (uint32_t i = 0; i < m_eventCount; i++)
   {
      Event *ev = fromHandle<Event>(m_events[i]);
      ourDeps += ev->ScheduleWait(deps, m_dstStageMask);
   }

   JobID jobID = ourDeps.Amalgamate();
   queue.RecordBarrierJob(jobID);
   return jobID;
}

} // namespace bvk
