/*=============================================================================
Broadcom Proprietary and Confidential. (c)2009 Broadcom.
All rights reserved.

Project  :  vcfw
Module   :  vcos
File     :  $RCSfile: $
Revision :  $Revision: $

FILE DESCRIPTION
VideoCore OS Abstraction Layer - pthreads types
=============================================================================*/

#define _GNU_SOURCE /* required for CPU_SET/CPU_ZERO (defined in sched.h) */
#define VCOS_INLINE_BODIES
#include "interface/vcos/vcos.h"
#include "interface/vcos/vcos_msgqueue.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/param.h>
#include <sched.h>

#ifdef HAVE_CMAKE_CONFIG
#include "cmake_config.h"
#endif

#ifdef HAVE_MTRACE
#include <mcheck.h>
#endif

#ifdef ANDROID
#include <cutils/log.h>
#endif

#ifndef VCOS_DEFAULT_STACK_SIZE
#define VCOS_DEFAULT_STACK_SIZE 4096
#endif

static int vcos_argc;
static const char **vcos_argv;

typedef void (*LEGACY_ENTRY_FN_T)(int, void *);

static VCOS_THREAD_ATTR_T default_attrs = {
   0,
   VCOS_DEFAULT_STACK_SIZE,
   0, /* ta_priority */
   0, /* ta_affinity */
   0, /* ta_timeslice */
   0  /* legacy */
};

static VCOS_MUTEX_T lock;
pthread_key_t _vcos_thread_current_key;

#ifdef ANDROID
static VCOS_MUTEX_T printf_lock;
#endif

/* A VCOS wrapper for the thread which called vcos_init. */
static VCOS_THREAD_T vcos_thread_main;

static void *vcos_thread_entry(void *arg)
{
   int i;
   void *ret;
   VCOS_THREAD_T *thread = (VCOS_THREAD_T *)arg;
   pthread_setspecific(_vcos_thread_current_key, thread);

   if (thread->legacy)
   {
      LEGACY_ENTRY_FN_T fn = (LEGACY_ENTRY_FN_T)thread->entry;
      (*fn)(0, thread->arg);
      ret = 0;
   }
   else
   {
      ret = (*thread->entry)(thread->arg);
   }

   // call termination functions
   for (i=0; thread->at_exit[i].pfn != NULL; i++)
   {
      thread->at_exit[i].pfn(thread->at_exit[i].cxt);
   }

   return ret;
}

VCOS_STATUS_T vcos_thread_create(VCOS_THREAD_T *thread,
                                 const char *name,
                                 VCOS_THREAD_ATTR_T *attrs,
                                 VCOS_THREAD_ENTRY_FN_T entry,
                                 void *arg)
{
   VCOS_STATUS_T st;
   pthread_attr_t pt_attrs;
   struct sched_param param;
   VCOS_THREAD_ATTR_T *local_attrs = attrs ? attrs : &default_attrs;
   int policy, priority;

   int rc = pthread_attr_init(&pt_attrs);
   if (rc < 0)
      return VCOS_ENOMEM;

   st = vcos_semaphore_create(&thread->suspend, NULL, 0);
   if (st != VCOS_SUCCESS)
   {
      pthread_attr_destroy(&pt_attrs);
      return st;
   }

   pthread_attr_setstacksize(&pt_attrs, local_attrs->ta_stacksz);
#if VCOS_CAN_SET_STACK_ADDR
   if (local_attrs->ta_stackaddr)
   {
      pthread_attr_setstackaddr(&pt_attrs, local_attrs->ta_stackaddr);
   }
#else
   vcos_demand(local_attrs->ta_stackaddr == 0);
#endif

   /* way to use the CPP to print compilation flags during compilation */
#define XSTR(x) STR(x)
#define STR(x) #x

   /* example - set as -DSCHED_OVERRIDE=SCHED_RR */
#if SCHED_OVERRIDE
   policy = SCHED_OVERRIDE;
#pragma message ("scheduling policy overridden " XSTR(SCHED_OVERRIDE))
#else
   policy = SCHED_FIFO;
#endif /* SCHED_OVERRIDE */

   /* example - set as -DPRIORITY_OVERRIDE=99 */
#if PRIORITY_OVERRIDE
   priority = PRIORITY_OVERRIDE;
#pragma message ("scheduling priority overridden " XSTR(PRIORITY_OVERRIDE))
#else
   priority = sched_get_priority_max(policy);
#endif /* PRIORITY_OVERRIDE */

   if (local_attrs->ta_priority != 0)
   {
      pthread_attr_setschedpolicy(&pt_attrs, policy);
      pthread_attr_getschedparam(&pt_attrs, &param);
      param.sched_priority = priority;
      /* set it to realtime for anything other than '0' */
      pthread_attr_setschedparam(&pt_attrs, &param);
   }

#ifndef ANDROID
   if (local_attrs->ta_affinity != 0)
   {
      cpu_set_t cpuset;

      CPU_ZERO(&cpuset);
      if (local_attrs->ta_affinity & VCOS_AFFINITY_CPU0)
         CPU_SET(0, &cpuset);
      if (local_attrs->ta_affinity & VCOS_AFFINITY_CPU1)
         CPU_SET(1, &cpuset);
      if (local_attrs->ta_affinity & VCOS_AFFINITY_CPU2)
         CPU_SET(2, &cpuset);
      if (local_attrs->ta_affinity & VCOS_AFFINITY_CPU3)
         CPU_SET(3, &cpuset);

      pthread_attr_setaffinity_np(&pt_attrs, sizeof(cpu_set_t), &cpuset);
   }
#endif

   vcos_assert(local_attrs->ta_stackaddr == 0); /* Not possible */

   thread->entry = entry;
   thread->arg = arg;
   thread->legacy = local_attrs->legacy;

   rc = pthread_create(&thread->thread, &pt_attrs, vcos_thread_entry, thread);

   /* reapply again, as some versions of NPTL ignore attributes set prior to creation */
   if (local_attrs->ta_priority != 0)
   {
      pthread_setschedparam(thread->thread, policy, &param);
   }

   pthread_attr_destroy(&pt_attrs);

   strncpy(thread->name, name, sizeof(thread->name));
   thread->name[sizeof(thread->name)-1] = '\0';
   memset(thread->at_exit, 0, sizeof(thread->at_exit));

   if (rc < 0)
   {
      vcos_semaphore_delete(&thread->suspend);
      return VCOS_ENOMEM;
   }
   else
   {
      return VCOS_SUCCESS;
   }
}

void vcos_thread_join(VCOS_THREAD_T *thread,
                             void **pData)
{
   if (thread)
   {
      pthread_join(thread->thread, pData);
      vcos_semaphore_delete(&thread->suspend);
   }
}

VCOSPRE_ VCOS_STATUS_T VCOSPOST_ vcos_thread_create_classic(VCOS_THREAD_T *thread,
                                                            const char *name,
                                                            void *(*entry)(void *arg),
                                                            void *arg,
                                                            void *stack,
                                                            VCOS_UNSIGNED stacksz,
                                                            VCOS_UNSIGNED priaff,
                                                            VCOS_UNSIGNED timeslice,
                                                            VCOS_UNSIGNED autostart)
{
   VCOS_THREAD_ATTR_T attrs;
   VCOS_UNUSED(timeslice);
   VCOS_UNUSED(autostart);
   vcos_thread_attr_init(&attrs);
   vcos_thread_attr_setstacksize(&attrs, stacksz);
   vcos_thread_attr_setpriority(&attrs, priaff & ~_VCOS_AFFINITY_MASK);
   vcos_thread_attr_setaffinity(&attrs, priaff & _VCOS_AFFINITY_MASK);

   if (VCOS_CAN_SET_STACK_ADDR)
   {
      vcos_thread_attr_setstack(&attrs, stack, stacksz);
   }

   return vcos_thread_create(thread, name, &attrs, entry, arg);
}

#ifndef V3D_LEAN
void vcos_log_impl(const VCOS_LOG_CAT_T *cat, const char *fmt, ...)
{
   va_list ap;
   if (cat->flags.want_prefix)
      fprintf(stdout, "%s:", cat->name);
   va_start(ap,fmt);
   vfprintf(stdout, fmt, ap);
   fprintf(stdout, "\n");
   va_end(ap);
}

void vcos_vlog_impl(const VCOS_LOG_CAT_T *cat, const char *fmt, va_list args)
{
   if (cat->flags.want_prefix)
      fprintf(stdout, "%s:", cat->name);
   vfprintf(stdout, fmt, args);
   fprintf(stdout, "\n");
}
#endif

VCOS_STATUS_T vcos_init(void)
{
   VCOS_STATUS_T st = VCOS_SUCCESS;

#ifdef HAVE_MTRACE
   // enable glibc memory debugging, if the environment
   // variable MALLOC_TRACE names a valid file.
   mtrace();
#endif

   vcos_mutex_create(&lock, "global_lock");

#ifdef ANDROID
   vcos_mutex_create(&printf_lock, "printf");
#endif

   _vcos_named_semaphore_init();

   vcos_demand(pthread_key_create(&_vcos_thread_current_key, NULL) == 0);

   /* Initialise a VCOS wrapper for the thread which called vcos_init. */
   vcos_semaphore_create(&vcos_thread_main.suspend, NULL, 0);
   vcos_thread_main.thread = pthread_self();
   pthread_setspecific(_vcos_thread_current_key, &vcos_thread_main);

#ifndef V3D_LEAN
   st = vcos_msgq_init();
   vcos_assert(st == VCOS_SUCCESS);
   vcos_logging_init();
#endif

   return st;
}

void vcos_deinit(void)
{
   _vcos_named_semaphore_deinit();
}

void vcos_global_lock(void)
{
   vcos_mutex_lock(&lock);
}

void vcos_global_unlock(void)
{
   vcos_mutex_unlock(&lock);
}

void vcos_thread_exit(void *arg)
{
   pthread_exit(arg);
}


void vcos_thread_attr_init(VCOS_THREAD_ATTR_T *attrs)
{
   *attrs = default_attrs;
}

VCOS_STATUS_T vcos_pthreads_map_errno(void)
{
   switch (errno)
   {
   case ENOMEM:
      return VCOS_ENOMEM;
   case ENXIO:
      return VCOS_ENXIO;
   case EAGAIN:
      return VCOS_EAGAIN;
   case ENOSPC:
      return VCOS_ENOSPC;
   default:
      return VCOS_EINVAL;
   }
}

int vcos_snprintf(char *buf, size_t buflen, const char *fmt, ...)
{
   int ret;
   va_list ap;
   va_start(ap,fmt);
   ret = vsnprintf(buf, buflen, fmt, ap);
   va_end(ap);
   return ret;
}

int vcos_llthread_running(VCOS_THREAD_T *t) {
   VCOS_UNUSED(t);
   vcos_assert(0);   // this function only exists as a nasty hack for the video codecs!
   return 1;
}

int vcos_have_rtos(void)
{
   return 1;
}

const char * vcos_thread_get_name(const VCOS_THREAD_T *thread)
{
   return thread->name;
}

void vcos_pthreads_logging_assert(const char *file, const char *func, unsigned int line, const char *fmt, ...)
{
#ifdef ANDROID
   /* LOGD buffer length is max 1k.  Android doesn't expose a #define for this. */
   char str[1024];
   va_list ap;

   /* serialize through here */
   vcos_mutex_lock(&printf_lock);
   va_start(ap, fmt);

   LOGD("assertion failure:%s:%d:%s():", file, line, func);
   vsnprintf(str, sizeof(str), fmt, ap);
   va_end(ap);
   LOGD("%s\n", str);

   vcos_mutex_unlock(&printf_lock);
#else
   va_list ap;
   va_start(ap, fmt);
   fprintf(stderr, "assertion failure:%s:%d:%s():",
           file, line, func);
   vfprintf(stderr, fmt, ap);
   va_end(ap);
   fprintf(stderr, "\n");
#ifdef VCOS_HAVE_BACKTRACE
   vcos_backtrace_self();
#endif
#endif /* ANDROID */
}

extern VCOS_STATUS_T vcos_thread_at_exit(void (*pfn)(void*), void *cxt)
{
   int i;
   VCOS_THREAD_T *self = vcos_thread_current();
   if (!self)
   {
      vcos_assert(0);
      return VCOS_EINVAL;
   }
   for (i=0; i<VCOS_MAX_EXIT_HANDLERS; i++)
   {
      if (self->at_exit[i].pfn == NULL)
      {
         self->at_exit[i].pfn = pfn;
         self->at_exit[i].cxt = cxt;
         return VCOS_SUCCESS;
      }
   }
   return VCOS_ENOSPC;
}

void vcos_set_args(int argc, const char **argv)
{
   vcos_argc = argc;
   vcos_argv = argv;
}

int vcos_get_argc(void)
{
   return vcos_argc;
}

const char ** vcos_get_argv(void)
{
   return vcos_argv;
}

// we can't inline this, because HZ comes from sys/param.h which
// dumps all sorts of junk into the global namespace, notable MIN and
// MAX.
uint32_t _vcos_get_ticks_per_second(void)
{
   return HZ;
}

VCOS_STATUS_T vcos_once(VCOS_ONCE_T *once_control,
                        void (*init_routine)(void))
{
   int rc = pthread_once(once_control, init_routine);
   if (rc != 0)
   {
      switch (errno)
      {
      case EINVAL:
         return VCOS_EINVAL;
      default:
         vcos_assert(0);
         return VCOS_EACCESS;
      }
   }
   else
   {
      return VCOS_SUCCESS;
   }
}

int vcos_thread_num_processors(void)
{
   return sysconf(_SC_NPROCESSORS_ONLN);
}
