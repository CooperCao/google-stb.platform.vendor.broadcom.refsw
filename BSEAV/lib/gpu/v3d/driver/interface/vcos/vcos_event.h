/******************************************************************************
 *  Copyright (C) 2018 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#include "interface/vcos/vcos_types.h"
#include "vcos_platform.h"

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
VCOS_INLINE_DECL
VCOS_STATUS_T vcos_event_create(VCOS_EVENT_T *event, const char *name);

#ifndef vcos_event_signal

/**
  * Signal the event. The event will return to being unsignalled
  * after exactly one waiting thread has been woken up. If no
  * threads are waiting it remains signalled.
  *
  * @param event The event to signal
  */
VCOS_INLINE_DECL
void vcos_event_signal(VCOS_EVENT_T *event);

/**
  * Wait for the event.
  *
  * @param event The event to wait for
  * @return VCOS_SUCCESS on success, VCOS_EAGAIN if the wait was interrupted.
  */
VCOS_INLINE_DECL
VCOS_STATUS_T vcos_event_wait(VCOS_EVENT_T *event);

/**
* Wait for the event with timeout.
*
* @param event The event to wait for
* @return VCOS_SUCCESS on success, VCOS_ETIMEDOUT if the wait timedout.
*/
VCOS_INLINE_DECL
VCOS_STATUS_T vcos_event_timed_wait(VCOS_EVENT_T *event, uint32_t timeout_ms);

#endif

/*
 * Destroy an event.
 */
VCOS_INLINE_DECL
void vcos_event_delete(VCOS_EVENT_T *event);

#ifdef __cplusplus
}
#endif
