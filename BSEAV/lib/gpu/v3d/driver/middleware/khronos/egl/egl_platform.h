/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_image.h"

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"

#define EGL_PLATFORM_WIN_NONE 0xffffffff

extern EGLDisplay egl_server_platform_get_display(EGLenum platform,
      void *native_display, const EGLint *attrib_list, EGLint *error);
EGLint egl_server_platform_init(EGLDisplay display);
EGLint egl_server_platform_term(EGLDisplay display);

/*
   Used on platforms with server-side pixmaps. Retrieves all of the relevant
   information about a pixmap.
*/

extern MEM_HANDLE_T egl_server_platform_create_pixmap_info(void *platform_pixmap, bool invalid);

extern void *egl_server_platform_dequeue(BEGL_WindowState *windowState, KHRN_IMAGE_FORMAT_T colorformat, int *fd);
extern bool egl_server_platform_queue(BEGL_WindowState *windowState, void * opaque_buffer_handle, int swap_interval, int fd);
extern bool egl_server_platform_cancel(BEGL_WindowState *windowState, void * opaque_buffer_handle, int fd);
extern bool egl_server_platform_create_window_state(BEGL_WindowState **windowState, uintptr_t window, bool secure);
extern void egl_server_platform_destroy_window_state(BEGL_WindowState  *windowState);

extern void *egl_server_platform_get_native_buffer(EGLenum target, EGLClientBuffer *egl_buffer);
extern MEM_HANDLE_T egl_server_platform_image_wrap(EGLenum target, void *native_buffer);
extern MEM_HANDLE_T egl_server_platform_image_new(EGLenum target, void *native_buffer, EGLint *error);
extern uint32_t egl_server_platform_get_color_format(KHRN_IMAGE_FORMAT_T format);
extern bool egl_server_platform_get_info(EGLenum target, void *native_buffer, uint32_t *w, uint32_t *h, uint32_t *stride, KHRN_IMAGE_FORMAT_T *format, uint32_t *offset, void **p);
