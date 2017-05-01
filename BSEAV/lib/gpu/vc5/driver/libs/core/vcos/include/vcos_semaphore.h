/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
/*=============================================================================
VideoCore OS Abstraction Layer - public header file
=============================================================================*/

#ifndef VCOS_SEMAPHORE_H
#define VCOS_SEMAPHORE_H

#include "vcos_types.h"
#ifndef VCOS_PLATFORM_H
#include "vcos_platform.h"
#endif
#include "libs/util/demand.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * \file vcos_semaphore.h
 *
 * \section sem Semaphores
 *
 * This provides counting semaphores. Semaphores are not re-entrant. On sensible
 * operating systems a semaphore can always be posted but can only be taken in
 * thread (not interrupt) context.
 *
 * \subsection timeout Timeout
 *
 * Not supported.
 *
 * \subsection sem_nucleus Changes from Nucleus:
 *
 *  Semaphores are always "FIFO" - i.e. sleeping threads are woken in FIFO order. That's
 *  because:
 *  \arg there's no support for NU_PRIORITY in threadx (though it can be emulated, slowly)
 *  \arg we don't appear to actually consciously use it - for example, Dispmanx uses
 *  it, but all threads waiting are the same priority.
 *
 */

/**
  * \brief Create a semaphore.
  *
  * Create a semaphore.
  *
  * @param sem   Pointer to memory to be initialized
  * @param name  A name for this semaphore. The name may be truncated internally.
  * @param count The initial count for the semaphore.
  *
  * @return VCOS_SUCCESS if the semaphore was created.
  *
  */
static inline VCOS_STATUS_T vcos_semaphore_create(VCOS_SEMAPHORE_T *sem, const char *name, VCOS_UNSIGNED count);

/**
  * \brief Wait on a semaphore.
  *
  * There is no timeout option on a semaphore, as adding this will slow down
  * implementations on some platforms. If you need that kind of behaviour, use
  * an event group.
  *
  * On most platforms this always returns VCOS_SUCCESS, and so would ideally be
  * a void function, however some platforms allow a wait to be interrupted so
  * it remains non-void.
  *
  * @param sem Semaphore to wait on
  * @return VCOS_SUCCESS - semaphore was taken.
  *         VCOS_EAGAIN  - could not take semaphore
  *
  */
static inline VCOS_STATUS_T vcos_semaphore_wait(VCOS_SEMAPHORE_T *sem);

/**
  * \brief Try to wait for a semaphore.
  *
  * Try to obtain the semaphore. If it is already taken, return VCOS_TIMEOUT.
  * @param sem Semaphore to wait on
  * @return VCOS_SUCCESS - semaphore was taken.
  *         VCOS_EAGAIN - could not take semaphore
  */
static inline VCOS_STATUS_T vcos_semaphore_trywait(VCOS_SEMAPHORE_T *sem);

/**
  * \brief Post a semaphore.
  *
  * @param sem Semaphore to wait on
  */
static inline VCOS_STATUS_T vcos_semaphore_post(VCOS_SEMAPHORE_T *sem);

/**
  * \brief Delete a semaphore, releasing any resources consumed by it.
  *
  * @param sem Semaphore to wait on
  */
static inline void vcos_semaphore_delete(VCOS_SEMAPHORE_T *sem);

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

namespace vcos
{

class semaphore
{
   VCOS_SEMAPHORE_T m_sem;

   /* Non-copyable */
   semaphore(const semaphore &);
   semaphore &operator=(const semaphore &);

public:

   semaphore(const char *name, VCOS_UNSIGNED count=0)
   { demand(vcos_semaphore_create(&m_sem, name, count) == VCOS_SUCCESS); }
   semaphore(VCOS_UNSIGNED count=0) : semaphore("vcos::semaphore", count) {}
   ~semaphore() { vcos_semaphore_delete(&m_sem); }

   void post() { verif(vcos_semaphore_post(&m_sem) == VCOS_SUCCESS); }
   void wait() { verif(vcos_semaphore_wait(&m_sem) == VCOS_SUCCESS); }

   bool try_wait()
   {
      switch (vcos_semaphore_trywait(&m_sem))
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
