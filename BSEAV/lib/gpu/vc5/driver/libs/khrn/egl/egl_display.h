/*=============================================================================
Broadcom Proprietary and Confidential. (c)2013 Broadcom.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#ifndef EGL_DISPLAY_H
#define EGL_DISPLAY_H
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "libs/core/lfmt/lfmt.h"
#include "egl_map.h"

/*
 * Check if egl has been initialized, and if check_display, also that dpy is
 * valid. Set thread->error to the correct error (including EGL_SUCCESS for no
 * error) and return true if everything is OK.
 *
 * All entry points to EGL except eglInitialize itself and eglGetDisplay must
 * call this before going any further, and this guarantees that everything
 * else can rely on the existence of thread and an initialized display.
 */
extern bool egl_initialized(EGLDisplay dpy, bool check_display);

/* Returns true if dpy is a valid display handle. */
extern bool egl_is_valid_display(EGLDisplay dpy);

/* Normal lookups-- actual objects from client handles */
extern EGL_CONTEXT_T *egl_get_context(EGLContext context);
extern EGL_SURFACE_T *egl_get_surface(EGLSurface surface);

/* Returns true if any surface was created using the given native win */
bool egl_any_surfaces_using_native_win(EGLNativeWindowType win);

/* Returns true if any surface was created using the given native pixmap */
bool egl_any_surfaces_using_native_pixmap(EGLNativePixmapType pixmap);

/*
 * Looks up object from client handle. If found, an incremented ref count of
 * that object is returned. The caller must release the returned object by
 * calling egl_image_release
 */
extern EGL_IMAGE_T *egl_get_image_refinc(EGLImageKHR image);

/*
 * Looks up object from client handle. If found, an incremented ref count of
 * that object is returned.
 * The caller must release the returned object by calling egl_sync_release
 */
extern EGL_SYNC_T* egl_get_sync_refinc(EGLSyncKHR sync_id);

/*
 * Reverse lookups-- handles back from objects. Needed for apis like
 * eglGetCurrentContext
 */
extern EGLContext egl_get_context_handle(EGL_CONTEXT_T *context);
extern EGLSurface egl_get_surface_handle(EGL_SURFACE_T *surface);
extern EGLImageKHR egl_get_image_handle(EGL_IMAGE_T *image);


/* Returns NULL for no memory */
extern EGLSurface egl_map_surface(EGL_SURFACE_T *surface);

/* Returns NULL if surface wasn't mapped */
extern EGL_SURFACE_T* egl_unmap_surface(EGLSurface surface);

/* Returns NULL for no memory */
extern EGLContext egl_map_context(EGL_CONTEXT_T *context);

/* Returns NULL if context wasn't mapped */
extern EGL_CONTEXT_T* egl_unmap_context(EGLContext context);

/* Returns NULL for no memory */
extern EGLImageKHR egl_map_image(EGL_IMAGE_T *image);
extern EGL_IMAGE_T* egl_unmap_image(EGLImageKHR image);

/* Returns NULL for no memory */
extern EGLSyncKHR egl_map_sync(EGL_SYNC_T *sync);
extern EGL_SYNC_T* egl_unmap_sync(EGLSyncKHR sync_id);

/* Removes the display */
extern void egl_terminate(void);

#endif /* EGL_DISPLAY_H */
