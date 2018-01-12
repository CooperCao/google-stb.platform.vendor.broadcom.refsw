/******************************************************************************
 *  Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "wayland_egl_priv.h"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <stdlib.h>

WL_EGL_EXPORT void wl_egl_window_resize(struct wl_egl_window *egl_window,
      int width, int height, int dx, int dy)
{
   egl_window->width = width;
   egl_window->height = height;
   egl_window->dx = dx;
   egl_window->dy = dy;
   if (egl_window->resize_callback)
      egl_window->resize_callback(egl_window, egl_window->callback_private);
}

WL_EGL_EXPORT struct wl_egl_window *wl_egl_window_create(
      struct wl_surface *wl_surface, int width, int height)
{
   struct wl_egl_window *egl_window = calloc(1, sizeof(*egl_window));
   if (egl_window)
   {
      egl_window->surface = wl_surface;
      egl_window->width = width;
      egl_window->height = height;
      /* the rest is zeroed by calloc() */
   }
   return egl_window;
}

WL_EGL_EXPORT void wl_egl_window_destroy(struct wl_egl_window *egl_window)
{
   if (egl_window->destroy_window_callback)
      egl_window->destroy_window_callback(egl_window->callback_private);
   free(egl_window);
}

WL_EGL_EXPORT void wl_egl_window_get_attached_size(
      struct wl_egl_window *egl_window, int *width, int *height)
{
   /* Note: Mesa implementation doesn't have get_attached_size_callback */
   if (egl_window->get_attached_size_callback)
      egl_window->get_attached_size_callback(egl_window,
            egl_window->callback_private);

   if (width)
      *width = egl_window->attached_width;
   if (height)
      *height = egl_window->attached_height;
}
