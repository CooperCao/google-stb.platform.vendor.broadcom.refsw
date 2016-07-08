/******************************************************************************
 *   Broadcom Proprietary and Confidential. (c)2011-2012 Broadcom.  All rights reserved.
 *
 * This program is the proprietary software of Broadcom and/or its
 * licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY WAY,
 * AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF THE
 * SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 *
 * 1.     This program, including its structure, sequence and organization,
 * constitutes the valuable trade secrets of Broadcom, and you shall use all
 * reasonable efforts to protect the confidentiality thereof, and to use this
 * information only in connection with your use of Broadcom integrated circuit
 * products.
 *
 * 2.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED
 * "AS IS" AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS
 * OR WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 * RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 * IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS FOR
 * A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS, QUIET
 * ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU ASSUME THE
 * ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 *
 * 3.     TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR
 * ITS LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL,
 * INDIRECT, OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY
 * RELATING TO YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM
 * HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN
 * EXCESS OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1,
 * WHICHEVER IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY
 * FAILURE OF ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 *****************************************************************************/

#ifndef __BSG_TASK_H__
#define __BSG_TASK_H__

#include <stdint.h>
#include <list>
#include <memory>

#include "bsg_common.h"
#include "bsg_no_copy.h"
#include "bsg_exception.h"

namespace bsg
{

class Tasker;

// These classes are implemented (opaquely) by specific platforms
class MutexData;
class EventData;
class TaskData;

//! @ingroup task
//! @{

//! Mutex provides a simple platform independent interface to mutual-exclusion primitives.
//!
//! Threads can use the mutex Lock() and Unlock() to protect critical sections.
class Mutex : public NoCopy
{
public:
   Mutex();
   ~Mutex();

   void Lock();
   void Unlock();

private:
   std::auto_ptr<MutexData>  m_platform;
};

//! MutexLock
//!
//! This object is used to lock a mutex.
//! Use as:
//! \code
//! {
//!    MutexLock  lock(myMutex);
//!    do stuff
//! } // Mutex is released in MutexLock destructor
//! \endcode
//! Very useful if a method can throw exceptions or returns early as
//! the mutex is guarenteed to be released.
class MutexLock : public NoCopy
{
public:
   MutexLock(Mutex &mutex) :
      m_mutex(mutex)
   {
      m_mutex.Lock();
   }

   ~MutexLock()
   {
      m_mutex.Unlock();
   }

private:
   Mutex &m_mutex;
};

//! Event
//!
//! Event provides a platform independent "Event" object which can be used
//! to synchronise tasks
class Event : public NoCopy
{
public:
   Event();
   ~Event();

   void Signal();
   void Wait();

private:
   std::auto_ptr<EventData>   m_platform;
};

//! Task
//!
//! The Task is an abstract base class for jobs of work that need to be
//! performed on a secondary thread.
//! The task has two overridable methods:
//! OnThread() is run in the secondary (worker) thread
//! OnCallback() is run in the main thread (automatically at finish)
//!
//! Typical usage is to perform slow operations such as File I/O  to read texture data in the secondary thread,
//! and to submit the texture data in the main thread on completion.  Note that because the secondary thread will
//! typically not have a GL context, graphics operations (either direct or via a BSG method) should not be performed
//! therein.  Note also that, for efficiency, BSG is in general not thread safe.  In particular creation and deletion
//! of handles should be performed in the main thread.
//!
class Task : public NoCopy
{
public:
   Task();
   virtual ~Task();

   void Spawn(Tasker *tasker);

   //! Callback is used by the framework to enter items into the callback queue when the task
   //! has finished.  It can also be used during execution to issue callbacks to the main thread,
   //! for example, to indicate progress.  In this latter usage, the secondary thread is not suspended
   //! when issuing a callback, so care must be taken where data might be shared between threads.
   void Callback(bool finished = false);

   //! Invoked in the worker thread
   virtual void OnThread() = 0;

   //! Invoked in the main thread (automatically at finish)
   virtual void OnCallback(bool /*finished*/) = 0;

private:
   Tasker                  *m_tasker;
   std::auto_ptr<TaskData> m_platform;
};

// @cond

// These classes provide the necessary "functor" support for implementing the synchronous
// callback mechanism
class TaskCallable
{
public:
   virtual ~TaskCallable() {}
   virtual void Call() = 0;
};

template <class O, class T>
class TaskFunctor
{
};

template <class O>
class TaskFunctor<O, void()> : public TaskCallable
{
public:
   TaskFunctor(O *object, void (O::*method)()) :
      m_object(object),
      m_method(method)
   {
   }

   virtual void Call()
   {
      (m_object->*m_method)();
   }

private:
   O     *m_object;
   void  (O::*m_method)();
};

template <class O, class A1>
class TaskFunctor<O, void (A1)> : public TaskCallable
{
public:
   TaskFunctor(O *object, void (O::*method)(A1), A1 a1) :
      m_object(object),
      m_method(method),
      m_a1(a1)
   {
   }

   virtual void Call()
   {
      (m_object->*m_method)(m_a1);
   }

private:
   O     *m_object;
   void  (O::*m_method)(A1);
   A1    m_a1;
};

template <class O, class A1, class A2>
class TaskFunctor<O, void (A1, A2)> : public TaskCallable
{
public:
   TaskFunctor(O *object, void (O::*method)(A1, A2), A1 a1, A2 a2) :
      m_object(object),
      m_method(method),
      m_a1(a1),
      m_a2(a2)
   {
   }

   virtual void Call()
   {
      (m_object->*m_method)(m_a1, m_a2);
   }

private:
   O     *m_object;
   void  (O::*m_method)(A1, A2);
   A1    m_a1;
   A2    m_a2;
};

template <class O, class A1, class A2, class A3>
class TaskFunctor<O, void (A1, A2, A3)> : public TaskCallable
{
public:
   TaskFunctor(O *object, void (O::*method)(A1, A2, A3), A1 a1, A2 a2, A3 a3) :
      m_object(object),
      m_method(method),
      m_a1(a1),
      m_a2(a2),
      m_a3(a3)
   {
   }

   virtual void Call()
   {
      (m_object->*m_method)(m_a1, m_a2, m_a3);
   }

private:
   O     *m_object;
   void  (O::*m_method)(A1, A2, A3);
   A1    m_a1;
   A2    m_a2;
   A3    m_a3;
};

template <class O, class A1, class A2, class A3, class A4>
class TaskFunctor<O, void (A1, A2, A3, A4)> : public TaskCallable
{
public:
   TaskFunctor(O *object, void (O::*method)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4) :
      m_object(object),
      m_method(method),
      m_a1(a1),
      m_a2(a2),
      m_a3(a3),
      m_a4(a4)
   {
   }

   virtual void Call()
   {
      (m_object->*m_method)(m_a1, m_a2, m_a3, m_a4);
   }

private:
   O     *m_object;
   void  (O::*m_method)(A1, A2, A3, A4);
   A1    m_a1;
   A2    m_a2;
   A3    m_a3;
   A4    m_a4;
};

template <class O, class A1, class A2, class A3, class A4, class A5>
class TaskFunctor<O, void (A1, A2, A3, A4, A5)> : public TaskCallable
{
public:
   TaskFunctor(O *object, void (O::*method)(A1, A2, A3, A4, A5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5) :
      m_object(object),
      m_method(method),
      m_a1(a1),
      m_a2(a2),
      m_a3(a3),
      m_a4(a4),
      m_a5(a5)
   {
   }

   virtual void Call()
   {
      (m_object->*m_method)(m_a1, m_a2, m_a3, m_a4, m_a5);
   }

private:
   O     *m_object;
   void  (O::*m_method)(A1, A2, A3, A4, A5);
   A1    m_a1;
   A2    m_a2;
   A3    m_a3;
   A4    m_a4;
   A5    m_a5;
};

// @endcond

//! The CallbackTask is a specialization of Task suitable for situations where the
//! worker thread wants frequently and synchronously invoke routines in the main
//! thread.  Communications between the worker and main thread should use Call()
//! in the worker and Return() in the main thread.
//! Calls will wait until the main thread does a Return(), so this mechanism can
//! also be used to throttle a worker task.
class CallbackTask : public Task
{
public:
   //! Invoke a synchronous call to the main thread, no arguments
   template <class O>
   void Call(void (O::*method)())
   {
      m_callable = new TaskFunctor<O, void()>(dynamic_cast<O *>(this), method);
      CallbackWait();
   }

   //! Invoke a synchronous call to the main thread, one argument
   template <class O, class A1>
   void Call(void (O::*method)(A1), A1 a1)
   {
      m_callable = new TaskFunctor<O, void(A1)>(dynamic_cast<O *>(this), method, a1);
      CallbackWait();
   }

   //! Invoke a synchronous call to the main thread, two arguments
   template <class O, class A1, class A2>
   void Call(void (O::*method)(A1, A2), A1 a1, A2 a2)
   {
      m_callable = new TaskFunctor<O, void(A1, A2)>(dynamic_cast<O *>(this), method, a1, a2);
      CallbackWait();
   }

   //! Invoke a synchronous call to the main thread, three arguments
   template <class O, class A1, class A2, class A3>
   void Call(void (O::*method)(A1, A2, A3), A1 a1, A2 a2, A3 a3)
   {
      m_callable = new TaskFunctor<O, void(A1, A2, A3)>(dynamic_cast<O *>(this), method, a1, a2, a3);
      CallbackWait();
   }

   //! Invoke a synchronous call to the main thread, four arguments
   template <class O, class A1, class A2, class A3, class A4>
   void Call(void (O::*method)(A1, A2, A3, A4), A1 a1, A2 a2, A3 a3, A4 a4)
   {
      m_callable = new TaskFunctor<O, void(A1, A2, A3, A4)>(dynamic_cast<O *>(this), method, a1, a2, a3, a4);
      CallbackWait();
   }

   //! Invoke a synchronous call to the main thread, five arguments
   template <class O, class A1, class A2, class A3, class A4, class A5>
   void Call(void (O::*method)(A1, A2, A3, A4, A5), A1 a1, A2 a2, A3 a3, A4 a4, A5 a5)
   {
      m_callable = new TaskFunctor<O, void(A1, A2, A3, A4, A5)>(dynamic_cast<O *>(this), method, a1, a2, a3, a4, a5);
      CallbackWait();
   }

   //! This method dispatches the callback to the "callable" registered by the
   //! Call() method.  It runs in the main thread.
   //! Do not override this method
   virtual void OnCallback(bool finished)
   {
      if (m_callable != 0)
      {
         m_callable->Call();
         delete m_callable;
         m_callable = 0;
      }
      else
      {
         DefaultCallback(finished);
      }

      CallbackDone();
   }

   //! This method is invoked when there is no "callable" registered.  This
   //! would be called if Task::Callback method is used to invoke an asynchronous
   //! callback, or automatically at the end of the task.
   virtual void DefaultCallback(bool /*finished*/)
   {
   }

private:
   // Runs on worker thread
   // Issue the callback and wait for completion
   void CallbackWait()
   {
      Callback();
      m_event.Wait();
   }

   // Runs in main thread
   // Signals the worker thread that the main thread is ready.
   void CallbackDone()
   {
      m_event.Signal();
   }


private:
   // Used to syncronise callbacks
   Event          m_event;
   TaskCallable   *m_callable;
};

// @cond
class TaskCallback
{
public:
   TaskCallback(Task *task, bool finished) :
      m_task(task),
      m_finished(finished)
   {
   }

   void OnCallback()
   {
      m_task->OnCallback(m_finished);
      if (m_finished)
      {
         delete m_task;
      }
   }

   bool GetFinished() const { return m_finished; }
   Task *GetTask() const    { return m_task;     }

private:
   Task  *m_task;
   bool  m_finished;
};
// @endcond

//! Queue is a generic thread safe queue.
template <class T>
class Queue
{
public:
   //! Push adds an item to the back of the queue.
   void Push(T v)
   {
      MutexLock   lock(m_mutex);
      m_queue.push_back(v);
   }

   //! Inserts an item to the head of the queue, it will ne the next popped
   void PushFront(T v)
   {
      MutexLock   lock(m_mutex);
      m_queue.push_front(v);
   }

   //! Dequeue the oldest item from the queue.
   T Pop()
   {
      MutexLock   lock(m_mutex);

      if (m_queue.size() == 0)
         BSG_THROW("Can't Pop() an empty Queue");

      T ret(m_queue.front());

      m_queue.pop_front();

      return ret;
   }

   T Peek()
   {
      MutexLock   lock(m_mutex);

      if (m_queue.size() == 0)
         BSG_THROW("Can't Peek() an empty Queue");

      return m_queue.front();
   }

   uint32_t NumPending() const
   {
      MutexLock   lock(m_mutex);

      return m_queue.size() != 0;
   }

   bool Pending() const
   {
      return NumPending() != 0;
   }

   void Clear()
   {
      MutexLock   lock(m_mutex);
      m_queue.clear();
   }

private:
   mutable Mutex  m_mutex; // Mutable because it is not part of the observable state and we want e.g. Pending() to be const
   std::list<T>   m_queue;
};

//! The tasker manages a Task queue and is used by applications to manage threaded tasks.  Applications
//! launch tasks via Submit() and service callbacks from these threads using DoCallbacks().
//!
class Tasker : public NoCopy
{
public:
   Tasker() :
      m_activeTasks(0)
   {
   }

   //! Start a worker task.  The queue now owns this pointer, so typical
   //! usage would be Submit(new BlahTask(...));
   //! Memory is freed after the task has completed (including final callback stage)
   void Submit(Task *newedTask)
   {
      if (newedTask != 0)
      {
         newedTask->Spawn(this);
         m_activeTasks += 1;
      }
   }

   void Callback(Task *task, bool finished)
   {
      if (task != 0)
         m_callbacks.Push(new TaskCallback(task, finished));
   }

   enum
   {
      ALL = 0xffffffff
   };

   //! Invoked by the application in the main thread to handle any callbacks
   //! from the task queue.  Use n to control how many callbacks are allowed.
   //! By default, the callback queue is emptied.
   uint32_t DoCallbacks(uint32_t n = ALL)
   {
      uint32_t done = 0;

      while (n != 0 && m_callbacks.Pending())
      {
         TaskCallback *callback = m_callbacks.Pop();
         callback->OnCallback();

         if (callback->GetFinished())
            m_activeTasks -= 1;

         done += 1;
         delete callback;
         n -= 1;
      }

      return done;
   }

   //! Return the number of outstanding callbacks
   uint32_t GetNumCallbacks()
   {
      return m_callbacks.NumPending();
   }

private:
   Queue<TaskCallback *> m_callbacks;
   uint32_t              m_activeTasks;
};

//! @}

}


#endif /* __BSG_TASK_H__ */

