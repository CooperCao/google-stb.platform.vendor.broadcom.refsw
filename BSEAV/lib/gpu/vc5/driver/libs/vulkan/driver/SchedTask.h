/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#pragma once

#include "SchedDependencies.h"
#include "libs/platform/v3d_scheduler.h"

namespace bvk {

// Base class for all tasks that can be scheduled
class SchedTask
{
public:
   virtual ~SchedTask() {}

   // Base class helper for scheduling CPU based tasks
   JobID ScheduleCPUTask(const SchedDependencies &deps);

   // Called to schedule a task either on the GPU or CPU
   virtual JobID ScheduleTask(const SchedDependencies &deps) = 0;

   // Called to execute a CPU task - will be called on a separate thread
   virtual void CPUExecute() { assert(0); }

   // Called when a bin task is complete - will be called on a separate thread
   virtual void BinCompleted(uint64_t jobId, enum bcm_sched_job_error error) {}

   // Called when a render task is complete - will be called on a separate thread
   virtual void RenderCompleted(uint64_t jobId, enum bcm_sched_job_error error) {}

   // Called when a TFU task is complete - will be called on a separate thread
   virtual void TFUCompleted(uint64_t jobId, enum bcm_sched_job_error error) {}

protected:
   static void CPUTaskHandler(void *data);
   static void BinTaskHandler(void *data, uint64_t jobId, enum bcm_sched_job_error error);
   static void RenderTaskHandler(void *data, uint64_t jobId, enum bcm_sched_job_error error);
   static void TFUTaskHandler(void *data, uint64_t jobId, enum bcm_sched_job_error error);
};

} //namespace bvk
