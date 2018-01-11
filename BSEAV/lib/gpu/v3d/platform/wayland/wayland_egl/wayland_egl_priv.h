/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#pragma once

#include <wayland-client.h>

/* GCC visibility */
#if defined(__GNUC__) && __GNUC__ >= 4
#define WL_EGL_EXPORT __attribute__ ((visibility("default")))
#else
#define WL_EGL_EXPORT
#endif

#ifdef __cplusplus
extern "C"
{
#endif

struct wl_egl_window
{
   struct wl_surface *surface;

   int width;
   int height;
   int dx;
   int dy;

   int attached_width;
   int attached_height;

   void *callback_private;
   void (*resize_callback)(struct wl_egl_window *, void *);
   void (*get_attached_size_callback)(struct wl_egl_window *, void *);
   void (*destroy_window_callback)(void *);
};

#ifdef __cplusplus
}
#endif
