/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_image.h"

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/begl_platform.h>

#define EGL_PLATFORM_WIN_NONE 0xffffffff

extern EGLDisplay egl_server_platform_get_display(EGLenum platform,
      void *native_display, const EGLint *attrib_list, EGLint *error);
EGLint egl_server_platform_init(EGLDisplay display);
EGLint egl_server_platform_term(EGLDisplay display);

/*
   Used on platforms with server-side pixmaps. Retrieves all of the relevant
   information about a pixmap.
*/

extern KHRN_IMAGE_T *egl_server_platform_create_pixmap(EGLNativePixmapType platform_pixmap);

extern KHRN_IMAGE_T *egl_server_platform_dequeue(void *native_window_state, KHRN_IMAGE_FORMAT_T colorformat, void **swapchain_buffer, int *fd);
extern bool egl_server_platform_queue(void *native_window_state, void *swapchain_buffer, int swap_interval, int fd);
extern bool egl_server_platform_cancel(void *native_window_state, void *swapchain_buffer, int fd);

extern bool egl_server_platform_create_window_state(void **native_window_state, uintptr_t window, bool secure);
extern void egl_server_platform_destroy_window_state(void *native_window_state);

extern KHRN_IMAGE_T *egl_server_platform_image_new(EGLenum target, EGLClientBuffer egl_buffer, EGLint *error);

extern uint32_t egl_server_platform_get_color_format(KHRN_IMAGE_FORMAT_T format);
