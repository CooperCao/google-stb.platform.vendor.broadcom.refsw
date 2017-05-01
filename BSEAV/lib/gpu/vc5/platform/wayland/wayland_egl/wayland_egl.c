/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "wayland_egl_priv.h"

#include <wayland-client.h>
#include <wayland-egl.h>
#include <stdlib.h>

WL_EGL_EXPORT void wl_egl_window_resize(
  struct wl_egl_window *egl_window,
  int width, int height,
  int dx, int dy)
{
   pthread_mutex_lock(&egl_window->mutex);
   egl_window->width = width;
   egl_window->height = height;
   egl_window->dx += dx;
   egl_window->dy += dy;
   if (egl_window->resize.callback != NULL)
      egl_window->resize.callback(egl_window->resize.context, egl_window);
   pthread_mutex_unlock(&egl_window->mutex);
}

WL_EGL_EXPORT struct wl_egl_window *wl_egl_window_create(
  struct wl_surface *wl_surface,
  int                width,
  int                height)
{
   struct wl_egl_window *egl_window = calloc(1, sizeof(*egl_window));
   if (egl_window)
   {
      if (pthread_mutex_init(&egl_window->mutex, NULL) == 0)
      {
         egl_window->surface = wl_surface;
         egl_window->width = width;
         egl_window->height = height;
         /* the rest is zeroed by calloc() */
      }
      else
      {
         free(egl_window);
         egl_window = NULL;
      }
   }

   return egl_window;
}

WL_EGL_EXPORT void wl_egl_window_destroy(
  struct wl_egl_window *egl_window)
{
   if (egl_window)
   {
      pthread_mutex_destroy(&egl_window->mutex);
      free(egl_window);
   }
}

WL_EGL_EXPORT void wl_egl_window_get_attached_size(
  struct wl_egl_window *egl_window,
  int                  *width,
  int                  *height)
{
   pthread_mutex_lock(&egl_window->mutex);
   if (width)
      *width = egl_window->attached_width;
   if (height)
      *height = egl_window->attached_height;
   pthread_mutex_unlock(&egl_window->mutex);
}
