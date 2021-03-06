/*=============================================================================
Copyright (c) 2013 Broadcom Europe Limited.
All rights reserved.

Project  :  khronos

FILE DESCRIPTION
=============================================================================*/

#include "vcos.h"
#include "middleware/khronos/egl/egl_thread.h"
#include "middleware/khronos/egl/egl_context_gl.h"
#include "middleware/khronos/egl/egl_surface_base.h"
#include "middleware/khronos/egl/egl_display.h"

#include "middleware/khronos/glxx/glxx_hw.h"
#include "interface/khronos/glxx/glxx_int_config.h"

v3d_scheduler_deps *egl_surface_flush_rendering(EGL_SURFACE_T *surface)
{
   return egl_context_flush_rendering(surface->context, surface);
}

bool egl_surface_copy(EGL_SURFACE_T *surface, KHRN_IMAGE_T *dst)
{
   return egl_context_copy_surface(surface->context, surface, dst);
}

static bool surface_get_attrib(const EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR *value)
{
   if (surface->fns->get_attrib)
      return surface->fns->get_attrib(surface, attrib, value);
   else
      return egl_surface_base_get_attrib(surface, attrib, value);
}

EGLint egl_surface_set_attrib(EGL_SURFACE_T *surface,
      EGLint attrib, EGLAttribKHR value)
{
   if (surface->fns->set_attrib)
      return surface->fns->set_attrib(surface, attrib, value);
   else
      return egl_surface_base_set_attrib(surface, attrib, value);
}

static bool attrib_ignored(EGLint attrib)
{
   switch (attrib)
   {
   case EGL_LARGEST_PBUFFER:
   case EGL_TEXTURE_FORMAT:
   case EGL_TEXTURE_TARGET:
   case EGL_MIPMAP_TEXTURE:
   case EGL_MIPMAP_LEVEL:
      /*
       * These aren't errors; they just don't do anything for non-pbuffer
       * surfaces (EGL 1.4 3.5.6)
       */
      return true;
   default:
      return false;
   }
}

EGLAPI EGLBoolean EGLAPIENTRY eglQuerySurface(EGLDisplay dpy, EGLSurface surf,
      EGLint attribute, EGLint *value)
{
   EGLint error = EGL_BAD_ATTRIBUTE;
   EGL_SURFACE_T *surface = NULL;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   surface = egl_surface_lock(surf);
   if (!surface)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   EGLAttribKHR attr_value;
   if (surface_get_attrib(surface, attribute, &attr_value))
   {
      *value = (EGLint)attr_value;
      error = EGL_SUCCESS;
   }
   else
   {
      error = attrib_ignored(attribute) ? EGL_SUCCESS : EGL_BAD_ATTRIBUTE;
   }

end:
   egl_surface_unlock(surface);
   egl_thread_set_error(error);
   return error == EGL_SUCCESS;
}

EGLAPI EGLBoolean EGLAPIENTRY eglSurfaceAttrib(EGLDisplay dpy, EGLSurface surf,
      EGLint attribute, EGLint value)
{
   EGLint error = EGL_BAD_PARAMETER;
   EGL_SURFACE_T *surface = NULL;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   surface = egl_surface_lock(surf);
   if (!surface)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   error = egl_surface_set_attrib(surface, attribute, value);
   if ((error == EGL_BAD_ATTRIBUTE) && attrib_ignored(attribute))
   {
      error = EGL_SUCCESS;
   }

end:
   egl_thread_set_error(error);
   egl_surface_unlock(surface);
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

KHRN_IMAGE_T *egl_surface_get_back_buffer(const EGL_SURFACE_T *surface)
{
   return surface->fns->get_back_buffer(surface);
}

KHRN_IMAGE_T *egl_surface_get_aux_buffer(const EGL_SURFACE_T *surface,
      egl_aux_buf_t which)
{
   return egl_surface_base_get_aux_buffer(surface, which);
}

/*
 * Get the surface's current dimensions, returning true if they have changed.
 * If it has a back buffer, use that, since those are the dimensions we are
 * going to be using to draw the next frame.
 */
static bool get_new_dimensions(EGL_SURFACE_T *surface,
      unsigned *width, unsigned *height)
{
   KHRN_IMAGE_T *back_buffer = egl_surface_get_back_buffer(surface);

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
   EGL_SURFACE_T *surface = NULL;
   EGL_CONTEXT_T *context;
   bool preserve;
   egl_swap_result_t swap_result = EGL_SWAP_NOT_SWAPPED;

   if (!egl_initialized(dpy, true))
      return EGL_FALSE;

   surface = egl_surface_lock(surf);
   if (!surface)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   context = egl_thread_get_context();
   if (!context || context->draw != surface)
   {
      error = EGL_BAD_SURFACE;
      goto end;
   }

   preserve = surface->swap_behavior == EGL_BUFFER_PRESERVED;
   if (surface->fns->swap_buffers)
      swap_result = surface->fns->swap_buffers(surface, preserve);

   switch (swap_result)
   {
   case EGL_SWAP_NO_MEMORY:
      error = EGL_BAD_ALLOC;
      goto end;

   case EGL_SWAP_SWAPPED:
      if (!egl_surface_resize(surface))
      {
         error = EGL_BAD_ALLOC;
         goto end;
      }

      if (surface->context)
         egl_context_reattach(surface->context);

      egl_surface_invalidate(surface, !preserve);
      break;

   case EGL_SWAP_NOT_SWAPPED:
      // In this case we just invalidate the auxiliary buffers.
      egl_surface_invalidate(surface, false);
      break;
   }

   error = EGL_SUCCESS;
end:
   egl_surface_unlock(surface);
   egl_thread_set_error(error);
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

static void invalidate_image(KHRN_IMAGE_T *image)
{
   KHRN_RES_INTERLOCK_T *res_i;

   /*
    * FIXME: Images and interlocks shouldn't be considered part of GL, but we
    * aren't going to separate that out now.
    */
   if (egl_context_gl_lock())
   {
      res_i = khrn_image_get_res_interlock(image);
      khrn_interlock_invalidate(&res_i->interlock);

      egl_context_gl_unlock();
   }
}

void egl_surface_invalidate(EGL_SURFACE_T *surface, bool include_color)
{
   KHRN_IMAGE_T *image;
   egl_aux_buf_t aux_buf;

   for (aux_buf = 0; aux_buf < AUX_MAX; aux_buf++)
   {
      image = egl_surface_get_aux_buffer(surface, aux_buf);
      if (image) invalidate_image(image);
   }

   if (include_color)
   {
      image = egl_surface_get_back_buffer(surface);
      if (image) invalidate_image(image);
   }
}
