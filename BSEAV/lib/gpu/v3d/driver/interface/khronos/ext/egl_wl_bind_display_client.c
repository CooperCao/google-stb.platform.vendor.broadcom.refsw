/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#define EGL_EGLEXT_PROTOTYPES /* we want the prototypes so the compiler will check that the signatures match */

#include "interface/khronos/common/khrn_int_common.h"
#include "interface/khronos/common/khrn_client.h"
#include "middleware/khronos/ext/egl_wl_bind_display.h"

/* Implementation of the functions defined in the wl_bind_display extension
 * requires accessing Wayland-specific data structures and API.
 *
 * As this is platform-independent code we pass the parameters down
 * to the platform layer, which, for Wayland platform, contains the actual
 * implementation. Calling the below functions on any other platform
 * is a no-op and simply returns EGL_FALSE.
 */

EGLAPI EGLBoolean EGLAPIENTRY eglBindWaylandDisplayWL(EGLDisplay dpy,
      struct wl_display *display)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();
   EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy,
         EGL_TRUE);
   if (state)
      result = eglBindWaylandDisplayWL_impl(dpy, display);
   else
      result = false;
   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglUnbindWaylandDisplayWL(EGLDisplay dpy,
      struct wl_display *display)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();
   EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy,
         EGL_TRUE);
   if (state)
      result = eglUnbindWaylandDisplayWL_impl(dpy, display);
   else
      result = false;
   CLIENT_UNLOCK();

   return result;
}

EGLAPI EGLBoolean EGLAPIENTRY eglQueryWaylandBufferWL(EGLDisplay dpy,
      struct wl_resource *buffer, EGLint attribute, EGLint *value)
{
   CLIENT_THREAD_STATE_T *thread = CLIENT_GET_THREAD_STATE();
   EGLBoolean result;

   if (!egl_ensure_init_once())
      return EGL_FALSE;

   CLIENT_LOCK();
   EGL_SERVER_STATE_T *state = egl_get_process_state(thread, dpy,
         EGL_TRUE);
   if (state)
      result = eglQueryWaylandBufferWL_impl(dpy, buffer, attribute, value);
   else
      result = false;
   CLIENT_UNLOCK();

   return result;
}
