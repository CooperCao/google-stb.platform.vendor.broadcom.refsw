/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef EGL_SURFACE_H
#define EGL_SURFACE_H
#include "../common/khrn_image.h"
#include "egl_types.h"
#include <EGL/eglext.h>

typedef enum
{
   LINEAR,
   SRGB
}
egl_surface_colorspace_t;

extern GFX_LFMT_T egl_surface_lfmt_to_srgb(GFX_LFMT_T lfmt);

/* Flush any render state writing to the surface's back buffer and return a
 * pointer to dependencies to wait for the writes to complete. */
extern const v3d_scheduler_deps* egl_surface_flush_back_buffer_writer(EGL_SURFACE_T *surface);

extern bool egl_surface_get_attrib(const EGL_SURFACE_T *surface,
      EGLint attrib, EGLint *value);

/*
 * Destroy and free the surface if it isn't bound to a context or mapped to a
 * user handle. So you should call this whenever you unbind or unmap it.
 * Returns true if surface was actually deleted.
 */
extern bool egl_surface_try_delete(EGL_SURFACE_T *surface);

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
extern khrn_image *egl_surface_get_back_buffer(const EGL_SURFACE_T *surface);
extern khrn_image *egl_surface_get_aux_buffer(const EGL_SURFACE_T *surface,
      egl_aux_buf_t which);

// Caller must khrn_mem_release() returned pointer
extern khrn_image *egl_surface_get_back_buffer_with_gl_colorspace(const EGL_SURFACE_T *surface);

extern EGL_SURFACE_T *egl_surface_lock(EGLSurface handle);
extern void egl_surface_unlock(EGL_SURFACE_T *surface); /* tolerates NULL */

/*
 * Check if the surface has been resized by the platform and reallocate
 * auxiliary buffers if necessary. Returns false for no memory.
 */
extern bool egl_surface_resize(EGL_SURFACE_T *surface);

#endif /* EGL_SURFACE_H */
