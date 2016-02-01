/*=============================================================================
Copyright (c) 2014 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION

=============================================================================*/
#ifndef EGL_SYNC_H
#define EGL_SYNC_H
#include "interface/khronos/include/EGL/egl.h"
#include "middleware/khronos/common/khrn_fence.h"
#include "middleware/khronos/egl/egl_types.h"

struct egl_sync
{
   EGLenum type;
   EGLenum condition; /* signalling condition : e.g prior commands */
   bool signaled;
   KHRN_FENCE_T *fence;
   volatile int ref_count;
};

extern bool egl_syncs_lock_init(void);
extern void egl_syncs_lock_deinit(void);

extern EGL_SYNC_T* egl_sync_create(EGLenum type, EGLenum condition, const
      KHRN_FENCE_T *fence);
extern EGL_SYNC_T* egl_sync_create_from_fd(EGLenum type, EGLenum condition,
      int fd);
extern EGL_SYNC_T* egl_sync_create_from_cl_event(EGLenum type, EGLenum condition,
      void *cl_event);

extern void egl_sync_refdec(EGL_SYNC_T* sync);
extern void egl_sync_refinc(EGL_SYNC_T *sync);

/* an eglsync object is signaled when the dependencies specified in fence
 * reach state = completed */
#define EGL_SYNC_SIGNALED_DEPS_STATE V3D_SCHED_DEPS_COMPLETED
extern bool egl_sync_is_signaled(EGL_SYNC_T*);
extern void egl_sync_set_signaled(EGL_SYNC_T * sync);

#endif /* EGL_SYNC_H */
