/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "SchedTask.h"

namespace bvk {

// Static method called for all CPU tasks when they want to execute
void SchedTask::CPUTaskHandler(void *data)
{
   SchedTask *task = static_cast<SchedTask*>(data);
   task->CPUExecute();
   delete task;
}

void SchedTask::BinTaskHandler(void *data, uint64_t jobId, enum bcm_sched_job_error error)
{
   SchedTask *task = static_cast<SchedTask*>(data);
   task->BinCompleted(jobId, error);
   // Note: do not delete task
}

void SchedTask::RenderTaskHandler(void *data, uint64_t jobId, enum bcm_sched_job_error error)
{
   SchedTask *task = static_cast<SchedTask*>(data);
   task->RenderCompleted(jobId, error);
   delete task;
}

void SchedTask::TFUTaskHandler(void *data, uint64_t jobId, enum bcm_sched_job_error error)
{
   SchedTask *task = static_cast<SchedTask*>(data);
   task->TFUCompleted(jobId, error);
   delete task;
}

JobID SchedTask::ScheduleCPUTask(const SchedDependencies &deps)
{
   return v3d_scheduler_submit_usermode_job(&deps, CPUTaskHandler, this);
}

} //namespace bvk
