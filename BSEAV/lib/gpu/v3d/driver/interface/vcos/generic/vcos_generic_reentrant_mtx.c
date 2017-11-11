/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "interface/vcos/vcos.h"
#include "interface/vcos/vcos_reentrant_mutex.h"

VCOS_STATUS_T vcos_generic_reentrant_mutex_create(VCOS_REENTRANT_MUTEX_T *m, const char *name)
{
   m->count = 0;
   m->owner = 0;
   return vcos_mutex_create(&m->mutex, name);
}

void vcos_generic_reentrant_mutex_delete(VCOS_REENTRANT_MUTEX_T *m)
{
   assert(m->count == 0);
   vcos_mutex_delete(&m->mutex);
}

void vcos_generic_reentrant_mutex_lock(VCOS_REENTRANT_MUTEX_T *m)
{
   assert(m);
   VCOS_THREAD_T *thread = vcos_thread_current();

   assert(thread != 0);

   if (m->owner != thread)
   {
      vcos_mutex_lock(&m->mutex);
      m->owner = thread;
      assert(m->count == 0);
   }
   m->count++;
}

void vcos_generic_reentrant_mutex_unlock(VCOS_REENTRANT_MUTEX_T *m)
{
   assert(m->count != 0);
   assert(m->owner == vcos_thread_current());
   m->count--;
   if (m->count == 0)
   {
      m->owner = 0;
      vcos_mutex_unlock(&m->mutex);
   }
}
