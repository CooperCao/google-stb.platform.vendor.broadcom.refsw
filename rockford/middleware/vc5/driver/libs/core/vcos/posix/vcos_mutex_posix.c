/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/
#include "vcos_mutex.h"
#include "vcos_platform.h"

VCOS_STATUS_T vcos_mutex_create(VCOS_MUTEX_T *latch, const char *name)
{
   int rc = pthread_mutex_init(latch, NULL);
   if (rc == 0) return VCOS_SUCCESS;
   else return vcos_pthreads_map_errno();
}

VCOS_STATUS_T vcos_reentrant_mutex_create(VCOS_REENTRANT_MUTEX_T *m, const char *name)
{
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
