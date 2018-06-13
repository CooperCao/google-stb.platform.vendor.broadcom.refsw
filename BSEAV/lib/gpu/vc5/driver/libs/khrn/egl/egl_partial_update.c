/******************************************************************************************************
 * Copyright (C) 2017 Broadcom. The term "Broadcom" refers to Broadcom Inc. and/or its subsidiaries.
 *****************************************************************************************************/

#include "egl_display.h"
#include "egl_surface.h"
#include "egl_thread.h"
#include "egl_surface_base.h"
#include "egl_context_base.h"

#include "libs/util/assert_helpers.h"
#include "libs/util/gfx_util/gfx_util.h"
#include "libs/util/gfx_util/gfx_util_rect.h"

LOG_DEFAULT_CAT("egl_partial_update")

static void clip_to_surface(unsigned surf_w, unsigned surf_h, gfx_rect *rect)
{
   gfx_rect surf = { 0, 0, surf_w, surf_h };
   gfx_rect_intersect(rect, &surf);
}

static unsigned rect_area(const gfx_rect *rect)
{
   return rect->width * rect->height;
}

static int calc_rects_area(gfx_rect *cur_rect, int start, int sign,
                           EGLint n_rects, const gfx_rect *rects)
{
   int area = 0;

   for (int r = start; r < n_rects; r++)
   {
      gfx_rect rect = rects[r];

      if (cur_rect != NULL)
         gfx_rect_intersect(&rect, cur_rect);

      area += sign * rect_area(&rect);

      // inclusive-exclusive recursion
      area += calc_rects_area(&rect, r + 1, sign * -1, n_rects, rects);
   }

   return area;
}

static int estimate_rects_area(EGLint n_rects, const gfx_rect *rects)
{
   int area = 0;

   if (n_rects > 0)
   {
      gfx_rect rect = rects[0];

      for (int r = 1; r < n_rects; r++)
      {
         const gfx_rect *r1 = &rects[r];
         gfx_rect_union(&rect, r1);
      }

      area = rect_area(&rect);
   }

   return area;
}

static float calc_rect_coverage(unsigned surf_w, unsigned surf_h, EGLint n_rects, const gfx_rect *rects)
{
   // NOTE: The area calculations expect all rects to lie within the surface, so they must be
   // clipped to the surface prior to calling this.

   int area = 0;

   // A fully accurate rectangle coverage is slow when there are more than ~8 rects. At that
   // point, an estimate will work just as well.
   if (n_rects <= 8)
      area = calc_rects_area(NULL, 0, 1, n_rects, rects);
   else
      area = estimate_rects_area(n_rects, rects);

   return (float)area / (float)(surf_w * surf_h);
}

EGLAPI EGLBoolean EGLAPIENTRY eglSetDamageRegionKHR(EGLDisplay dpy, EGLSurface surf,
                                                    EGLint *rects, EGLint n_rects)
{
   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   EGL_SURFACE_T *surface = egl_surface_lock(surf);
   EGL_CONTEXT_T *context = egl_thread_get_context();

   EGL_BUFFER_AGE_DAMAGE_STATE_T *state = &surface->age_damage_state;

   EGLint error;
   if (!surface)
      error = EGL_BAD_SURFACE;
   else if (surface->type != EGL_SURFACE_TYPE_NATIVE_WINDOW)
      error = EGL_BAD_MATCH;
   else if (!context || context->draw != surface)
      error = EGL_BAD_MATCH;
   else if (state->num_damage_rects != -1) // Damage already called since last swap
      error = EGL_BAD_ACCESS;
   else if (!state->buffer_age_queried)    // Buffer age not queried since last swap
      error = EGL_BAD_ACCESS;
   else
   {
      assert(state->damage_rects == NULL);
      assert(state->num_damage_rects == -1);

      if (n_rects > 0)
      {
         state->damage_rects = khrn_mem_alloc(sizeof(gfx_rect) * n_rects, "damage_rects");
         if (state->damage_rects)
         {
            state->num_damage_rects = n_rects;

            // Copy the damage rects
            const EGLint *pr = rects;
            for (int r = 0; r < n_rects; r++)
            {
               state->damage_rects[r].x      = *pr++;
               state->damage_rects[r].y      = *pr++;
               state->damage_rects[r].width  = *pr++;
               state->damage_rects[r].height = *pr++;

               // Clip rect to the surface size
               clip_to_surface(surface->width, surface->height, &state->damage_rects[r]);
            }

            error = EGL_SUCCESS;

            log_trace("eglSetDamageRegionKHR (%d rects)", n_rects);
            for (int r = 0; r < n_rects; r++)
               log_trace("   %d %d %d %d", state->damage_rects[r].x, state->damage_rects[r].y,
                  state->damage_rects[r].width, state->damage_rects[r].height);

            if (!khrn_options.disable_big_damage_opt)
            {
               // Calculate the fraction of the surface covered by damage rects
               state->damage_coverage = calc_rect_coverage(surface->width, surface->height,
                                                           n_rects, state->damage_rects);
               log_trace("   (damage coverage = %f)", state->damage_coverage);
            }
         }
         else
            error = EGL_BAD_ALLOC;
      }
      else
      {
         log_trace("eglSetDamageRegionKHR (0 rects)");
         state->num_damage_rects = 0;
         error = EGL_SUCCESS;
      }

      // This ensures that the changed damage rects in the surface will propagate into
      // the damage rects in the installed framebuffer
      if (error == EGL_SUCCESS)
         egl_context_reattach(context);
   }

   egl_surface_unlock(surface);

   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}
