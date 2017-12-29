/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "AllObjects.h"
#include "Common.h"
#include "Command.h"
#include "SchedDependencies.h"

#include "libs/util/log/log.h"

namespace bvk {

LOG_DEFAULT_CAT("bvk::Queue");

Queue::Queue(
      const VkAllocationCallbacks *pCallbacks,
      Device *pDevice,
      float   priority) :
   Allocating(pCallbacks),
   m_device(pDevice),
   m_priority(priority)
{
}

Queue::~Queue() noexcept
{

}

// Schedule multiple command buffers for execution
VkResult Queue::QueueSubmit(
   uint32_t              submitCount,
   const VkSubmitInfo   *pSubmits,
   bvk::Fence           *fence) noexcept
{
   try
   {
      for (uint32_t b = 0; b < submitCount; b++)
      {
         SchedDependencies   waitSemJobs;
         SchedDependencies   queuedJobs;
         const VkSubmitInfo *batch = &pSubmits[b];

         for (uint32_t i = 0; i < batch->waitSemaphoreCount; i++)
         {
            // Schedule semaphore waits as requested
            Semaphore *sem = fromHandle<Semaphore>(batch->pWaitSemaphores[i]);
            waitSemJobs += sem->ScheduleWait(batch->pWaitDstStageMask[i]);
         }

         // Combine waitBarriers and waitSemJobs
         SchedDependencies waitDeps(m_waitBarriers);
         waitDeps += waitSemJobs;
         waitDeps.Amalgamate();

         // Schedule each command buffer
         for (uint32_t i = 0; i < batch->commandBufferCount; i++)
         {
            const CommandBuffer *cb = fromHandle<CommandBuffer>(batch->pCommandBuffers[i]);

            // Schedule each command in the command buffer
            for (const Command *cmd : cb->CommandList())
            {
               // Schedule the command to wait for waitDeps. allPreviousJobs is provided for
               // barrier type commands that might also need to wait on those.

               log_trace("Scheduling %s [%p]", cmd->CommandName(), cmd);

               JobID job = cmd->Schedule(*this, waitDeps);
               queuedJobs += job;
               m_allPreviousJobs += job;

               // m_waitBarriers may have been changed by the Schedule()
               waitDeps += m_waitBarriers;
            }
         }

         // Collapse into single jobIds
         queuedJobs.Amalgamate();
         m_waitBarriers.Amalgamate();
         m_allPreviousJobs.Amalgamate();

         for (uint32_t i = 0; i < batch->signalSemaphoreCount; i++)
         {
            // Schedule semaphore signals as requested
            Semaphore *sem = fromHandle<Semaphore>(batch->pSignalSemaphores[i]);
            sem->ScheduleSignal(queuedJobs);
         }
      }

      // Flush any batched jobs to the scheduler.
      // sem->ScheduleSignal flushes internally so no need to flush before that.
      v3d_scheduler_flush();

      if (fence != nullptr)
      {
         // Section 6.6 : After a fence or semaphore is signaled, it is guaranteed that :
         // All commands in any command buffer submitted to the queue before and including
         // the submission that signals the fence, or the batch that signals the semaphore,
         // have completed execution.
         fence->ScheduleSignal(m_allPreviousJobs);
      }
   }
   catch (std::bad_alloc&)
   {
      return VK_ERROR_OUT_OF_HOST_MEMORY;
   }
   catch (bvk::bad_device_alloc&)
   {
      return VK_ERROR_OUT_OF_DEVICE_MEMORY;
   }

   return VK_SUCCESS;
}

VkResult Queue::QueueWaitIdle() noexcept
{
   v3d_scheduler_wait_jobs(&m_allPreviousJobs, V3D_SCHED_DEPS_FINALISED);
   return VK_SUCCESS;
}

VkResult Queue::QueueBindSparse(
   uint32_t                 bindInfoCount,
   const VkBindSparseInfo  *pBindInfo,
   bvk::Fence              *fence) noexcept
{
   VkResult result;

   result = VK_ERROR_INCOMPATIBLE_DRIVER;

   NOT_IMPLEMENTED_YET;
   return result;
}

VkResult Queue::QueuePresentKHR(
   const VkPresentInfoKHR  *pPresentInfo) noexcept
{
   V3DPlatform &platform = m_device->GetPlatform();
   return platform.QueueFrame(m_device, pPresentInfo);
}

} // namespace bvk
