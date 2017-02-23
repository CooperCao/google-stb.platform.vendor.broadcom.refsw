/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  vcfw
Module   :  vcos

FILE DESCRIPTION
VideoCore OS Abstraction Layer - pthreads types
=============================================================================*/

/* Do not include this file directly - instead include it via vcos.h */

/** @file
  *
  * Pthreads implementation of VCOS.
  *
  */

#ifndef VCOS_PLATFORM_H
#define VCOS_PLATFORM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <alloca.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>
#include <time.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdbool.h>
#if defined(__ANDROID__) || defined(USE_ANDROID)
  #include <cutils/properties.h>
#endif

#include "libs/util/assert_helpers.h"

#define VCOS_HAVE_RTOS         1
#define VCOS_HAVE_SEMAPHORE    1
#define VCOS_HAVE_EVENT        1
#define VCOS_HAVE_CANCELLATION_SAFE_TIMER 1
#define VCOS_HAVE_ONCE         1
#define VCOS_HAVE_ATOMIC_INT   1

#define VCOS_SO_EXT  ".so"

typedef sem_t                 VCOS_SEMAPHORE_T;
typedef uint32_t              VCOS_UNSIGNED;
typedef uint32_t              VCOS_OPTION;
typedef pthread_key_t         VCOS_TLS_KEY_T;
typedef pthread_once_t        VCOS_ONCE_T;

typedef struct
{
   pthread_mutex_t   mutex;
   pthread_cond_t    cond;
   int               cond_met;
} VCOS_EVENT_T;

#define VCOS_ONCE_INIT        PTHREAD_ONCE_INIT

#define VCOS_TICKS_PER_SECOND _vcos_get_ticks_per_second()

/** Convert errno values into the values recognized by vcos */
VCOS_STATUS_T vcos_pthreads_map_error(int error);
VCOS_STATUS_T vcos_pthreads_map_errno(void);

extern uint32_t _vcos_get_ticks_per_second(void);

/*
 * Counted Semaphores
 */
static inline VCOS_STATUS_T vcos_semaphore_wait(VCOS_SEMAPHORE_T *sem) {
   int ret;
   /* gdb causes sem_wait() to EINTR when a breakpoint is hit, retry here */
   while ((ret = sem_wait(sem)) == -1 && errno == EINTR)
      continue;
   assert(ret==0);
   return VCOS_SUCCESS;
}

static inline VCOS_STATUS_T vcos_semaphore_trywait(VCOS_SEMAPHORE_T *sem) {
   int ret;
   while ((ret = sem_trywait(sem)) == -1 && errno == EINTR)
      continue;
   if (ret == 0)
      return VCOS_SUCCESS;
   else if (errno == EAGAIN)
      return VCOS_EAGAIN;
   else {
      assert(0);
      return VCOS_EINVAL;
   }
}

/**
  * \brief Wait on a semaphore with a timeout.
  *
  * Note that this function may not be implemented on all
  * platforms, and may not be efficient on all platforms
  * (see comment in vcos_semaphore_wait)
  *
  * Try to obtain the semaphore. If it is already taken, return
  * VCOS_EAGAIN.
  * @param sem Semaphore to wait on
  * @param timeout Number of milliseconds to wait before
  *                returning if the semaphore can't be acquired.
  * @return VCOS_SUCCESS - semaphore was taken.
  *         VCOS_EAGAIN - could not take semaphore (i.e. timeout
  *         expired)
  *         VCOS_EINVAL - Some other error (most likely bad
  *         parameters).
  */
static inline VCOS_STATUS_T vcos_semaphore_wait_timeout(VCOS_SEMAPHORE_T *sem, VCOS_UNSIGNED timeout) {
   struct timespec ts;
   int ret;
   /* sem_timedwait takes an absolute, not a relative timeout */
   clock_gettime(CLOCK_REALTIME, &ts);
   ts.tv_sec  += timeout/1000;
   ts.tv_nsec += (timeout%1000)*1000*1000;

   /* tv_nsec may have overflowed */
   ts.tv_sec  += ts.tv_nsec/1000000000;
   ts.tv_nsec = ts.tv_nsec%1000000000;

   while ((ret = sem_timedwait(sem, &ts)) == -1 && errno == EINTR)
      continue;

   if (ret == 0)
      return VCOS_SUCCESS;
   else
   {
      if (errno == ETIMEDOUT)
         return VCOS_EAGAIN;
      else {
         assert(0);
         return VCOS_EINVAL;
      }
   }
}

static inline VCOS_STATUS_T vcos_semaphore_create(VCOS_SEMAPHORE_T *sem,
                                    const char *name,
                                    VCOS_UNSIGNED initial_count) {
   int rc = sem_init(sem, 0, initial_count);
   (void)name;
   if (rc != -1) return VCOS_SUCCESS;
   else return vcos_pthreads_map_errno();
}

static inline void vcos_semaphore_delete(VCOS_SEMAPHORE_T *sem) {
   int rc = sem_destroy(sem);
   assert(rc != -1);
   (void)rc;
}

static inline VCOS_STATUS_T vcos_semaphore_post(VCOS_SEMAPHORE_T *sem) {
   int rc = sem_post(sem);
   assert(rc == 0);
   (void)rc;
   return VCOS_SUCCESS;
}

/***********************************************************
 *
 * Threads
 *
 ***********************************************************/

extern uint64_t vcos_getmicrosecs64_internal(void);

static inline uint32_t vcos_getmicrosecs(void) { return (uint32_t)vcos_getmicrosecs64_internal(); }

static inline uint64_t vcos_getmicrosecs64(void) { return vcos_getmicrosecs64_internal(); }


/*
 * Events
 */

static inline VCOS_STATUS_T vcos_event_create(VCOS_EVENT_T *event, const char *debug_name)
{
   /* unused. */
   (void)debug_name;

   /* note: non shared process condition.
    *       (equivalent to former semaphore based implementation).
    */
   if (pthread_cond_init(&event->cond, NULL)) {
      assert(0);
      return VCOS_EINVAL;
   }

   if (pthread_mutex_init(&event->mutex, NULL)) {
      assert(0);
      if (pthread_cond_destroy(&event->cond)) {
         assert(0);
      }
      return VCOS_EINVAL;
   }

   event->cond_met = 0;
   return VCOS_SUCCESS;
}

static inline void vcos_event_signal(VCOS_EVENT_T *event)
{
   if (pthread_mutex_lock(&event->mutex)) {
      assert(0);
      return;
   }

   event->cond_met = 1;
   if (pthread_cond_signal(&event->cond)) {
      assert(0);
   }

   pthread_mutex_unlock(&event->mutex);
   return;
}

static inline VCOS_STATUS_T vcos_event_wait(VCOS_EVENT_T *event)
{
   VCOS_STATUS_T status = VCOS_EINVAL;
   int ret = 0;

   if (pthread_mutex_lock(&event->mutex)) {
      assert(0);
      return status;
   }

   while (!event->cond_met && (ret == 0) /* no error, try again */) {
      ret = pthread_cond_wait(&event->cond, &event->mutex);
   }

   /* always clear condition here.  return error if applicable.
    */
   event->cond_met = 0;
   if (!ret) {
      status = VCOS_SUCCESS;
   }

   pthread_mutex_unlock(&event->mutex);
   return status;
}

static inline VCOS_STATUS_T vcos_event_timed_wait(VCOS_EVENT_T *event, uint32_t timeout_ms)
{
   VCOS_STATUS_T status = VCOS_EINVAL;
   int ret = 0;
   struct timespec abs_timeout;
   clockid_t clock;

#if defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC)
   clock = CLOCK_MONOTONIC;
#else
   clock = CLOCK_REALTIME;
#endif

   uint32_t timeout_sec = timeout_ms / 1000;
   uint32_t timeout_ns = (timeout_ms - timeout_sec*1000)*1000000;

   clock_gettime( clock, &abs_timeout );
   abs_timeout.tv_sec += timeout_sec;
   abs_timeout.tv_nsec += timeout_ns;
   if (abs_timeout.tv_nsec >= 1000000000)
   {
      abs_timeout.tv_nsec -= 1000000000;
      abs_timeout.tv_sec += 1;
      assert(abs_timeout.tv_nsec < 1000000000);
   }

   if (pthread_mutex_lock(&event->mutex)) {
      assert(0);
      return status;
   }

   while (!event->cond_met && (ret == 0) /* no error, try again */) {
#if defined(HAVE_PTHREAD_COND_TIMEDWAIT_MONOTONIC) && !defined(__LP64__)
      /* under bionic - always returns 0 or ETIMEDOUT */
      ret = pthread_cond_timedwait_monotonic_np(&event->cond, &event->mutex, &abs_timeout);
#else
      ret = pthread_cond_timedwait(&event->cond, &event->mutex, &abs_timeout);
#endif
      if (ret == ETIMEDOUT)
      {
          /* condition may be true due to race between predicate
           * setting and timer timeout.
           */
          if (event->cond_met)
             status = VCOS_SUCCESS;
          else
             status = VCOS_ETIMEDOUT;
          event->cond_met = 0;
          pthread_mutex_unlock(&event->mutex);
          return status;
      }
   }

   /* always clear condition here.  return error if applicable.
    */
   event->cond_met = 0;
   if (!ret) {
      status = VCOS_SUCCESS;
   }
   pthread_mutex_unlock(&event->mutex);
   return status;
}

static inline VCOS_STATUS_T vcos_event_try(VCOS_EVENT_T *event)
{
   VCOS_STATUS_T status = VCOS_EINVAL;

   if (pthread_mutex_lock(&event->mutex)) {
      assert(0);
      return status;
   }

   if (!event->cond_met) {
      status = VCOS_EAGAIN;
   } else {
      /* condition met, clear and return success. */
      event->cond_met = 0;
      status = VCOS_SUCCESS;
   }

   pthread_mutex_unlock(&event->mutex);
   return status;
}

static inline void vcos_event_delete(VCOS_EVENT_T *event)
{
   if (pthread_cond_destroy(&event->cond)) {
      assert(0);
   }

   if (pthread_mutex_destroy(&event->mutex)) {
      assert(0);
   }
}

/*
 * Atomic int
 */
static inline int vcos_atomic_inc(volatile int *v) {
   return __sync_fetch_and_add(v, 1);
}

static inline int vcos_atomic_dec(volatile int *v) {
   return __sync_fetch_and_sub(v, 1);
}

static inline int vcos_atomic_swap(int nv, volatile int *v) {
   /* __sync_lock_test_and_set sets *v, and returns the old *v */
   return __sync_lock_test_and_set(v, nv);
}

static inline int vcos_atomic_cmpxchg(int ov, int nv, volatile int *v) {
   return __sync_bool_compare_and_swap(v, ov, nv) ? 0 : 1;
}

static inline int vcos_strcasecmp(const char *s1, const char *s2) {
   return strcasecmp(s1,s2);
}

static inline int vcos_strncasecmp(const char *s1, const char *s2, size_t n) {
   return strncasecmp(s1,s2,n);
}

static inline VCOS_STATUS_T vcos_tls_create(VCOS_TLS_KEY_T *key, void (*destructor)(void *)) {
   int st = pthread_key_create(key, destructor);
   return st == 0 ? VCOS_SUCCESS: VCOS_ENOMEM;
}

static inline void vcos_tls_delete(VCOS_TLS_KEY_T tls) {
   pthread_key_delete(tls);
}

static inline VCOS_STATUS_T vcos_tls_set(VCOS_TLS_KEY_T tls, void *v) {
   pthread_setspecific(tls, v);
   return VCOS_SUCCESS;
}

static inline void *vcos_tls_get(VCOS_TLS_KEY_T tls) {
   return pthread_getspecific(tls);
}

#if defined(linux) || defined(_HAVE_SBRK)

/* not exactly the free memory, but a measure of it */

static inline unsigned long vcos_get_free_mem(void) {
   return (unsigned long)sbrk(0);
}

#else

static inline unsigned long vcos_get_free_mem(void)
{
   /* Not implemented */
   assert(0);
   return 0;
}

#endif

#define vcos_alloca alloca

#define VCOS_APPLICATION_ARGC          vcos_get_argc()
#define VCOS_APPLICATION_ARGV          vcos_get_argv()

/*
 * Named Properties
 */
#if defined(ANDROID) || defined(USE_ANDROID)
  #define VCOS_PROPERTY_VALUE_MAX PROPERTY_VALUE_MAX
  #define VCOS_PROPERTY_KEY_MAX PROPERTY_KEY_MAX
#else
  #define VCOS_PROPERTY_VALUE_MAX 256
  #define VCOS_PROPERTY_KEY_MAX 32
#endif

#ifdef __cplusplus
}
#endif
#endif /* VCOS_PLATFORM_H */
