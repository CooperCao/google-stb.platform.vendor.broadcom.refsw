/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#ifndef EGL_CONTEXT_H
#define EGL_CONTEXT_H
#include <stdbool.h>
#include "egl_types.h"
#include "../common/khrn_image.h"

extern void egl_context_flush(EGL_CONTEXT_T *context);

/* see flush_rendering in egl_context_base.h */
extern v3d_scheduler_deps *egl_context_flush_rendering(
      EGL_CONTEXT_T *context, EGL_SURFACE_T *surface);

/* see copy_surface in egl_context_base.h */
extern bool egl_context_copy_surface(EGL_CONTEXT_T *context,
      EGL_SURFACE_T *surface, KHRN_IMAGE_T *dst);

/*
 * Prepare context for rendering to the draw surface.
 *
 * Sets context's draw and read surfaces.
 */
extern void egl_context_attach(EGL_CONTEXT_T *context,
      EGL_SURFACE_T *draw, EGL_SURFACE_T *read);

/*
 * Call this whenever context's draw surface's back buffer changes.
 *
 * FIXME: Would be better if GL called functions on the context to get the
 * back buffers when it needed them rather than requiring this to be called so
 * it can make copies of them. Better to get rid of the copies. Then you
 * wouldn't need attach and detach to be virtual functions at all-- you'd just
 * set those surfaces and that's all there is to it.
 */
extern void egl_context_reattach(EGL_CONTEXT_T *context);

/*
 * Detach surfaces from context and context from thread it may be bound to.
 * Surfaces and contexts are deleted if they are no longer in use.
 */
extern void egl_context_detach(EGL_CONTEXT_T *context);

/* Wait for all rendering to finish (see eglWaitClient) */
extern void egl_context_wait(EGL_CONTEXT_T *context);

/*
 * Destroy and free the context if has no surfaces bound to it and isn't
 * mapped to a user handle. So you should call this whenever you detach or
 * unmap it. Returns true if context was actually deleted.
 */
extern bool egl_context_try_delete(EGL_CONTEXT_T *context);

/* Returns the version number of the client API that this context supports. */
extern int egl_context_client_version(const EGL_CONTEXT_T *context);

/*
 * Add a fence for context to wait for before doing any rendering to
 * surface's back buffer. If context is NULL, uses the current context.
 */
extern void egl_context_add_fence(EGL_CONTEXT_T *context,
      const EGL_SURFACE_T *surface, int fence);

#endif /* EGL_CONTEXT_H */
