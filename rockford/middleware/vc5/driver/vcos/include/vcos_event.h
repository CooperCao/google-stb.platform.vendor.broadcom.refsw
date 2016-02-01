/*
 * Copyright (c) 2010-2011 Broadcom. All rights reserved.
 *
 * This program is the proprietary software of Broadcom Corporation and/or
 * its licensors, and may only be used, duplicated, modified or distributed
 * pursuant to the terms and conditions of a separate, written license
 * agreement executed between you and Broadcom (an "Authorized License").
 * Except as set forth in an Authorized License, Broadcom grants no license
 * (express or implied), right to use, or waiver of any kind with respect to
 * the Software, and Broadcom expressly reserves all rights in and to the
 * Software and all intellectual property rights therein.  IF YOU HAVE NO
 * AUTHORIZED LICENSE, THEN YOU HAVE NO RIGHT TO USE THIS SOFTWARE IN ANY
 * WAY, AND SHOULD IMMEDIATELY NOTIFY BROADCOM AND DISCONTINUE ALL USE OF
 * THE SOFTWARE.
 *
 * Except as expressly set forth in the Authorized License,
 * 1. This program, including its structure, sequence and organization,
 *    constitutes the valuable trade secrets of Broadcom, and you shall use
 *    all reasonable efforts to protect the confidentiality thereof, and to
 *    use this information only in connection with your use of Broadcom
 *    integrated circuit products.
 * 2. TO THE MAXIMUM EXTENT PERMITTED BY LAW, THE SOFTWARE IS PROVIDED "AS IS"
 *    AND WITH ALL FAULTS AND BROADCOM MAKES NO PROMISES, REPRESENTATIONS OR
 *    WARRANTIES, EITHER EXPRESS, IMPLIED, STATUTORY, OR OTHERWISE, WITH
 *    RESPECT TO THE SOFTWARE.  BROADCOM SPECIFICALLY DISCLAIMS ANY AND ALL
 *    IMPLIED WARRANTIES OF TITLE, MERCHANTABILITY, NONINFRINGEMENT, FITNESS
 *    FOR A PARTICULAR PURPOSE, LACK OF VIRUSES, ACCURACY OR COMPLETENESS,
 *    QUIET ENJOYMENT, QUIET POSSESSION OR CORRESPONDENCE TO DESCRIPTION. YOU
 *    ASSUME THE ENTIRE RISK ARISING OUT OF USE OR PERFORMANCE OF THE SOFTWARE.
 * 3. TO THE MAXIMUM EXTENT PERMITTED BY LAW, IN NO EVENT SHALL BROADCOM OR ITS
 *    LICENSORS BE LIABLE FOR (i) CONSEQUENTIAL, INCIDENTAL, SPECIAL, INDIRECT,
 *    OR EXEMPLARY DAMAGES WHATSOEVER ARISING OUT OF OR IN ANY WAY RELATING TO
 *    YOUR USE OF OR INABILITY TO USE THE SOFTWARE EVEN IF BROADCOM HAS BEEN
 *    ADVISED OF THE POSSIBILITY OF SUCH DAMAGES; OR (ii) ANY AMOUNT IN EXCESS
 *    OF THE AMOUNT ACTUALLY PAID FOR THE SOFTWARE ITSELF OR U.S. $1, WHICHEVER
 *    IS GREATER. THESE LIMITATIONS SHALL APPLY NOTWITHSTANDING ANY FAILURE OF
 *    ESSENTIAL PURPOSE OF ANY LIMITED REMEDY.
 */

/*=============================================================================
VideoCore OS Abstraction Layer - public header file for events
=============================================================================*/

#ifndef VCOS_EVENT_H
#define VCOS_EVENT_H

#include "vcos_types.h"
#include "vcos_platform.h"

#include "helpers/assert.h"

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

   event(const char *name) { throw_if_error(vcos_event_create(&m_event, name)); }
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
