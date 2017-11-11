/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <mutex>
#include <condition_variable>

namespace helper
{

class Semaphore
{
public:
   Semaphore(int initial = 0) :
      m_count(initial)
   {}

   void notify()
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_count++;
      m_condition.notify_one();
   }

   void wait()
   {
      std::unique_lock<std::mutex> lock(m_mutex);
      m_condition.wait(lock, [&]() { return m_count; });
      m_count--;
   }

   void reset()
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_count = 0;
   }

private:
   std::mutex m_mutex;
   std::condition_variable m_condition;
   unsigned m_count;
};

}
