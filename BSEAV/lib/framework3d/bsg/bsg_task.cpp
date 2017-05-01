/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "bsg_task.h"

namespace bsg
{

Task::Task()
{
}

Task::~Task()
{
   if (m_thread)
      m_thread->join();
}

void Task::Spawn(Tasker *tasker)
{
   m_tasker = tasker;

   auto f = [this]() {
      OnThread();
      Callback(true);
   };
   m_thread = new std::thread(f);
}

void Task::Callback(bool finished)
{
   if (m_tasker != 0)
      m_tasker->Callback(this, finished);
}

}
