/*=============================================================================
Broadcom Proprietary and Confidential. (c)2016 Broadcom.
All rights reserved.
=============================================================================*/

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_wayland.h>

EGLAPI EGLBoolean EGLAPIENTRY eglBindWaylandDisplayWL(
      EGLDisplay dpy,
      struct wl_display *display)
{
   return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglUnbindWaylandDisplayWL(
      EGLDisplay dpy,
      struct wl_display *display)
{
   return EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglQueryWaylandBufferWL(
      EGLDisplay dpy,
      struct wl_resource *buffer,
      EGLint attribute,
      EGLint *value)
{
   return EGL_FALSE;
}
