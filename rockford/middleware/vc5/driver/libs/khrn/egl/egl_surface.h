/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

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

typedef enum
{
   NONPRE,
   PRE
}
egl_surface_alphaformat_t;

/* Flush any render state writing to the surface's back buffer and return a
 * pointer to dependencies to wait for the writes to complete. */
extern const v3d_scheduler_deps* egl_surface_flush_back_buffer_writer(EGL_SURFACE_T *surface);

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
 * Check if the surface has been resized by the platform and reallocate
 * auxiliary buffers if necessary. Returns false for no memory.
 */
extern bool egl_surface_resize(EGL_SURFACE_T *surface);

#endif /* EGL_SURFACE_H */
