/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
/*=============================================================================
VideoCore OS Abstraction Layer - public header file for events
=============================================================================*/

#ifndef VCOS_EVENT_H
#define VCOS_EVENT_H

#include "vcos_types.h"
#include "vcos_platform.h"
#include "libs/util/demand.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
  * \file
  *
  * An event is akin to the Win32 auto-reset event.
  *
  *
  * Signalling an event will wake up one waiting thread only. Once one
  * thread has been woken the event atomically returns to the unsignalled
  * state.
  *
  * If no threads are waiting on the event when it is signalled it remains
  * signalled.
  *
  * This is almost, but not quite, completely unlike the "event flags"
  * object based on Nucleus event groups and ThreadX event flags.
  *
  * In particular, it should be similar in speed to a semaphore, unlike
  * the event flags.
  */

/**
  * Create an event instance.
  *
  * @param event  Filled in with constructed event.
  * @param name   Name of the event (for debugging)
  *
  * @return VCOS_SUCCESS on success, or error code.
  */
static inline VCOS_STATUS_T vcos_event_create(VCOS_EVENT_T *event, const char *name);

#ifndef vcos_event_signal

/**
  * Signal the event. The event will return to being unsignalled
  * after exactly one waiting thread has been woken up. If no
  * threads are waiting it remains signalled.
  *
  * @param event The event to signal
  */
static inline void vcos_event_signal(VCOS_EVENT_T *event);

/**
  * Wait for the event.
  *
  * @param event The event to wait for
  * @return VCOS_SUCCESS on success, VCOS_EAGAIN if the wait was interrupted.
  */
static inline VCOS_STATUS_T vcos_event_wait(VCOS_EVENT_T *event);

/**
  * Try event, but don't block.
  *
  * @param event The event to try
  * @return VCOS_SUCCESS on success, VCOS_EAGAIN if the event is not currently signalled
  */
static inline VCOS_STATUS_T vcos_event_try(VCOS_EVENT_T *event);

#endif

/*
 * Destroy an event.
 */
static inline void vcos_event_delete(VCOS_EVENT_T *event);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace vcos
{

class event
{
   VCOS_EVENT_T m_event;

   /* Non-copyable */
   event(const event &);
   event &operator=(const event &);

public:

   event(const char *name) { demand(vcos_event_create(&m_event, name) == VCOS_SUCCESS); }
   event() : event("vcos::event") {}
   ~event() { vcos_event_delete(&m_event); }

   void signal() { vcos_event_signal(&m_event); }
   void wait() { verif(vcos_event_wait(&m_event) == VCOS_SUCCESS); }
   VCOS_STATUS_T wait(uint32_t ms) { return vcos_event_timed_wait(&m_event, ms); }

   bool try_wait()
   {
      switch (vcos_event_try(&m_event))
      {
      case VCOS_SUCCESS:   return true;
      case VCOS_EAGAIN:    return false;
      default:             unreachable(); return false;
      }
   }
};

}

#endif

#endif
