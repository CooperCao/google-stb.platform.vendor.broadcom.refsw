/******************************************************************************
 *  Copyright (C) 2016 Broadcom. The term "Broadcom" refers to Broadcom Limited and/or its subsidiaries.
 ******************************************************************************/
#include "vcos.h"
#include "egl_thread.h"
#include "egl_context_gl.h"
#include "egl_surface_base.h"
#include "egl_display.h"

#include "../glxx/glxx_hw.h"
#include "../glxx/glxx_int_config.h"
#include "libs/util/profile/profile.h"

GFX_LFMT_T egl_surface_lfmt_to_srgb(GFX_LFMT_T lfmt)
{
   switch (lfmt & GFX_LFMT_FORMAT_MASK)
   {
   case GFX_LFMT_R8_G8_B8_A8_UNORM:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_SRGB_SRGB_SRGB_UNORM;

   case GFX_LFMT_R8_G8_B8_X8_UNORM:
   case GFX_LFMT_R8_G8_B8_UNORM:
      return (lfmt & ~GFX_LFMT_TYPE_MASK) | GFX_LFMT_TYPE_SRGB;

   default:
      return lfmt; /* ignore sRGB-ness (EGL_KHR_gl_colorspace extension allows that) */
   }
}

const v3d_scheduler_deps *egl_surface_flush_back_buffer_writer(EGL_SURFACE_T *surface)
{
   const v3d_scheduler_deps *deps = NULL;

   egl_context_gl_lock();
   khrn_resource *resource = khrn_image_get_resource(egl_surface_get_back_buffer(surface));
   khrn_resource_flush_writer(resource);
   deps = &resource->pre_read;
   egl_context_gl_unlock();

   return deps;
}

EGLint egl_surface_get_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLint *value)
{
   if (surface->fns->get_attrib)
      return surface->fns->get_attrib(surface, attrib, value);
   else
      return egl_surface_base_get_attrib(surface, attrib, value);
}

static EGLint surface_set_attrib(EGL_SURFACE_T *surface, EGLint attrib, EGLAttribKHR value)
{
   if (surface->fns->set_attrib)
      return surface->fns->set_attrib(surface, attrib, value);
   else
      return egl_surface_base_set_attrib(surface, attrib, value);
}

EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surf,
      EGLint attribute, EGLint *value)
{
   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   EGL_SURFACE_T *surface = egl_surface_lock(surf);
   EGLint error;
   if (!surface)
      error = EGL_BAD_SURFACE;
   else
      error = egl_surface_get_attrib(surface, attribute, value);

   egl_surface_unlock(surface);

   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surf,
      EGLint attribute, EGLint value)
{
   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   EGL_SURFACE_T *surface = egl_surface_lock(surf);
   if (!surface)
   {
      egl_thread_set_error(EGL_BAD_SURFACE);
      return EGL_FALSE;
   }
   EGLint error = surface_set_attrib(surface, attribute, value);
   egl_surface_unlock(surface);

   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

bool egl_surface_try_delete(EGL_SURFACE_T *surface)
{
   if (!surface || surface->context)
      return false;

   if (egl_get_surface_handle(surface) != EGL_NO_SURFACE)
      return false;

   if (surface->fns->delete_fn)
      surface->fns->delete_fn(surface);
   else
      free(surface);

   return true;
}

EGL_THREAD_T *egl_surface_get_thread(const EGL_SURFACE_T *surface)
{
   if (!surface || !surface->context)
      return NULL;

   return surface->context->bound_thread;
}

khrn_image *egl_surface_get_back_buffer(const EGL_SURFACE_T *surface)
{
   return surface->fns->get_back_buffer(surface);
}

khrn_image *egl_surface_get_aux_buffer(const EGL_SURFACE_T *surface,
      egl_aux_buf_t which)
{
   return egl_surface_base_get_aux_buffer(surface, which);
}

khrn_image *egl_surface_get_back_buffer_with_gl_colorspace(const EGL_SURFACE_T *surface)
{
   khrn_image *img = egl_surface_get_back_buffer(surface);
   khrn_mem_acquire(img);

   const GFX_BUFFER_DESC_T *desc = &img->blob->desc[img->level];
   assert(desc->num_planes == 1);
   GFX_LFMT_T lfmt = desc->planes[0].lfmt;

   GFX_LFMT_T lfmt_with_colorspace = (surface->gl_colorspace == SRGB) ?
      egl_surface_lfmt_to_srgb(lfmt) : lfmt;
   if (lfmt_with_colorspace != lfmt)
   {
      // Make a shallow copy of the blob so we can fiddle with the format
      khrn_image *img_with_colorspace = khrn_image_shallow_blob_copy(img);
      if (img_with_colorspace)
      {
         khrn_mem_release(img);
         img = img_with_colorspace;
         img->blob->desc[img->level].planes[0].lfmt = lfmt_with_colorspace;
      }
      // else: silently fall back to linear colorspace. This is permitted by the EGL spec.
   }

   return img;
}

/*
 * Get the surface's current dimensions, returning true if they have changed.
 * If it has a back buffer, use that, since those are the dimensions we are
 * going to be using to draw the next frame.
 */
static bool get_new_dimensions(EGL_SURFACE_T *surface,
      unsigned *width, unsigned *height)
{
   khrn_image *back_buffer = egl_surface_get_back_buffer(surface);

   if (back_buffer)
      khrn_image_get_dimensions(back_buffer, width, height, NULL, NULL);

   else if (surface->fns->get_dimensions)
      surface->fns->get_dimensions(surface, width, height);

   else
   {
      *width = surface->width;
      *height = surface->height;
   }

   return *width != surface->width || *height != surface->height;
}

bool egl_surface_resize(EGL_SURFACE_T *surface)
{
   unsigned new_width, new_height;

   if (!get_new_dimensions(surface, &new_width, &new_height))
      return true;

   egl_surface_base_delete_aux_bufs(surface);

   surface->width = new_width;
   surface->height = new_height;

   return egl_surface_base_init_aux_bufs(surface);
}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapBuffers(EGLDisplay dpy, EGLSurface surf)
{
   EGLint error = EGL_BAD_ALLOC;
   {
      PROFILE_FUNCTION_MT("GL");

      if (!egl_initialized(dpy, true))
         return EGL_FALSE;

      EGL_SURFACE_T *surface = egl_surface_lock(surf);
      if (!surface)
      {
         error = EGL_BAD_SURFACE;
         goto end;
      }

      EGL_CONTEXT_T *context = egl_thread_get_context();
      if (!context || context->draw != surface)
      {
         error = EGL_BAD_SURFACE;
         goto end;
      }
      assert(surface->context == context);

      /* Invalidate all auxiliary buffers. Do this before flushing so we can
       * avoid storing the invalidated buffers out! */
      egl_context_invalidate_draw(context, /*color=*/false, /*color_ms=*/true, /*other_aux=*/true);

      /* swap_buffers() will:
       * 1. Flush.
       * 2. Queue the back buffer for display.
       * 3. Switch to the next back buffer. */
      if (surface->fns->swap_buffers)
      {
         egl_result_t swap_result = surface->fns->swap_buffers(surface);

         switch (swap_result)
         {
         case EGL_RES_NO_MEM:
            error = EGL_BAD_ALLOC;
            goto end;
         case EGL_RES_BAD_NATIVE_WINDOW:
            error = EGL_BAD_NATIVE_WINDOW;
            goto end;

         case EGL_RES_SUCCESS:
            if (!egl_surface_resize(surface))
            {
               error = EGL_BAD_ALLOC;
               goto end;
            }

            if (surface->context)
               egl_context_reattach(surface->context);

            break;
         }
      }

      // Getting the new back buffer will update the buffer_age in the surface which we use below.
      khrn_image *back_buffer = egl_surface_get_back_buffer(surface);

      int buffer_age = surface->buffer_age;

      /* If no-one has ever queried the buffer age on this surface, treating it as undefined
       * allows later optimizations, so force age to 0. */
      if (!surface->buffer_age_enabled)
         buffer_age = 0;

      if (buffer_age == 0)
      {
         /* Buffers with age 0 have undefined content, so we can invalidate the new back-buffer.
          * This will prevent unnecessary tile loads of undefined data in certain cases. */
         egl_context_gl_lock();
         khrn_image_invalidate(back_buffer);
         egl_context_gl_unlock();
      }

      /* Reset any per-swap state in the surface */
      egl_surface_base_swap_done(surface, buffer_age);

      error = EGL_SUCCESS;
   end:
      egl_surface_unlock(surface);
      egl_thread_set_error(error);
   }
   profile_on_swap();
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSwapInterval(EGLDisplay dpy,
      EGLint interval)
{
   EGLint error;
   EGL_SURFACE_T *surface;
   EGL_CONTEXT_T *context;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   context = egl_thread_get_context();
   if (!context)
   {
      error = EGL_BAD_CONTEXT;
      goto end;
   }

   surface = context->draw;
   if (!surface)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   if (surface->fns->swap_interval)
      surface->fns->swap_interval(surface, interval);

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglDestroySurface(EGLDisplay dpy,
      EGLSurface surface)
{
   EGL_SURFACE_T *surf = NULL;
   EGLint error = EGL_BAD_SURFACE;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   surf = egl_surface_lock(surface);
   if (!surf) goto end;

   egl_unmap_surface(surface);

   /*
    * Once it's unmapped no other threads can see it so it should be safe to
    * unlock it.
    */
   egl_surface_unlock(surf);
   egl_surface_try_delete(surf);

   error = EGL_SUCCESS;
end:
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGL_SURFACE_T *egl_surface_lock(EGLSurface handle)
{
   EGL_SURFACE_T *surface = egl_get_surface(handle);
   if (!surface) return NULL;
   vcos_mutex_lock(&surface->lock);
   return surface;
}

void egl_surface_unlock(EGL_SURFACE_T *surface)
{
   if (!surface) return;
   vcos_mutex_unlock(&surface->lock);
}
