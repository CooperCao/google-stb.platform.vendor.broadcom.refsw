/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 *****************************************************************************************************/

#include "egl_display.h"
#include "egl_surface.h"
#include "egl_thread.h"
#include "egl_surface_base.h"
#include "egl_context_base.h"

EGLAPI EGLBoolean EGLAPIENTRY eglSetDamageRegionKHR(EGLDisplay dpy, EGLSurface surf,
                                                    EGLint *rects, EGLint n_rects)
{
   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   EGL_SURFACE_T *surface = egl_surface_lock(surf);
   EGL_CONTEXT_T *context = egl_thread_get_context();

   EGLint error;
   if (!surface)
      error = EGL_BAD_SURFACE;
   else if (surface->type != EGL_SURFACE_TYPE_NATIVE_WINDOW)
      error = EGL_BAD_MATCH;
   else if (!context || context->draw != surface)
      error = EGL_BAD_MATCH;
   else if (surface->num_damage_rects != -1) // Damage already called since last swap
      error = EGL_BAD_ACCESS;
   else if (!surface->buffer_age_queried)    // Buffer age not queried since last swap
      error = EGL_BAD_ACCESS;
   else
   {
      assert(surface->damage_rects == NULL);
      assert(surface->num_damage_rects == -1);

      if (n_rects > 0)
      {
         surface->damage_rects = (int*)malloc(sizeof(int) * n_rects);
         if (surface->damage_rects)
         {
            surface->num_damage_rects = n_rects;
            memcpy(surface->damage_rects, rects, sizeof(int) * n_rects);
            error = EGL_SUCCESS;
         }
         else
            error = EGL_BAD_ALLOC;
      }
      else
      {
         surface->num_damage_rects = 0;
         error = EGL_SUCCESS;
      }
   }

   egl_surface_unlock(surface);

   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}
