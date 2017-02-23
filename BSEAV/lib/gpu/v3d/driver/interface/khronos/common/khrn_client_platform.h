/*=============================================================================
Broadcom Proprietary and Confidential. (c)2008 Broadcom.
All rights reserved.

Project  :  khronos
Module   :

FILE DESCRIPTION
client side API
=============================================================================*/
#ifndef KHRN_CLIENT_PLATFORM_H
#define KHRN_CLIENT_PLATFORM_H

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_int_image.h"
#include "vcfw/rtos/abstract/rtos_abstract_mem.h"
#include <stdlib.h> // for size_t

/* Per-platform types are defined in here. Most platforms can be supported
 * via vcos, but 'direct' has its own header and types, which is why
 * the indirection is required.
 */
#include "interface/khronos/common/abstract/khrn_client_platform_filler_abstract.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct CLIENT_PLATFORM_INFO_S
{
   uint32_t width;
   uint32_t height;
   uint32_t swapchain_count;
   uint32_t format;
   uint32_t colorFormat;
   uint32_t refreshRate;
} CLIENT_PLATFORM_INFO_T;

/*
   named counting semaphore
*/

/* Uses forward declared ref to avoid problems with mixing direct and vcos-based
 * semaphores.
 */


/*
   workaround for broken platforms which don't detect threads exiting
*/
extern void platform_hint_thread_finished(void);

/*
   heap
*/

/*
   void *khrn_platform_malloc(size_t size, const char *desc)

   Preconditions:

   desc is a literal, null-terminated string
   The caller of this function is contracted to later call khrn_platform_free
   (or pass such responsibility on) if we don't return NULL

   Postconditions:

   Return value is NULL or a memory allocation of at least size bytes,
   valid until khrn_platform_free is called. The memory is sufficiently
   aligned that it can be used for normal data structures.
*/

extern void *khrn_platform_malloc(size_t size, const char *desc);

/*
   void khrn_platform_free(void *v)

   Preconditions:

   v is a valid pointer returned from khrn_platform_malloc

   Postconditions:

   v is freed

   Invariants preserved:

   -

   Invariants used:

   -
*/
extern void khrn_platform_free(void *v);

extern void khrn_platform_maybe_free_process(void);

/*
   uint64_t khronos_platform_get_process_id()

   get process id

   Preconditions:

   -

   Postconditions:

   Repeated calls within a process return the same value. Calls from a different process
   return a different value.
*/

extern uint64_t khronos_platform_get_process_id(void);

/*
   window system
*/

#define PLATFORM_WIN_NONE     ((uint32_t)0xffffffff)

static INLINE uint32_t platform_get_handle(EGLNativeWindowType win)
{
   return (uint32_t)win;
}

#define GI_WIDTH 1
#define GI_HEIGHT 2
#define GI_FMT 4
#define GI_SWAPCC 8
#define GI_REFRESH 16

void platform_get_info(EGLNativeWindowType win, unsigned int flags, CLIENT_PLATFORM_INFO_T * info);
bool platform_supported_format(KHRN_IMAGE_FORMAT_T format);
bool platform_lock_buffer(void * opaque_buffer_handle, BEGL_WindowState *window_state, int *fence);
bool platform_get_buffersettings(void * opaque_buffer_handle, BEGL_BufferSettings * bufferSettings);

extern void platform_surface_update(uint32_t handle);

/*
   bool platform_get_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image);

   Preconditions:

   image is a valid pointer

   Postconditions:

   Either:
   - false is returned because pixmap is an invalid pixmap handle, or
   - true is returned and image is set up to describe the pixmap, and if it's a
     client-side pixmap the pointer is also set
*/

extern bool platform_get_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image);
/*
   should look something like this:

   if (regular server-side pixmap) {
      handle[0] = handle;
   } else if (global image server-side pixmap) {
      handle[0] = id[0];
      handle[1] = id[1];
   }
*/

extern void platform_wait_EGL(uint32_t handle);
extern void platform_retrieve_pixmap_completed(EGLNativePixmapType pixmap);
extern void platform_send_pixmap_completed(EGLNativePixmapType pixmap);

/*
   bool platform_match_pixmap_api_support(EGLNativePixmapType pixmap, uint32_t api_support);

   Preconditions:

   pixmap is probably a valid native pixmap handle
   api_support is a bitmap which is a subset of (EGL_OPENGL_ES_BIT | EGL_OPENVG_BIT | EGL_OPENGL_ES2_BIT)

   Postconditions:

   If result is true then rendering to this pixmap using these APIs is supported on this platform
*/

extern bool platform_match_pixmap_api_support(EGLNativePixmapType pixmap, uint32_t api_support);

/* Platform optimised versions of memcpy and memcmp */
extern uint32_t platform_memcmp(const void * aLeft, const void * aRight, size_t aLen);
extern void platform_memcpy(void * aTrg, const void * aSrc, size_t aLength);

struct CLIENT_THREAD_STATE;
extern void platform_client_lock(void);
extern void platform_client_release(void);
extern void platform_init_rpc(struct CLIENT_THREAD_STATE *state);
extern void platform_term_rpc(struct CLIENT_THREAD_STATE *state);
extern void platform_maybe_free_process(void);
extern void platform_destroy_winhandle(void *a, uint32_t b);

#ifndef __SYMBIAN32__
// hack for now - we want prototypes
extern void rpc_call8_makecurrent(void);
extern void egl_gce_win_change_image(void);
#endif

#ifdef __cplusplus
}
#endif
#endif

extern bool khrn_platform_decode_native(EGLClientBuffer buffer,
                                        uint32_t *w,
                                        uint32_t *h,
                                        uint32_t *stride,
                                        KHRN_IMAGE_FORMAT_T *format,
                                        uint32_t *offset,
                                        void **p);

extern MEM_HANDLE_T khrn_platform_image_new(EGLClientBuffer buffer, EGLenum target, EGLint *error);
extern MEM_HANDLE_T khrn_platform_image_wrap(EGLClientBuffer buffer);

extern EGLDisplay khrn_platform_set_display_id(EGLNativeDisplayType display_id);

extern void khrn_platform_inc_refcnt(EGLClientBuffer buffer);
extern void khrn_platform_dec_refcnt(EGLClientBuffer buffer);

extern uint32_t khrn_platform_get_window_position(EGLNativeWindowType win);

extern void khrn_platform_release_pixmap_info(EGLNativePixmapType pixmap, KHRN_IMAGE_WRAP_T *image);
extern void khrn_platform_unbind_pixmap_from_egl_image(EGLImageKHR egl_image);
extern uint32_t khrn_platform_get_color_format(KHRN_IMAGE_FORMAT_T format);