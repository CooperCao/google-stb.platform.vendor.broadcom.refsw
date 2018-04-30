/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include "interface/khronos/common/khrn_int_image.h"
#include "interface/khronos/egl/egl_int.h"
#include "interface/khronos/egl/egl_client_surface.h"
#include "interface/khronos/common/khrn_client.h"
#include "middleware/khronos/glxx/glxx_server.h"

extern bool egl_create_surface(
   EGL_SURFACE_T *surface,
   uintptr_t win,
   uint32_t buffers,
   uint32_t width,
   uint32_t height,
   bool secure,
   uint32_t mipmap,
   KHRN_IMAGE_T *pixmap_image,
   uint32_t type);

extern bool egl_create_wrapped_surface(
   EGL_SURFACE_T *surface,
   void *pixmap);

// Create server states. To actually use these, call, eglIntMakeCurrent.
extern void *egl_create_glxx_server_state(void *share_context, EGL_CONTEXT_TYPE_T share_type, bool secure);

// Flushes one or both context, and waits for the flushes to complete before returning.
// Equivalent to:
// if (flushgl) glFinish())
extern void eglIntFlush_impl(bool flushgl);
extern void eglIntFinish_impl(bool finishgl);

extern bool egl_back_buffer_dims(EGL_SURFACE_T *surface, uint32_t *width, uint32_t *height);

extern void egl_swapbuffers(EGL_SURFACE_T *surface);
extern void egl_select_mipmap(EGL_SURFACE_T *surface, int level);

extern int egl_copybuffers(EGL_SURFACE_T *surface, EGLNativePixmapType pixmap);
extern void egl_get_color_data(EGL_SURFACE_T *surface, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, void *data);
extern void egl_set_color_data(EGL_SURFACE_T *surface, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, const void *data);

extern bool egl_bind_tex_image(EGL_SURFACE_T *surface);
extern void egl_release_tex_image(EGL_SURFACE_T *surface);

extern void egl_update_gl_buffers(EGL_CURRENT_T *opengl);

#if EGL_KHR_image
extern EGLImageKHR eglCreateImageKHR_impl(EGLenum target, EGLClientBuffer buffer, EGLint texture_level, EGLint *results);
extern EGLBoolean eglDestroyImageKHR_impl(EGLImageKHR image);
#endif

#ifdef WAYLAND
struct wl_display;
struct wl_resource;
extern  EGLBoolean eglBindWaylandDisplayWL_impl(EGLDisplay dpy, struct wl_display *display);
extern  EGLBoolean eglUnbindWaylandDisplayWL_impl(EGLDisplay dpy, struct wl_display *display);
extern  EGLBoolean eglQueryWaylandBufferWL_impl(EGLDisplay dpy, struct wl_resource *buffer, EGLint attribute, EGLint *value);
#endif
