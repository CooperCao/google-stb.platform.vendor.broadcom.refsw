/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :  Header file

FILE DESCRIPTION
Platform abstraction layer API declaration.
=============================================================================*/

#ifndef EGL_PLATFORM_H
#define EGL_PLATFORM_H

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_image.h"

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"

#define EGL_PLATFORM_WIN_NONE 0xffffffff

/*
   Used on platforms with server-side pixmaps. Retrieves all of the relevant
   information about a pixmap.
*/

MEM_HANDLE_T egl_server_platform_create_pixmap_info(uint32_t pixmap, EGLint *error);

void egl_server_platform_init(void);

extern void egl_server_platform_set_position(uint32_t handle, uint32_t position, uint32_t width, uint32_t height);
extern uint32_t egl_server_platform_create_buffer(KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height,
                                                  KHRN_IMAGE_CREATE_FLAG_T flags, BEGL_WindowState *windowState,
                                                  BEGL_BufferUsage usage, bool secure);
extern uint32_t egl_server_platform_get_buffer(KHRN_IMAGE_FORMAT_T format, uint32_t width, uint32_t height,
                                               KHRN_IMAGE_CREATE_FLAG_T flags, BEGL_WindowState *windowState,
                                               BEGL_BufferUsage usage, bool secure);
extern bool egl_server_platform_create_window_state(BEGL_WindowState **windowState, uint32_t window);
extern void egl_server_platform_destroy_buffer(uint32_t bufHandle, BEGL_WindowState *windowState);

#endif