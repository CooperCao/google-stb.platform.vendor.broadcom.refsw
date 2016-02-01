/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  vcos
File     :  $RCSfile: $
Revision :  $Revision: $

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

#include <pthread.h>
#include <semaphore.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/types.h>
#include <sched.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <signal.h>
#include <sys/time.h>

#define typeof __typeof__  /* hack as we are c89 */

#define VCOS_UNUSED(X) (void)X
#define VCOS_HAVE_RTOS         0
#define VCOS_HAVE_SEMAPHORE    1
#define VCOS_HAVE_EVENT        1
#define VCOS_HAVE_QUEUE        0
#define VCOS_HAVE_LEGACY_ISR   0
#define VCOS_HAVE_MEMPOOL      0
#define VCOS_HAVE_ISR          0
#define VCOS_HAVE_ATOMIC_FLAGS 0
#define VCOS_HAVE_THREAD_AT_EXIT        1
#define VCOS_HAVE_ONCE         1

#ifndef V3D_LEAN
#ifdef __linux__
#define VCOS_HAVE_BACKTRACE    1
#endif
#endif

#define VCOS_SO_EXT  ".so"

typedef struct {
   unsigned int    flag;
   pthread_mutex_t mutex;
   pthread_cond_t  cond;
} VCOS_EVENT_T;

typedef sem_t                 VCOS_SEMAPHORE_T;
typedef pthread_mutex_t       VCOS_MUTEX_T;

typedef uint32_t              VCOS_UNSIGNED;
typedef uint32_t              VCOS_OPTION;
typedef pthread_key_t         VCOS_TLS_KEY_T;
typedef pthread_once_t        VCOS_ONCE_T;

#define VCOS_ONCE_INIT        PTHREAD_ONCE_INIT

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
   VCOS_UNSIGNED legacy;
   char name[8];                 /**< Record the name of this thread, for diagnostics */

   /** Callback invoked at thread exit time */
   VCOS_THREAD_EXIT_T at_exit[VCOS_MAX_EXIT_HANDLERS];
} VCOS_THREAD_T;

#ifdef VCOS_PTHREADS_WANT_HISR_EMULATION

typedef struct
{
   VCOS_THREAD_T thread;
   char stack[1024];
   VCOS_SEMAPHORE_T waitsem;
} VCOS_HISR_T;

#endif

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
#define VCOS_THREAD_PRI_ABOVE_NORMAL (((VCOS_THREAD_PRI_MAX+VCOS_THREAD_PRI_MIN)/2) + 1)
#define VCOS_THREAD_PRI_REALTIME VCOS_THREAD_PRI_MAX

#define _VCOS_AFFINITY_DEFAULT 0
#define _VCOS_AFFINITY_CPU0    0x100
#define _VCOS_AFFINITY_CPU1    0x200
#define _VCOS_AFFINITY_CPU2    0x400
#define _VCOS_AFFINITY_CPU3    0x800
#define _VCOS_AFFINITY_MASK    0xF00
#define VCOS_CAN_SET_STACK_ADDR  0

typedef struct {
   uint32_t flags;
} VCOS_ATOMIC_FLAGS_T;

#define VCOS_TICKS_PER_SECOND _vcos_get_ticks_per_second()

#include "interface/vcos/generic/vcos_mem_from_malloc.h"

extern pthread_key_t _vcos_thread_current_key;

/** Convert errno values into the values recognized by vcos */
VCOSPRE_ VCOS_STATUS_T VCOSPOST_ vcos_pthreads_map_errno(void);

/** Register a function to be called when the current thread exits.
  */
extern VCOS_STATUS_T vcos_thread_at_exit(void (*pfn)(void*), void *cxt);

extern uint32_t _vcos_get_ticks_per_second(void);

#if defined(VCOS_INLINE_BODIES)

#undef VCOS_ASSERT_LOGGING_DISABLE
#define VCOS_ASSERT_LOGGING_DISABLE 1

VCOS_INLINE_IMPL
uint32_t vcos_thread_get_id(void)
{
   return (uint32_t)pthread_self();
}

VCOS_INLINE_IMPL
uint32_t vcos_getmicrosecs(void)
{
   struct timeval tv;
   uint32_t tm = 0;

   if (!gettimeofday(&tv, NULL))
      tm = (tv.tv_sec * 1000000) + tv.tv_usec;

   return tm;
}

/*
 * Counted Semaphores
 */
VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_semaphore_wait(VCOS_SEMAPHORE_T *sem) {
   int ret;
   /* gdb causes sem_wait() to EINTR when a breakpoint is hit, retry here */
   while ((ret = sem_wait(sem)) == -1 && errno == EINTR)
      continue;
   vcos_assert(ret==0);
   return VCOS_SUCCESS;
}

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_semaphore_wait_timeout(VCOS_SEMAPHORE_T *sem, int timeout) {

   if (timeout == 0) {
      int ret;
      while ((ret = sem_trywait(sem)) == -1 && errno == EINTR)
         continue;
      if (ret == 0)
         return VCOS_SUCCESS;
      else if (errno == EAGAIN)
         return VCOS_EAGAIN;
      else {
         vcos_assert(0);
         return VCOS_EINVAL;
      }
   }
   else {

      /* calculate the absolute timeout */
      struct timespec ts;
      int ret;
      long timeout_seconds, timeout_nanoseconds;
      clock_gettime(CLOCK_REALTIME, &ts);

      timeout_seconds = (long)timeout / 1000;
      timeout = timeout % 1000;
      timeout_nanoseconds = timeout * 1000000;

      ts.tv_nsec += timeout_nanoseconds;
      if (ts.tv_nsec >= 1000000000) {
         ts.tv_nsec -= 1000000000;
         ts.tv_sec++;
      }
      ts.tv_sec += timeout_seconds;

      /* need to spin over because a timed wait can abort on EINTR, not just success or timeout */
      while ((ret = sem_timedwait(sem, &ts)) == -1 && errno == EINTR)
         continue;       /* Restart if interrupted by handler */
      if (ret == -1) {
         if (errno == ETIMEDOUT) {
            return VCOS_EAGAIN;
         }
         else {
            return VCOS_EINVAL;
         }
      }

      return VCOS_SUCCESS;
   }
}

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_semaphore_create(VCOS_SEMAPHORE_T *sem,
                                                        const char *name,
                                                        VCOS_UNSIGNED initial_count) {
   int rc = sem_init(sem, 0, initial_count);

   VCOS_UNUSED(name);

   if (rc != -1) return VCOS_SUCCESS;
   else return vcos_pthreads_map_errno();
}

VCOS_INLINE_IMPL
void vcos_semaphore_delete(VCOS_SEMAPHORE_T *sem) {
#ifndef NDEBUG
   int rc = sem_destroy(sem);
   vcos_assert(rc != -1);
#else
   sem_destroy(sem);
#endif
}

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_semaphore_post(VCOS_SEMAPHORE_T *sem) {
#ifndef NDEBUG
   int rc = sem_post(sem);
   vcos_assert(rc == 0);
#else
   sem_post(sem);
#endif
   return VCOS_SUCCESS;
}

/***********************************************************
 *
 * Threads
 *
 ***********************************************************/

VCOS_INLINE_IMPL
VCOS_THREAD_T *vcos_thread_current(void) {
   void *ret = pthread_getspecific(_vcos_thread_current_key);
#ifdef __cplusplus
   return static_cast<VCOS_THREAD_T*>(ret);
#else
   return ret;
#endif
}

VCOS_INLINE_IMPL
void vcos_sleep(uint32_t ms) {
   struct timespec ts;
   ts.tv_sec = ms/1000;
   ts.tv_nsec = ms % 1000 * (1000000);
   nanosleep(&ts, NULL);
}

VCOS_INLINE_IMPL
void vcos_thread_attr_setstack(VCOS_THREAD_ATTR_T *attr, void *addr, VCOS_UNSIGNED sz) {
      attr->ta_stackaddr = addr;
      attr->ta_stacksz = sz;
   }

VCOS_INLINE_IMPL
void vcos_thread_attr_setstacksize(VCOS_THREAD_ATTR_T *attr, VCOS_UNSIGNED sz) {
      attr->ta_stacksz = sz;
   }

VCOS_INLINE_IMPL
void vcos_thread_attr_setpriority(VCOS_THREAD_ATTR_T *attr, VCOS_UNSIGNED pri) {
   attr->ta_priority = pri;
}

VCOS_INLINE_IMPL
void vcos_thread_set_priority(VCOS_THREAD_T *thread, VCOS_UNSIGNED p) {
   /* not implemented */
   VCOS_UNUSED(thread);
   VCOS_UNUSED(p);
}
VCOS_INLINE_IMPL
VCOS_UNSIGNED vcos_thread_get_priority(VCOS_THREAD_T *thread) {
   /* not implemented */
   VCOS_UNUSED(thread);
   return 0;
}


VCOS_INLINE_IMPL
void vcos_thread_attr_setaffinity(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED affinity) {
   attrs->ta_affinity = affinity;
}

VCOS_INLINE_IMPL
void vcos_thread_attr_settimeslice(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED ts) {
   attrs->ta_timeslice = ts;
}

VCOS_INLINE_IMPL
void _vcos_thread_attr_setlegacyapi(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED legacy) {
   attrs->legacy = legacy;
}

VCOS_INLINE_IMPL
void vcos_thread_attr_setautostart(VCOS_THREAD_ATTR_T *attrs, VCOS_UNSIGNED autostart) {
   VCOS_UNUSED(attrs);
   VCOS_UNUSED(autostart);
}

/*
 * Mutexes
 */

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_mutex_create(VCOS_MUTEX_T *latch, const char *name) {
   int rc = pthread_mutex_init(latch, NULL);
   VCOS_UNUSED(name);
   if (rc == 0) return VCOS_SUCCESS;
   else return vcos_pthreads_map_errno();
}

VCOS_INLINE_IMPL
void vcos_mutex_delete(VCOS_MUTEX_T *latch) {
#ifndef NDEBUG
   int rc = pthread_mutex_destroy(latch);
   vcos_assert(rc==0);
#else
   pthread_mutex_destroy(latch);
#endif
}

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_mutex_lock(VCOS_MUTEX_T *latch) {
#ifndef NDEBUG
   int rc = pthread_mutex_lock(latch);
   vcos_assert(rc==0);
#else
   pthread_mutex_lock(latch);
#endif
   return VCOS_SUCCESS;
}

VCOS_INLINE_IMPL
void vcos_mutex_unlock(VCOS_MUTEX_T *latch) {
#ifndef NDEBUG
   int rc = pthread_mutex_unlock(latch);
   vcos_assert(rc==0);
#else
   pthread_mutex_unlock(latch);
#endif
}

VCOS_INLINE_IMPL
int vcos_mutex_is_locked(VCOS_MUTEX_T *m) {
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

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_mutex_trylock(VCOS_MUTEX_T *m) {
   int rc = pthread_mutex_trylock(m);
   return (rc == 0) ? VCOS_SUCCESS : VCOS_EAGAIN;
}

/*
 * Events
 */

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_event_create(VCOS_EVENT_T *event, const char *debug_name)
{
   if (pthread_mutex_init(&event->mutex, NULL) != 0)
      goto error0;
   if (pthread_cond_init(&event->cond, NULL) != 0)
      goto error1;
   event->flag = 0;

   return VCOS_SUCCESS;

error1:
   pthread_mutex_destroy(&event->mutex);
error0:
   return vcos_pthreads_map_errno();
}

VCOS_INLINE_IMPL
void vcos_event_signal(VCOS_EVENT_T *event)
{
   pthread_mutex_lock(&event->mutex);

   event->flag = 1;
   pthread_cond_signal(&event->cond);

   pthread_mutex_unlock(&event->mutex);
}

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_event_wait(VCOS_EVENT_T *event)
{
   pthread_mutex_lock(&event->mutex);

   while (!event->flag)
      pthread_cond_wait(&event->cond, &event->mutex);

   event->flag = 0;

   pthread_mutex_unlock(&event->mutex);

   return VCOS_SUCCESS;
}

VCOS_INLINE_IMPL
void vcos_event_delete(VCOS_EVENT_T *event)
{
   pthread_cond_destroy(&event->cond);
   pthread_mutex_destroy(&event->mutex);
   event->flag = 0;
}

VCOS_INLINE_IMPL
VCOS_UNSIGNED vcos_process_id_current(void) {
   return (VCOS_UNSIGNED) getpid();
}

VCOS_INLINE_IMPL
int vcos_strcasecmp(const char *s1, const char *s2) {
   return strcasecmp(s1,s2);
}

VCOS_INLINE_IMPL
int vcos_strncasecmp(const char *s1, const char *s2, size_t n) {
   return strncasecmp(s1,s2,n);
}

VCOS_INLINE_IMPL
int vcos_in_interrupt(void) {
   return 0;
}

/* For support event groups - per thread semaphore */
VCOS_INLINE_IMPL
void _vcos_thread_sem_wait(void) {
   VCOS_THREAD_T *t = vcos_thread_current();
   vcos_semaphore_wait(&t->suspend);
}

VCOS_INLINE_IMPL
void _vcos_thread_sem_post(VCOS_THREAD_T *target) {
   vcos_semaphore_post(&target->suspend);
}

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_tls_create(VCOS_TLS_KEY_T *key, void (*destructor)(void *)) {
   int st = pthread_key_create(key, destructor);
   return st == 0 ? VCOS_SUCCESS: VCOS_ENOMEM;
}

VCOS_INLINE_IMPL
void vcos_tls_delete(VCOS_TLS_KEY_T tls) {
   pthread_key_delete(tls);
}

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_tls_set(VCOS_TLS_KEY_T tls, void *v) {
   int res = pthread_setspecific(tls, v);
   if (res != 0)
      return VCOS_EINVAL;
   else
      return VCOS_SUCCESS;
}

VCOS_INLINE_IMPL
void *vcos_tls_get(VCOS_TLS_KEY_T tls) {
   return pthread_getspecific(tls);
}


/*
 * Atomic flags
 */

VCOS_INLINE_IMPL
VCOS_STATUS_T vcos_atomic_flags_create(VCOS_ATOMIC_FLAGS_T *atomic_flags)
{
   atomic_flags->flags = 0;
   return VCOS_SUCCESS;
}

VCOS_INLINE_IMPL
void vcos_atomic_flags_delete(VCOS_ATOMIC_FLAGS_T *atomic_flags)
{
}

VCOS_INLINE_IMPL
void vcos_atomic_flags_or(VCOS_ATOMIC_FLAGS_T *atomic_flags, uint32_t flags)
{
   if (atomic_flags)
   {
      __sync_fetch_and_or( &atomic_flags->flags, flags );
   }
}

VCOS_INLINE_IMPL
uint32_t vcos_atomic_flags_get_and_clear(VCOS_ATOMIC_FLAGS_T *atomic_flags)
{
   uint32_t flags = 0;

   if (atomic_flags)
   {
      flags = __sync_fetch_and_and( &atomic_flags->flags, 0 );
   }

   return flags;
}







#if defined(linux) || defined(_HAVE_SBRK)

// not exactly the free memory, but a measure of it

VCOS_INLINE_IMPL
unsigned long vcos_get_free_mem(void) {
   return (unsigned long)sbrk(0);
}

#endif

#ifdef VCOS_PTHREADS_WANT_HISR_EMULATION
VCOS_STATUS_T vcos_legacy_hisr_create(VCOS_HISR_T *hisr, const char *name,
                                      void (*entry)(void),
                                      VCOS_UNSIGNED pri,
                                      void *stack, VCOS_UNSIGNED stack_size);

void vcos_legacy_hisr_activate(VCOS_HISR_T *hisr);

void vcos_legacy_hisr_delete(VCOS_HISR_T *hisr);

#endif

#if defined(__mips__) && defined(BCG_VC4_FAST_ATOMICS)

/* FAST ATOMICS
 *
 * These fast atomics do not have a sync before and after the atomic operation, which
 * the gcc built-in does.
 */
VCOS_INLINE_IMPL unsigned int AtomicAddConstant(volatile unsigned int *addr, unsigned int inc)
{
   unsigned int temp, res;

   __asm__ __volatile__(
      ".set push                   \n"
      ".set noreorder              \n"
      "1:                          \n"
      "ll %0, %2                   \n"  /* temp = *addr                     */
      "addu %0, %0, %3             \n"  /* temp = temp + inc                */
      "move %1, %0                 \n"  /* after = temp                     */
      "sc %0, %2                   \n"  /* *ptr  = temp; temp = ok?         */
      "beqz %0, 1b                 \n"  /* check ok                         */
      "nop                         \n"
      ".set pop                    \n"
      : "=&r" (temp), "=&r" (res), "=m" (*addr)
      : "Ir" (inc), "m" (*addr)
      : "memory");

   return res;
}

VCOS_INLINE_IMPL unsigned int AtomicSubConstant(volatile unsigned int *addr, unsigned int dec)
{
   unsigned int temp, res;

   __asm__ __volatile__(
      ".set push                   \n"
      ".set noreorder              \n"
      "1:                          \n"
      "ll %0, %2                   \n"  /* temp = *addr                     */
      "subu %0, %0, %3             \n"  /* temp = temp - dec                */
      "move %1, %0                 \n"  /* after = temp                     */
      "sc %0, %2                   \n"  /* *ptr  = temp; temp = ok?         */
      "beqz %0, 1b                 \n"  /* check ok                         */
      "nop                         \n"
      ".set pop                    \n"
      : "=&r" (temp), "=&r" (res), "=m" (*addr)
      : "Ir" (dec), "m" (*addr)
      : "memory");

   return res;
}

VCOS_INLINE_IMPL
unsigned int vcos_atomic_increment( volatile unsigned int * p )
{
   return AtomicAddConstant(p, 1);
}

VCOS_INLINE_IMPL
unsigned int vcos_atomic_decrement( volatile unsigned int * p )
{
   return AtomicSubConstant(p, 1);
}

#else

/* USE GCC built-in atomic operations
 */

VCOS_INLINE_IMPL
unsigned int vcos_atomic_increment( volatile unsigned int * p )
{
   /* semantics define value is returned post addition */
   return __sync_add_and_fetch(p, 1);
}

VCOS_INLINE_IMPL
unsigned int vcos_atomic_decrement( volatile unsigned int * p )
{
   /* semantics define value is returned post subtraction */
   return __sync_sub_and_fetch(p, 1);
}

#endif

#undef VCOS_ASSERT_LOGGING_DISABLE
#define VCOS_ASSERT_LOGGING_DISABLE 0

#else

extern VCOS_STATUS_T vcos_atomic_flags_create(VCOS_ATOMIC_FLAGS_T *atomic_flags);
extern void vcos_atomic_flags_delete(VCOS_ATOMIC_FLAGS_T *atomic_flags);
extern void vcos_atomic_flags_or(VCOS_ATOMIC_FLAGS_T *atomic_flags, uint32_t flags);
extern uint32_t vcos_atomic_flags_get_and_clear(VCOS_ATOMIC_FLAGS_T *atomic_flags);

extern unsigned int vcos_atomic_increment( volatile unsigned int * p );
extern unsigned int vcos_atomic_decrement( volatile unsigned int * p );

#endif /* VCOS_INLINE_BODIES */

VCOS_INLINE_DECL void _vcos_thread_sem_wait(void);
VCOS_INLINE_DECL void _vcos_thread_sem_post(VCOS_THREAD_T *);

#define VCOS_APPLICATION_ARGC          vcos_get_argc()
#define VCOS_APPLICATION_ARGV          vcos_get_argv()

#ifndef V3D_LEAN
#include "interface/vcos/generic/vcos_generic_reentrant_mtx.h"
#endif
#include "interface/vcos/generic/vcos_generic_named_sem.h"
#ifndef V3D_LEAN
#include "interface/vcos/generic/vcos_generic_quickslow_mutex.h"
#endif
#include "interface/vcos/generic/vcos_common.h"

#define _VCOS_LOG_LEVEL() getenv("VC_LOGLEVEL")

#ifdef __cplusplus
}
#endif
#endif /* VCOS_PLATFORM_H */
