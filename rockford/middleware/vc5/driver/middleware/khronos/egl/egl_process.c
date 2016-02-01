/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.
=============================================================================*/

#include "vcos.h"
#include "vcos_mutex.h"
#include "helpers/demand.h" /* TODO Shouldn't be using demand in production code... */
#include "egl_process.h"
#include "egl_platform.h"
#include "middleware/khronos/common/khrn_process.h"

#define VCOS_LOG_CATEGORY (&egl_process_log)
VCOS_LOG_CAT_T egl_process_log = VCOS_LOG_INIT("egl_process", VCOS_LOG_INFO);

static struct
{
   VCOS_ONCE_T    once;
   bool           once_ok;
   VCOS_MUTEX_T   lock;
   unsigned       refs;
   bool           init;
} egl_process;

static void init_once(void)
{
   egl_process.once_ok = vcos_mutex_create(&egl_process.lock, "egl_process.lock") == VCOS_SUCCESS;

   if (!egl_process.once_ok)
   {
      vcos_log_error("Fatal: unable to init once egl_display");
   }
}

static bool ensure_init_once(void)
{
   demand(vcos_once(&egl_process.once, init_once) == VCOS_SUCCESS);
   return egl_process.once_ok;
}

/*
 * Set up all the per-process state. Note that this state can be initialized
 * and terminated many times during the life of the process.
 */
bool egl_process_init(void)
{
   if (!ensure_init_once())
   {
      return false;
   }

   vcos_mutex_lock(&egl_process.lock);

   if (!egl_process.init)
   {
      if (egl_platform_init())
      {
         egl_process.init = khrn_process_init();

         if (!egl_process.init)
            egl_platform_terminate();
      }
   }
   egl_process.refs += (unsigned)egl_process.init;

   vcos_mutex_unlock(&egl_process.lock);

   return egl_process.init;
}

void egl_process_add_ref(void)
{
   if (!ensure_init_once())
   {
      return;
   }

   vcos_mutex_lock(&egl_process.lock);
   egl_process.refs += 1;
   vcos_mutex_unlock(&egl_process.lock);
}

void egl_process_release(void)
{
   if (!ensure_init_once())
   {
      return;
   }

   vcos_mutex_lock(&egl_process.lock);

   assert(egl_process.refs > 0);
   if (!--egl_process.refs)
   {
      if (egl_process.init)
      {
         khrn_process_shutdown();
         egl_platform_terminate();
         egl_process.init = false;
      }
   }

   vcos_mutex_unlock(&egl_process.lock);
}
