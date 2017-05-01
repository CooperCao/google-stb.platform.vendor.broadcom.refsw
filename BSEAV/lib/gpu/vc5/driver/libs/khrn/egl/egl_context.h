/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_CONTEXT_H
#define EGL_CONTEXT_H
#include <stdbool.h>
#include "egl_types.h"
#include "../common/khrn_image.h"

extern void egl_context_flush(EGL_CONTEXT_T *context);

/* see convert_image in egl_context_base.h */
extern bool egl_context_convert_image(EGL_CONTEXT_T *context,
      khrn_image *dst, khrn_image *src);

extern void egl_context_invalidate_draw(EGL_CONTEXT_T *context,
   bool color, bool color_ms, bool other_aux);

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

#endif /* EGL_CONTEXT_H */
