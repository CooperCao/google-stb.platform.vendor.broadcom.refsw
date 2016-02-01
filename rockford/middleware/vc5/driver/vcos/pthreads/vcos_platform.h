/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
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
#include <signal.h>
#include <stddef.h>
#include <stdlib.h>
#if defined(__ANDROID__) || defined(USE_ANDROID)
  #include <cutils/properties.h>
#endif

#include "helpers/assert.h"

#define VCOS_HAVE_RTOS         1
#define VCOS_HAVE_SEMAPHORE    1
#define VCOS_HAVE_EVENT        1
#define VCOS_HAVE_TIMER        1
#define VCOS_HAVE_CANCELLATION_SAFE_TIMER 1
#define VCOS_HAVE_ATOMIC_FLAGS 1
#define VCOS_HAVE_THREAD_AT_EXIT        1
#define VCOS_HAVE_ONCE         1
#define VCOS_HAVE_ALIEN_THREADS  1
#define VCOS_HAVE_ATOMIC_INT   1
#define VCOS_HAVE_TIMED_EVENT  1
#define VCOS_HAVE_RWLOCK       1
#define VCOS_HAVE_FAIR_MUTEX   1

#define VCOS_SO_EXT  ".so"

/* Linux/pthreads seems to have different timer characteristics */
#define VCOS_TIMER_MARGIN_EARLY 0
#define VCOS_TIMER_MARGIN_LATE 15

typedef sem_t                 VCOS_SEMAPHORE_T;
typedef uint32_t              VCOS_UNSIGNED;
typedef uint32_t              VCOS_OPTION;
typedef pthread_key_t         VCOS_TLS_KEY_T;
typedef pthread_once_t        VCOS_ONCE_T;
typedef pthread_rwlock_t      VCOS_RWLOCK_T;

typedef struct VCOS_LLTHREAD_T
{
   pthread_t thread; // Must be first field.
} VCOS_LLTHREAD_T;

typedef pthread_mutex_t       VCOS_MUTEX_T;
typedef pthread_mutex_t       VCOS_REENTRANT_MUTEX_T;

typedef struct
{
   pthread_mutex_t   mutex;
   pthread_cond_t    cond;
   int               cond_met;
} VCOS_EVENT_T;

#define VCOS_ONCE_INIT        PTHREAD_ONCE_INIT

#if defined(__arm__) && !defined(_HAVE_TIMER_T) && !defined(ANDROID) && !defined(USE_ANDROID)
typedef __timer_t timer_t;
#endif
typedef struct VCOS_TIMER_T
{
   pthread_t thread;                      /**< id of the timer thread */

   pthread_mutex_t lock;                  /**< lock protecting all other members of the struct */
   pthread_cond_t settings_changed;       /**< cond. var. for informing the timer thread about changes*/
   int quit;                              /**< non-zero if the timer thread is requested to quit*/

   struct timespec expires;               /**< absolute time of next expiration, or 0 if disarmed*/

   void (*orig_expiration_routine)(void*);/**< the expiration routine provided by the user of the timer*/
   void *orig_context;                    /**< the context for exp. routine provided by the user*/

} VCOS_TIMER_T;

/** Thread attribute structure. Don't use pthread_attr directly, as
  * the calls can fail, and inits must match deletes.
  */
typedef struct VCOS_THREAD_ATTR_T
{
   void *ta_stackaddr;
   VCOS_UNSIGNED ta_stacksz;
   VCOS_UNSIGNED ta_priority;
   VCOS_UNSIGNED ta_affinity;
   VCOS_UNSIGNED ta_timeslice;
   VCOS_UNSIGNED legacy;
} VCOS_THREAD_ATTR_T;

/** Called at thread exit.
  */
typedef struct VCOS_THREAD_EXIT_T
{
   void (*pfn)(void *);
   void *cxt;
} VCOS_THREAD_EXIT_T;
#define VCOS_MAX_EXIT_HANDLERS  4

typedef struct VCOS_THREAD_T
{
   pthread_t thread;             /**< The thread itself */
   VCOS_THREAD_ENTRY_FN_T entry; /**< The thread entry point */
   void *arg;                    /**< The argument to be passed to entry */
   VCOS_SEMAPHORE_T suspend;     /**< For support event groups and similar - a per thread semaphore */

   VCOS_TIMER_T task_timer;
   int task_timer_created;       /**< non-zero if the task timer has already been created*/
   void (*orig_task_timer_expiration_routine)(void*);
   void *orig_task_timer_context;

   VCOS_UNSIGNED legacy;
   char name[16];                /**< Record the name of this thread, for diagnostics */
   VCOS_UNSIGNED dummy;          /**< Dummy thread created for non-vcos created threads */

   /** Callback invoked at thread exit time */
   VCOS_THREAD_EXIT_T at_exit[VCOS_MAX_EXIT_HANDLERS];
} VCOS_THREAD_T;

#define VCOS_SUSPEND          -1
#define VCOS_NO_SUSPEND       0

#define VCOS_START 1
#define VCOS_NO_START 0

#define VCOS_THREAD_PRI_MIN   (sched_get_priority_min(SCHED_OTHER))
#define VCOS_THREAD_PRI_MAX   (sched_get_priority_max(SCHED_OTHER))

#define VCOS_THREAD_PRI_INCREASE (1)
#define VCOS_THREAD_PRI_HIGHEST  VCOS_THREAD_PRI_MAX
#define VCOS_THREAD_PRI_LOWEST   VCOS_THREAD_PRI_MIN
#define VCOS_THREAD_PRI_NORMAL ((VCOS_THREAD_PRI_MAX+VCOS_THREAD_PRI_MIN)/2)
#define VCOS_THREAD_PRI_BELOW_NORMAL (VCOS_THREAD_PRI_NORMAL-VCOS_THREAD_PRI_INCREASE)
#define VCOS_THREAD_PRI_ABOVE_NORMAL (VCOS_THREAD_PRI_NORMAL+VCOS_THREAD_PRI_INCREASE)
#define VCOS_THREAD_PRI_REALTIME VCOS_THREAD_PRI_MAX

#define _VCOS_AFFINITY_DEFAULT 0
#define _VCOS_AFFINITY_CPU0    0x100
#define _VCOS_AFFINITY_CPU1    0x200
#define _VCOS_AFFINITY_MASK    0x300
#define VCOS_CAN_SET_STACK_ADDR  0

#define VCOS_TICKS_PER_SECOND _vcos_get_ticks_per_second()

/** Convert errno values into the values recognized by vcos */
VCOS_STATUS_T vcos_pthreads_map_error(int error);
VCOS_STATUS_T vcos_pthreads_map_errno(void);

/** Register a function to be called when the current thread exits.
  */
extern VCOS_STATUS_T vcos_thread_at_exit(void (*pfn)(void*), void *cxt);

extern uint32_t _vcos_get_ticks_per_second(void);

/**
 * Set to 1 by default when ANDROID / USE_ANDROID is defined. Allows runtime
 * switching for console apps.
 */
extern int vcos_use_android_log;

typedef struct {
   VCOS_MUTEX_T mutex;
   uint32_t flags;
} VCOS_ATOMIC_FLAGS_T;

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


extern VCOS_THREAD_T *vcos_dummy_thread_create(void);
extern pthread_key_t _vcos_thread_current_key;
extern uint64_t vcos_getmicrosecs64_internal(void);

static inline uint32_t vcos_getmicrosecs(void) { return (uint32_t)vcos_getmicrosecs64_internal(); }

static inline uint64_t vcos_getmicrosecs64(void) { return vcos_getmicrosecs64_internal(); }

static inline VCOS_THREAD_T *vcos_thread_current(void) {
   void *ret = pthread_getspecific(_vcos_thread_current_key);
   if (ret == NULL)
   {
      ret = vcos_dummy_thread_create();
   }

#ifdef __cplusplus
   return static_cast<VCOS_THREAD_T*>(ret);
#else
   return (VCOS_THREAD_T *)ret;
#endif
}

static inline void vcos_sleep(uint32_t ms) {
   struct timespec ts;
   ts.tv_sec = ms/1000;
   ts.tv_nsec = ms % 1000 * (1000000);
   nanosleep(&ts, NULL);
}

static inline void vcos_sleep_us(uint32_t us) {
   struct timespec ts;
   ts.tv_sec = us/1000000;
   ts.tv_nsec = (us % 1000000) * 1000;
   nanosleep(&ts, NULL);
}

#if !defined(ANDROID) && !defined(USE_ANDROID)
static inline void vcos_yield(void)
{
   int success = pthread_yield();
   assert(success == 0);
}
#endif

static inline void vcos_thread_attr_setstack(VCOS_THREAD_ATTR_T *attr, void *addr, VCOS_UNSIGNED sz) {
   attr->ta_stackaddr = addr;
   attr->ta_stacksz = sz;
}

static inline void vcos_thread_attr_setstacksize(VCOS_THREAD_ATTR_T *attr, VCOS_UNSIGNED sz) {
   attr->ta_stacksz = sz;
}

static inline void vcos_thread_attr_setpriority(VCOS_THREAD_ATTR_T *attr, VCOS_UNSIGNED pri) {
   (void)attr;
   (void)pri;
}

static inline void vcos_thread_set_priority(VCOS_THREAD_T *thread, VCOS_UNSIGNED p) {
   /* not implemented */
   (void)thread;
   (void)p;
}

static inline VCOS_UNSIGNED vcos_thread_get_priority(VCOS_THREAD_T *thread) {
   /* not implemented */
   (void)thread;
   return 0;
}

static inline void vcos_thread_set_affinity(VCOS_THREAD_T *thread, VCOS_UNSIGNED affinity) {
   /* not implemented */
   vcos_unused(thread);
   vcos_unused(affinity);
}

static inline VCOS_UNSIGNED vcos_thread_get_affinity(VCOS_THREAD_T *thread)
{
   (void)thread;
   return 0;
}

static inline void vcos_thread_attr_setaffinity(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED affinity) {
   attrs->ta_affinity = affinity;
}

static inline void vcos_thread_attr_settimeslice(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED ts) {
   attrs->ta_timeslice = ts;
}

static inline void _vcos_thread_attr_setlegacyapi(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED legacy) {
   attrs->legacy = legacy;
}

static inline void vcos_thread_attr_setautostart(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED autostart) {
   assert(autostart == VCOS_START); /* VCOS_NO_START is not currently supported with pthreads */
   (void)attrs;
   (void)autostart;
}

static inline VCOS_LLTHREAD_T *vcos_llthread_current(void) {
   return (VCOS_LLTHREAD_T *)pthread_self();
}

/*
 * Mutexes
 */

static inline VCOS_STATUS_T vcos_mutex_create(VCOS_MUTEX_T *latch, const char *name) {
   int rc = pthread_mutex_init(latch, NULL);
   (void)name;
   if (rc == 0) return VCOS_SUCCESS;
   else return vcos_pthreads_map_errno();
}

static inline void vcos_mutex_delete(VCOS_MUTEX_T *latch) {
   int rc = pthread_mutex_destroy(latch);
   (void)rc;
   assert(rc==0);
}

static inline VCOS_STATUS_T vcos_mutex_lock(VCOS_MUTEX_T *latch) {
   int rc = pthread_mutex_lock(latch);
   assert(rc==0);
   (void)rc;
   return VCOS_SUCCESS;
}

static inline void vcos_mutex_unlock(VCOS_MUTEX_T *latch) {
   int rc = pthread_mutex_unlock(latch);
   (void)rc;
   assert(rc==0);
}

static inline int vcos_mutex_is_locked(VCOS_MUTEX_T *m) {
   int rc = pthread_mutex_trylock(m);
   if (rc == 0) {
      pthread_mutex_unlock(m);
      /* it wasn't locked */
      return 0;
   }
   else {
      return 1; /* it was locked */
   }
}

static inline VCOS_STATUS_T vcos_mutex_trylock(VCOS_MUTEX_T *m) {
   int rc = pthread_mutex_trylock(m);
   (void)rc;
   return (rc == 0) ? VCOS_SUCCESS : VCOS_EAGAIN;
}

/*
 * Reentrant Mutexes.
 */

static inline VCOS_STATUS_T vcos_reentrant_mutex_create(VCOS_REENTRANT_MUTEX_T *m, const char *name) {
   pthread_mutexattr_t attr;
   int rc;

   (void)name;
   pthread_mutexattr_init(&attr);
#ifdef __linux__
   /* Linux uses PTHREAD_MUTEX_RECURSIVE_NP to mean PTHREAD_MUTEX_RECURSIVE */
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE_NP);
#else
   pthread_mutexattr_settype(&attr, PTHREAD_MUTEX_RECURSIVE);
#endif

   rc = pthread_mutex_init(m, &attr);
   if (rc == 0)
      return VCOS_SUCCESS;
   else
      return vcos_pthreads_map_errno();
}

static inline void vcos_reentrant_mutex_delete(VCOS_REENTRANT_MUTEX_T *m) {
   int rc = pthread_mutex_destroy(m);
   (void)rc;
   assert(rc==0);
}

static inline void vcos_reentrant_mutex_lock(VCOS_REENTRANT_MUTEX_T *m) {
   int rc = pthread_mutex_lock(m);
   (void)rc;
   assert(rc==0);
}

static inline void vcos_reentrant_mutex_unlock(VCOS_REENTRANT_MUTEX_T *m) {
   int rc = pthread_mutex_unlock(m);
   (void)rc;
   assert(rc==0);
}

/***********************************************************
 *
 * rwlock
 *
 ***********************************************************/
static inline VCOS_STATUS_T vcos_rwlock_create(VCOS_RWLOCK_T *rw, const char *name) {
   int rc = pthread_rwlock_init(rw, NULL);
   (void)name;
   if (rc == 0)
      return VCOS_SUCCESS;
   return vcos_pthreads_map_errno();
}

static inline void vcos_rwlock_delete(VCOS_RWLOCK_T *rw) {
   int rc = pthread_rwlock_destroy(rw);
   (void)rc;
   assert(rc == 0);
}

static inline VCOS_STATUS_T vcos_rwlock_read_lock(VCOS_RWLOCK_T *rw) {
   if (pthread_rwlock_rdlock(rw) != 0)
      return VCOS_EAGAIN;
   return VCOS_SUCCESS;
}

static inline VCOS_STATUS_T vcos_rwlock_write_lock(VCOS_RWLOCK_T *rw) {
   if (pthread_rwlock_wrlock(rw) != 0)
      return VCOS_EAGAIN;
   return VCOS_SUCCESS;
}

static inline void vcos_rwlock_read_unlock(VCOS_RWLOCK_T *rw) {
   pthread_rwlock_unlock(rw);
}

static inline void vcos_rwlock_write_unlock(VCOS_RWLOCK_T *rw) {
   pthread_rwlock_unlock(rw);
}

static inline VCOS_STATUS_T vcos_rwlock_read_trylock(VCOS_RWLOCK_T *rw) {
   if (pthread_rwlock_tryrdlock(rw) == 0)
      return VCOS_SUCCESS;
   return VCOS_EAGAIN;
}

static inline VCOS_STATUS_T vcos_rwlock_write_trylock(VCOS_RWLOCK_T *rw) {
   if (pthread_rwlock_trywrlock(rw) == 0)
      return VCOS_SUCCESS;
   return VCOS_EAGAIN;
}

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

static inline VCOS_UNSIGNED vcos_process_id_current(void) {
   return (VCOS_UNSIGNED) getpid();
}

static inline int vcos_strcasecmp(const char *s1, const char *s2) {
   return strcasecmp(s1,s2);
}

static inline int vcos_strncasecmp(const char *s1, const char *s2, size_t n) {
   return strncasecmp(s1,s2,n);
}

static inline int vcos_in_interrupt(void) {
   return 0;
}

/* For support event groups - per thread semaphore */
static inline void _vcos_thread_sem_wait(void) {
   VCOS_THREAD_T *t = vcos_thread_current();
   vcos_semaphore_wait(&t->suspend);
}

static inline void _vcos_thread_sem_post(VCOS_THREAD_T *target) {
   vcos_semaphore_post(&target->suspend);
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

#if VCOS_HAVE_ATOMIC_FLAGS

/*
 * Atomic flags
 */

/* TODO implement properly... */

static inline VCOS_STATUS_T vcos_atomic_flags_create(VCOS_ATOMIC_FLAGS_T *atomic_flags)
{
   atomic_flags->flags = 0;
   return vcos_mutex_create(&atomic_flags->mutex, "VCOS_ATOMIC_FLAGS_T");
}

static inline void vcos_atomic_flags_or(VCOS_ATOMIC_FLAGS_T *atomic_flags, uint32_t flags)
{
   vcos_mutex_lock(&atomic_flags->mutex);
   atomic_flags->flags |= flags;
   vcos_mutex_unlock(&atomic_flags->mutex);
}

static inline uint32_t vcos_atomic_flags_get_and_clear(VCOS_ATOMIC_FLAGS_T *atomic_flags)
{
   uint32_t flags;
   vcos_mutex_lock(&atomic_flags->mutex);
   flags = atomic_flags->flags;
   atomic_flags->flags = 0;
   vcos_mutex_unlock(&atomic_flags->mutex);
   return flags;
}

static inline void vcos_atomic_flags_delete(VCOS_ATOMIC_FLAGS_T *atomic_flags)
{
   vcos_mutex_delete(&atomic_flags->mutex);
}

#endif

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

#undef VCOS_ASSERT_LOGGING_DISABLE
#define VCOS_ASSERT_LOGGING_DISABLE 0

#define  vcos_log_platform_init()               _vcos_log_platform_init()
void             _vcos_log_platform_init(void);

static inline void _vcos_thread_sem_wait(void);
static inline void _vcos_thread_sem_post(VCOS_THREAD_T *);

#define VCOS_APPLICATION_ARGC          vcos_get_argc()
#define VCOS_APPLICATION_ARGV          vcos_get_argv()

#include "generic/vcos_common.h"

VCOS_STATUS_T vcos_timer_create_pthreads(VCOS_TIMER_T *timer,
                                const char *name,
                                void (*expiration_routine)(void *context),
                                void *context);
void vcos_timer_set_pthreads(VCOS_TIMER_T *timer, VCOS_UNSIGNED delay_ms);
void vcos_timer_cancel_pthreads(VCOS_TIMER_T *timer);
void vcos_timer_reset_pthreads(VCOS_TIMER_T *timer, VCOS_UNSIGNED delay_ms);
void vcos_timer_delete_pthreads(VCOS_TIMER_T *timer);

static inline VCOS_STATUS_T vcos_timer_create(VCOS_TIMER_T *timer,
                                const char *name,
                                void (*expiration_routine)(void *context),
                                void *context)
{
   return vcos_timer_create_pthreads(timer, name, expiration_routine, context);
}

static inline void vcos_timer_set(VCOS_TIMER_T *timer, VCOS_UNSIGNED delay_ms)
{
   vcos_timer_set_pthreads(timer, delay_ms);
}

static inline void vcos_timer_cancel(VCOS_TIMER_T *timer)
{
   vcos_timer_cancel_pthreads(timer);
}

static inline void vcos_timer_reset(VCOS_TIMER_T *timer, VCOS_UNSIGNED delay_ms)
{
   vcos_timer_reset_pthreads(timer, delay_ms);
}

static inline void vcos_timer_delete(VCOS_TIMER_T *timer)
{
   vcos_timer_delete_pthreads(timer);
}

#define _VCOS_LOG_LEVEL() getenv("VC_LOGLEVEL")

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

/*
 * Fair mutexes
 */

/* Strictly speaking these are not fair -- a thread could in theory get starved
 * waiting to acquire a ticket. I'm not sure how to make an actually fair mutex
 * though, and this seems to work well enough in practice... */

typedef struct
{
   pthread_mutex_t mutex;
   pthread_cond_t cv;
   unsigned next_ticket, calling_ticket;
} VCOS_FAIR_MUTEX_T;

static inline VCOS_STATUS_T vcos_fair_mutex_create(VCOS_FAIR_MUTEX_T *m, const char *name)
{
   vcos_unused(name);

   if (pthread_mutex_init(&m->mutex, NULL) != 0)
      return vcos_pthreads_map_errno();

   if (pthread_cond_init(&m->cv, NULL) != 0)
   {
      VCOS_STATUS_T status = vcos_pthreads_map_errno();
      int destroy_success = pthread_mutex_destroy(&m->mutex);
      assert(destroy_success == 0);
      vcos_unused_in_release(destroy_success);
      return status;
   }

   m->next_ticket = 0;
   m->calling_ticket = 0;

   return VCOS_SUCCESS;
}

static inline void vcos_fair_mutex_delete(VCOS_FAIR_MUTEX_T *m)
{
   int success;

   /* If this fires, probably someone is still holding the lock... */
   assert(m->next_ticket == m->calling_ticket);

   success = pthread_cond_destroy(&m->cv);
   assert(success == 0);

   success = pthread_mutex_destroy(&m->mutex);
   assert(success == 0);

   vcos_unused_in_release(success);
}

static inline void vcos_fair_mutex_lock(VCOS_FAIR_MUTEX_T *m)
{
   int success;
   unsigned ticket;

   success = pthread_mutex_lock(&m->mutex);
   assert(success == 0);

   ticket = m->next_ticket++;
   while (m->calling_ticket != ticket)
   {
      success = pthread_cond_wait(&m->cv, &m->mutex);
      assert(success == 0);
   }

   success = pthread_mutex_unlock(&m->mutex);
   assert(success == 0);

   vcos_unused_in_release(success);
}

static inline void vcos_fair_mutex_unlock(VCOS_FAIR_MUTEX_T *m)
{
   int success;

   success = pthread_mutex_lock(&m->mutex);
   assert(success == 0);

   ++m->calling_ticket;
   success = pthread_cond_broadcast(&m->cv);
   assert(success == 0);

   success = pthread_mutex_unlock(&m->mutex);
   assert(success == 0);

   vcos_unused_in_release(success);
}

#ifdef __cplusplus
}
#endif
#endif /* VCOS_PLATFORM_H */
