/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
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
   KHRN_IMAGE_FORMAT_T colorformat,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisampleformat,
   uint32_t mipmap,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits,
   uint32_t type);

extern bool egl_create_wrapped_surface(
   EGL_SURFACE_T *surface,
   void *pixmap,
   KHRN_IMAGE_FORMAT_T depthstencilformat,
   KHRN_IMAGE_FORMAT_T maskformat,
   KHRN_IMAGE_FORMAT_T multisample,
   uint32_t config_depth_bits,
   uint32_t config_stencil_bits);

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

extern int egl_copybuffers(EGL_SURFACE_T *surface, void *pixmap);
extern void egl_get_color_data(EGL_SURFACE_T *surface, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, void *data);
extern void egl_set_color_data(EGL_SURFACE_T *surface, KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height, int32_t stride, uint32_t y_offset, const void *data);

extern bool egl_bind_tex_image(EGL_SURFACE_T *surface);
extern void egl_release_tex_image(EGL_SURFACE_T *surface);

extern void egl_update_gl_buffers(EGL_CURRENT_T *opengl);

#if EGL_KHR_image
extern EGLImageKHR eglCreateImageKHR_impl(EGLenum target, EGLClientBuffer buffer, EGLint texture_level, EGLint *results);
extern EGLBoolean eglDestroyImageKHR_impl(EGLImageKHR image);
#endif

#if EGL_BRCM_driver_monitor
extern bool eglInitDriverMonitorBRCM_impl(EGLint hw_bank, EGLint l3c_bank);
extern void eglTermDriverMonitorBRCM_impl(void);
extern void eglGetDriverMonitorXMLBRCM_impl(EGLint bufSize, char *xmlStats);
#endif

#ifdef WAYLAND
struct wl_display;
struct wl_resource;
extern  EGLBoolean eglBindWaylandDisplayWL_impl(EGLDisplay dpy, struct wl_display *display);
extern  EGLBoolean eglUnbindWaylandDisplayWL_impl(EGLDisplay dpy, struct wl_display *display);
extern  EGLBoolean eglQueryWaylandBufferWL_impl(EGLDisplay dpy, struct wl_resource *buffer, EGLint attribute, EGLint *value);
#endif
