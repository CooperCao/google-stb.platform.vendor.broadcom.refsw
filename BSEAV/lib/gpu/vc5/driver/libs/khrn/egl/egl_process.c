/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "vcos_mutex.h"
#include "libs/util/demand.h" /* TODO Shouldn't be using demand in production code... */
#include "egl_process.h"
#include "egl_platform.h"
#include "egl_context_gl.h"
#include "../common/khrn_process.h"

LOG_DEFAULT_CAT("egl_process")

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
      log_error("Fatal: unable to init once egl_display");
   }

   // Create the gl context lock for that process
   egl_context_gl_create_lock();
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
