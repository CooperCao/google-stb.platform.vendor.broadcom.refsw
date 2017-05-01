/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once
#include "libs/util/assert_helpers.h"
#include <errno.h>
#include <pthread.h>

typedef pthread_mutex_t VCOS_MUTEX_T;
typedef pthread_mutex_t VCOS_REENTRANT_MUTEX_T;

#ifdef __cplusplus
extern "C" {
#endif

static inline void vcos_mutex_delete(VCOS_MUTEX_T *latch)
{
   verif(pthread_mutex_destroy(latch) == 0);
}

static inline void vcos_mutex_lock(VCOS_MUTEX_T *latch)
{
   verif(pthread_mutex_lock(latch) == 0);
}

static inline void vcos_mutex_unlock(VCOS_MUTEX_T *latch)
{
   verif(pthread_mutex_unlock(latch) == 0);
}

static inline bool vcos_mutex_trylock(VCOS_MUTEX_T *m)
{
   int rc = pthread_mutex_trylock(m);
   assert(rc == 0 || rc == EBUSY);
   return rc == 0;
}

static inline bool vcos_mutex_is_locked(VCOS_MUTEX_T *m)
{
   int rc = pthread_mutex_trylock(m);
   if (rc == 0) {
      pthread_mutex_unlock(m);
      /* it wasn't locked */
      return false;
   }
   else {
      return true; /* it was locked */
   }
}


static inline void vcos_reentrant_mutex_delete(VCOS_REENTRANT_MUTEX_T *m)
{
   verif(pthread_mutex_destroy(m) == 0);
}

static inline void vcos_reentrant_mutex_lock(VCOS_REENTRANT_MUTEX_T *m)
{
   verif(pthread_mutex_lock(m) == 0);
}

static inline void vcos_reentrant_mutex_unlock(VCOS_REENTRANT_MUTEX_T *m)
{
   verif(pthread_mutex_unlock(m) == 0);
}

#ifdef __cplusplus
}
#endif