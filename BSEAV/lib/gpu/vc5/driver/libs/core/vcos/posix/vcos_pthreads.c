/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include <string.h>
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>
#include <sys/param.h>

#ifdef HAVE_CMAKE_CONFIG
#include "cmake_config.h"
#endif

#ifdef HAVE_MTRACE
#include <mcheck.h>
#endif

/** Singleton global lock used for vcos_global_lock/unlock(). */
static pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER;

uint64_t vcos_getmicrosecs64_internal(void)
{
   struct timeval tv;
   uint64_t tm = 0;

   if (!gettimeofday(&tv, NULL))
   {
      tm = (tv.tv_sec * 1000000LL) + tv.tv_usec;
   }

   return tm;
}

VCOS_STATUS_T vcos_platform_init(void)
{
#ifdef HAVE_MTRACE
   /* enable glibc memory debugging, if the environment
    * variable MALLOC_TRACE names a valid file.
    */
   mtrace();
#endif

   return VCOS_SUCCESS;
}

void vcos_platform_deinit(void)
{
}

void vcos_global_lock(void)
{
   pthread_mutex_lock(&lock);
}

void vcos_global_unlock(void)
{
   pthread_mutex_unlock(&lock);
}


VCOS_STATUS_T vcos_pthreads_map_error(int error)
{
   switch (error)
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

VCOS_STATUS_T vcos_pthreads_map_errno(void)
{
   return vcos_pthreads_map_error(errno);
}

/* we can't inline this, because HZ comes from sys/param.h which
 * dumps all sorts of junk into the global namespace, notable MIN and
 * MAX.
 */
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
         assert(0);
         return VCOS_EACCESS;
      }
   }
   else
   {
      return VCOS_SUCCESS;
   }
}
