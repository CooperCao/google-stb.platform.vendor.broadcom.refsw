/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#ifndef __WAYLAND_EGL_PRIV_H__
#define __WAYLAND_EGL_PRIV_H__

#include <wayland-client.h>
#include <pthread.h>

/* GCC visibility */
#if defined(__GNUC__) && __GNUC__ >= 4
#define WL_EGL_EXPORT __attribute__ ((visibility("default")))
#else
#define WL_EGL_EXPORT
#endif

#ifdef __cplusplus
extern "C" {
#endif

struct wl_egl_window
{
   pthread_mutex_t mutex;
   struct wl_surface *surface;
   int width;
   int height;
   int dx;
   int dy;
   int attached_width;
   int attached_height;
   struct resize
   {
      void *context;
      void (*callback)(void *context, struct wl_egl_window * egl_window);
   } resize;
};

#ifdef __cplusplus
}
#endif

#endif /*__WAYLAND_EGL_PRIV_H__*/
