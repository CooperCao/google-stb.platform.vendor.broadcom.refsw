/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <list>
#include <mutex>
#include <memory>
#include "semaphore.h"

namespace helper
{

template <class T> class MessageQueue
{
public:
   MessageQueue() {}
   ~MessageQueue() {}

   void Push(std::unique_ptr<T> &&item)
   {
      std::lock_guard<std::mutex> lock(m_mutex);
      m_q.push_back(std::move(item));
      m_sem.notify();
   }

   std::unique_ptr<T> Pop(void)
   {
      m_sem.wait();
      std::lock_guard<std::mutex> lock(m_mutex);
      auto item = std::move(m_q.front());
      m_q.pop_front();
      return item;
   }

private:
   std::list<std::unique_ptr<T>> m_q;
   helper::Semaphore m_sem;
   std::mutex m_mutex;
};

}
