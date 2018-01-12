/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include "interface/khronos/include/EGL/egl.h"
#include "interface/khronos/include/EGL/eglext.h"
#include "interface/khronos/include/EGL/eglext_wayland.h"

EGLBoolean eglBindWaylandDisplayWL_impl(
      EGLDisplay dpy,
      struct wl_display *display);

EGLBoolean eglUnbindWaylandDisplayWL_impl(
      EGLDisplay dpy,
      struct wl_display *display);

EGLBoolean eglQueryWaylandBufferWL_impl(
      EGLDisplay dpy,
      struct wl_resource *buffer,
      EGLint attribute,
      EGLint *value);
