/*=============================================================================
 Copyright (c) 2010 Broadcom Europe Limited.
 All rights reserved.

 Project  :  khronos
 Module   :  EGL display

 FILE DESCRIPTION
 EGL server-side display management.
 =============================================================================*/

#ifndef EGL_DISP_H
#define EGL_DISP_H

#include "interface/khronos/common/khrn_int_common.h"
#include "middleware/khronos/common/khrn_mem.h"
#include "interface/khronos/include/EGL/begl_dispplatform.h"

#define EGL_DISP_MAX_BUFFERS 3

typedef struct EGL_DISP_FIFO
{
   uint32_t          push_at;
   uint32_t          pop_from;
   MEM_HANDLE_T      buffer[EGL_DISP_MAX_BUFFERS];   /* Handle to a KHRN_IMAGE_T */
   VCOS_MUTEX_T      mutex;
   VCOS_SEMAPHORE_T  semaphore;
} EGL_DISP_FIFO_T;

typedef struct EGL_DISP_THREAD_STATE
{
   VCOS_THREAD_T     thread;
   EGL_DISP_FIFO_T   display_fifo;
   VCOS_SEMAPHORE_T  running;
   BEGL_WindowState *platform_state;
} EGL_DISP_THREAD_STATE_T;

extern void egl_disp_create_display_thread(EGL_DISP_THREAD_STATE_T *thread_state, BEGL_WindowState *platformState);
extern void egl_disp_destroy_display_thread(EGL_DISP_THREAD_STATE_T *thread_state);
extern void egl_disp_push(MEM_HANDLE_T handle);

#endif
