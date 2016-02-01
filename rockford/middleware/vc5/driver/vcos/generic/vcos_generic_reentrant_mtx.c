/*=============================================================================
Copyright (c) 2009 Broadcom Europe Limited.
All rights reserved.

Project  :  vcfw
Module   :  chip driver

FILE DESCRIPTION
VideoCore OS Abstraction Layer - reentrant mutexes created from regular ones.
=============================================================================*/

#include "vcos.h"
#include "vcos_reentrant_mutex.h"

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
   VCOS_THREAD_T *thread = vcos_thread_current();
   assert(m);

   assert(thread != 0);
   assert(m->count >= 0);

   /* loudly detect double-unlocks */
   if (m->count < 0) {
      VCOS_THREAD_T *owner = m->owner;
      VCOS_ALERT("%s: double unlock (%d) in thread %s by %s",
                 VCOS_FUNCTION,
                 m->count,
                 vcos_thread_get_name(thread),
                 owner ? vcos_thread_get_name(owner) : "?");
   }

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
   VCOS_THREAD_T *owner = m->owner;

   if (m->owner != vcos_thread_current())
   {
      VCOS_ALERT("%s: unlocking mtx owned by %s by thread %s",
                 VCOS_FUNCTION,
                 owner ? vcos_thread_get_name(owner) : "?",
                 vcos_thread_get_name(vcos_thread_current()));
      assert(0);
   }

   m->count--;

   /* loudly detect double-unlocks */
   if (m->count < 0)
   {
      VCOS_ALERT("%s: double unlock (%d) in thread %s for thread %s",
                 VCOS_FUNCTION,
                 m->count,
                 vcos_thread_get_name(vcos_thread_current()),
                 owner ? vcos_thread_get_name(owner) : "?");
      assert(0);
   }

   if (m->count == 0)
   {
      m->owner = 0;
      vcos_mutex_unlock(&m->mutex);
   }
}
