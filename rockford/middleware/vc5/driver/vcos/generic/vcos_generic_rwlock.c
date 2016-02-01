/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  vcos

FILE DESCRIPTION
VideoCore OS Abstraction Layer - generic support
=============================================================================*/

#include "vcos.h"
#include "generic/vcos_generic_rwlock.h"

VCOS_STATUS_T vcos_generic_rwlock_create(VCOS_RWLOCK_T *rw, const char *name) {
   (void)name;
   rw->readers = 0;
   vcos_semaphore_create(&rw->rw_lock, "rw lock", 1);
   vcos_semaphore_create(&rw->read_lock, "readers access lock", 1);
   return VCOS_SUCCESS;
}

void vcos_generic_rwlock_delete(VCOS_RWLOCK_T *rw) {
   vcos_semaphore_delete(&rw->rw_lock);
   vcos_semaphore_delete(&rw->read_lock);
}

VCOS_STATUS_T vcos_generic_rwlock_read_lock(VCOS_RWLOCK_T *rw) {
   vcos_semaphore_wait(&rw->read_lock);
   if (rw->readers == 0)
      vcos_semaphore_wait(&rw->rw_lock);
   rw->readers++;
   vcos_semaphore_post(&rw->read_lock);
   return VCOS_SUCCESS;
}

VCOS_STATUS_T vcos_generic_rwlock_write_lock(VCOS_RWLOCK_T *rw) {
   vcos_semaphore_wait(&rw->rw_lock);
   return VCOS_SUCCESS;
}

void vcos_generic_rwlock_read_unlock(VCOS_RWLOCK_T *rw) {
   vcos_semaphore_wait(&rw->read_lock);
   rw->readers--;
   if (rw->readers == 0)
      vcos_semaphore_post(&rw->rw_lock);
   vcos_semaphore_post(&rw->read_lock);
}

void vcos_generic_rwlock_write_unlock(VCOS_RWLOCK_T *rw) {
   vcos_semaphore_post(&rw->rw_lock);
}

VCOS_STATUS_T vcos_generic_rwlock_read_trylock(VCOS_RWLOCK_T *rw)
{
   if (vcos_semaphore_trywait(&rw->read_lock) != VCOS_SUCCESS)
      return VCOS_EAGAIN;
   if (rw->readers == 0)
   {
      if (vcos_semaphore_trywait(&rw->rw_lock) != VCOS_SUCCESS)
      {
         vcos_semaphore_post(&rw->read_lock);
         return VCOS_EAGAIN;
      }
   }
   rw->readers++;
   vcos_semaphore_post(&rw->read_lock);
   return VCOS_SUCCESS;
}

VCOS_STATUS_T vcos_generic_rwlock_write_trylock(VCOS_RWLOCK_T *rw)
{
   return vcos_semaphore_trywait(&rw->rw_lock);
}
