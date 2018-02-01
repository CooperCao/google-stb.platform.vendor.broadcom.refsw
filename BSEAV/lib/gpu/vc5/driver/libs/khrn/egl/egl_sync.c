/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "egl_sync.h"
#include "egl_context_gl.h"
#include "../common/khrn_process.h"
#include "vcos.h"

static bool sync_init_from_khrn_fence(EGL_SYNC_T* egl_sync, EGLenum type,
      EGLenum condition, const khrn_fence *fence)
{
   egl_sync->type = type;
   egl_sync->condition = condition;

   /* for any operations with khrn_fence we need a gl lock because we are
    * addding the new fence to existing frames */
   egl_context_gl_lock();
   egl_sync->fence = khrn_fence_dup(fence);
   egl_context_gl_unlock();

   if (!egl_sync->fence)
      return false;

   egl_sync->ref_count = 1;
   return true;
}

static bool sync_init_from_job(EGL_SYNC_T* egl_sync, EGLenum type,
      EGLenum condition, uint64_t job_id)
{
   egl_sync->type = type;
   egl_sync->condition = condition;

   egl_sync->fence = khrn_fence_create();
   if (!egl_sync->fence)
      return false;

   khrn_fence_job_add(egl_sync->fence, job_id);

   egl_sync->ref_count = 1;
   return true;
}

static bool sync_init_from_cl_event(EGL_SYNC_T* egl_sync, EGLenum type,
      EGLenum condition, void *cl_event)
{
   /* TODO: Implement once cl_events are supported. */
   return false;
}

static void sync_destroy(EGL_SYNC_T *egl_sync)
{
   khrn_fence_refdec(egl_sync->fence);
}

EGL_SYNC_T* egl_sync_create(EGLenum type, EGLenum condition,
      const khrn_fence *fence)
{
   EGL_SYNC_T *egl_sync = calloc(1, sizeof(EGL_SYNC_T));

   if (!egl_sync)
      return NULL;

   if (!sync_init_from_khrn_fence(egl_sync, type, condition, fence))
   {
      free(egl_sync);
      egl_sync = NULL;
   }
   return egl_sync;
}

EGL_SYNC_T* egl_sync_create_from_job(EGLenum type, EGLenum condition,
      uint64_t job_id)
{
   EGL_SYNC_T *egl_sync = calloc(1, sizeof(EGL_SYNC_T));

   if (!egl_sync)
      return NULL;

   if (!sync_init_from_job(egl_sync, type, condition, job_id))
   {
      free(egl_sync);
      egl_sync = NULL;
   }
   return egl_sync;
}

EGL_SYNC_T* egl_sync_create_from_cl_event(EGLenum type, EGLenum condition,
      void *cl_event)
{
   EGL_SYNC_T *egl_sync = calloc(1, sizeof(EGL_SYNC_T));

   if (!egl_sync)
      return NULL;

   if (!sync_init_from_cl_event(egl_sync, type, condition, cl_event))
   {
      free(egl_sync);
      egl_sync = NULL;
   }
   return egl_sync;
}

void egl_sync_refdec(EGL_SYNC_T *egl_sync)
{
   int before_dec;

   if (!egl_sync)
      return;

   before_dec = vcos_atomic_dec(&egl_sync->ref_count);

   if (before_dec == 1)
   {
      sync_destroy(egl_sync);
      free(egl_sync);
   }
}

void egl_sync_refinc(EGL_SYNC_T *egl_sync)
{
   vcos_atomic_inc(&egl_sync->ref_count);
}

bool egl_sync_is_signaled(EGL_SYNC_T *egl_sync)
{
   bool res;

   egl_context_gl_lock();
   res = khrn_fence_reached_state(egl_sync->fence, EGL_SYNC_SIGNALED_DEPS_STATE);
   egl_context_gl_unlock();
   return res;
}
