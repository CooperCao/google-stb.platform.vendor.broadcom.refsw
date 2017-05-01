/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <EGL/eglext_wayland.h>

#include "egl_platform_abstract.h"

/* Implementation of the functions defined in the wl_bind_display extension
 * requires accessing Wayland-specific data structures and API.
 *
 * As this is platform-independent code we pass the parameters down
 * to the platform layer, which, for Wayland platform, contains the actual
 * implementation. Calling the below functions on any other platform
 * is a no-op and simply returns EGL_FALSE.
 */

EGLAPI EGLBoolean EGLAPIENTRY eglBindWaylandDisplayWL(
      EGLDisplay dpy,
      struct wl_display *display)
{
   if (!g_bcgPlatformData.displayInterface.BindWaylandDisplay)
      return EGL_FALSE;

   return g_bcgPlatformData.displayInterface.BindWaylandDisplay(
         g_bcgPlatformData.displayInterface.context, dpy, display) ?
               EGL_TRUE : EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglUnbindWaylandDisplayWL(
      EGLDisplay dpy,
      struct wl_display *display)
{
   if (!g_bcgPlatformData.displayInterface.UnbindWaylandDisplay)
      return EGL_FALSE;

   return g_bcgPlatformData.displayInterface.UnbindWaylandDisplay(
         g_bcgPlatformData.displayInterface.context, dpy, display) ?
               EGL_TRUE : EGL_FALSE;
}

EGLAPI EGLBoolean EGLAPIENTRY eglQueryWaylandBufferWL(
      EGLDisplay dpy,
      struct wl_resource *buffer,
      EGLint attribute,
      EGLint *value)
{
   if (!g_bcgPlatformData.displayInterface.QueryBuffer || !value)
      return EGL_FALSE;

   int32_t out_value; /* in case EGLint is not int32_t */
   bool result = g_bcgPlatformData.displayInterface.QueryBuffer(
         g_bcgPlatformData.displayInterface.context, dpy, buffer, attribute,
         &out_value);
   *value = out_value;
   return result ? EGL_TRUE : EGL_FALSE;
}
