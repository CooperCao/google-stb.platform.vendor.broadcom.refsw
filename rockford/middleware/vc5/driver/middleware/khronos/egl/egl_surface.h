/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#ifndef EGL_SURFACE_H
#define EGL_SURFACE_H
#include "middleware/khronos/common/khrn_image.h"
#include "middleware/khronos/egl/egl_types.h"
#include "interface/khronos/include/EGL/eglext.h"

typedef enum
{
   LINEAR,
   SRGB
}
egl_surface_colorspace_t;

typedef enum
{
   NONPRE,
   PRE
}
egl_surface_alphaformat_t;

/* see flush_rendering in egl_context_base.h */
extern v3d_scheduler_deps* egl_surface_flush_rendering(EGL_SURFACE_T *surface);

/* see copy_surface in egl_context_base.h */
extern bool egl_surface_copy(EGL_SURFACE_T *surface, KHRN_IMAGE_T *dst);

/*
 * Destroy and free the surface if it isn't bound to a context or mapped to a
 * user handle. So you should call this whenever you unbind or unmap it.
 * Returns true if surface was actually deleted.
 */
extern bool egl_surface_try_delete(EGL_SURFACE_T *surface);

/* Returns an EGL error code */
extern EGLint egl_surface_set_attrib(EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR value);

/*
 * Get the thread which the context surface is bound to is in (or NULL if it's
 * not bound or is EGL_NO_SURFACE)
 */
extern EGL_THREAD_T *egl_surface_get_thread(const EGL_SURFACE_T *surface);

/* Important: if surface->context != NULL,
 * the image returned by egl_back_buffer or egl_surface_get_aux_buffer is
 * shared with the client api, so make sure you have the correct lock when
 * doing operations in egl with this image;
 *( e.g : see how egl_surface_get_back_buffer is used in egl_context_gl.c) */
extern KHRN_IMAGE_T *egl_surface_get_back_buffer(const EGL_SURFACE_T *surface);
extern KHRN_IMAGE_T *egl_surface_get_aux_buffer(const EGL_SURFACE_T *surface,
      egl_aux_buf_t which);

extern EGL_SURFACE_T *egl_surface_lock(EGLSurface handle);
extern void egl_surface_unlock(EGL_SURFACE_T *surface); /* tolerates NULL */

/*
 * Call this to indicate that the contents of all surface's auxiliary buffers
 * and optionally also its current back buffer (i.e. color buffer) contain
 * undefined data. This saves copying that data to the hw for it to render on
 * top of.
 */
extern void egl_surface_invalidate(EGL_SURFACE_T *surface, bool include_color);

/*
 * Check if the surface has been resized by the platform and reallocate
 * auxiliary buffers if necessary. Returns false for no memory.
 */
extern bool egl_surface_resize(EGL_SURFACE_T *surface);

#endif /* EGL_SURFACE_H */
