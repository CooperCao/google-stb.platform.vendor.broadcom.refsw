/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/

#include "egl_wl_bind_display.h"

EGLBoolean eglBindWaylandDisplayWL_impl(
      EGLDisplay dpy,
      struct wl_display *display)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   if (!driverInterfaces->displayInterface->BindWaylandDisplay)
      return EGL_FALSE;

   return driverInterfaces->displayInterface->BindWaylandDisplay(
         driverInterfaces->displayInterface->context, dpy, display) ?
               EGL_TRUE : EGL_FALSE;
}

EGLBoolean eglUnbindWaylandDisplayWL_impl(
      EGLDisplay dpy,
      struct wl_display *display)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   if (!driverInterfaces->displayInterface->UnbindWaylandDisplay)
      return EGL_FALSE;

   return driverInterfaces->displayInterface->UnbindWaylandDisplay(
         driverInterfaces->displayInterface->context, dpy, display) ?
               EGL_TRUE : EGL_FALSE;
}

EGLBoolean eglQueryWaylandBufferWL_impl(
      EGLDisplay dpy,
      struct wl_resource *buffer,
      EGLint attribute,
      EGLint *value)
{
   BEGL_DriverInterfaces *driverInterfaces = BEGL_GetDriverInterfaces();

   if (!driverInterfaces->displayInterface->QueryBuffer || !value)
      return EGL_FALSE;

   int32_t out_value; /* in case EGLint is not int32_t */
   bool result = driverInterfaces->displayInterface->QueryBuffer(
         driverInterfaces->displayInterface->context, dpy, buffer, attribute,
         &out_value);
   *value = out_value;
   return result ? EGL_TRUE : EGL_FALSE;
}
